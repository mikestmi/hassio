#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define wifi_ssid "router_ssid"
#define wifi_password "router_password"

#define mqtt_server "hassio_ip"
#define mqtt_user "mqtt_username"
#define mqtt_password "mqtt_password"

#define brightness_topic "home/indoor/brightness"
#define humidity_topic "home/indoor/humidity"
#define temperature_topic "home/indoor/temperature"
#define motion_topic "home/indoor/motion"

#define photocell_pin A0
#define DHTTYPE DHT22
#define DHTPIN D5
#define pir_pin D1

#define motion_on "ON"
#define motion_off "OFF"

int pir_state = LOW;
int pir_value = 0;

DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void setup() {

  Serial.begin(115200);
  dht.begin();

  // init the WiFi connection
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);

}

long lastMsg = 0;
float bright = 0.0;
float diff = 1.0;
float temp = 0.0;
float hum = 0.0;

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  pir_value = digitalRead(pir_pin);
  if (pir_value == HIGH) {
    if (pir_state == LOW) {
      // a motion is detected
      client.publish(motion_topic, motion_on, true);
      pir_state = HIGH;
    }
  } else {
      if (pir_state == HIGH) {
        client.publish(motion_topic, motion_off, true);
        Serial.println("Motion ended");
        pir_state = LOW;
      }
  }

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    float newBright = analogRead(photocell_pin);
    float newBri100 = map(newBright,0,1023,0,100);
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (checkBound(newBri100, bright, diff)) {
      bright = newBri100;
      //bright = newBright;
      Serial.print("New brightness:");
      Serial.println(String(bright).c_str());
      client.publish(brightness_topic, String(bright).c_str(), true);
    }

    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      client.publish(temperature_topic, String(temp).c_str(), true);
    }

    if (checkBound(newHum, hum, diff)) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      client.publish(humidity_topic, String(hum).c_str(), true);
    }
  }
}