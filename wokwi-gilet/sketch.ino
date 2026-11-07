/*
 * Gilet connecté — démonstrateur Wokwi (ESP32)
 * Module : Wearable Technologies, 5e année EELT, ESPRIM 2026-2027
 *
 * HYPOTHÈSES DE DÉMONSTRATION (non validées cliniquement) :
 *  - Seuil de pouls : alerte si hr_bpm > BPM_MAX (strict), BPM_MAX = 140 (énoncé).
 *  - Plage de température cutanée/ambiante : [TEMP_MIN, TEMP_MAX] = [35.0, 38.5] °C,
 *    valeur d'illustration, à remplacer par la plage retenue dans le dossier.
 *  - Le potentiomètre simule le pouls : 40 à 200 bpm sur la course ADC.
 *
 * Commandes série (115200 bauds) pour des essais reproductibles :
 *  BPM=<valeur>   force le pouls simulé (ignore le potentiomètre)
 *  BPM=AUTO      revient au potentiomètre
 *  TEMP=<valeur>  force la température (ignore le DHT22), ex. TEMP=39.5
 *  TEMP=AUTO     revient au DHT22
 *  NET=DOWN      simule une perte de liaison MQTT (pas de reconnexion)
 *  NET=UP        rétablit la liaison MQTT
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ---------- Paramètres configurables ----------
const int   BPM_MAX  = 140;      // seuil strict : alerte si bpm > 140
const float TEMP_MIN = 35.0;     // hypothèse de démonstration
const float TEMP_MAX = 38.5;     // hypothèse de démonstration

const char* WIFI_SSID = "Wokwi-GUEST";   // réseau simulé Wokwi
const char* WIFI_PASS = "";
const int   WIFI_CHANNEL = 6;

const char* MQTT_HOST = "test.mosquitto.org";  // broker public, démonstration uniquement
const int   MQTT_PORT = 1883;
const char* MQTT_TOPIC = "gilet-demo/athlete01/telemetry";
const char* DEVICE_ID  = "gilet-demo-01";

// ---------- Brochage ----------
const int PIN_DHT    = 15;   // DHT22 DATA
const int PIN_POT    = 34;   // potentiomètre (ADC1_CH6, entrée seule)
const int PIN_BUZZER = 25;   // buzzer actif

#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ---------- État ----------
int   forcedBpm   = -1;      // -1 = potentiomètre
float forcedTemp  = NAN;     // NAN = DHT22
bool  netDown     = false;   // simulation de perte MQTT
unsigned long lastSample = 0;
unsigned long lastPublish = 0;
unsigned long lastMqttTry = 0;
const unsigned long SAMPLE_MS  = 2000;
const unsigned long PUBLISH_MS = 5000;
const unsigned long RETRY_MS   = 5000;

int   hrBpm = 0;
float tempC = NAN;
float humPct = NAN;
bool  alertActive = false;
String alertReason = "";

void setupWifi() {
  Serial.print("Wi-Fi: connexion à ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi OK, IP = ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi non connecté : alerte locale continue de fonctionner.");
  }
}

void tryMqttConnect() {
  if (netDown) return;
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt.connected()) return;
  if (millis() - lastMqttTry < RETRY_MS) return;
  lastMqttTry = millis();
  String clientId = String("gilet-") + String((uint32_t)ESP.getEfuseMac(), HEX);
  Serial.print("MQTT: connexion à ");
  Serial.print(MQTT_HOST);
  Serial.print(":");
  Serial.println(MQTT_PORT);
  if (mqtt.connect(clientId.c_str())) {
    Serial.println("MQTT connecté.");
  } else {
    Serial.print("MQTT échec, rc=");
    Serial.println(mqtt.state());
  }
}

void readSensors() {
  // Pouls : potentiomètre (40..200 bpm) ou valeur forcée
  if (forcedBpm >= 0) {
    hrBpm = forcedBpm;
  } else {
    int raw = analogRead(PIN_POT);
    hrBpm = 40 + (raw * 160) / 4095;
  }

  // Température / humidité : DHT22 ou valeur forcée
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  humPct = isnan(h) ? NAN : h;
  if (!isnan(forcedTemp)) {
    tempC = forcedTemp;
  } else {
    tempC = t;   // peut rester NAN si le capteur ne répond pas
  }
}

void evaluateAlert() {
  alertReason = "";
  bool hrHigh = hrBpm > BPM_MAX;
  bool tempBad = isnan(tempC) ? false : (tempC < TEMP_MIN || tempC > TEMP_MAX);
  bool tempMissing = isnan(tempC);

  if (hrHigh) alertReason += "pouls_eleve";
  if (tempBad) {
    if (alertReason.length()) alertReason += ",";
    alertReason += "temperature_hors_plage";
  }
  if (tempMissing) {
    if (alertReason.length()) alertReason += ",";
    alertReason += "capteur_temperature_absent";
  }
  alertActive = hrHigh || tempBad;   // l'absence de capteur est signalée, pas un déclencheur de buzzer
}

void updateBuzzer() {
  digitalWrite(PIN_BUZZER, alertActive ? HIGH : LOW);
}

void publishTelemetry() {
  if (!mqtt.connected()) return;
  JsonDocument doc;
  doc["device_id"] = DEVICE_ID;
  doc["timestamp_ms"] = millis();
  doc["hr_bpm"] = hrBpm;
  if (isnan(tempC)) doc["temp_c"] = nullptr; else doc["temp_c"] = tempC;
  if (isnan(humPct)) doc["humidity_pct"] = nullptr; else doc["humidity_pct"] = humPct;
  doc["alert"] = alertActive;
  doc["alert_reason"] = alertReason;
  JsonObject units = doc["units"].to<JsonObject>();
  units["hr"] = "bpm";
  units["temp"] = "degC";
  units["humidity"] = "%RH";
  units["timestamp"] = "ms_since_boot";

  char buf[320];
  size_t n = serializeJson(doc, buf, sizeof(buf));
  bool ok = mqtt.publish(MQTT_TOPIC, buf, n);
  Serial.print(ok ? "MQTT publié : " : "MQTT publication échouée : ");
  Serial.println(buf);
}

void handleSerial() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  line.toUpperCase();
  if (line.startsWith("BPM=")) {
    String v = line.substring(4);
    if (v == "AUTO") forcedBpm = -1; else forcedBpm = v.toInt();
    Serial.print("Pouls forcé : "); Serial.println(forcedBpm);
  } else if (line.startsWith("TEMP=")) {
    String v = line.substring(5);
    if (v == "AUTO") forcedTemp = NAN; else forcedTemp = v.toFloat();
    Serial.print("Température forcée : "); Serial.println(forcedTemp);
  } else if (line == "NET=DOWN") {
    netDown = true;
    if (mqtt.connected()) mqtt.disconnect();
    Serial.println("Liaison MQTT coupée (simulation).");
  } else if (line == "NET=UP") {
    netDown = false;
    Serial.println("Liaison MQTT autorisée à reprendre.");
  } else if (line.length()) {
    Serial.println("Commande inconnue.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  analogReadResolution(12);
  dht.begin();
  setupWifi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  Serial.println("Démonstrateur gilet prêt. Seuils : BPM_MAX=140, TEMP ["
                 "35.0 ; 38.5] °C (hypothèses).");
}

void loop() {
  handleSerial();

  if (millis() - lastSample >= SAMPLE_MS) {
    lastSample = millis();
    readSensors();
    evaluateAlert();     // logique locale : indépendante du réseau
    updateBuzzer();
    Serial.print("hr="); Serial.print(hrBpm);
    Serial.print(" bpm  temp="); Serial.print(tempC);
    Serial.print(" degC  alert="); Serial.print(alertActive ? "OUI" : "non");
    Serial.print("  ["); Serial.print(alertReason); Serial.println("]");
  }

  if (WiFi.status() == WL_CONNECTED && !netDown) {
    tryMqttConnect();
    mqtt.loop();
  }

  if (millis() - lastPublish >= PUBLISH_MS) {
    lastPublish = millis();
    publishTelemetry();
  }
}
