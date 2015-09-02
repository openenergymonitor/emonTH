// Warm1,2 & 3 pulse count for Kamstrup and gas.  
bool debug = 1;

#define RF69_COMPAT 1
#include <JeeLib.h>

#include <avr/power.h>
#include <avr/sleep.h>
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

const int LED=            9;
const int BATT_ADC=       1;

typedef struct {                                                      // RFM12B RF payload datastructure
  	  unsigned long pulsecount;
          int battery;       	                                      
} Payload;
Payload emonth;

unsigned long pulsetime = 0;

// minimum width of interrupt pulse (default pulse output meters = 100ms)
const byte min_pulsewidth= 110;

void setup() {
  pinMode(LED,OUTPUT); digitalWrite(LED,HIGH);
  
  if (debug) {
    Serial.begin(9600);
    Serial.println("emonTH Pulse Counting example");
    delay(100);
  } else {
    power_usart0_disable();
  }
  power_twi_disable();
  
  rf12_initialize(3,RF12_433MHZ,210);
  rf12_sleep(RF12_SLEEP);
  
  emonth.pulsecount = 0;
  
  attachInterrupt(1, onPulse, FALLING);
  digitalWrite(LED,LOW);
}

void loop()
{
  // STATUS COUNT
  emonth.battery = analogRead(BATT_ADC) * 3.32;                    //read battery voltage, convert ADC to volts x10

  if (debug) {
    Serial.print("Count: ");
    Serial.println(emonth.pulsecount);
    delay(25);
    Serial.print("Battery: ");
    Serial.println(emonth.battery);
    delay(25);
  }
  
  power_spi_enable();  
  rf12_sleep(RF12_WAKEUP);
  delay(20);
  rf12_sendNow(0, &emonth, sizeof emonth);
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
  emonth.pulsecount++;   // TRY TO GET NUMBER SENT
  power_spi_disable();  
  
  Sleepy::loseSomeTime(60000);  //TAKEN ONE ZERO OFF FOR TESTING!!!
  
}

void onPulse() {
  
  if ( (millis() - pulsetime) > min_pulsewidth) {
    emonth.pulsecount++;
    pulsetime=millis(); 
  }
  
  digitalWrite(LED, HIGH); 
  delay(2); 
  digitalWrite(LED, LOW);
}
