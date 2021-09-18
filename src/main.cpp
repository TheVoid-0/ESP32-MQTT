#include <Arduino.h>

// MQTT
#include <WiFi.h>
#include <PubSubClient.h>

// MPU sensor
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>

// Bluetooth App
#include <BlynkSimpleEsp32_BLE.h>
BlynkEsp32_BLE Blynk(_blynkTransportBLE);
// #include <BLEDevice.h>
// #include <BLEServer.h>

// custom
#include <WifiManager.h>
#include <MqttManager.h>
#include <I2CManager.h>
#include "config/Mqtt_config.h"
#include "config/Wifi_config.h"
#include "service/sensor-service/SensorService.h"
#include "service/bluetooth-service/BluetoothService.h"

// MAIN DECLARED FUNCTIONS
void init_MPU();
void get_accelerometer_and_gyroscope_data();
void mqtt_callback(char *topic, byte *payload, unsigned int length);

// MAIN DECLARED VARIABLES
bool IS_EMULATOR = true;

// MQTT
MqttManager *mqtt;

// WIFI
WifiManager *wifi_manager;

// BLUETOOTH
BluetoothService *bluetooth_service;

// Sensor
Adafruit_MPU6050 mpu;
sensors_event_t accel, gyro, temperature;
SensorService *sensor_service;

I2CManager i2c_manager;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Setting up -- ESP32");

  pinMode(36, OUTPUT);
  pinMode(39, OUTPUT);
  pinMode(2, OUTPUT);

  if (!IS_EMULATOR)
  {
    init_MPU();
  }

  wifi_manager = new WifiManager(WIFI_SSID, WIFI_PASSWORD);

  mqtt = new MqttManager(WIFI_SSID, WIFI_PASSWORD, BROKER_MQTT, BROKER_MQTT_PORT, ID_MQTT, TOKEN_MQTT, mqtt_callback);
  mqtt->add_subscription_topic("TOPICO_TEMPERATURA");

  sensor_service = new SensorService(mqtt, IS_EMULATOR);
  bluetooth_service = new BluetoothService(mqtt);

  Wire.begin();
}

void loop()
{
  if (!IS_EMULATOR)
  {
    i2c_manager.find_I2C_Address();
  }

  wifi_manager->check_connection();
  get_accelerometer_and_gyroscope_data();

  char json[250];
  //Atribui para a cadeia de caracteres "json" os valores referentes a umidade e os envia para a variável do ubidots correspondente
  Serial.println("Temperatura");
  Serial.println(temperature.temperature);
  sprintf(json, "{\"%s\":{\"value\":%02.02f, \"context\":{\"temperatura\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", temperature.temperature, temperature.temperature, temperature.temperature, temperature.temperature);

  if (!mqtt->publish(TOPICO_UBIDOTS, json))
  {
    Serial.println("Error on sending");
  }
  else
  {
    Serial.println("Success");
  }
  char json2[250];
  sprintf(json2, "{\"%s\":{\"value\":%02.02f, \"context\":{\"aceleracao\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", accel.acceleration.x, temperature.temperature, temperature.temperature, temperature.temperature);
  mqtt->publish(TOPICO_UBIDOTS, json2);
  char json3[250];
  sprintf(json3, "{\"%s\":{\"value\":%02.02f, \"context\":{\"velocidade\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", gyro.gyro.x, temperature.temperature, temperature.temperature, temperature.temperature);
  mqtt->publish(TOPICO_UBIDOTS, json3);

  /* keep-alive da comunicação com broker MQTT */
  mqtt->check_connection();

  bluetooth_service->run();
  delay(1000);
}

BLYNK_WRITE(V1)
{
  Serial.println("Bluetooth command");
  bool mqttEnabled = (bool)param.asInt();
  Serial.println(mqttEnabled);
  mqtt->setMqttEnabled(mqttEnabled);
  // TODO: passar para o sensorService
  if (mqttEnabled)
  {
    Serial.println("Ligar led");
    digitalWrite(36, HIGH);
    digitalWrite(39, LOW);
    digitalWrite(2, HIGH);
  }
  else
  {
    Serial.println("desligar led");
    digitalWrite(36, LOW);
    digitalWrite(39, HIGH);
    digitalWrite(2, LOW);
  }
}

/* Função: função de callback 
 *          esta função é chamada toda vez que uma informação de 
 *          um dos tópicos subescritos chega)
 * Parâmetros: nenhum
 * Retorno: nenhum
 * */
void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
  String msg;

  //obtem a string do payload recebido
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }
  Serial.print("[MQTT] Mensagem recebida: ");
  Serial.println(msg);
}
