#include "mqtt_manager.h"

MqttManager::MqttManager()
{
    // Para utilizar como: WifiClient *wifi_client
    // this->wifi_client = new WiFiClient();
    // this->mqtt = new PubSubClient(*this->wifi_client);

    // Para utilizar como: std::unique_ptr<WiFiClient> wifi_client;
    this->wifi_client = std::unique_ptr<WiFiClient>(new WiFiClient());
    this->mqtt = std::unique_ptr<PubSubClient>(new PubSubClient(*this->wifi_client.get()));
}

MqttManager::MqttManager(std::string ssid, std::string password)
{
    this->wifi_client = std::unique_ptr<WiFiClient>(new WiFiClient());
    this->mqtt = std::unique_ptr<PubSubClient>(new PubSubClient(*this->wifi_client.get()));
    this->ssid = ssid;
    this->password = password;
}

MqttManager::MqttManager(std::string ssid, std::string password, std::string broker_address, int broker_port, std::string mqtt_id, std::string mqtt_token, MQTT_CALLBACK_SIGNATURE)
{
    this->wifi_client = std::unique_ptr<WiFiClient>(new WiFiClient());
    this->mqtt = std::unique_ptr<PubSubClient>(new PubSubClient(*this->wifi_client.get()));
    this->ssid = ssid;
    this->password = password;
    this->broker_address = broker_address;
    this->broker_port = broker_port;
    this->mqtt_id = mqtt_id;
    this->mqtt_token = mqtt_token;

    this->mqtt->setServer(this->broker_address.c_str(), this->broker_port);
    this->mqtt->setCallback(callback);
}

MqttManager::MqttManager(std::string ssid, std::string password, WiFiClient wifi_client, PubSubClient mqtt)
{
    throw std::logic_error("MqttManager::MqttManager(std::string ssid, std::string password, WiFiClient wifi_client, PubSubClient mqtt) Não foi implementado!");
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
            // this->mqtt->subscribe(TOPICO_TEMPERATURA);
        }
        else
        {
            Serial.println("Falha ao conectar no broker.");
            Serial.println("nova tentativa de conexao em 2s");
            delay(2000);
        }
    }
}