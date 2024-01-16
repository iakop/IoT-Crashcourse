#include <Arduino.h>
#include <WiFi.h> // WiFi functions
#include <WiFiClientSecure.h> // WiFi Client functions encrypted
#include <DHT.h> // DHT sensor library to read values
#include <MQTT.h> // MQTT client library
#include <LittleFS.h> // File system to load SSL certificate

// Enter WiFi credentials (SSID, password):
const char* ssid = "workshop";
const char* password = "password";

// Define client connections for MQTT:
WiFiClientSecure wifiSecure;
MQTTClient mqtt;

// MQTT credentials for our client:
const char* mqttClientId = "ESP32";
const char* mqttUsername = "public";
const char* mqttPassword = "public";

// LED we control is on pin 25:
const int ledPin = 25;
int ledLevel = 0;
// LED control topic:
#define LED_SET_LEVEL_TOPIC (String(mqttClientId) + "/led/set/level")

// DHT11 we control is on pin 26:
DHT myDHT(26, DHT11);
long dhtReadTime = 0;
// DHT publishing topics:
#define DHT_TEMP_TOPIC (String(mqttClientId) + "/dht/temp")
#define DHT_HUM_TOPIC (String(mqttClientId) + "/dht/hum")

// msgRecv callback function prototype:
void msgRecv(String &topic, String &payload);

void setup() {
	// Start Serial interface at 115200 baud:
	Serial.begin(115200);

	// Set WiFi mode to station, and connect to WiFi:
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while(WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	// When connected, print info:
	Serial.println("Connected to " + String(ssid) + ", IP address: " + WiFi.localIP().toString());

	// Set pinMode() and digitalWrite() the beginning state, LOW:
	pinMode(ledPin, OUTPUT);
	analogWrite(ledPin, ledLevel);

	// Begin DHT:
	myDHT.begin();

	// Mount the SPIFFS file system:
	LittleFS.begin();

	// Load rootCA:
	File rootCA = LittleFS.open("/rootCA.pem");
	String rootCAStr = rootCA.readString();
	rootCA.close();
	wifiSecure.setCACert(rootCAStr.c_str());

	// Setup the MQTT client, connect to port 8883 on secure network:
	mqtt.begin("mqtt.bechmann.xyz", 8883, wifiSecure);
	mqtt.onMessage(msgRecv);

	Serial.print("MQTT Connect");
	// Connect to MQTT server:
	while(!mqtt.connect(mqttClientId, mqttUsername, mqttPassword)) {
		delay(500);
		Serial.print(".");
	}

	mqtt.subscribe(LED_SET_LEVEL_TOPIC);
}

void loop() {
	// Update mqtt client loop:
	mqtt.loop();

	// Check if 2 seconds have passed, if true, then update:
	long curTime = millis();
	if((curTime - dhtReadTime) >= 2000){
		dhtReadTime = curTime;
		float temp;
		float hum;

		// Read data:
		do{
			// Read temperature as Celsius (the default), along with humidity:
			temp = myDHT.readTemperature();
			hum = myDHT.readHumidity();
		}while(isnan(temp) or isnan(hum));

		Serial.println("Sending: " + DHT_TEMP_TOPIC + ": " + String(temp));
		mqtt.publish(DHT_TEMP_TOPIC, String(temp));
		Serial.println("Sending: " + DHT_HUM_TOPIC + ": " + String(hum));
		mqtt.publish(DHT_HUM_TOPIC, String(hum));
	}

	// Delay to give background processes (WiFi handling etc.) more processing time.
	delay(2);
}

// msgRecv filters the incoming messages for interesting stuff, and then performs the relevant action:
void msgRecv(String &topic, String &payload) {
	Serial.println("Received: " + topic + ": " + payload);
	if(topic == LED_SET_LEVEL_TOPIC){
		analogWrite(ledPin, payload.toInt());
	}
}