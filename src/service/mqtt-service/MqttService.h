#pragma once

#include <MqttManager.h>
#include "config/Mqtt_config.h"
#include "config/Wifi_config.h"

class MqttService
{
private:
    std::unique_ptr<MqttManager> mqttManager = std::unique_ptr<MqttManager>(new MqttManager(WIFI_SSID, WIFI_PASSWORD, BROKER_MQTT, BROKER_MQTT_PORT, ID_MQTT, TOKEN_MQTT, mqtt_callback));

    static void mqtt_callback(char *topic, byte *payload, unsigned int length);

public:
    MqttService();
    void publish(std::string topic, std::string payload);
    /**
     * @brief keep-alive da conexão MQTT
     */
    void loop();
};

MqttService::MqttService()
{
}

void MqttService::publish(std::string topic, std::string payload)
{
    this->mqttManager->publish(topic, payload);
}

void MqttService::loop()
{
    this->mqttManager->check_connection();
}