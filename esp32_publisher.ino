/*
  BROKERDUH — ESP32 example publisher
  Publishes fake telemetry every 3s and listens for commands.
  Swap the fake reading for a real sensor read whenever you're ready.

  Library needed: PubSubClient (Install via Library Manager)
*/

#include <WiFi.h>
#include <PubSubClient.h>

// ---- fill these in ----
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_HOST     = "192.168.1.50";   // your broker's LAN IP
const int   MQTT_PORT     = 1883;
const char* DEVICE_ID     = "eyebot";         // change per board

// topics
String TOPIC_TELEMETRY;
String TOPIC_STATUS;
String TOPIC_COMMANDS;

WiFiClient espClient;
PubSubClient client(espClient);

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println(" connected.");
}

void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.printf("Command on %s: %s\n", topic, msg.c_str());

  // Example: react to a command
  if (msg == "blink") {
    // TODO: trigger your actual hardware action here
  }
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to broker...");
    String clientId = "esp32-" + String(DEVICE_ID);

    // Last Will and Testament: if this device drops off ungracefully,
    // the broker announces it automatically so the dashboard shows "offline" fast.
    if (client.connect(clientId.c_str(), NULL, NULL,
                        TOPIC_STATUS.c_str(), 1, true, "offline")) {
      Serial.println(" connected.");
      client.publish(TOPIC_STATUS.c_str(), "ok", true); // retained
      client.subscribe(TOPIC_COMMANDS.c_str());
    } else {
      Serial.printf(" failed, rc=%d. Retrying in 2s\n", client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  TOPIC_TELEMETRY = "devices/" + String(DEVICE_ID) + "/telemetry";
  TOPIC_STATUS     = "devices/" + String(DEVICE_ID) + "/status";
  TOPIC_COMMANDS   = "devices/" + String(DEVICE_ID) + "/commands";

  connectWiFi();
  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(onMessage);
}

unsigned long lastPublish = 0;

void loop() {
  if (!client.connected()) connectMQTT();
  client.loop();

  if (millis() - lastPublish > 3000) {
    lastPublish = millis();

    // Swap this for a real sensor reading
    float fakeReading = 20.0 + (random(0, 60) / 10.0);
    String payload = String(fakeReading, 1) + "C";

    client.publish(TOPIC_TELEMETRY.c_str(), payload.c_str());
    Serial.printf("Published %s -> %s\n", TOPIC_TELEMETRY.c_str(), payload.c_str());
  }
}
