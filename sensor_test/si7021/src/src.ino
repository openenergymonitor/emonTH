#include <Wire.h>
#include <SI7021.h>

SI7021 sensor;

void setup() {
    sensor.begin();
    Serial.begin(9600);
    Serial.println("Start Si7201 test");
    // this driver should work for SI7020 and SI7021, this returns 20 or 21
    int deviceid = sensor.getDeviceId();
    Serial.print("ID: "); Serial.println(deviceid);
}

void loop() {

    // temperature is an integer in hundredths
    float temperature = sensor.getCelsiusHundredths();
    temperature = temperature / 100.0;
    Serial.print("temp: "); Serial.println(temperature);


    // humidity is an integer representing percent
    float humidity = sensor.getHumidityPercent();
    Serial.print("humidity %: "); Serial.println(humidity);

    // enable internal heater for testing
    sensor.setHeater(true);
    delay(5000);
    sensor.setHeater(false);

    // see if heater changed temperature
    temperature = sensor.getCelsiusHundredths();
    temperature = temperature / 100.0;
    Serial.print("temp after heating: "); Serial.println(temperature);

    // get humidity and temperature in one shot, saves power because sensor takes temperature when doing humidity anyway
    si7021_env data = sensor.getHumidityAndTemperature();
      Serial.print("temp fast: "); Serial.println(data.celsiusHundredths/100.0);
      Serial.print("humidity fast: "); Serial.println(data.humidityBasisPoints/100.0);
    delay(5000);
}
