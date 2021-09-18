#pragma once

// MPU sensor
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>

// custom
#include <MqttManager.h>
#include "config/Mqtt_config.h"

class SensorService
{
private:
    MqttManager *mqttManager;
    Adafruit_MPU6050 mpu;
    sensors_event_t accel, gyro, temperature;
    bool is_emulator;
    void get_accelerometer_and_gyroscope_data();

public:
    SensorService(MqttManager *mqttManager, bool is_emulator);
    void publishData();
};

SensorService::SensorService(MqttManager *mqttManager, bool is_emulator)
{
    if (!is_emulator)
    {
        this->mpu = Adafruit_MPU6050();
        if (!mpu.begin())
        {
            Serial.println("Sensor init failed");
        }
        Serial.println("Found a MPU-6050 sensor");
    }
    this->mqttManager = mqttManager;
    this->is_emulator = is_emulator;
}

void SensorService::get_accelerometer_and_gyroscope_data()
{
    this->mpu.getEvent(&this->accel, &this->gyro, &this->temperature);
    Serial.print("EMULADOR: ");
    Serial.println(this->is_emulator ? "true" : "false");
}

void SensorService::publishData()
{
    this->get_accelerometer_and_gyroscope_data();
    char json[250];
    //Atribui para a cadeia de caracteres "json" os valores referentes a umidade e os envia para a variável do ubidots correspondente
    Serial.println("Temperatura");
    Serial.println(temperature.temperature);
    sprintf(json, "{\"%s\":{\"value\":%02.02f, \"context\":{\"temperatura\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", temperature.temperature, temperature.temperature, temperature.temperature, temperature.temperature);

    if (!this->mqttManager->publish(TOPICO_UBIDOTS, json))
    {
        Serial.println("Error on sending");
    }
    else
    {
        Serial.println("Success");
    }
    char json2[250];
    sprintf(json2, "{\"%s\":{\"value\":%02.02f, \"context\":{\"aceleracao\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", accel.acceleration.x, temperature.temperature, temperature.temperature, temperature.temperature);
    this->mqttManager->publish(TOPICO_UBIDOTS, json2);
    char json3[250];
    sprintf(json3, "{\"%s\":{\"value\":%02.02f, \"context\":{\"velocidade\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", gyro.gyro.x, temperature.temperature, temperature.temperature, temperature.temperature);
    this->mqttManager->publish(TOPICO_UBIDOTS, json3);
}