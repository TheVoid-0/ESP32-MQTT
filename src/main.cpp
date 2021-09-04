#include <Arduino.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
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
//Endereco I2C do MPU6050 (giroscopio e acelerometro)
const int MPU = 0x68;
uint8_t addresses[2];
bool isSameAddress = true;
bool DONE_SCANNING = false;
const char *SSID = "Visitantes";
const char *PASSWORD = "";
WiFiClient espClient;
PubSubClient MQTT(espClient);

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

// DATA STRUCTURES
struct GyroAccelData
{
  char tipo = 'T';
  float acelerometroX, acelerometroY, acelerometroZ;
  float temperatura;
  float giroscopioX, giroscopioY, giroscopioZ;
};

GyroAccelData gyroAccelData;

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("Setting up -- ESP32");
  init_MPU();
  init_wifi();
  init_mqtt();

  Wire.begin();
}

void loop()
{
  find_I2C_Address();
  verifica_conexoes_wifi_mqtt();
  get_accelerometer_and_gyroscope_data();

  char json[250];
  //Atribui para a cadeia de caracteres "json" os valores referentes a umidade e os envia para a variável do ubidots correspondente
  Serial.println("Temperatura");
  Serial.println(gyroAccelData.temperatura);
  sprintf(json, "{\"%s\":{\"value\":%02.02f, \"context\":{\"temperatura\":%02.02f, \"aceleracao\": %02.02f, \"velocidade\": %02.02f}}}", "temperatura", gyroAccelData.temperatura, gyroAccelData.temperatura, gyroAccelData.temperatura, gyroAccelData.temperatura);

  if (!MQTT.publish(TOPICO_UBIDOTS, json))
    return;

  /* keep-alive da comunicação com broker MQTT */
  MQTT.loop();
  /* Agurda 1 segundo para próximo envio */
  delay(1000);
}

void init_MPU()
{
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  //Inicializa o MPU-6050
  Wire.write(0);
  Wire.endTransmission(true);
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

void init_wifi(void)
{
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");
  reconnect_wifi();
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

/* Função: reconecta-se ao WiFi
 * Parâmetros: nenhum
 * Retorno: nenhum
*/
void reconnect_wifi()
{
  /* se já está conectado a rede WI-FI, nada é feito. 
       Caso contrário, são efetuadas tentativas de conexão */
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

/* Função: verifica o estado das conexões WiFI e ao broker MQTT. 
 *         Em caso de desconexão (qualquer uma das duas), a conexão
 *         é refeita.
 * Parâmetros: nenhum
 * Retorno: nenhum
 */
void verifica_conexoes_wifi_mqtt(void)
{
  /* se não há conexão com o WiFI, a conexão é refeita */
  reconnect_wifi();
  /* se não há conexão com o Broker, a conexão é refeita */
  if (!MQTT.connected())
    reconnect_mqtt();
}

void get_accelerometer_and_gyroscope_data()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  //Solicita os dados do sensor
  Wire.requestFrom(MPU, 14, true);
  //Armazena o valor dos sensores nas variaveis correspondentes
  gyroAccelData.acelerometroX = (Wire.read() << 8 | Wire.read()) / (16384 * 9.80); //0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  gyroAccelData.acelerometroY = (Wire.read() << 8 | Wire.read()) / (16384 * 9.80); //0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  gyroAccelData.acelerometroZ = (Wire.read() << 8 | Wire.read()) / (16384 * 9.80); //0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  gyroAccelData.temperatura = (Wire.read() << 8 | Wire.read()) / (16384 * 9.80);   //0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  gyroAccelData.giroscopioX = (Wire.read() << 8 | Wire.read()) / (16384 * 9.80);   //0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  gyroAccelData.giroscopioY = (Wire.read() << 8 | Wire.read()) / (16384 * 9.80);   //0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  gyroAccelData.giroscopioZ = (Wire.read() << 8 | Wire.read()) / (16384 * 9.80);   //0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  log();
  gyroAccelData.temperatura = gyroAccelData.temperatura / 340.00 + 36.53;
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
  delay(1000); // wait 5 seconds for next scan
}

void log()
{
  Serial.println("Slave sending");
  Serial.println("struct GyroAccelData");
  Serial.print("AX:");
  Serial.println(gyroAccelData.acelerometroX);
  Serial.print("AY:");
  Serial.println(gyroAccelData.acelerometroY);
  Serial.print("AZ:");
  Serial.println(gyroAccelData.acelerometroZ);
  Serial.print("GX:");
  Serial.println(gyroAccelData.giroscopioX);
  Serial.print("GY:");
  Serial.println(gyroAccelData.giroscopioY);
  Serial.print("GZ:");
  Serial.println(gyroAccelData.giroscopioZ);
  Serial.print("T:");
  Serial.println(gyroAccelData.temperatura);
}
