/*
  openEMC Humidity, Temperature, and Equilibrium Moisture Content Node
  
  based on:
  emonTH Low Power DHT22 Humidity & Temperature & DS18B20 Temperature Node Example 

  Checkes at startup for presence of a DS18B20 temp sensor , DHT22 (temp + humidity) or both
  If it finds both sensors the temperature value will be taken from the DS18B20 (external) and DHT22 (internal) and humidity from DHT22
  If it finds only DS18B20 then no humidity value will be reported
  If it finds only a DHT22 then both temperature and humidity values will be obtained from this sesor
  
  Technical hardware documentation wiki: http://wiki.openenergymonitor.org/index.php?title=EmonTH
 
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
 
  Authors: Glyn Hudson
  Builds upon JCW JeeLabs RF12 library, Arduino and Martin Harizanov's work

  THIS SKETCH REQUIRES:

  Libraries in the standard arduino libraries folder:
	- RFu JeeLib		https://github.com/openenergymonitor/RFu_jeelib   //library to work with CISECO RFu328 module
	- DHT22 Sensor Library  https://github.com/adafruit/DHT-sensor-library - be sure to rename the sketch folder to remove the '-'
        - OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
	- DallasTemperature	http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_LATEST.zip

  Recommended node ID allocation
  -----------------------------------------------------------------------------------------------------------
  -ID-	-Node Type- 
  0	- Special allocation in JeeLib RFM12 driver - reserved for OOK use
  1-4     - Control nodes 
  5-10	- Energy monitoring nodes
  11-14	--Un-assigned --
  15-16	- Base Station & logging nodes
  17-30	- Environmental sensing nodes (temperature humidity etc.)
  31	- Special allocation in JeeLib RFM12 driver - Node31 can communicate with nodes on any network group
  -------------------------------------------------------------------------------------------------------------
;
  Change log:
  V1.5         add support for emonTH V1.5 with ATmega328 onboard, RFM69CW and RF node ID DIP switch 
  V1.5.1      11/05/15 fix bug to make RF node ID DIP switches work 
*/

#define RF69_COMPAT 1                                                              // Set to 1 if using RFM69CW or 0 is using RFM12B
#include <JeeLib.h>                                                                      //https://github.com/jcw/jeelib - Tested with JeeLib 3/11/14

boolean debug=1;                                       //Set to 1 to few debug serial output, turning debug off increases battery life

#define RF_freq RF12_433MHZ                 // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
int nodeID = 19;                               // EmonTH temperature RFM12B node ID - should be unique on network
const int networkGroup = 210;                // EmonTH RFM12B wireless network group - needs to be same as emonBase and emonGLCD
                                                                      // DS18B20 resolution 9,10,11 or 12bit corresponding to (0.5, 0.25, 0.125, 0.0625 degrees C LSB), lower resolution means lower power

const int time_between_readings= 1;                                   // in minutes
const int TEMPERATURE_PRECISION=11;                                   // 9 (93.8ms),10 (187.5ms) ,11 (375ms) or 12 (750ms) bits equal to resplution of 0.5C, 0.25C, 0.125C and 0.0625C
#define ASYNC_DELAY 375                                               // 9bit requres 95ms, 10bit 187ms, 11bit 375ms and 12bit resolution takes 750ms

// See block comment above for library info
#include <avr/power.h>
#include <avr/sleep.h>                                           
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                            // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

// Hardwired emonTH pin allocations 
const int DS18B20_PWR=    5;
const int DHT22_PWR=      6;
const int LED=            9;
const int BATT_ADC=       1;
const int DIP_switch1=    7;
const int DIP_switch2=    8;
#define ONE_WIRE_BUS      19
#define DHTPIN            18   

// Humidity code adapted from ladyada' example                        // emonTh DHT22 data pin
// Uncomment whatever type you're using!
// #define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);
boolean DHT22_status;                                                 // create flag variable to store presence of DS18B20

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
boolean DS18B20;                                                      // create flag variable to store presence of DS18B20 

typedef struct {                                                      // RFM12B RF payload datastructure
  	  int temp;
          int temp_external;
          int humidity;    
          int battery; 
          int emc;         	                                      
} Payload;
Payload emonth;



int numSensors; 
//addresses of sensors, MAX 4!!  
byte allAddress [4][8];                                              // 8 bytes per address

//################################################################################################################################
//################################################################################################################################
void setup() {
//################################################################################################################################
  
  pinMode(LED,OUTPUT); digitalWrite(LED,HIGH);                       // Status LED on
     
  //READ DIP SWITCH POSITIONS - LOW when switched on (default off - pulled up high)
  pinMode(DIP_switch1, INPUT_PULLUP);
  pinMode(DIP_switch2, INPUT_PULLUP);
  boolean DIP1 = digitalRead(DIP_switch1);
  boolean DIP2 = digitalRead(DIP_switch2);
  
  if ((DIP1 == HIGH) && (DIP2 == HIGH)) nodeID=nodeID;
  if ((DIP1 == LOW) && (DIP2 == HIGH)) nodeID=nodeID+1;
  if ((DIP1 == HIGH) && (DIP2 == LOW)) nodeID=nodeID+2;
  if ((DIP1 == LOW) && (DIP2 == LOW)) nodeID=nodeID+3;
  
   rf12_initialize(nodeID, RF_freq, networkGroup);                       // Initialize RFM12B
  
  // Send RFM12B test sequence (for factory testing)
  for (int i=10; i>-1; i--)                                         
  {
    emonth.temp=i; 
    rf12_sendNow(0, &emonth, sizeof emonth);
    delay(100);
  }
  rf12_sendWait(2);
  emonth.temp=0;
  // end of factory test sequence
  
  rf12_sleep(RF12_SLEEP);
  if (debug==1)
  {
    Serial.begin(9600);
    Serial.print(DIP1); Serial.println(DIP2);
    Serial.println("emonTH - Firmware V1.5.1"); 
    Serial.println("OpenEnergyMonitor.org");
    #if (RF69_COMPAT)
      Serial.println("RFM69CW Init> ");
    #else
      Serial.println("RFM12B Init> ");
    #endif
    Serial.print("Node: "); 
    Serial.print(nodeID); 
    Serial.print(" Freq: "); 
    if (RF_freq == RF12_433MHZ) Serial.print("433Mhz");
    if (RF_freq == RF12_868MHZ) Serial.print("868Mhz");
    if (RF_freq == RF12_915MHZ) Serial.print("915Mhz"); 
    Serial.print(" Network: "); 
    Serial.println(networkGroup);
    delay(100);
  }
  
  pinMode(DHT22_PWR,OUTPUT);
  pinMode(DS18B20_PWR,OUTPUT);
  pinMode(BATT_ADC, INPUT);
  digitalWrite(DHT22_PWR,LOW);




  //################################################################################################################################
  // Power Save  - turn off what we don't need - http://www.nongnu.org/avr-libc/user-manual/group__avr__power.html
  //################################################################################################################################
  ACSR |= (1 << ACD);                     // disable Analog comparator    
  if (debug==0) power_usart0_disable();   //disable serial UART
  power_twi_disable();                    //Disable the Two Wire Interface module.
  // power_timer0_disable();              //don't disable necessary for the DS18B20 library
  power_timer1_disable();
  power_spi_disable();
 
  //################################################################################################################################
  // Test for presence of DHT22
  //################################################################################################################################
  digitalWrite(DHT22_PWR,HIGH);
  dodelay(2000);                                                        // wait 2s for DH22 to warm up
  dht.begin();
  float h = dht.readHumidity();                                         // Read Humidity
  float t = dht.readTemperature();                                      // Read Temperature
  digitalWrite(DHT22_PWR,LOW);                                          // Power down
  
  if (isnan(t) || isnan(h))                                             // check if returns are valid, if they are NaN (not a number) then something went wrong!
  {
    if (debug==1) Serial.println(" - Unable to find DHT22 Sensor..trying agin"); delay(100);
    Sleepy::loseSomeTime(1500); 
    float h = dht.readHumidity();  float t = dht.readTemperature();
    if (isnan(t) || isnan(h))   
    {
      if (debug==1) Serial.println(" - Unable to find DHT22 Sensor for 2nd time..giving up"); 
      DHT22_status=0;
    } 
  } 
  else 
  {
    DHT22_status=1;
    if (debug==1) Serial.println("Detected DHT22 temp & humidity sesnor");  
  }   
 
  //################################################################################################################################
  // Setup and for presence of DS18B20
  //################################################################################################################################
  digitalWrite(DS18B20_PWR, HIGH); delay(50); 
  sensors.begin();
  sensors.setWaitForConversion(false);                             //disable automatic temperature conversion to reduce time spent awake, conversion will be implemented manually in sleeping http://harizanov.com/2013/07/optimizing-ds18b20-code-for-low-power-applications/ 
  numSensors=(sensors.getDeviceCount()); 
  
  byte j=0;                                        // search for one wire devices and
                                                   // copy to device address arrays.
  while ((j < numSensors) && (oneWire.search(allAddress[j])))  j++;
  digitalWrite(DS18B20_PWR, LOW);
  
  if (numSensors==0)
  {
    if (debug==1) Serial.println("No DS18B20 detected");
    DS18B20=0; 
  } 
  else 
  {
    DS18B20=1; 
    if (debug==1) {
      Serial.print("Detected "); Serial.print(numSensors); Serial.println(" DS18B20");
       if (DHT22_status==1) Serial.println("DS18B20 and DHT22 found, assuming DS18B20 is external sensor");
    }
    
  }
  if (debug==1) delay(200);
  
  //################################################################################################################################
  
  // Serial.print(DS18B20); Serial.print(DHT22_status);
  // if (debug==1) delay(200);
   
  digitalWrite(LED,LOW);
} // end of setup


//################################################################################################################################
//################################################################################################################################
void loop()
//################################################################################################################################
{ 
  
  if ((DS18B20==0) && (DHT22_status==0))        //if neither DS18B20 or DHT22 is detected flash the LED then goto forever sleep
  {
    for (int i=0; i<20; i++)
    {
      digitalWrite(LED, HIGH); delay(200); digitalWrite(LED,LOW); delay(200);
    }
    cli();                                      //stop responding to interrupts 
    Sleepy::powerDown();                        //sleep forever
  }

  if (DS18B20==1)
  {
    digitalWrite(DS18B20_PWR, HIGH); dodelay(50); 
    for(int j=0;j<numSensors;j++) sensors.setResolution(allAddress[j], TEMPERATURE_PRECISION);      // and set the a to d conversion resolution of each.
    sensors.requestTemperatures();                                        // Send the command to get temperatures
    dodelay(ASYNC_DELAY); //Must wait for conversion, since we use ASYNC mode
    float temp=(sensors.getTempC(allAddress[0]));
    digitalWrite(DS18B20_PWR, LOW);
    if ((temp<125.0) && (temp>-40.0))
    {
      if (DHT22_status==0) emonth.temp=(temp*10);            // if DHT22 is not present assume DS18B20 is primary sensor (internal)
      if (DHT22_status==1) emonth.temp_external=(temp*10);   // if DHT22 is present assume DS18B20 is external sensor wired into terminal block
    }
  }
  
  if (DHT22_status==1)
  { 
    digitalWrite(DHT22_PWR,HIGH);                                                                                                  // Send the command to get temperatures
    dodelay(2000);                                             //sleep for 1.5 - 2's to allow sensor to warm up
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    emonth.humidity = ((dht.readHumidity())*10);

    float temp=(dht.readTemperature());
    if ((temp<85.0) && (temp>-40.0)) emonth.temp = (temp*10);

    digitalWrite(DHT22_PWR,LOW); 
  }
  
  
  emonth.battery=int(analogRead(BATT_ADC)*0.03225806);                    //read battery voltage, convert ADC to volts x10
  
  if (debug==1) 
  {
    if (DS18B20)
    {
      Serial.print("DS18B20 Temperature: ");
      if (DHT22_status) Serial.print(emonth.temp_external/10.0); 
      if (!DHT22_status) Serial.print(emonth.temp/10.0);
      Serial.print("C, ");
    }
    
    if (DHT22_status)
    {
      Serial.print("DHT22 Temperature: ");
      Serial.print(emonth.temp/10.0); 
      Serial.print("C, DHT22 Humidity: ");
      Serial.print(emonth.humidity/10.0);
      Serial.print("%, ");
    }
    
    if (DHT22_status) {
      emonth.emc = EMCfromTH(CtoF(emonth.temp / 10.0), emonth.humidity/10.0) * 10.0;
      Serial.print("EMC: ");
      Serial.print(emonth.emc / 10.0);
      Serial.print("%, ");
    }
    else {
      emonth.emc = 0;
    }
    
    Serial.print("Battery voltage: ");  
    Serial.print(emonth.battery/10.0);
    Serial.println("V");
    delay(100);
  }

  
  power_spi_enable();  
  rf12_sleep(RF12_WAKEUP);
  rf12_sendNow(0, &emonth, sizeof emonth);
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
  power_spi_disable();  
  // digitalWrite(LED,HIGH);
  // dodelay(100);
  // digitalWrite(LED,LOW);  
  
  byte oldADCSRA=ADCSRA;
  byte oldADCSRB=ADCSRB;
  byte oldADMUX=ADMUX;   
  Sleepy::loseSomeTime(time_between_readings*60*1000);  
  //Sleepy::loseSomeTime(2000);
  ADCSRA=oldADCSRA; // restore ADC state
  ADCSRB=oldADCSRB;
  ADMUX=oldADMUX;
}

void dodelay(unsigned int ms)
{
  byte oldADCSRA=ADCSRA;
  byte oldADCSRB=ADCSRB;
  byte oldADMUX=ADMUX;
      
  Sleepy::loseSomeTime(ms); // JeeLabs power save function: enter low power mode for x seconds (valid range 16-65000 ms)
      
  ADCSRA=oldADCSRA;         // restore ADC state
  ADCSRB=oldADCSRB;
  ADMUX=oldADMUX;
}

float EMCfromTH(float T, float H) {
  // formula from this page: http://www.csgnetwork.com/emctablecalc.html
  
  H /= 100;
  float Tsquared = T * T;
  float W = 330 + 0.452 * T + 0.00415 * Tsquared;
  float K = 0.791 + 0.000463 * T - 0.000000844 * Tsquared;
  float Ka = 6.34 + 0.000775 * T - 0.0000935 * Tsquared;
  float Kb = 1.09 + 0.0284 * T - 0.0000904 * Tsquared;
  float M = 1800 / W * ( K * H / (1 - K * H) + (Ka * K * H + 2 * Ka * Kb * K * K * H * H) / (1 + Ka * K * H + Ka * Kb * K * K * H * H)); 
  return M;
}

float CtoF(float C) {
  return C * 1.8 + 32.0;
}

