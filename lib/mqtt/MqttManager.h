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
    std::vector<std::string> subscribe_topics;
    bool mqttEnabled = true;

    void initializeClients();
    void subscribe_to_topics();

public:
    MqttManager();
    MqttManager(std::string ssid, std::string password);
    MqttManager(std::string ssid, std::string password, std::string broker_address, int broker_port, std::string mqtt_id, std::string mqtt_token, MQTT_CALLBACK_SIGNATURE, std::vector<std::string> subscribe_topics = std::vector<std::string>());
    MqttManager(std::string ssid, std::string password, WiFiClient wifi_client, PubSubClient mqtt);
    void check_connection();

    // MqttTopics
    void add_subscription_topic(std::string subscribe_topic);
    void add_subscription_topic(std::vector<std::string> subscribe_topics);
    void remove_subscription_topic(std::string subscribe_topic);
    void remove_subscritpion_topic_all();
    void publish_to_all(std::string payload);
    boolean publish(std::string topic, std::string payload);
    void setMqttEnabled(bool mqttEnabled);
};