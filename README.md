# emonTH - Wireless Temperature and Humidity monitoring node 

Part of the openenergymonitor.org project

Main emonTH page: 
xxxxx

Builds on JeeLabs, Adafruit and Miles Burton 

## Libraries Needed
* RFu_JeeLib: https://github.com/openenergymonitor/RFu_jeelib
* Temperature control library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html
* DHT22 Sensor Library  https://github.com/adafruit/DHT-sensor-library - be sure to rename the sketch folder to remove the '-'


## emonTH Firmwarwe
* **emonTH_DHT22_DS18B20** - Main emonTH temperature and humidity sensing firmware. Searches for either DHT22 or DS18B20 and reads temperature and humidity once per min (by default) and tx's data back to the emonBase via RFM12B. If both sensors are detected temperature will be sensed from DS18B20 and humidity from DHT22 

* **Simple emonTH Sensor Test** - Barebones testing sketches for both DS18B20 and DHT22 sensors

**Note:**
* Default RFM12B settings: 433Mhz, network: 210, Node: 19 
* Readings are converted to integer when sent over RF multiple by 0.1 in emoncms to restore reading

# License
The emonTH hardware designs (schematics and CAD files hosted on http://solderpad.com/openenergymon) are licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

The emonTH firmware is released under the GNU GPL V3 license

The documentation is subject to GNU Free Documentation License 

The emonTH hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0.






 
