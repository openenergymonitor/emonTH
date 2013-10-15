/*
  emonTH V1.4 LowPower DHT22 Humidity & Temperature & DS18B20 Node Example 

  Checkes at startup for presence of a DS18B20, DHT22 or both
  If it finds both sensors the temperature value will be taken from the Ds18B20 and humidity from DHT22
  If it finds only DS18B20 then no humidity value will be reported
  If it finds only a DHT22 then both temperature and humidity values will be obtained from this sesor
 
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
 
  Authors: Glyn Hudson
  Builds upon JeeLabs RF12 library and Arduino

  THIS SKETCH REQUIRES:

  Libraries in the standard arduino libraries folder:
	- RFu JeeLib		https://github.com/openenergymonitor/RFu_jeelib
	- DHT22 Sensor Library  https://github.com/adafruit/DHT-sensor-library - be sure to rename the sketch folder to remove the '-'
        - OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
	- DallasTemperature	http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_LATEST.zip
        
*/

/*Recommended node ID allocation
------------------------------------------------------------------------------------------------------------
-ID-	-Node Type- 
0	- Special allocation in JeeLib RFM12 driver - reserved for OOK use
1-4     - Control nodes 
5-10	- Energy monitoring nodes
11-14	--Un-assigned --
15-16	- Base Station & logging nodes
17-30	- Environmental sensing nodes (temperature humidity etc.)
31	- Special allocation in JeeLib RFM12 driver - Node31 can communicate with nodes on any network group
-------------------------------------------------------------------------------------------------------------
*/



#define freq RF12_433MHZ                                              // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 19;                                                // emonth temperature RFM12B node ID - should be unique on network
const int networkGroup = 210;                                           // emonth RFM12B wireless network group - needs to be same as emonBase and emonGLCD
                                                                      //DS18B20 resolution 9,10,11 or 12bit corresponding to (0.5, 0.25, 0.125, 0.0625 degrees C LSB), lower resolution means lower power

const int time_between_readings= 1;                                   //in min

#include <RFu_JeeLib.h>                                                 
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                             // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

//Hardwired emonTH pin allocations 
const int DS18B20_PWR=5;
const int DHT22_PWR=6;
const int LED=9;
const int BATT_ADC=1;
#define ONE_WIRE_BUS 19
#define DHTPIN 18   

// Humidity code adapted from ladyada' example                        //emonTh DHT22 data pin
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);
boolean DHT22_status;                                                       //create flag variable to store presence of DS18B20

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensor;  
boolean DS18B20;                                                     //create flag variable to store presence of DS18B20 

typedef struct {                                                    //RFM12B RF payload datastructure
  	  int temp;
          int humidity;    
          int battery;          	                                      
} Payload;
Payload emonth;

boolean debug;

void setup() {
  if (Serial) debug = 1; else debug=0;                              //if serial UART to USB is connected show debug O/P. If not then disable serial
  
    if (debug==1)
  {
    Serial.begin(9600);
    Serial.println("emonTH DHT22 example"); 
    Serial.println("OpenEnergyMonitor.org");
    Serial.print("Node: "); 
    Serial.print(nodeID); 
    Serial.print(" Freq: "); 
    if (freq == RF12_433MHZ) Serial.print("433Mhz");
    if (freq == RF12_868MHZ) Serial.print("868Mhz");
    if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
    Serial.print(" Network: "); 
    Serial.println(networkGroup);
  }
  
  pinMode(DHT22_PWR,OUTPUT);
  pinMode(DS18B20_PWR,OUTPUT);
  pinMode(BATT_ADC, INPUT);
  digitalWrite(DHT22_PWR,LOW);

  pinMode(LED,OUTPUT); digitalWrite(LED,HIGH);                     //Status LED on
  
  //Test for presence of DS18B20
  digitalWrite(DS18B20_PWR, HIGH); delay(2); 
  sensors.begin();
  if (!sensors.getAddress(sensor, 0)) 
    {
      if (debug==1) Serial.println("Unable to find DS18B20 Temperature Sensor");
      DS18B20=0; 
    } 
    else 
    {
      DS18B20=1; 
      if (debug==1) Serial.println("Detected DS18B20..using this for temperature reading");
    }
 digitalWrite(DS18B20_PWR, LOW);
 
 //Test for presence of DHT22
  digitalWrite(DHT22_PWR,HIGH);
  delay(2000);                                                          // wait 2s for DH22 to warm up
  dht.begin();
  float h = dht.readHumidity();  float t = dht.readTemperature();
  if (isnan(t) || isnan(h))                                             // check if returns are valid, if they are NaN (not a number) then something went wrong!
    {
      if (debug==1) Serial.println(" - Unable to find DHT22 Sensor..trying agin"); delay(100);
      Sleepy::loseSomeTime(1500); 
      float h = dht.readHumidity();  float t = dht.readTemperature();
      if (isnan(t) || isnan(h))   
      {
        if (debug==1) Serial.println(" - Unable to find DHT22 Sensor for 2nd time..giving up"); delay(100);
        DHT22_status=0;
      } 
    } 
    else 
    {
    DHT22_status=1;
    if (debug==1) Serial.println("Detected DHT22");
    }   
  digitalWrite(DHT22_PWR,LOW);
  delay(10);
  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RFM12B
  rf12_control(0xC040);                                                 // set low-battery level to 2.2V i.s.o. 3.1V
  delay(10);
  rf12_sleep(RF12_SLEEP);
  
  digitalWrite(LED,LOW);
}

void loop()
{ 

  if (DS18B20==1)
  {
     digitalWrite(DS18B20_PWR, HIGH); delay(2); 
     sensors.requestTemperatures();                                        // Send the command to get temperatures
     float temp=(sensors.getTempCByIndex(0));
     digitalWrite(DS18B20_PWR, LOW);
     if ((temp<125.0) && (temp>-40.0)) emonth.temp=(temp*10);            //if reading is within range for the sensor convert float to int ready to send via RF
  }
  
 
 if (DHT22_status==1)
{ 
  digitalWrite(DHT22_PWR,HIGH);                                                                                                  // Send the command to get temperatures
  Sleepy::loseSomeTime(2000);                                             //sleep for 1.5 - 2's to allow sensor to warm up
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  emonth.humidity = ((dht.readHumidity())*10);
  if (DS18B20==0) 
  {
    float temp=(dht.readTemperature());
    if ((temp<85.0) && (temp>-40.0)) emonth.temp = (temp*10);
  }
  delay(5);
  digitalWrite(DHT22_PWR,LOW); 
}
  
  emonth.battery=int(analogRead(BATT_ADC)*0.3225806);                    //take battery voltatge reading
                                                 
  
  
  if (debug==1) 
  {
    Serial.print (emonth.humidity); Serial.print(" "); Serial.print(emonth.temp); Serial.print(" ");  Serial.println(emonth.battery);
    delay(15);
  }
  else
    Serial.end();
  
  
  rf12_sleep(RF12_WAKEUP);
  rf12_sendNow(0, &emonth, sizeof emonth);
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);

  //digitalWrite(LED,HIGH);
  //delay(100);
  //digitalWrite(LED,LOW);  
  
  Sleepy::loseSomeTime(time_between_readings*60*1000);                        //But ATMEGA328 into WDT sleep
//Sleepy::loseSomeTime(3000);
}

