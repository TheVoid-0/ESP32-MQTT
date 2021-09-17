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

// custom
#include <WifiManager.h>
#include <MqttManager.h>
#include <I2CManager.h>
#include "config/Mqtt_config.h"
#include "config/Wifi_config.h"
#include "service/mqtt-service/MqttService.h"

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

// Sensor
Adafruit_MPU6050 mpu;
sensors_event_t accel, gyro, temperature;

I2CManager i2c_manager;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Setting up -- ESP32");

  if (!IS_EMULATOR)
    init_MPU();

  wifi_manager = new WifiManager(WIFI_SSID, WIFI_PASSWORD);

  mqtt = new MqttManager(WIFI_SSID, WIFI_PASSWORD, BROKER_MQTT, BROKER_MQTT_PORT, ID_MQTT, TOKEN_MQTT, mqtt_callback);
  mqtt->add_subscription_topic("TOPICO_TEMPERATURA");

  Wire.begin();
}

void loop()
{
  if (!IS_EMULATOR)
    i2c_manager.find_I2C_Address();

  wifi_manager->check_connection();
  get_accelerometer_and_gyroscope_data();

  char json[250];
  //Atribui para a cadeia de caracteres "json" os valores referentes a umidade e os envia para a variável do ubidots correspondente
  Serial.println("Temperatura");
  Serial.println(temperature.temperature);
  sprintf(json, "{\"%s\":{\"value\":%02.02f, \"context\":{\"temperatura\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", temperature.temperature, temperature.temperature, temperature.temperature, temperature.temperature);

  if (!mqtt->publish(TOPICO_UBIDOTS, json))
    Serial.println("Error on sending");
  else
    Serial.println("Success");

  /* keep-alive da comunicação com broker MQTT */
  mqtt->check_connection();
  delay(1000);
}

void init_MPU()
{
  if (!mpu.begin())
  {
    Serial.println("Sensor init failed");
    // while (1)
    //   yield();
  }
  Serial.println("Found a MPU-6050 sensor");
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

void get_accelerometer_and_gyroscope_data()
{
  mpu.getEvent(&accel, &gyro, &temperature);
  Serial.print("EMULADOR: ");
  Serial.println(IS_EMULATOR ? "true" : "false");
}
