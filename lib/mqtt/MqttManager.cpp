#include "MqttManager.h"

MqttManager::MqttManager()
{
    this->initializeClients();

    this->check_connection();
}

MqttManager::MqttManager(std::string ssid, std::string password)
{
    this->initializeClients();
    this->ssid = ssid;
    this->password = password;

    this->check_connection();
}

MqttManager::MqttManager(std::string ssid, std::string password, std::string broker_address, int broker_port, std::string mqtt_id, std::string mqtt_token, MQTT_CALLBACK_SIGNATURE, std::vector<std::string> subscribe_topics /*std::vector<std::string>()*/)
{
    this->initializeClients();
    this->ssid = ssid;
    this->password = password;
    this->broker_address = broker_address;
    this->broker_port = broker_port;
    this->mqtt_id = mqtt_id;
    this->mqtt_token = mqtt_token;

    this->mqtt->setServer(this->broker_address.c_str(), this->broker_port);
    this->mqtt->setCallback(callback);

    if (subscribe_topics.size() > 0)
    {
        this->add_subscription_topic(subscribe_topics);
    }

    this->check_connection();
}

MqttManager::MqttManager(std::string ssid, std::string password, WiFiClient wifi_client, PubSubClient mqtt)
{
    throw std::logic_error("MqttManager::MqttManager(std::string ssid, std::string password, WiFiClient wifi_client, PubSubClient mqtt) Não foi implementado!");
}

void MqttManager::initializeClients()
{
    // Para utilizar como: WifiClient *wifi_client
    // this->wifi_client = new WiFiClient();
    // this->mqtt = new PubSubClient(*this->wifi_client);

    // Para utilizar como: std::unique_ptr<WiFiClient> wifi_client;
    this->wifi_client = std::unique_ptr<WiFiClient>(new WiFiClient());
    this->mqtt = std::unique_ptr<PubSubClient>(new PubSubClient(*this->wifi_client.get()));

    // Para utilizar como WifiClient wifi_client
    //this->wifi_client = WiFiClient();
    //this->mqtt = PubSubClient(this->wifi_client);
}

void MqttManager::check_connection()
{
    while (!this->mqtt->connected())
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(this->broker_address.c_str());
        if (this->mqtt->connect(this->mqtt_id.c_str(), this->mqtt_token.c_str(), ""))
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            this->subscribe_to_topics();
        }
        else
        {
            Serial.println("Falha ao conectar no broker.");
            Serial.println("nova tentativa de conexao em 2s");
            delay(2000);
        }
    }

    this->mqtt->loop();
}

void MqttManager::subscribe_to_topics()
{
    for (auto topic : this->subscribe_topics)
    {
        this->mqtt->subscribe(topic.c_str());
    }
}

void MqttManager::add_subscription_topic(std::vector<std::string> subscribe_topics)
{
    for (std::string topic : subscribe_topics)
    {
        this->subscribe_topics.push_back(topic);
    }
}

void MqttManager::add_subscription_topic(std::string subscribe_topic)
{
    this->subscribe_topics.push_back(subscribe_topic);
}

void MqttManager::remove_subscription_topic(std::string subscribe_topic)
{
    for (auto topic = this->subscribe_topics.begin(); topic != this->subscribe_topics.end(); topic++)
    {
        if (*topic == subscribe_topic)
        {
            this->subscribe_topics.erase(topic);
        }
    }
}

void MqttManager::remove_subscritpion_topic_all()
{
    this->subscribe_topics.clear();
}

void MqttManager::publish_to_all(std::string payload)
{
    for (auto topic : this->subscribe_topics)
    {
        this->mqtt->publish(topic.c_str(), payload.c_str());
    }
}

boolean MqttManager::publish(std::string topic, std::string payload)
{
    return this->mqtt->publish(topic.c_str(), payload.c_str());
}