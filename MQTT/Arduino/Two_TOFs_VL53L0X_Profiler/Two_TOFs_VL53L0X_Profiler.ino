#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_VL53L0X.h>

// Toggle Debug Messages
bool debugOn = false;

// Set Interval
uint16_t hz = 10;

// Replace with your network credentials
const char* ssid = "iotlab";
const char* password = "iotlab18";

const char* mqtt_server = "kleve.cool";

// MQTT broker username and password
const char* mqtt_username = "pub";
const char* mqtt_password = "pub";

// Setup WiFi client and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// Set up an instance for each sensor
Adafruit_VL53L0X tof1 = Adafruit_VL53L0X();
Adafruit_VL53L0X tof2 = Adafruit_VL53L0X();

// Shutdown pins
#define SHT_TOF1 32
#define SHT_TOF2 33

// Sensor readings
uint16_t x = 0;
uint16_t z = 0;

// Define the data object for sensor measurement
VL53L0X_RangingMeasurementData_t measure1;
VL53L0X_RangingMeasurementData_t measure2;

unsigned long lastMsg = 0;

// LED Pins
const int dueseLedPin = 4;
const int OoRLedPin = 25;

void setup() {
  if (debugOn) Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(dueseLedPin, OUTPUT);
  pinMode(OoRLedPin, OUTPUT);

  digitalWrite(dueseLedPin, LOW);
  digitalWrite(OoRLedPin, LOW);

  pinMode(SHT_TOF1, OUTPUT);
  pinModTOF, OUTPUT);

  digitalWrite(SHT_TOF1, LOW);
  digitalWritTOF, LOW);

  delay(10);
  digitalWrite(SHT_TOF1, HIGH);
  delay(10);

  // Initialize sensor 1
  if (!tof1.begin(0x30)) {
    debug(F("Failed to boot VL53L0X - 01"));
    while (1)
      ;
  }

  digitalWritTOF, HIGH);
  delay(10);

  // Initialize sensor 2
  if (!tof2.begin()) {
    debug(F("Failed to boot VL53L0X - 02"));
    while (1);
  }

  digitalWrite(SHT_TOF1, HIGH);
  digitalWritTOF, HIGH);

  delay(10);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > long(1000 / hz)) {  // Data transmission rate of 10Hz
    lastMsg = now;

    // Reading distance from both sensors
    tof1.getSingleRangingMeasurement(&measure1, false);
    if (measure1.RangeStatus != 4) {
      x = measure1.RangeMilliMeter;
    }
  
    tof2.getSingleRangingMeasurement(&measure2, false);
    if (measure2.RangeStatus != 4) {
      z = measure2.RangeMilliMeter;
    }

    // Current time in milliseconds.
    long t = now;

    // Create data payload
    String payload = String("(") + String(t) + ", " + String(z) + ", " + String(x) + ")";

    // Publish data
    if (x != 8191 && z != 8191 && x > 90 && z > 90) {
      digitalWrite(OoRLedPin, LOW);
      client.publish("gw/duese002", (char*)payload.c_str());
    } else {
      digitalWrite(OoRLedPin, HIGH);        
    }
  }
}

void setup_wifi() {
  delay(10);
  debug("");
  debug("Connecting to ");
  debug(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debug(".");
  }

  debug("");
  debug("WiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      // Once connected, resubscribe to the required topic
      client.subscribe("gw/duese002-licht");
    } else {
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "gw/duese002-licht") {
    if(messageTemp == "on"){
      digitalWrite(dueseLedPin, HIGH);
    }
    else if(messageTemp == "off"){
      digitalWrite(dueseLedPin, LOW);
    }
  }
}

void debug(String msg) {
  if (debugOn) {
    Serial.println(msg);
  }
}