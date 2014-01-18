// emonTH DS18B20 temperature sensor address discovery script
// - for finiding hardware addresses of one or more DS18B20 sensors connected to emonTH one-wire bus

// THIS SKETCH REQUIRES:

// Libraries in the standard arduino libraries folder:
//
//	- OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
//	- DallasTemperature	http://download.milesburton.com/Arduino/MaximTemperature

#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino



// emonTH pin allocations 
const int BATT_ADC     = 1;
const int DS18B20_PWR  = 5;
const int DHT_PWR      = 6;
const int LED          = 9;
const int DHT_PIN      = 18; 
#define ONE_WIRE_BUS 19

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup()
{
  pinMode(DS18B20_PWR, OUTPUT);
  digitalWrite(DS18B20_PWR, HIGH);

  pinMode(LED, OUTPUT); digitalWrite(LED, HIGH);
  delay(1000);
  Serial.begin(9600);
  Serial.println("emonTH Temperature search");
  Serial.println("waiting 6 seconds before printing");
  delay(6000);

  sensors.begin();
  
  DeviceAddress tmp_address;
  int numberOfDevices = sensors.getDeviceCount();
  
  for(int i=0;i<numberOfDevices; i++)
  {
    sensors.getAddress(tmp_address, i);
    printAddress(tmp_address);
    Serial.println();
  }

digitalWrite(LED, LOW);
}

void loop()
{ 
 
}

void printAddress(DeviceAddress deviceAddress)
{
  Serial.print("{ ");
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    Serial.print("0x");
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i<7) Serial.print(", ");
    
  }
  Serial.print(" }");
}

