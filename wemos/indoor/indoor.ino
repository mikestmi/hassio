/* 
   Indoor WeMos unit 

   ESP8266 going with Arduino IDE
    - https://github.com/esp8266/Arduino#installing-with-boards-manager
    - Required libraries (sketch -> include library -> manage libraries)
    - PubSubClient by Nick ‘O Leary
    - DHT sensor library by Adafruit

 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Provide local router credentials
#define wifi_ssid "router_ssid"
#define wifi_password "router_password"

// Provide MQTT credentials
#define mqtt_server "hassio_ip"
#define mqtt_user "mqtt_username"
#define mqtt_password "mqtt_password"

// Define sensor topics for the MQTT broker
#define brightness_topic "home/indoor/brightness"
#define humidity_topic "home/indoor/humidity"
#define temperature_topic "home/indoor/temperature"
#define motion_topic "home/indoor/motion"

// Define the pins where the sensors are attached
#define photocell_pin A0
#define DHTTYPE DHT22
#define DHTPIN D5
#define pir_pin D1

// Initiate the variables for the motion sensor states
#define motion_on "ON"
#define motion_off "OFF"

int pir_state = LOW;
int pir_value = 0;

// Define the DHT sensor type
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

// Initiate espClient
WiFiClient espClient;
PubSubClient client(espClient);


// Function for connecting to the MQTT broker
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

/* Function to check the current and the previous value of a sensor,
   if the value is greater than a defined limit, then update the 
   sensor value with the new value
*/
bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void setup() {
  // Start the serial connection and the DHT sensor
  Serial.begin(115200);
  dht.begin();

  // Initiate the WiFi connection
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // Debugging messages for WiFi connection
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, 1883);

}
// Variable initiation
long lastMsg = 0;
float bright = 0.0;
float diff = 1.0;
float temp = 0.0;
float hum = 0.0;

void loop() {
  // Check if there is an establisted connection with the MQTT broker
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Get the motion sensor value, if there is a movement
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

  // Update the sensor values every 2 seconds
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;

    // Read the values of the sensors
    float newBright = analogRead(photocell_pin);
    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    /*Check the new value and the previous, if the difference is
      above 1.0 the update the variable with the new value
    */

    if (checkBound(newBri100, bright, diff)) {
      bright = map(newBright,0,1023,0,100);
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