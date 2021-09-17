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
#include <wifi_manager.h>

// MAIN DECLARED FUNCTIONS
void find_I2C_Address();
void init_MPU();
void get_accelerometer_and_gyroscope_data();
void init_wifi();
void reconnect_wifi();
void init_mqtt();
void reconnect_mqtt();
void verifica_conexoes_wifi_mqtt();
void mqtt_callback(char *topic, byte *payload, unsigned int length);
void log();

// MAIN DECLARED VARIABLES
uint8_t addresses[2];
bool isSameAddress = true;
bool DONE_SCANNING = false;
bool IS_EMULATOR = true;

// MQTT
const char *SSID = "Visitantes";
const char *PASSWORD = "";
WiFiClient espClient;
PubSubClient MQTT(espClient);

// WIFI
WifiManager *wifi_manager;

// Sensor
Adafruit_MPU6050 mpu;
sensors_event_t accel, gyro, temperature;

/* URL do broker MQTT que deseja utilizar */
const char *BROKER_MQTT = "things.ubidots.com";
/* Porta do Broker MQTT */
int BROKER_PORT = 1883;
#define ID_MQTT "6132b1cd326cb625bb765f41"
#define TOKEN "BBFF-XnysvxYa9Hj2JeCLmEGDXm9o0l6XZp"
#define TOPICO_ACELERACAO "aceleracao"
#define TOPICO_VELOCIDADE "velocidade"
#define TOPICO_TEMPERATURA "temperatura"
#define TOPICO_UBIDOTS "/v1.6/devices/esp32marco"

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Setting up -- ESP32");
  if (!IS_EMULATOR)
    init_MPU();
  wifi_manager = new WifiManager("Pikachu", "dacasa275");
  init_mqtt();

  Wire.begin();
}

void loop()
{
  find_I2C_Address();
  wifi_manager->check_connection();
  verifica_conexoes_wifi_mqtt();
  get_accelerometer_and_gyroscope_data();

  char json[250];
  //Atribui para a cadeia de caracteres "json" os valores referentes a umidade e os envia para a variável do ubidots correspondente
  Serial.println("Temperatura");
  Serial.println(temperature.temperature);
  sprintf(json, "{\"%s\":{\"value\":%02.02f, \"context\":{\"temperatura\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", temperature.temperature, temperature.temperature, temperature.temperature, temperature.temperature);

  if (!MQTT.publish(TOPICO_UBIDOTS, json))
    return;

  /* keep-alive da comunicação com broker MQTT */
  MQTT.loop();
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

/* Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
 *          em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void reconnect_mqtt(void)
{
  while (!MQTT.connected())
  {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT, TOKEN, ""))
    {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_TEMPERATURA);
    }
    else
    {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }
  }
}

/* Função: inicializa parâmetros de conexão MQTT(endereço do  
 *         broker, porta e seta função de callback)
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void init_mqtt(void)
{
  /* informa a qual broker e porta deve ser conectado */
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  /* atribui função de callback (função chamada quando qualquer informação do 
    tópico subescrito chega) */
  MQTT.setCallback(mqtt_callback);
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

/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
 *         Em caso de desconexão (qualquer uma das duas), a conexão
 *         é refeita.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void verifica_conexoes_wifi_mqtt(void)
{
  /* se não há conexão com o Broker, a conexão é refeita */
  if (!MQTT.connected())
    reconnect_mqtt();
}

void get_accelerometer_and_gyroscope_data()
{
  mpu.getEvent(&accel, &gyro, &temperature);
  log();
}

void find_I2C_Address()
{
  if (DONE_SCANNING)
    return;
  byte error, address;
  int nDevices;

  isSameAddress = true;

  Serial.println("Scanning...");
  int foundThisRound = 0;

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");

      if (foundThisRound % 2 == 0)
      {
        if (addresses[0] != address)
        {
          Serial.println("address 0 diferente");
          addresses[0] = address;
          isSameAddress = false;
        }
      }
      else
      {
        if (addresses[1] != address)
        {
          Serial.println("address 1 diferente");
          addresses[1] = address;
          isSameAddress = false;
        }
      }
      foundThisRound++;
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
  {
    Serial.println("done\n found: ");
    Serial.println(foundThisRound);
    DONE_SCANNING = true;
  }
  delay(1000); // wait 1 seconds for next scan
}

void log()
{
  Serial.print("EMULADOR: ");
  Serial.println(IS_EMULATOR);
}
