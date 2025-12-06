#include "secrets.h"                // Certificats i configuració AWS
#include <WiFiClientSecure.h>       // Connexió segura (TLS)
#include <MQTTClient.h>             // Client MQTT per AWS
#include <ArduinoJson.h>            // Per a enviar dades en format JSON
#define LED_PIN 32

// Nom del dispositiu (ha de coincidir amb el que tens a AWS IoT)
#define THINGNAME "ESP32"

// Temes MQTT (topics) per publicar i subscriure
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"

// Clients per la connexió segura i MQTT
WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);


// ----------------------------------------------------
// Configuració inicial de connexió amb AWS
// ----------------------------------------------------
void SetupAWS() {
  Serial.println("[AWS] Inicialitzant connexió amb AWS IoT...");

  // Carregar certificats i claus
  net.setCACert(AWS_CERT_CA);          // Certificat Root CA d'Amazon
  net.setCertificate(AWS_CERT_CRT);    // Certificat del dispositiu
  net.setPrivateKey(AWS_CERT_PRIVATE); // Clau privada del dispositiu

  // Connectar el client MQTT amb el servidor AWS
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  client.onMessage(CallbackAWS);

  // Intentem connectar
  Serial.println("[AWS] Connectant amb el broker MQTT...");
  if (client.connect(THINGNAME)) {
    Serial.println("[AWS] Connexió amb AWS IoT establerta correctament!");
    AWSConnected = true;
    // Ens subscrivim al topic per si AWS ens envia missatges
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  } else {
    Serial.print("[AWS] Error connectant amb AWS IoT. Codi: ");
    Serial.println(client.lastError());
    AWSConnected = false;
  }
}

// ----------------------------------------------------
// Revisar i mantenir connexió amb AWS IoT
// ---------------------------------------------------- 
void CheckAWS() {
  if (!client.connected()) {
    Serial.println("[AWS] Connexió amb AWS perduda. Reintentant...");

    // Reintentem la connexió
    if (client.connect(THINGNAME)) {
      Serial.println("[AWS] Reconnectat correctament a AWS IoT!");
      AWSConnected = true;
      client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
    } else {
      Serial.println("[AWS] No s'ha pogut reconnectar a AWS IoT.");
      AWSConnected = false;
    }
  } else {
    // Si està connectat, mantenim viva la connexió MQTT
    client.loop();
    AWSConnected = true;
  }
}

// ----------------------------------------------------
// Publicar un UID (tag) al topic d’AWS
// ----------------------------------------------------
void PublicaTag(String tagID) {
  if (!AWSConnected) {
    Serial.println("[AWS] No connectat, no es pot publicar el missatge.");
    return;
  }

  Serial.println("[AWS] Publicant tag a AWS IoT...");

  // Crea un JSON senzill amb el tag
  String jsonPayload = "{\"tag\": \"" + tagID + "\"}";

  if (client.publish(AWS_IOT_PUBLISH_TOPIC, jsonPayload.c_str())) {
    Serial.print("[AWS] Publicat correctament -> ");
    Serial.println(jsonPayload);
  } else {
    Serial.println("[AWS] Error al publicar el missatge.");
  }
}

void SubscripcioAWS() {
  if (!AWSConnected) {
    Serial.println("[AWS] No connectat, no es pot subscriure al topic.");
    return;
  }

  Serial.println("[AWS] Subscripció al topic AWS IoT...");

  if (client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC)) {
    Serial.print("[AWS] Subscrit correctament a -> ");
    Serial.println(AWS_IOT_SUBSCRIBE_TOPIC);
  } else {
    Serial.println("[AWS] Error en la subscripció.");
  }
}

void CallbackAWS(String &topic, String &payload) {
  Serial.println("=== CALLBACK INVOCAT ===");
  Serial.print("Topic: "); Serial.println(topic);
  Serial.print("Payload: "); Serial.println(payload);

  //  Encender LED siempre que llegue un mensaje
  digitalWrite(LED_PIN, HIGH);
  Serial.println("[AWS] LED encès per missatge rebut");

  delay(2000);
  digitalWrite(LED_PIN, LOW);
  Serial.println("[AWS] LED apagat després de 2 segons");
}


