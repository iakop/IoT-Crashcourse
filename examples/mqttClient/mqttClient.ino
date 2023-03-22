#include <WiFi.h> // WiFi functions
#include <WiFiClientSecure.h> // WiFi Client functions encrypted
#include <ESPmDNS.h> // mDNS, to be visible under friendly name
#include <DHT.h> // DHT sensor library to read values
#include <MQTT.h> // MQTT client library
#include <SPIFFS.h> // File system to load SSL certificate

#define JSON_DOC_SIZE JSON_OBJECT_SIZE(8) // Max number of objects in JSON doc

// Enter WiFi credentials (SSID, password):
const char* ssid = "workshop";
const char* password = "password";

// Define client connections for MQTT:
WiFiClientSecure wifiSecure;
MQTTClient mqtt;

// LED we control is on pin 25:
const int ledPin = 25;
int ledLevel = 0;

// DHT11 we control is on pin 26:
DHT myDHT(26, DHT11);
long dhtReadTime = 0;

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
	SPIFFS.begin();

	// Load rootCA:
	File rootCA = SPIFFS.open("rootCA.pem");
	String rootCAStr = rootCA.readString();
	rootCA.close();
	wifiSecure.setCACert(rootCAStr.c_str());

	// Setup the MQTT client, connect to port 8883 on secure network:
	mqtt.begin("mqtt.bechmann.xyz", 8883, wifiSecure);
	mqtt.onMessage(msgRecv);

	// Connect to MQTT server:
	while(!mqtt.connect("esp32", "public", "public")) {
		delay(500);
		Serial.print(".");
	}

	mqtt.subscribe("esp32/led/set/level");
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

		Serial.println("Sending: esp32/dht/temp: " + String(temp));
		mqtt.publish("esp32/dht/temp", String(temp));
		Serial.println("Sending: esp32/dht/hum: " + String(hum));
		mqtt.publish("esp32/dht/hum", String(hum));
	}

	// Delay to give background processes (WiFi handling etc.) more processing time.
	delay(2);
}

void msgRecv(String &topic, String &payload) {
	Serial.println("Received: " + topic + ": " + payload);
	if(topic == "esp32/led/set/level"){
		analogWrite(ledPin, payload.toInt());
	}
}
