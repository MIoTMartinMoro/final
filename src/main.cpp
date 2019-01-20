
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

uint8_t num_pulsera = 0;
uint8_t id_pulsera = 0;
char numPusRX[5];
char numMesaRX[5];
char *misMesas[50];
int numMisMesas=0;

struct idappdata DATOS;


void setup_wifi() {

  struct idappdata OPERACION;
  struct idappdata* RESULTADO;
  uint8_t id_msg = 0;

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
    DATOS.id=id_pulsera;
    DATOS.op=num_pulsera;
    Serial.print("Num pulsera: ");
    Serial.println(DATOS.op);
    Serial.print("ID pulsera: ");
    Serial.println(DATOS.id);
    Serial.println("**************************************************");
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  char msgRX[50];
  Serial.print("Message arrived from topic [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msgRX[i]=((char)payload[i]);
  }
  Serial.println(msgRX);

  int asigIndex = (int)(strstr(topic, "asignada")-topic);
  int mesaRXindex = (int)(strstr(topic, "mesa")-topic);
  int vaciada = (int)(strstr(topic, "vaciada")-topic);

  if(mesaRXindex>0){
    memcpy(numMesaRX, topic+mesaRXindex+strlen("mesa")+1, 2);
    Serial.print("Numero mesa recibido: ");
    Serial.println(numMesaRX);
  }

  if(asigIndex>0){
    memcpy(numPusRX, topic+asigIndex+strlen("asignada")+1, strlen(topic));
    Serial.print("Numero pulsera recibido: ");
    Serial.println(numPusRX);
    char num_pulseraChar[5]={'0','0','0','0','0'};
    itoa(DATOS.op, num_pulseraChar, 10);
    Serial.println(DATOS.op);
    //sprintf(num_pulseraChar, "%03d", num_pulsera);
    Serial.print("Mi pulsera: ");
    Serial.println(num_pulseraChar);
    Serial.print("Pulsera recibida en topic: ");
    Serial.println(numPusRX);
    if(strcmp(numPusRX,num_pulseraChar)==0){//añade el número de la mesa a las de la pulsera
      misMesas[numMisMesas]=numMesaRX;
      Serial.print("Mesa añadida a esta pulsera: ");
      Serial.println(misMesas[numMisMesas]);
      numMisMesas++;
    }
  } else if(vaciada>0){
    Serial.print("Numero mesa vaciada: ");
    Serial.println(numMesaRX);
    for(int i=0; i<numMisMesas; i++){
      if(strcmp(misMesas[i],numMesaRX)==0){
        misMesas[i]="0";                  //limpiar el array!!!!!!!!!!!!!!!
      }
    }
  } else{

    for(int i=0; i<numMisMesas; i++){
      Serial.print("misMesas[] : ");
      Serial.println(misMesas[i]);
      Serial.print("numMesaRX : ");
      Serial.println(numMesaRX);
      if(strcmp(misMesas[i],numMesaRX)==0){
        int index = strchr(topic, '/')-topic;
        char printMsg[30];
        memcpy(printMsg, topic+index, strlen(topic));

        u8g2.begin();
        u8g2.clearBuffer();					// clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        u8g2.drawStr(0,10,printMsg);	// write something to the internal memory
        u8g2.sendBuffer();					// transfer internal memory to the display
        delay(1000);
      }
    }
}
  Serial.println("---------------------------------------------");
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
