# HTU21D / Si72021  Temperature & Humidity Sensor

- [OctoPart](https://octopart.com/htu21d-measurement+specialties-30374934)
- [DataSheet](http://datasheet.octopart.com/HTU21D-Measurement-Specialites-datasheet-22149496.pdf)
- [Farnell](http://uk.farnell.com/measurement-specialties/htu21d/humidity-digital-3-rh-dfn-6/dp/2393536?CMP=GRHB-OCTOPART)

- DFM footprint
- Digital, pre-clibrated
- I2C
- 3.3V (min 2.1V, Max 3.6)
- Typical accuracy of ±2% RH 5%-95%
- Temperature:  ±1°C from -30~90°C.
- 2 - 15 ms measurement time (8-12bit)
- sleep current: 0.02uA
- Measurement current: 450uA = 0.045mA


| Metric              | HTU21D             |Si72021| DHT22          | DHT22 vs HTU21D Difference|
| ------------- | -------------            |------------- | ------------- | ------------- |
| Cost in 1k off      | £1.42 (July16)     |£2.15|  £4.57 (July16)    | 3.2 times cheaper (£3.15 less!) |
| Vcc                 | 2.1v - 3.6V        | 1.9V - 3.6V | | |
| Humidity accuracy   | ±2% RH             | ±2% RH |  ±2%RH         | n/a |
| Humidity Range      | 0-100% RH          | 0-100% RH |  0-100% RH     | n/a|
| Temperature accuracy | ±0.3°C            | ±0.4°C|  ±0.5°C        | 40% more accurate |
| Temperature Range   | -40°C +125°C       |  -40°C +125°C  |  -40°C +80°C   | 56% more accurate |
| Sleep Current       | 0.02uA             | 0.06uA |  15uA          | 750 times less power |
| Measurement Current | 0.045mA            | 0.09 mA |  0.5mA         | 11 times less power |
| Measurement time    | 0.01s - 0.0026s    | 0.01s - 0.0026s |  2s            | 200 times faster |
| Energy consumed per sample | 0.00045mW   ||  1mW           | 2222 times less power |
| Time sampling per day* |  14.4s          ||  2800s         | |
| Time sleeping per day* | 86386s          ||  83600s        | |
| Energy consumed per day* | 2.36mW [1]    ||  2836mW [2]    | 1201 times less energy per day!  |


## Energy consumed per day*

- **HTU21D:** (14.4s * 0.045mA) + (86386 * 0.00002mA) = 0.63mW + 1.73mW = **2.36mW** [1]
- **DHT22:**  (2800s * 0.55mA) + (86386 * 0.015mA) = 1540mW + 1295.8mW = **2836mW**  [2]

\*Assuming 1 sample per min and sleeping in between samples, 1440 min per day = 86400s per day

[Adafruit HTU21D Library](https://github.com/adafruit/Adafruit_HTU21DF_Library)
