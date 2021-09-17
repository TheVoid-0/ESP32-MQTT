#pragma once
#include <WiFi.h>
#include <PubSubClient.h>

class MqttManager
{
private:
    std::string ssid;
    std::string password;
    std::unique_ptr<WiFiClient> wifi_client;
    std::unique_ptr<PubSubClient> mqtt;
    std::string broker_address;
    int broker_port;
    std::string mqtt_id;
    std::string mqtt_token;

public:
    MqttManager();
    MqttManager(std::string ssid, std::string password);
    MqttManager(std::string ssid, std::string password, std::string broker_address, int broker_port, std::string mqtt_id, std::string mqtt_token, MQTT_CALLBACK_SIGNATURE);
    MqttManager(std::string ssid, std::string password, WiFiClient wifi_client, PubSubClient mqtt);
    void check_connection();
};