#include "WifiManager.h"

WifiManager::WifiManager()
{
}

WifiManager::WifiManager(std::string ssid, std::string password)
{
    if (Serial)
    {
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(ssid.c_str());
    Serial.println("Aguarde");
    }
    this->ssid = ssid;
    this->password = password;
    this->check_connection();
}

void WifiManager::check_connection()
{
    /* se já está conectado a rede WI-FI, nada é feito. 
       Caso contrário, são efetuadas tentativas de conexão */
    if (WiFi.status() == WL_CONNECTED)
        return;

    WiFi.begin(this->ssid.c_str(), this->password.c_str());

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        if (Serial)
            Serial.print(".");
    }

    if (Serial)
    {
        Serial.println();
        Serial.print("Conectado com sucesso na rede ");
        Serial.print(this->ssid.c_str());
        Serial.println("IP obtido: ");
        Serial.println(WiFi.localIP());
    }
}