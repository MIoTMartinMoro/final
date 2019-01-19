
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <U8g2lib.h>
#include <WiFiUdp.h>
#include "common.h"


// Update these with values suitable for your network.

const char* ssid = "MartinMoro0911";
const char* password = "MartinMoro0911";
const char* mqtt_server = "10.10.10.1";

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ 16, /* clock=*/ 5, /* data=*/ 4);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char msgSD[50];

unsigned int localPort = 8888;
WiFiUDP Udp;


void setup_wifi() {

  struct idappdata OPERACION;
  struct idappdata* RESULTADO;
  uint8_t id_pulsera = 0;
  uint8_t id_msg = 0;
  uint8_t num_pulsera = 0;
  char buffer[MAXDATASIZE + 1];

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Udp.begin(PUERTO_CLIENTE);

  memset(OPERACION.data, '\0', MAXDATASIZE - ID_HEADER_LEN);
  strcpy(OPERACION.data, "Quien soy (PULSERA)");
  Serial.println("Quien soy (PULSERA)");

  OPERACION.op = OP_WHOAMI_PULSERA;
  OPERACION.id = (uint16_t) ((PULSERA_PREF + id_pulsera) << 8) + id_msg;  // El primer byte se corresponde con el id de la mesa y el segundo con el nº de mensaje enviado
  OPERACION.len = (uint8_t)strlen(OPERACION.data);
  delay(1000);
  Udp.beginPacket(mqtt_server, PUERTO_SERVIDOR);
  Udp.write((char*) &OPERACION, ID_HEADER_LEN + OPERACION.len);
  Udp.endPacket();

  id_msg++;
  delay(100);
  memset(buffer, '\0', MAXDATASIZE + 1);

  int packetSize=0;//=Udp.parsePacket();
  while(packetSize==0){
    packetSize=Udp.parsePacket();
  }
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i =0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());


    // read the packet into packetBufffer
    Udp.read(buffer, MAXDATASIZE+1);
    delay(1000);
    RESULTADO =(struct idappdata*) &buffer;
    num_pulsera= (uint8_t) strtol(RESULTADO->data, NULL, 10);
    id_pulsera = (uint8_t) (PULSERA_PREF + num_pulsera);
    Serial.print("Num pulsera: ");
    Serial.println(num_pulsera);
    Serial.print("ID pulsera: ");
    Serial.println(id_pulsera);
    Serial.println("**************************************************");
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println(topic);

  int index = strchr(topic, '/')-topic;

  char printMsg[30];
  memcpy(printMsg, topic+index, strlen(topic));

  u8g2.begin();
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,printMsg);	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
  delay(1000);

  char msgRX[50];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msgRX[i]=((char)payload[i]);
  }

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe("restaurante/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  u8g2.begin();
  Serial.println("Starting Setup");
  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  u8g2.drawStr(0,10,"Starting Setup");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
  delay(1000);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  sprintf(msgSD, " ");

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
