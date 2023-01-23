# emonTH - Wireless Temperature and Humidity monitoring node

**This repo is for emonTH V1.x**, See [emonTH2 repo](https://github.com/openenergymonitor/emonth2) for latest emonTH V2.x

---

## Overview

The emonTH is an open-source, battery powered, Temperature and Humidity monitoring wireless node.

It's been designed to be an easy to deploy tool for monitoring building / room temperature and humidity.

The data from the emonTH is transmitted via wireless (433/868MHz) to an emonBase web-connected base-station (we recommend a Raspberry Pi with an RFM69Pi) which then posts the data to an emonCMS server (e.g. http://emoncms.org) for logging, processing and graphing. The temperature and humidity data can be used to inform a heating control system, feed into a building performance model or simply for general interest. 

### Features

- Temperature and Humidity sensing options - Using DHT22 temperature and humidity sensor, or if humidity data is not required a DS18B20 temperature sensor. Both DHT22 and DS18B20 can be used together as shown above for internal and external readings.
- Easy to set-up - the unit comes pre-assembled and pre-loaded with Arduino compatible firmware. If desired, the code is easily changed via the Arduino IDE and a USB to UART cable.
- Long Battery Life - The emonTH is powered by two AA batteries through a high efficiency DC-DC boost converter circuit. Taking a reading once every 60s, the emonTH batteries should last for 1-3 years. We recommend rechargeable alkaline batteries for best performance and environmental impact (see blog post).
- Expansion Options - If desired, the emonTH function can easily be expanded: remote DS18B20 temperature sensors can be attached to the terminal block for outdoor temperature monitoring. Multiple DS18B20 temperature sensors can be connected at once on a digital one-wire bus.
- An optical sensor can be added for interfacing with a pulse-output utility meter or a relay board could be connected for controlling an appliance.
- Update: the emonTH now supports multiple DS18B20's. See blog post
- New v1.5: Node ID select DIP switch: Select from four unique node ID's via on-board DIP switch

### Technical

- Microcontroller: ATmega328 @ 3.3V
- Sensors: DHT22 (temperature & Humidity) / DS18B20 (temperature) sensor options
- Power: 2 x AA batteries in an on-board holder. LTC3525 3.3V DC-DC boost converter to extend battery life.
- RF Radio: RFM69CW (RFM12B can also be used)
- Battery life: 1-3 years expected. See blog post
- On-board LTC3525-3.3 DC-DC boost converter. See emonTH hardware blog post

### Accuracy

**DHT22 Temperature and Humidity Sensor**

- Power supply: 3.3-6V DC
- Output signal: digital signal via single-bus
- Sensing element: Polymer capacitor
- Operating range: humidity 0-100%RH; temperature -40 to ~80Celsius
- Accuracy: humidity +-2%RH(Max +-5%RH); temperature <+-0.5Celsius
- Resolution: humidity 0.1%RH; temperature 0.1Celsius
- Repeatability: humidity +-1%RH; temperature +-0.2Celsius
- Humidity hysteresis: +-0.3%RH
- Long-term Stability: +-0.5%RH/year
- Sensing period Average: 2s
- Independent sensor test report

**DS18B20 Temperature Sensor**

- Power supply range: 3.0V to 5.5V
- Accuracy over the range of -10°C to +85°C: ±0.5°C.
- Storage temperature range:-55°C to +125°C (-67°F to +257°F)

## DIP Switch node ID

New for emonTH V1.5: On-board DIP switch enables 4 RF node IDs to be selected by changing the switch positions. This will enable up to four emonTHs to be configured for use with a single emonBase / emonPi without the need to change the firmware (as before). Up to 30 emonTHs can theoretically connect to a single emonBase / emonPi. A USB to UART cable and Arduino IDE can be used to set additional unique node IDs by changing the nodeID variable at the beginning of the sketch. Alternatively, we are happy to set the node ID for you, before shipping. (Leave us a note with your order) 

| DIP 1  | DIP 2  | RF node ID V1.x firmware  | RF node ID V2.x firmware  |
|--------|--------|---------------------------|---------------------------|
| OFF    | OFF    | 19 (default)              | 23 (default)              |
| ON     | OFF    | 20                        | 24                        |
| OFF    | ON     | 21                        | 25                        |
| ON     | ON     | 22                        | 26                        |

## RF Connectivity

RFM69CW RF module (default 433MHz) is used to transmit data to emonBase (Raspberry Pi + RFM12Pi) or emonPi. The JeeLabs JeeLib Arduino library is used as the driver for the RFM69CW module. The data is in JeeLabs packet format. 

## External DS18B20 Temperature Sensor Connections

Only one DS18B20 sensor can be connected to an emonTH. If a DHT22 sensor is detected, it's assumed the DS18B20 is connected externally. If more than one DS18B20 is required, see alternate emonTH firmware.

- Black - GND
- Red - Dig 5 (digital I/O 5 is used as the power pin to enable sensor power-down between readings to save power)
- White - ADC5 (Dig 18) one-wire bus 

## Pulse Sensor Connection

emonTH with V2.6+ pulse firmware supports Optical Pulse counting sensor 

- Black - GND
- Red - 3.3V
- Blue - IRQ 1 / Dig3 - Blue 

There is an input pull-up inside the pulse (IRQ) input that is enabled in the standard sketch. Therefore, you can connect a volt-free contact or an SO output between screw terminal 4 (IRQ input, SO+) and screw terminal 3 (GND, SO-) without the need for an additional resistor. If you must connect your contacts between VCC (screw terminal 2) and screw terminal 4, then you must add a pull-down resistor of resistance low enough to overcome the internal pull-up resistor, or you can use a higher-value resistor and modify the sketch to disable the internal pull-up.

If you are using a reed switch, you may find that you get more than one count per pulse. Adding a 0.1 µF capacitor across the reed switch has been shown to eliminate this problem. 


## Hardware

Eagle schematic and board files [https://github.com/openenergymonitor/emonTH/tree/master/hardware](https://github.com/openenergymonitor/emonTH/tree/master/hardware).

### Port map

| Arduino        | ATmega328 Port  | Special Function  | emonTH V1.5            |
|----------------|-----------------|-------------------|------------------------|
| Analog 0 (D14) | PC0             |                   |                        |
| Analog 1 (D15) | PC1             |                   | 2x AA Battery Voltage  |
| Analog 2 (D16) | PC2             |                   |                        |
| Analog 3 (D17) | PC3             |                   |                        |
| Analog 4 (D18) | PC4             | (SDA)             | DHT22 Data             |
| Analog 5 (D19) | PC5             | (SCL)             | DS18B20 One-wire Data  |
| Analog 6 (D20) |                 |                   |                        |
| Analog 7 (D21) |                 |                   |                        |
| Digital 0      | PD0             | (RXD)             | FTDI Tx                |
| Digital 1      | PD1             | (TXD)             | FTDI Rx                |
| Digital 2      | PD2             | (int0) PWM        | RFM12B IRQ             |
| Digital 3      | PD3             | (int1) PWM        | Terminal block         |
| Digital 4      | PD4             |                   |                        |
| Digital 5      | PD5             | PWM               | DS18B20 PWR            |
| Digital 6      | PD6             | PWM               | DHT22 PWR              |
| Digital 7      | PD7             |                   | DIP 1                  |
| Digital 8      | PB0             |                   | DIP 2                  |
| Digital 9      | PB1             | PWM               | LED                    |
| Digital 10     | PB2             | (SS) PWM          | RFM12B SEL             |
| Digital 11     | PB3             | (MOSI) PWM        | RFM12 SDI              |
| Digital 12     | PB4             | (MISO)            | RFM12 SDO              |
| Digital 13     | PB5             | (SCK)             | RFM12 SCK              |

**Enclosure: 71 x 71 x 27 mm**<br>
By default, the emonTH is shipped without an SMT mini-B USB connector since the standard case does not allow use of the USB port to power the emonTH. However, a community-contributed 3D printable case design, which does allow use of the USB connector, is available at: http://www.thingiverse.com/thing:365035 

## Firmware

### Libraries Needed
* JeeLib: https://github.com/jcw/jeelib (CURRENT emonTH V1.5)
* RFu_JeeLib: https://github.com/openenergymonitor/RFu_jeelib (OLD emonTH V1.4)
* Temperature control library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html
* DHT22 Sensor Library  https://github.com/adafruit/DHT-sensor-library - be sure to rename the sketch folder to remove the '-'


### emonTH Firmwarwe

**emonTH_DHT22_DS18B20_RFM69CW_Pulse**  Current main emonTH temperature and humidity sensing firmware (Nov2015). Searches for either DHT22 or DS18B20 and reads temperature and humidity once per min (by default) and tx's data back to the emonBase via RFM69CW. If both sensors are detected temperature will be sensed from DS18B20 and humidity from DHT22. Supports on-board RF nodeID setting via DIP switch selectors. Now supports optical counting sensor. See Wiki for more details http://wiki.openenergymonitor.org/index.php/EmonTH_V1.5

**User Contributed:**

* **emonTH_DHT22_DS18B20_RFM69CW** - FOR emonTH V1.5+: Searches for either DHT22 or DS18B20 and reads temperature and humidity once per min (by default) and tx's data back to the emonBase via RFM69CW. If both sensors are detected temperature will be sensed from DS18B20 and humidity from DHT22. Supports on-board node ID DIP switch selectors

* **emonTH_DHT22_DS18B20** - FOR emonTH V1.4 - emonTH temperature and humidity sensing firmware. Searches for either DHT22 or DS18B20 and reads temperature and humidity once per min (by default) and tx's data back to the emonBase via RFM12B. If both sensors are detected temperature will be sensed from DS18B20 and humidity from DHT22

* **emonTH_DHT22_dual_DS18B20** - Derived from the main emonTH firmware, but capable of monitoring two (or more) DS18B20 external sensors. You'll need to discover your sensors' addresses to make use of this script - discover them with 'emonTH temperature search' utility sketch in 'Simple emonTH Sensor Test' folder

* **emonTH_DHT22_multiple_DS18B20** - Derived from the dual sensor emonTH firmware by Marshall Scholz. Capable of automatically discovering and monitoring up to 60 connected DS18B20 sensors, one DHT22/DHT11, and one analog pin. The downfalls of this version are that it uses slightly more power than the one sensor sketch, and that the sensor order will probably change if an extra sensor is added once the node has been set up. (This can be easily rectified by changing the input logging feed in emonCMS)

* **emonTH_DHT22_DS18B20_RFM69CW_REEDSWITCH**

Low-power sketch for EmonTH V1.5 that counts pulses from a reed switch with debouncing. It aso sends the temperature/humidity every minute. By [Eirc_AMANN](https://openenergymonitor.org/emon/user/5027) March 2016
[Forum thread development](https://openenergymonitor.org/emon/node/12165)

**Simple emonTH Sensor Test** -
	* emonTH DHT22 Test
	* emonTH DS18B20 Test
	* emonTH temperature search - utility sketch for finding hardware addresses of one or more DS18B20 sensors connected to emonTH one-wire bus - The DallasTemperature library's "tester" sketch may do a better job of this


**Note:**
* Default RFM12B/RFM69CW settings: 433Mhz, network: 210, Node: 23 (node ID can be changed using [on-board DIP switches](https://wiki.openenergymonitor.org/index.php/EmonTH_V1.5#DIP_Switch_node_ID))
* Readings are converted to integer when sent over RF multiple by 0.1 in emoncms to restore reading
* As the JeeLib library sends out packets in individual bytes, 16 bit integers are split into two received values according to Arduino's "little endian" topology

## Design & related Blog posts:

- http://openenergymonitor.blogspot.com/2013/06/emonth-prototype.html
- http://openenergymonitor.blogspot.com/2013/10/emonth-update-hardware.html
- http://openenergymonitor.blogspot.com/2013/10/emonth-update-software-power.html
- http://openenergymonitor.blogspot.com/2013/10/aa-battery-considerations.html
- http://openenergymonitor.blogspot.com/2013/11/hardware-manufacture-begins-part-1.html
- http://openenergymonitor.blogspot.com/2014/01/emonth-multiple-ds18b20-sensors.html

## License

- The hardware designs (schematics and CAD files) are licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.
- The firmware is released under the GNU GPL V3 license
- The documentation is subject to GNU Free Documentation License
- The hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0. 

## Environmental & Life Cycle

We are passionate about sustainability and are aware of the embodied energy and use of resources involved in electronic manufacture. We try our best to reduce environmental impact wherever possible. We have been inspired by a few projects taking a lead in promoting and making steps towards Ethical and Sustainable Electronics, see our [blog post](http://openenergymonitor.blogspot.com/2013/08/ethical-and-sustainable-electronics.html).

- The printed circuit boards are manufactured in the UK by a manufacturer who uses lead free techniques, complies to the highest environmental industry standard and is actively investing in techniques and equipment to reduce waste and minimize environmental impact (e.g water treatment and recycling). Hot-air leveling was chosen instead of immersion gold finish to reduce environmental impact.
- Assembly is done in the UK. All components are RoHS compliant and free of conflict materials.
- Surface freight is used in preference to air shipping when ordering parts in bulk. This consumes 33 times less energy.
- We have strived to optimise electrical consumption in our hardware to be as low was possible and recommend the use of green, rechargable batteries, see [blog post](http://openenergymonitor.blogspot.com/2013/10/aa-battery-considerations.html).

## Disclaimer

OUR PRODUCTS AND ASSEMBLY KITS MAY BE USED BY EXPERIENCED, SKILLED USERS, AT THEIR OWN RISK. TO THE FULLEST EXTENT PERMISSIBLE BY THE APPLICABLE LAW, WE HEREBY DISCLAIM ANY AND ALL RESPONSIBILITY, RISK, LIABILITY AND DAMAGES ARISING OUT OF DEATH OR PERSONAL INJURY RESULTING FROM ASSEMBLY OR OPERATION OF OUR PRODUCTS.

The OpenEnergyMonitor system is sold as a development kit to empower members of the openenergymonitor community to to get involved with the OpenEnergyMonitor open-source energy monitoring development project.

Your safety is your own responsibility, including proper use of equipment and safety gear, and determining whether you have adequate skill and experience. OpenEnergyMonitor and Megni registered partnership disclaims all responsibility for any resulting damage, injury, or expense. It is your responsibility to make sure that your activities comply with applicable laws, including copyright. Always check the webpage associated with each unit before you get started. There may be important updates or corrections! All use of the instructions, kits, projects and suggestions given both by megni.co.uk, openenergymonitor.org and shop.openenergymonitor.org are to be used at your own risk. The technology (hardware , firmware and software) are constantly changing, documentation (including build guide and instructions) may not be complete or correct.

If you feel uncomfortable with assembling or using any part of the kit, simply return it to us for a full refund.
