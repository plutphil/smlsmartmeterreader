#pragma once
#include "Arduino.h"
#include<Sensor.hpp>

const char *ssid = "SSID";
const char *password = "Wifi-Password";
const char *fallbackap_ssid = "ESP_METER";
const char *fallbackap_password = "Meter1234";
static const SensorConfig SENSOR_CONFIGS[] = {
    {.pin = 0,
     .name = "1",
     .numeric_only = false,
     .status_led_enabled = false,
     .status_led_inverted = true,
     .status_led_pin = LED_BUILTIN,
     .interval = 0}};
const uint8_t NUM_OF_SENSORS = sizeof(SENSOR_CONFIGS) / sizeof(SensorConfig);

const char* mqttServer = "";
const int mqttPort = 1883;
const char* mqttUser = "mqtt";
const char* mqttPassword = "password here";