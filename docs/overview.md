# Overview

The emonTH is an open-source, battery powered, Temperature and Humidity monitoring wireless node.

It's been designed to be an easy to deploy tool for monitoring building / room temperature and humidity.

The data from the emonTH is transmitted via wireless (433/868MHz) to an emonBase web-connected base-station (we recommend a Raspberry Pi with an RFM69Pi) which then posts the data to an emonCMS server (e.g. http://emoncms.org) for logging, processing and graphing. The temperature and humidity data can be used to inform a heating control system, feed into a building performance model or simply for general interest. 

## Features

- Temperature and Humidity sensing options - Using DHT22 temperature and humidity sensor, or if humidity data is not required a DS18B20 temperature sensor. Both DHT22 and DS18B20 can be used together as shown above for internal and external readings.
- Easy to set-up - the unit comes pre-assembled and pre-loaded with Arduino compatible firmware. If desired, the code is easily changed via the Arduino IDE and a USB to UART cable.
- Long Battery Life - The emonTH is powered by two AA batteries through a high efficiency DC-DC boost converter circuit. Taking a reading once every 60s, the emonTH batteries should last for 1-3 years. We recommend rechargeable alkaline batteries for best performance and environmental impact (see blog post).
- Expansion Options - If desired, the emonTH function can easily be expanded: remote DS18B20 temperature sensors can be attached to the terminal block for outdoor temperature monitoring. Multiple DS18B20 temperature sensors can be connected at once on a digital one-wire bus.
  - An optical sensor can be added for interfacing with a pulse-output utility meter or a relay board could be connected for controlling an appliance.
  - Update: the emonTH now supports multiple DS18B20's. See blog post
- New v1.5: Node ID select DIP switch: Select from four unique node ID's via on-board DIP switch

## Technical

- Microcontroller: ATmega328 @ 3.3V
- Sensors: DHT22 (temperature & Humidity) / DS18B20 (temperature) sensor options
- Power: 2 x AA batteries in an on-board holder. LTC3525 3.3V DC-DC boost converter to extend battery life.
- RF Radio: RFM69CW (RFM12B can also be used)
- Battery life: 1-3 years expected. See blog post
- On-board LTC3525-3.3 DC-DC boost converter. See emonTH hardware blog post

## Accuracy

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
- [Independent sensor test report](http://www.kandrsmith.org/RJS/Misc/calib_dht22.html)

**DS18B20 Temperature Sensor**

- Power supply range: 3.0V to 5.5V
- Accuracy over the range of -10°C to +85°C: ±0.5°C.
- Storage temperature range:-55°C to +125°C (-67°F to +257°F)

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
