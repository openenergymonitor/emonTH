# openEMC - Wireless Estimated Moisture Content, Temperature, and Humidity monitoring node

Read more about this project [here](http://openenergymonitor.org/emon/node/10899), including what EMC is, and why monitoring it is vital to getting the most out of your shop.

This software is based on the openenergymonitor.org emonTH project.

Main emonTH page: http://openenergymonitor.org/emon/modules/emonTH

Technical Hardware Wiki: http://wiki.openenergymonitor.org/index.php?title=EmonTH 

Schematic + CAD: http://solderpad.com/git/openenergymon/emonth

Design & related Blog posts: 
http://openenergymonitor.blogspot.com/2013/06/emonth-prototype.html
http://openenergymonitor.blogspot.com/2013/10/emonth-update-hardware.html
http://openenergymonitor.blogspot.com/2013/10/emonth-update-software-power.html
http://openenergymonitor.blogspot.com/2013/10/aa-battery-considerations.html
http://openenergymonitor.blogspot.com/2013/11/hardware-manufacture-begins-part-1.html
http://openenergymonitor.blogspot.com/2014/01/emonth-multiple-ds18b20-sensors.html

Builds on JeeLabs, Adafruit and Miles Burton 

## Libraries Needed
* RFu_JeeLib: https://github.com/openenergymonitor/RFu_jeelib
* Temperature control library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html
* DHT22 Sensor Library  https://github.com/adafruit/DHT-sensor-library - be sure to rename the sketch folder to remove the '-'


## openEMC Firmware
* **openEMC_DHT22_DS18B20_RFM69CW** - Main EMC, temperature and humidity sensing firmware. Searches for either DHT22 or DS18B20 and reads temperature and humidity once per min (by default), calculates Equilibrium Moisture Content, and tx's data back to the emonBase via RFM69CW. If both sensors are detected temperature will be sensed from DS18B20 and humidity from DHT22 

# License
The emonTH hardware designs (schematics and CAD files hosted on http://solderpad.com/openenergymon) are licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

The emonTH firmware is released under the GNU GPL V3 license

The documentation is subject to GNU Free Documentation License 

The emonTH hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0.






 
