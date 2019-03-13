#include <Arduino.h>
#include <Ultrasonic.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define SSID "sua_rede_wifi"
#define WIFI_PASSWORD "sua_senha_wifi"

//Trocar os valores de acordo com seu reservatório
#define DT 75 //Distancia(cm) entre o sensor e o fundo do reservatório
#define DS 5 //Distancia(cm) entre o sensor e o nível máximo em que a água pode chegar;

#define ECHO_PIN 5 //D1
#define TRIG_PIN 4 //D2

#define TOKEN "..."

float distancia_real;

int nivel_atual;

Ultrasonic ultrasonic(TRIG_PIN, ECHO_PIN);

char thingsboardServer[] = "demo.thingsboard.io";

WiFiClient wifiClient;

unsigned long lastSend;

PubSubClient client(wifiClient);

void readSensorUltrasonic() {
  distancia_real = DT - ultrasonic.read();
  nivel_atual = distancia_real / (DT - DS) * 100;
  Serial.println(nivel_atual);
  delay(1000);
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
}

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  setupWiFi();
  mqttConnect();
}

void loop() {
  readSensorUltrasonic();
  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 1000 ) {
    readSensorUltrasonic();
    publishData();
    lastSend = millis();
  }

  client.loop();

}
