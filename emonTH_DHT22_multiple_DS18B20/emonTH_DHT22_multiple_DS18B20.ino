/*
 emonTH low power temperature & humidity node
 ============================================
 
 !!! this is currently a hogepoge of code! do not use!!!
 
 Ambient humidity & temperature (DHT22 on-board)
 Multiple remote temperature (DS18B20)
 
 Provides the following inputs to emonCMS:
 
 1. Battery voltage
 2. Humidity (with DHT22 on-board sensor, otherwise zero)
 3. Ambient temperature (with DHT22 on-board sensor, otherwise zero)
 4. External temperature 1 (first DS18B20)
 5. External temperature 2 (second DS18B20)....
 6. and so on. Should automatically detect any DS18B20 connected to the one wire bus. 
 note - if you connect additional DS18B20 sensors, the rf packet sensor order may change. check your inputs in emoncms after adding additional sensors and reassign accordingly.
 
 -----------------------------------------------------------------------------------------------------------  
 Technical hardware documentation wiki: http://wiki.openenergymonitor.org/index.php?title=EmonTH
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Dave McCraw, Marshall Scholz
 
 Based on the emonTH_DHT22_DS18B20 sketch by Glyn Hudson and the dallas temp library tester-simple sketch.
 
 THIS SKETCH REQUIRES:
 
 Libraries in the standard arduino libraries folder:
   - RFu JeeLib           https://github.com/openenergymonitor/RFu_jeelib   - to work with CISECO RFu328 module
   - DHT22 Sensor Library https://github.com/adafruit/DHT-sensor-library    - be sure to rename the sketch folder to remove the '-'
   - OneWire library      http://www.pjrc.com/teensy/td_libs_OneWire.html   - DS18B20 sensors
   - DallasTemperature    http://download.milesburton.com/Arduino/MaximTemperature/DallasTemperature_LATEST.zip - DS18B20 sensors
 */
 
#include <avr/power.h>
#include <avr/sleep.h>
#include <RFu_JeeLib.h>                                                 
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

// Attach JeeLib sleep function to Atmega328 watchdog - enables MCU to be put into sleep mode between readings to reduce power consumption 
ISR(WDT_vect) { 
  Sleepy::watchdogEvent(); 
} 

 
/*
 Network configuration
 =====================
 
  - RFM12B frequency can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
  - RFM12B wireless network group - needs to be same as emonBase and emonGLCD
  - RFM12B node ID - should be unique on network
 
 Recommended node ID allocation
 ------------------------------------------------------------------------------------------------------------
 -ID-	-Node Type- 
 0	- Special allocation in JeeLib RFM12 driver - reserved for OOK use
 1-4    - Control nodes 
 5-10	- Energy monitoring nodes
 11-14	--Un-assigned --
 15-16	- Base Station & logging nodes
 17-30	- Environmental sensing nodes (temperature humidity etc.)
 31	- Special allocation in JeeLib RFM12 driver - Node31 can communicate with nodes on any network group
 -------------------------------------------------------------------------------------------------------------
 */
#define FREQUENCY RF12_433MHZ 
const int NETWORK_GROUP = 210;
const int NODE_ID       = 19;


// Monitoring configuration
// ========================
 
 const int SECS_BETWEEN_READINGS = 60;  // How long to wait between readings, in seconds. - default = 60
 
 const int ASYNC_DELAY           = 375; // delay for onewire sensors to send data
 
 const int TEMPERATURE_PRECISION = 11;  // onewire temperature sensor precisionn. details found below. - default = 11
 /*
  NOTE: There is a trade off between power consumption and sensor resolution.
        A higher resolution will keep the processor awake longer - approximate values found below.
                                     
  - DS18B20 temperature precision:
      9bit: 0.5C,  10bit: 0.25C,  11bit: 0.1125C, 12bit: 0.0625C
  - Required delay when reading DS18B20
      9bit: 95ms,  10bit: 187ms,  11bit: 375ms,   12bit: 750ms
     
  More info can be found here: http://harizanov.com/2013/07/optimizing-ds18b20-code-for-low-power-applications/   
 */


// emonTH pin allocations 
const int BATT_ADC     = 1; // adc 1
const int DS18B20_PWR  = 5; 
const int DHT_PWR      = 6;
const int LED          = 9;
const int DHT_PIN      = 18; 
const int ONE_WIRE_BUS = 19;  

// end of configuration
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// start of part that does stuff

// On board DHT22
DHT dht(DHT_PIN, DHT22);
boolean DHT_PRESENT;                                                  

// OneWire for DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found onewire device address

//int onewireTemp[20];  // supports howevermany onewire temperature sensors that this number allows for. increase if you have more sensors. 
                         // watch out for ram overflows though.....  

// RFM12B RF payload datastructure
typedef struct {       
  int battery;    
  int humidity;                                                  
  int internalTemp;       	                                      
  int onewireTemp[20];	                                      
  // If you have more sensors, add further variables here.
} 
Payload;

Payload rfPayload; // make a new variable "rfPayload" with type "payload"


boolean debug; // variable to store is debug is avalable or not. 
 
 

/**
 * setup() - called once on boot to initialise the emonTH
 */
void setup() {

  // Output only if serial UART to USB is connected
  debug = Serial ? 1 : 0;                              

  print_welcome_message();  
  set_pin_modes();

  // LED on
  digitalWrite(LED, HIGH);

  // Initialize RFM12B
  rf12_initialize(NODE_ID, FREQUENCY, NETWORK_GROUP);                       
  rf12_sleep(RF12_SLEEP);

  // turn off non crital chip functions to reduce battery drain
  reduce_power();
  
  // Find & Initialise sensors
  initialise_DHT22();
  initialise_DS18B20();

  // Confirm we've got at least one sensor, or shut down.
  validate_sensor_presence();

  // LED off
  digitalWrite(LED, LOW);

} // end of setup

/**
 * Perform temperature and humidity logging
 */
void loop()
{ 


  // External temperature readings
  if (numberOfDevices >= 0) 
    take_ds18b20_reading();

  // Internal temperature / humidity readings
  if (DHT_PRESENT)
    take_dht22_reading();

  // Battery reading
  take_battery_reading();

  // Debugging
  print_payload();                                             

  // power up radio and send data packet
  power_spi_enable();  
  rf12_sleep(RF12_WAKEUP);
  rf12_sendNow(0, &rfPayload, sizeof rfPayload);
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
  power_spi_disable();  

  if (debug){
    flash_led(50);
  }

  // That's it - wait until next time :)
  sleep_until_next_reading();
}


//////////////////////////////////////////////////
/**
 * Set the pin modes required for this sketch
 */
void set_pin_modes()
{
  pinMode(LED,         OUTPUT); 
  pinMode(DHT_PWR,   OUTPUT);
  pinMode(DS18B20_PWR, OUTPUT);
  pinMode(BATT_ADC,    INPUT);
}

/////////////////////////////////////////////////
/**
 * Flash the LED for the stated period
 */
void flash_led (int duration){
  digitalWrite(LED,HIGH);
  dodelay(duration);
  digitalWrite(LED,LOW); 
}

//////////////////////////////////////////////////
/**
 * Find the DHT22 sensor, if fitted
 */
void initialise_DHT22()
{
  DHT_PRESENT=1;

  // Switch on and wait for warm up
  digitalWrite(DHT_PWR,HIGH);
  dodelay(2000);                   
  dht.begin();

  // We should get numeric readings, or something's up.
  if (isnan(dht.readTemperature()) || isnan(dht.readHumidity()))                                         
  {
    if (debug) Serial.println("Unable to find DHT22 temp & humidity sensor... trying again"); 
    Sleepy::loseSomeTime(1500); 

    // One last try
    if (isnan(dht.readTemperature()) || isnan(dht.readHumidity()))   
    {
      if (debug) Serial.println("Unable to find DHT22 temp & humidity sensor... giving up"); 
      DHT_PRESENT=0;
    } 
  } 

  if (debug && DHT_PRESENT) {
    
    Serial.println("Detected DHT22 temp & humidity sensor");  
  }

  // Power off for now
  digitalWrite(DHT_PWR,LOW);                                          
}

//////////////////////////////////////////////////
/**
 * Find the expected DS18B20 sensors
 *
 * automatically scans the entire onewire bus for sensors and stores their adressess in the device adresses array.
 * 
 * should support a large number of sensors. tested up to four sensors currently.
 */
void initialise_DS18B20()
{
  // Switch on
  digitalWrite(DS18B20_PWR, HIGH); 
  dodelay(50); 

  // start up oonewire library
  sensors.begin();

  // Disable automatic temperature conversion to reduce time spent awake, instead we sleep for ASYNC_DELAY
  // see http://harizanov.com/2013/07/optimizing-ds18b20-code-for-low-power-applications/ 
  sensors.setWaitForConversion(false);                             

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();

// not needed
 // EXT_SENSOR1_PRESENT = sensors.getAddress(EXT_SENSOR1, 0);
 
 // bus info
  if (debug){
    
     // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");
  } 
 
 // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
	{
		if (debug) {
                  Serial.print("Found device ");
		  Serial.print(i, DEC);
		  Serial.print(" with address: ");
		  printAddress(tempDeviceAddress); // just prints sensor adress on serial bus
		  Serial.println();
		
		  Serial.print("Setting resolution to ");
		  Serial.println(TEMPERATURE_PRECISION, DEC);
                }
		
		// set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
		sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
		
		if (debug) {
		  Serial.print("Resolution actually set to: ");
		  Serial.print(sensors.getResolution(tempDeviceAddress), DEC); 
		  Serial.println();
                }
	}else{
              
	      if (debug) {
		Serial.print("Found ghost device at ");
		Serial.print(i, DEC);
		Serial.print(" but could not detect address. Check power and cabling");
              }
	}
  }



    
    
   // if (!EXT_SENSOR1_PRESENT){ }

  // Switch off for now
  digitalWrite(DS18B20_PWR, LOW);
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

//////////////////////////////////////////////////
/** 
 * If we don't have at least one sensor available, it's sleep time
 */
void validate_sensor_presence()
{
  if (!DHT_PRESENT && !numberOfDevices >= 1) 
  {
    if (debug) {
      
      Serial.print("Power down - no sensors detected at all!");
    }
    
    for (int i=0; i<10; i++)
    {
      flash_led(250); 
      dodelay(250);
    }
    cli();                                      //stop responding to interrupts 
    Sleepy::powerDown();                        //sleep forever
  }
}

//////////////////////////////////////////////////
/**
 * Convenience method; battery reading
 */
void take_battery_reading()
{
  // convert ADC to volts x10
  rfPayload.battery=int(analogRead(BATT_ADC)*0.03225806);                    
}

/**
 * Convenience method; DHT22 readings
 */
void take_dht22_reading()
{
  // Power on. It's a long wait for this sensor!
  digitalWrite(DHT_PWR, HIGH);                
  dodelay(2000);                                

  rfPayload.humidity = ( (dht.readHumidity() )* 10 );

  float temp=(dht.readTemperature());
  if (temperature_in_range(temp)) 
    rfPayload.internalTemp = (temp*10);

  digitalWrite(DHT_PWR, LOW); 
}

//////////////////////////////////////////////////
/**
 * Convenience method; read from all DS18B20s
 * 
 */
void take_ds18b20_reading () 
{
  // Power up
  digitalWrite(DS18B20_PWR, HIGH); 
  dodelay(50); 
  
  
// call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  
  
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++)
  {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i))
	{	
           sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
           
           // Get readings. We must wait for ASYNC_DELAY due to power-saving (waitForConversion = false)
           sensors.requestTemperatures();                                   
           dodelay(ASYNC_DELAY); 
           float temp1=(sensors.getTempC(tempDeviceAddress));
           
           // Payload will maintain previous reading unless the temperature is within range.
           if (temperature_in_range(temp1))
           rfPayload[3].onewireTemp[i] = temp1 * 10; 
	} 
	//else ghost device! Check your power requirements and cabling
	
  }

  // Power down
  digitalWrite(DS18B20_PWR, LOW);
 
}


//////////////////////////////////////////////////
/**
 * validate that the provided temperature is within acceptable bounds.
 */
boolean temperature_in_range(float temp)
{
  // Only accept the reading if it's within a desired range.
  float minimumTemp = -40.0;
  float maximumTemp = 125.0;

  return temp > minimumTemp && temp < maximumTemp;
}

//////////////////////////////////////////////////
/**
 * For debugging purposes: print the payload as it will shortly be sent to the emonBASE
 * Wise to extend this if you have extra sensors wired in.
 */
void print_payload()
{
  if (!debug)
    return;
  
  Serial.println("emonTH payload: ");

  Serial.print("  Battery voltage: ");
  Serial.print(rfPayload.battery/10.0);
  Serial.println("V");
  

 /* if (EXT_SENSOR1_PRESENT){
    Serial.print( rfPayload.externalTemp1/10.0); 
    Serial.println("C");
  }
  else {
    Serial.println(" not present");
  }
  */
  
  
  if (DHT_PRESENT){
    Serial.print("  Internal DHT22 temperature: ");
    Serial.print(rfPayload.internalTemp/10.0); 
    Serial.print("C, Humidity: ");
  
    Serial.print(rfPayload.humidity/10.0);
    Serial.println("% ");
  }
  else {
    Serial.println("Internal DHT22 sensor: not present");
  }
  
  Serial.println();
}

//////////////////////////////////////////////////
/**
 * Dumps useful intro to serial
 */
void print_welcome_message()
{
  if (!debug)
    return;

  Serial.begin(9600);

  Serial.println("emonTH : OpenEnergyMonitor.org");
  
  Serial.print("Node: "); 
  Serial.print(NODE_ID); 
 
  Serial.print(" Freq: "); 
  switch(FREQUENCY){
  case RF12_433MHZ:
    Serial.print("433Mhz");
    break;
  case RF12_868MHZ:
    Serial.print("868Mhz");
    break;
  case RF12_915MHZ:
    Serial.print("915Mhz");
    break;
  }

  
  Serial.print(" Network: "); 
  Serial.println(NETWORK_GROUP);

  
  dodelay(100);
}

//////////////////////////////////////////////////
/**
 * Turn off what we don't need.
 * see http://www.nongnu.org/avr-libc/user-manual/group__avr__power.html
 */
void reduce_power()
{
  ACSR |= (1 << ACD);              // Disable Analog comparator    
  power_twi_disable();             // Disable the Two Wire Interface module.

  power_timer1_disable();          // Timer 1
  power_spi_disable();             // Serial peripheral interface

    if (!debug){
    power_usart0_disable();        // Disable serial UART if not connected
  }  

  power_timer0_enable();           // Necessary for the DS18B20 library.
}

//////////////////////////////////////////////////
/**
 * Power-friendly delay
 */
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

//////////////////////////////////////////////////
/**
 * To save power, we go to sleep between readings
 */
void sleep_until_next_reading(){
  byte oldADCSRA=ADCSRA;
  byte oldADCSRB=ADCSRB;
  byte oldADMUX=ADMUX;   
  Sleepy::loseSomeTime(SECS_BETWEEN_READINGS*1000);  
  ADCSRA=oldADCSRA; // restore ADC state
  ADCSRB=oldADCSRB;
  ADMUX=oldADMUX;
}


