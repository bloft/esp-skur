#include <EasyMqtt.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <Adafruit_BMP085.h>
#include <Bounce2.h>
#include "config.h"

Bounce doorDebouncer = Bounce();
Bounce lockDebouncer = Bounce();

OneWire oneWire(D5); 
DallasTemperature sensors(&oneWire);

Adafruit_BMP085 bmp = Adafruit_BMP085();
const float ALTITUDE = 32;

EasyMqtt mqtt;

void setup() {
  Serial.begin(115200);

  mqtt.wifi(wifi_ssid, wifi_pass);
  mqtt.mqtt(mqtt_server, mqtt_port, mqtt_user, mqtt_password);


  pinMode(D3, INPUT_PULLUP);
  doorDebouncer.attach(D3);
  doorDebouncer.interval(5); // interval in ms
 
  mqtt["door"].setInterval(1, 60);
  mqtt["door"] << [](){ return doorDebouncer.read() == LOW ? String("CLOSED") : String("OPEN"); };


  pinMode(D7, INPUT_PULLUP);
  lockDebouncer.attach(D7);
  lockDebouncer.interval(5); // interval in ms

  mqtt["door"]["lock"].setInterval(1, 60);
  mqtt["door"]["lock"] << [](){ return lockDebouncer.read() == LOW ? String("CLOSED") : String("OPEN"); };


  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }

  mqtt["pressure"] << [](){
    return String((bmp.readSealevelPressure(ALTITUDE) / 100.0));
  };

  mqtt["temperature"] << [](){
    return String(bmp.readTemperature());
  };


  sensors.begin();

  mqtt["outdoor"]["temperature"] << []() {
    sensors.requestTemperatures();
    float value = sensors.getTempCByIndex(0);
    mqtt.debug("Value", String(value));
    if(value > -20 && value < 60) {
      return String(value);
    } else {
      return String("");
    }
  };
}

void loop() {
  doorDebouncer.update();
  lockDebouncer.update();
  mqtt.loop();
}
