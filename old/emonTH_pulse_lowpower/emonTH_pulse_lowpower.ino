// Low power emonTH pulse counting example
// Author: Eric_AMANN
// see forum post: http://openenergymonitor.org/emon/node/10834

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

#define RF_freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ.
const int nodeID = 21;                                                     // emonTh RFM12B node ID
const int networkGroup = 210;                                              // emonTh RFM12B wireless network group
const int UNO = 1;                                                         // Set to 0 if your not using the UNO bootloader (i.e using Duemilanove)
                                                                           // All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader
#include <avr/wdt.h>                                                       // the UNO bootloader 

#include <JeeLib.h>                                                        // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }
#include <avr/power.h>
#include <avr/sleep.h>

typedef struct { unsigned int power, pulse, totalpulse, battery; } PayloadTH;
PayloadTH emonth;                                                          // neat way of packaging data for RF comms

const  unsigned long WDT_PERIOD = 80;                                      // mseconds.
const  unsigned long WDT_MAX_NUMBER = 600;                                 // Data sent after WDT_MAX_NUMBER periods of  WDT_PERIOD ms without pulses
                                                                           //                              OR
const  unsigned long PULSE_MAX_NUMBER = 100;                               // Data sent after PULSE_MAX_NUMBER pulses
const  unsigned long PULSE_MAX_DURATION = 50;                              // Sensor is powered off during PULSE_MAX_DURATION mseconds after a pulse.
 
const int LEDpin = 9;                                                      // emonTH LED pin
const int BATT_ADC = 1;
const int PULSE_SENSOR_PWR = 5;                                            // **Labelled DS18B20 power on emonTH board**


volatile unsigned long pulseCount ;
volatile unsigned long elapsed_time;
int BATT_ADC_number;
unsigned long WDT_number;

boolean  debug;
boolean  p;                                                                 // flag for new pulse

void setup() 
{
  Serial.begin(9600);
  Serial.println("EmonTH Pulse conting Low Power V1.0c");
  Serial.print("Goup Id: ");
  Serial.println(networkGroup);
  Serial.print("Node Id: ");
  Serial.println(nodeID);
  Serial.print("Sending data after ");
  Serial.print(PULSE_MAX_NUMBER);
  Serial.println(" pulses");
  Serial.print("  or  ");
  Serial.print(WDT_MAX_NUMBER);
  Serial.print(" periods of ");
  Serial.print(WDT_PERIOD);
  Serial.println(" ms whitout any pulses");
  delay(100);

  Serial.println("Initializing RF..");
  delay(100);             
  rf12_initialize(nodeID, RF_freq, networkGroup);                         // initialize RF
  rf12_sleep(RF12_SLEEP);
  Serial.println("RF ready");

  pinMode(LEDpin, OUTPUT);                                                // Setup indicator LED
  pinMode(PULSE_SENSOR_PWR,OUTPUT);                                       // Optical pulse sensor power
  digitalWrite(PULSE_SENSOR_PWR, HIGH);
    
  emonth.power=0;
  emonth.pulse=0;
  emonth.totalpulse=0;
  emonth.battery=0;
  pulseCount=0;
    
  WDT_number=0;
  
  if (Serial) debug = 1; else debug=0;                                    // if serial UART to USB is connected show debug O/P. If not then disable serial
  p=0;
  attachInterrupt(1, onPulse, RISING);                                    // KWH interrupt attached to IRQ 0  = Digita 2 - hardwired to terminal block 

 
 }

void loop() 
{
  // if a pulse just came, wait PULSE_MAX_DURATION ms to power on the optical pulse sensor 
  if (p){
    Sleepy::loseSomeTime(PULSE_MAX_DURATION);
    digitalWrite(PULSE_SENSOR_PWR, HIGH);
    p=0;
  }

  if (Sleepy::loseSomeTime(WDT_PERIOD)==1) {                              // true if no pulse occured during the WDT
    WDT_number++;                                                         // number of WDT normally ended since the last RF sent
    if (debug) {
      Serial.print(".");
      delay(1);
    }
  }  
  else if (debug)     
  { 
    Serial.print("INT ");
    delay(5);
    Serial.println(pulseCount);
    delay(5);
  }


  if (WDT_number>=WDT_MAX_NUMBER || pulseCount>=PULSE_MAX_NUMBER) {
    cli(); // Disable interrupt just in case pulse comes in while we are updating the count
    emonth.totalpulse += (unsigned int) pulseCount;
    emonth.pulse = (unsigned int) pulseCount;      
    pulseCount = 0; 
    sei();

    emonth.battery=(unsigned int)(analogRead(BATT_ADC)*0.03225806);       //read battery voltage, convert ADC to volts x10

    if (debug==1) {
      Serial.print("Sending ");delay(5);
      Serial.print(emonth.pulse);delay(5);
      Serial.print("/");delay(5);
      Serial.print(emonth.totalpulse);delay(5);
      Serial.print("/");delay(5);
      Serial.print(emonth.power);delay(5);
      Serial.print("W");delay(5);
      Serial.print("/");delay(5);
      Serial.print(emonth.battery);delay(5);
      Serial.println("");delay(5);
    }   

    send_rf_data();                          // *SEND RF DATA* - see emontx_lib
    WDT_number=0;
  }
}


// The interrupt routine - runs each time a rising edge of a pulse is detected
void onPulse()                  
{
  digitalWrite(PULSE_SENSOR_PWR, LOW);       // optical pulse sensor power off
  p=1;                                       // flag for new pulse set to true
  pulseCount++;                              // number of pulses since the last RF sent
}



void send_rf_data()
{
  power_spi_enable(); 
  rf12_sleep(RF12_WAKEUP);
  rf12_sendNow(0, &emonth, sizeof emonth);   // send temperature data via RFM12B using new rf12_sendNow wrapper
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
  power_spi_disable(); 
}

