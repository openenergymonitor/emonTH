/*
 emonTH/gas : reflection-based (analogue) gas node
 =================================================
 
 Gas meter with reflective dial (phototransistor on analogue pin ADC5)
 
 Provides the following inputs to emonCMS:
 
 1. Battery voltage
 2. Pulses counted during period (using ADC5 input)
 3. Lowest ADC5 reading during period (for calibration / troubleshooting)
 4. Average ADC5 reading during period (for calibration / troubleshooting)
 
 -----------------------------------------------------------------------------------------------------------  
 Technical hardware documentation wiki: http://wiki.openenergymonitor.org/index.php?title=EmonTH
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Dave McCraw,
 
 THIS SKETCH REQUIRES:
 
 Libraries in the standard arduino libraries folder:
   - RFu JeeLib           https://github.com/openenergymonitor/RFu_jeelib   - to work with CISECO RFu328 module
 */
#include <avr/power.h>
#include <avr/sleep.h>
#include <RFu_JeeLib.h>

ISR(WDT_vect) { 
  Sleepy::watchdogEvent(); 
} // Attached JeeLib sleep function to Atmega328 watchdog - enables MCU to be put into sleep mode between readings to reduce power consumption 

 
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
 
 Also ACK/retry configuration:
 - maximum number of retries (more retries, more power demand)
 - how long to allow for an ack to be received
 */
#define FREQUENCY RF12_433MHZ 
const byte NETWORK_GROUP = 210;
const byte NODE_ID       = 7;

const byte RETRY_MAX_ATTEMPTS   = 3;
const byte MAX_ACK_TIME_MS      = 50;

/*
 Monitoring configuration
 ========================
 
  - how long your sensor needs to power up
  - how long to wait between analogue readings.
  - threshold below which we will register a 'pulse' (0-1023)
  - threshold above which we will allow another pulse to occur (0-1023)
  - minimum possible time between pulses (in seconds)
  
 Advice on configuration: if you fit the node in place and watch a 'realtime'-type visualisation
 of input #3 (min value) while using some gas, you should see a line with negative peaks. You need to
 set both threshold values to be somewhere between the 'normal' line and the peaks. 
 
 A reading below PULSE_THRESHOLD_VALUE will cause a pulse to be logged. No further pulses will be logged 
 until at least one reading higher than RESET_THRESHOLD_VALUE is detected, AND a length of time greater 
 than MIN_SEC_BETWEEN_PULSES has passed.
 
 The larger the figure for MS_DELAY_BETWEEN_READS, the better battery life is. However, you must take
 a reading more rapidly than the period it takes the reflective dial to pass below your sensor, or it could
 result in a missed pulse.
 
 NUMBER_OF_READS_PER_PAYLOAD and MS_DELAY_BETWEEN_READS define the reporting frequency to emonBase (default ~5 min). 
 Roughly, the node will report every MS_DELAY_BETWEEN_READS / 1000 * NUMBER_OF_READS_PER_PAYLOAD seconds.
 
 */
const byte MS_SENSOR_POWER_UP = 25;
const int MS_DELAY_BETWEEN_READS = 500;
const int NUMBER_OF_READS_PER_PAYLOAD = 600; 
const int PULSE_THRESHOLD_VALUE = 150;  
const int RESET_THRESHOLD_VALUE = 300;  
const byte MIN_SEC_BETWEEN_PULSES = 5;

// emonTH pin allocations  
const byte BATT_ADC        = 1;
const byte PHOTOSENSOR_PWR = 5;
const byte LED             = 9;
const byte ADC_PIN         = A5; 


// RFM12B RF payload datastructure
typedef struct {       
  int battery;              
  int pulses;
  int lowestValue;
  int avgValue;
  // If you have more sensors, add further variables here.
} 
Payload;

Payload rfPayload;

boolean debug;


/**
 * setup() - called once on boot to initialise the emonTH
 */
void setup() {  
  // Output only if serial UART to USB is connected
  debug = Serial ? 1 : 0;                              

  if (debug) Serial.begin(9600);

  print_welcome_message();  

  set_pin_modes();
  
  // LED on
  flash_led(500);

  // Initialize RFM12B
  rf12_initialize(NODE_ID, FREQUENCY, NETWORK_GROUP);                       
  rf12_sleep(RF12_SLEEP);

  // Low power settings
  reduce_power();
  
} // end of setup

/** 
 * Variables and constants associated with the actual reading process
 */
long cumulativeReading   = 0;
int  pulseCount          = 0;
int  lowestReading       = 1023;

const boolean RESET_PULSE_COUNTER     = true;
const boolean ROLL_OVER_PULSE_COUNTER = false;


/**
 * Perform gas logging
 * Should be extended to cope with on-board temp sensor too.
 */
void loop()
{   
  // Perform a certain number of gas monitoring cycles.
  int readCount;
  for (readCount = 0; readCount <= NUMBER_OF_READS_PER_PAYLOAD; readCount++){
    flash_led(50);
    take_IR_reading();
    sleep_until_next_reading(MS_DELAY_BETWEEN_READS - MS_SENSOR_POWER_UP - 50);
  }

  // Use the gathered data to update the gas part of the RF payload
  payloadUpdateGas( readCount );
  
  // How's our power doing?
  take_battery_reading();
  
  // Finally, send it all to the base station
  boolean payloadDelivered = sendPayloadWithAck();
  
  // Reset for next loop. Only reset the pulse count if the payload was delivered!
  initialiseState(payloadDelivered);
}

/**
 * To save power, we go to sleep between readings
 */
void sleep_until_next_reading(int sleepDuration){
  byte oldADCSRA=ADCSRA;
  byte oldADCSRB=ADCSRB;
  byte oldADMUX=ADMUX;   
  Sleepy::loseSomeTime(sleepDuration);  
  ADCSRA=oldADCSRA; // restore ADC state
  ADCSRB=oldADCSRB;
  ADMUX=oldADMUX;
}

/**
 * Update the payload based on the gas readings we've taken
 */
void payloadUpdateGas( int readCount ){
  // Number of pulses counted since the last RF payload was transmitted
  rfPayload.pulses = pulseCount;
  // The average and lowest values of the analogue pin since the last RF payload was transmitted 
  // which may be very useful for diagnostics...
  rfPayload.avgValue = cumulativeReading / readCount;
  rfPayload.lowestValue = lowestReading;
}

/**
 * Send the payload to the base station. Ask for an ACK and retry if required.
 * Rolls over the pulse counter until it can be delivered.
 */
boolean sendPayloadWithAck(){

  print_payload();
  
  for (byte i = 0; i <= RETRY_MAX_ATTEMPTS; ++i) { // tx and wait for ack up to RETRY_LIMIT times
    power_spi_enable();
        
    rf12_sleep(RF12_WAKEUP);

    rf12_sendNow(RF12_HDR_ACK, &rfPayload, sizeof rfPayload);
    rf12_sendWait(2); // Wait for RF to finish sending while in standby mode
    byte acked = waitForAck(); // Wait for ACK
    
    rf12_sleep(RF12_SLEEP);
  
    if (acked)
      return true;
    
    power_spi_disable();
    // Wait for a while before trying again.
    Sleepy::loseSomeTime(MS_DELAY_BETWEEN_READS); 
    // Take a reading between each retry attempt. This means retries don't block the reflector processing significantly,
    // since the retry delay is the same as the delay between reflector tests in normal operation.
    take_IR_reading(); 

  }
    
  return false;
}

static byte waitForAck() {
  MilliTimer ackTimer;
  while (!ackTimer.poll(MAX_ACK_TIME_MS)) {
    if (rf12_recvDone() && rf12_crc == 0 &&
        rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | NODE_ID))
      return 1;
  }
  return 0;
}

/**
 * Sets variables to initial state ready for monitoring period
 */
void initialiseState(boolean resetPulseCount){
  cumulativeReading = 0;
  lowestReading = 1023;
  
  if (resetPulseCount)
    pulseCount = 0;
}

long lastReflectionTime  = 0;
boolean resetSinceLastPulse = true;

/**
 * Convenience method; IR reading
 */
void take_IR_reading()
{  
  
  // Power on sensor and grab a reading.
  digitalWrite(PHOTOSENSOR_PWR, HIGH);                
  dodelay(MS_SENSOR_POWER_UP);          
  
  int reading = analogRead(ADC_PIN);
  
  // Power off ASAP
  digitalWrite(PHOTOSENSOR_PWR, LOW);
  
  if (reading < lowestReading){
    lowestReading = reading;
  }
  
  cumulativeReading += reading;
  
  long msSinceLastReflection=millis()-lastReflectionTime;
  
  // To qualify as a reflection, we need to see a reading below threshold and we also need 
  // to have waited at least so many seconds since the last reflection (based on your max gas use)
  if (reading <= PULSE_THRESHOLD_VALUE  && resetSinceLastPulse &&
    msSinceLastReflection > MIN_SEC_BETWEEN_PULSES * 1000){
    pulseCount++;
    lastReflectionTime = millis();
    resetSinceLastPulse = false;
  }
 
  
  // If a pulse has been logged, we need to see a value over the reset threshold 
  // to prove the dial has moved on, and so another pulse would be valid. This prevents
  // the node constantly logging pulses if the reflector isn't moving.
  if (reading >= RESET_THRESHOLD_VALUE) {
    resetSinceLastPulse = true;
  }
}

/**
 * Set the pin modes required for this sketch
 */
void set_pin_modes()
{
  pinMode(LED,              OUTPUT); 
  pinMode(PHOTOSENSOR_PWR,  OUTPUT);
  pinMode(ADC_PIN,          INPUT);
  pinMode(BATT_ADC,         INPUT);
}

/**
 * Flash the LED for the stated period
 */
void flash_led (int duration){
  digitalWrite(LED,HIGH);
  dodelay(duration);
  digitalWrite(LED,LOW); 
  dodelay(duration);
}

/**
 * Dumps useful intro to serial
 */
void print_welcome_message()
{
  if (!debug)
    return;

  Serial.begin(9600);

  Serial.println("emonTH/pulse : OpenEnergyMonitor.org");
  
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

/**
 * For debugging purposes: print the payload as it will shortly be sent to the emonBASE
 * Wise to extend this if you have extra sensors wired in.
 */
void print_payload()
{
  if (!debug)
    return;
  
  Serial.println("emonTH/gas payload: ");

  Serial.print("  Battery voltage: ");
  Serial.print(rfPayload.battery/10.0);
  Serial.println("V");
  
  Serial.print( rfPayload.pulses); 
  Serial.print(" pulses, ");
  Serial.print( rfPayload.lowestValue); 
  Serial.print(" low reading, ");
  Serial.print( rfPayload.avgValue); 
  Serial.println(" avg reading. ");
    
  Serial.println();
}

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

/**
 * Convenience method; battery reading
 */
void take_battery_reading()
{
  // convert ADC to volts x10
  rfPayload.battery=int(analogRead(BATT_ADC)*0.03225806);                    
}
