#pragma once

// MPU sensor
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>

// custom
#include <MqttManager.h>

class SensorService
{
private:
    MqttManager *mqttManager;
    Adafruit_MPU6050 mpu;
    sensors_event_t accel, gyro, temperature;

public:
    SensorService(MqttManager *mqttManager);
};

SensorService::SensorService(MqttManager *mqttManager)
{
    this->mqttManager = mqttManager;
    this->mpu = Adafruit_MPU6050();

    if (!mpu.begin())
    {
        Serial.println("Sensor init failed");
    }
    Serial.println("Found a MPU-6050 sensor");
}