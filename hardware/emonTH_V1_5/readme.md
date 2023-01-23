## emonTH V1.5

![photo.jpg](photo.jpg)

**Temperature and humidity monitoring noded**

Arduino compatible RFM69CW wireless low power temperature and humidity battery powered wireless node.

Documentation: [http://openenergymonitor.org/emon/modules/emonTH](http://openenergymonitor.org/emon/modules/emonTH)

### Schematic

![schematic.png](schematic.png)

### Board

![board.png](board.png)

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

### Open Hardware

Hardware designs (schematics and CAD) files are licensed under the [Creative Commons Attribution-ShareAlike 3.0 Unported License](http://creativecommons.org/licenses/by-sa/3.0/) and follow the terms of the [OSHW (Open-source hardware) Statement of Principles 1.0.](http://freedomdefined.org/OSHW)
