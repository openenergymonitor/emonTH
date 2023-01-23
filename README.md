# emonTH - Wireless Temperature and Humidity monitoring node

**This repo is for emonTH V1.x**, See [emonTH2 repo](https://github.com/openenergymonitor/emonth2) for latest emonTH V2.x

---

## [Overview](docs/overview.md)

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

**Enclosure: 71 x 71 x 27 mm**<br>
By default, the emonTH is shipped without an SMT mini-B USB connector since the standard case does not allow use of the USB port to power the emonTH. However, a community-contributed 3D printable case design, which does allow use of the USB connector, is available at: http://www.thingiverse.com/thing:365035 

## Firmware

### Libraries Needed
* JeeLib: https://github.com/jcw/jeelib (CURRENT emonTH V1.5)
* RFu_JeeLib: https://github.com/openenergymonitor/RFu_jeelib (OLD emonTH V1.4)
* Temperature control library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html
* DHT22 Sensor Library  https://github.com/adafruit/DHT-sensor-library - be sure to rename the sketch folder to remove the '-'

### emonTH Firmwarwe

**emonTH_DHT22_DS18B20_RFM69CW_Pulse**  Current main emonTH temperature and humidity sensing firmware (Nov2015). Searches for either DHT22 or DS18B20 and reads temperature and humidity once per min (by default) and tx's data back to the emonBase via RFM69CW. If both sensors are detected temperature will be sensed from DS18B20 and humidity from DHT22. Supports on-board RF nodeID setting via DIP switch selectors. Now supports optical counting sensor.

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
* Default RFM12B/RFM69CW settings: 433Mhz, network: 210, Node: 23 (node ID can be changed using on-board DIP switches.
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
