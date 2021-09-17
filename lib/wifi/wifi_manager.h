#pragma once
#include <WiFi.h>

class WifiManager
{
private:
    std::string ssid;
    std::string password;

public:
    WifiManager();
    WifiManager(std::string ssid, std::string password);
    void check_connection();
};