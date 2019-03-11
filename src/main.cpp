#include <Arduino.h>
#include <Ultrasonic.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define SSID "sua_rede_wifi"
#define WIFI_PASSWORD "sua_senha_wifi"

// Favor trocar os valores de acordo com seu reservatorio
#define DT 75 //Distancia(cm) entre o sensor e o fundo do reservatório
#define DS 5 //Distancia(cm) entre o sensor e o nível máximo em que a água pode chegar;

#define ECHO_PIN 4
#define TRIG_PIN 5

#define TOKEN ""

int distancia_atual = 0;

int nivel_atual = 0;

Ultrasonic ultrasonic(TRIG_PIN, ECHO_PIN);

char thingsboardServer[] = "demo.thingsboard.io";

WiFiClient wifiClient;

unsigned long lastSend;

PubSubClient client(wifiClient);

void readSensorUltrasonic() {
  distancia_atual = ultrasonic.read();
  nivel_atual = ((DT - DS - distancia_atual) / (DT - DS)) * 100;
}

void reconnect() {
  while (!client.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(SSID, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if ( client.connect("ESP8266", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      delay( 5000 );
    }
  }
}

void setupWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(SSID);
  WiFi.begin(SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void mqttConnect() {
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;
}

void publishData() {

  String payload = "{";
  payload += "\"nivel_atual\":"; payload += nivel_atual;
  payload += "}";

  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  setupWiFi();
  mqttConnect();
}

void loop() {

  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) {
    readSensorUltrasonic();
    while (nivel_atual > DT) {
      readSensorUltrasonic();
    }
    publishData();
    lastSend = millis();
  }

  client.loop();

}
