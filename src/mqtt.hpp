#pragma once 
#include<Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
WiFiClient espClient;
PubSubClient client(espClient);
const char* homea_watt=R"(
        {
  "device": {
    "identifiers": [
      "EnergyMeter"
    ],
    "manufacturer": "?",
    "model": "?",
    "name": "esp8266 energy meter sml reader",
    "sw_version": "1.0"
  },
  "availability_topic": "esp8266-energy-meter/energy-meter/status",
  "state_topic": "esp8266-energy-meter/energy-meter/state",
  "name": "EnergyMeter",
  "unit_of_measurement": "W",
  "value_template": "{{value_json.meter[5].value}}",
  "unique_id": "esp8266energymeter_w",
  "icon": "mdi:flash"
}
)";
const char* homea_wattstunden=R"(
{
  "device": {
    "identifiers": [
      "EnergyMeter"
    ],
    "manufacturer": "?",
    "model": "?",
    "name": "esp8266 energy meter sml reader",
    "sw_version": "1.0"
  },
  "availability_topic": "esp8266-energy-meter/energy-meter/status",
  "state_topic": "esp8266-energy-meter/energy-meter/state",
  "name": "EnergyMeter",
  "unit_of_measurement": "kWh",
  "value_template": "{{value_json.meter[2].value}}",
  "unique_id": "esp8266energymeter_kwh",
  "icon": "mdi:flash"
}
)";
const char* homea_rssi=R"(
{
  "device": {
    "identifiers": [
      "EnergyMeter"
    ],
    "manufacturer": "?",
    "model": "?",
    "name": "esp8266 energy meter sml reader",
    "sw_version": "1.0"
  },
  "availability_topic": "esp8266-energy-meter/energy-meter/status",
  "state_topic": "esp8266-energy-meter/energy-meter/state",
  "name": "EnergyMeter",
  "unit_of_measurement": "dBm",
  "value_template": "{{value_json}}",
  "unique_id": "esp8266energymeter_rssi",
  "icon": "mdi:flash"
}
)";
void mqtt_start(){
   client.beginPublish("homeassistant/sensor/esp8266-energy-meter/energy-meter-w/config",strlen(homea_watt),false);
      client.print(homea_watt);
      client.endPublish();

      client.beginPublish("homeassistant/sensor/esp8266-energy-meter/energy-meter-kwh/config",strlen(homea_wattstunden),false);
      client.print(homea_wattstunden);
      client.endPublish();

      client.beginPublish("homeassistant/sensor/esp8266-energy-meter/energy-meter-rssi/config",strlen(homea_rssi),false);
      client.print(homea_rssi);
      client.endPublish();

      client.publish("esp8266-energy-meter/energy-meter/status","online");
}
void mqtt_setup(){
  client.setServer(mqttServer, mqttPort);
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("mqtt connected"); 
      delay(1000);
      mqtt_start();
     
    }else {

      Serial.print("mqtt connnection failed with state ");
      Serial.print(client.state());
    }
}
long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect("arduinoClient")) {
    mqtt_start();
  }
  return client.connected();
}
void mqtt_loop(){
if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    client.loop();
  }
}