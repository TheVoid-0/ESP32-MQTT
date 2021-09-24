#include <Arduino.h>

// MQTT
#include <WiFi.h>
#include <PubSubClient.h>

// MPU sensor
#include <Wire.h>
#include <SPI.h>
#include "config/Mqtt_config.h"
#include "config/Wifi_config.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>

// Bluetooth App
/**
 * OBS: A biblioteca BlynkSimpleEsp32_BLE.h Possui variáveis erroneamente declaradas no arquivo de HEADER (*.h)
 * isso causa erros ao incluir a lib em vários arquivos, o que é necessário para incluir seus tipos e tornar seus
 * simbolos acessíveis, para resolver isso, é necessário retirar a instanciação do arquivo .h, expondo a variável
 * com 'extern' e então declarar ela onde deseja usar
 * EX: 
        static BlynkTransportEsp32_BLE _blynkTransportBLE;
        extern BlynkEsp32_BLE Blynk; <- Alterado essa linha adicionando o 'extern' e retirando a construção (_blynkTransportBLE)
 * TODO: Verificar para adicionar uma biblioteca personalizada que inclui a BlynkSimpleEsp32_BLE.h e define suas variáveis em um .cpp
 separado para então incluir nos arquivos do projeto a lib personalizada
 */
#include <BlynkSimpleEsp32_BLE.h>
BlynkEsp32_BLE Blynk(_blynkTransportBLE); // Construindo a variável no .cpp já que a expomos usando o extern no .h da lib
// #include <BLEDevice.h>
// #include <BLEServer.h>

// custom
#include <WifiManager.h>
#include <MqttManager.h>
#include <I2CManager.h>
#include "service/sensor-service/SensorService.h"
#include "service/bluetooth-service/BluetoothService.h"

// MAIN DECLARED FUNCTIONS
void get_accelerometer_and_gyroscope_data();
void mqtt_callback(char *topic, byte *payload, unsigned int length);
void core1_task(void *parameters);
void core2_task(void *parameters);

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

  wifi_manager = new WifiManager(WIFI_SSID, WIFI_PASSWORD);

  mqtt = new MqttManager(WIFI_SSID, WIFI_PASSWORD, BROKER_MQTT, BROKER_MQTT_PORT, ID_MQTT, TOKEN_MQTT, mqtt_callback);
  mqtt->add_subscription_topic("TOPICO_TEMPERATURA");

  sensor_service = new SensorService(mqtt, IS_EMULATOR);
  bluetooth_service = new BluetoothService(mqtt);

  Wire.begin();
  Serial.println("Creating tasks");
  xTaskCreatePinnedToCore(
      core1_task,   /* Function to implement the task */
      "core1_task", /* Name of the task */
      10000,        /* Stack size in words */
      NULL,         /* Task input parameter */
      0,            /* Priority of the task */
      NULL,         /* Task handle. */
      0);           /* Core where the task should run */

  xTaskCreatePinnedToCore(
      core2_task,   /* Function to implement the task */
      "core2_task", /* Name of the task */
      10000,        /* Stack size in words */
      NULL,         /* Task input parameter */
      2,            /* Priority of the task precisa de uma prioridade maior que 0 para disputar com o loop do arduino*/
      NULL,         /* Task handle. */
      1);           /* Core where the task should run */

  Serial.println("Done!");
}

void loop()
{
  Serial.print("Loop no core:"); // API do arduino maldita roda esse loop independentemente de ser util ou não
  Serial.println(xPortGetCoreID());
  vTaskDelete(NULL);  // Adeus loop do arduino :)
}

void core1_task(void *parameters)
{
  for (;;)
  {
    bluetooth_service->run();
    vTaskDelay(10); // Deixa o watchdog do freeRTOS fazer o necessário nesse core, caso contrário o loop inifinito bloqueará a execução no núcleo e causará um erro
  }
}
void core2_task(void *parameters)
{
  for (;;)
  {
    Serial.println("core2_task");
    if (!IS_EMULATOR)
    {
      i2c_manager.find_I2C_Address();
    }

    wifi_manager->check_connection();

    sensor_service->publishData();

    /* keep-alive da comunicação com broker MQTT */
    mqtt->check_connection();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
/**
 * Função chamada pelo App BLYNK ao apertar o botão designado como V1
 */
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
