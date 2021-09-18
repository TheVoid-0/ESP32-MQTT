#pragma once

#include <BlynkSimpleEsp32_BLE.h>
// #include <BLEDevice.h>
// #include <BLEServer.h>

//custom
#include "config/Bluetooth_config.h"
#include <MqttManager.h>

class BluetoothService
{

private:
    MqttManager *mqttManager;

public:
    // BluetoothService(MqttManager *mqttManager, BlynkEsp32_BLE Blynk);
    BluetoothService(MqttManager *mqttManager);
    void run();
    MqttManager *getMqttManager();
};