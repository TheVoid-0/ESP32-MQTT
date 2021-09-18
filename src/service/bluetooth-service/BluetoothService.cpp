#include "BluetoothService.h"

BluetoothService::BluetoothService(MqttManager *mqttManager)
{
    Blynk.setDeviceName("ESP32-Marco");
    Blynk.begin(BLUETOOTH_TOKEN);
    this->mqttManager = mqttManager;
}

void BluetoothService::run()
{
    Blynk.run();
}

MqttManager *BluetoothService::getMqttManager()
{
    return this->mqttManager;
}