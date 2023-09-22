#include <Arduino.h>
#include <WiFi.h> // WiFi functions
#include <WiFiClient.h> // WiFi Client functions
#include <ESPAsyncWebServer.h> // Async Web Server
#include <ArduinoJson.h> // Transmit JSON over websocket
#include <ESPmDNS.h> // mDNS, to be visible under friendly name
#include <LittleFS.h> // File system to load webpage from
#include <DHT.h> // DHT sensor library to read values

#define JSON_DOC_SIZE JSON_OBJECT_SIZE(8) // Max number of objects in JSON doc

// Enter WiFi credentials (SSID, password):
const char* ssid = "workshop";
const char* password = "password";

// Declare a webserver listening on port 80 with websocket:
const char* hostname = "websocketServer";
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// LED we control is on pin 25:
const int ledPin = 25;
int ledLevel = 0;

// DHT11 we control is on pin 26:
DHT myDHT(26, DHT11);
long dhtReadTime = 0;

// Websocket event handler prototype:
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

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
	Serial.println(); Serial.println("Connected to " + String(ssid) + ", IP address: " + WiFi.localIP().toString());

	// Begin MDNS hostname for network discovery:
	MDNS.begin(hostname);

	// Set pinMode() and digitalWrite() the beginning state, LOW:
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, ledLevel);

	// Begin DHT:
	myDHT.begin();

	// Mount the SPIFFS file system:
	LittleFS.begin();

	// Serve all statically from "/", default to "/websocketWebserver.html":
	server.serveStatic("/", LittleFS, "/").setDefaultFile("/websocketServer.html");

	// Register websocket events and handler:
	ws.onEvent(onWsEvent);
	server.addHandler(&ws);
	
	// Start server:
  	server.begin();
}

void loop() {
	// Clean up clients every loop:
	ws.cleanupClients();

	// Check if 2 seconds have passed, if true, then update:
	long curTime = millis();
	if((curTime - dhtReadTime) >= 2000){
		dhtReadTime = curTime;

		// Populate JSON doc:
		DynamicJsonDocument sendData(JSON_DOC_SIZE);
		do{
			// Read temperature as Celsius (the default), along with humidity:
			sendData["temp"] = myDHT.readTemperature();
			sendData["hum"] = myDHT.readHumidity();
		}while(isnan(sendData["temp"]) or isnan(sendData["hum"]));

		// Print JSON to string to send:
		String sendStr;
		serializeJson(sendData, sendStr);
		Serial.printf("Sending: %s\n", sendStr.c_str());
		ws.textAll(sendStr);
	}
	
	// Delay to give background processes (WiFi handling etc.) more processing time.
	delay(2);
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){				
	switch (type) {
	case WS_EVT_CONNECT:
		Serial.printf("WebSocket client #%d connected from: %s\n", client->id(), client->remoteIP().toString());
		break;
	case WS_EVT_DISCONNECT:
		Serial.printf("WebSocket client #%d disconnected...", client->id());
		break;
	case WS_EVT_DATA:
	{
		// Check if all data received, and matches datatype:
		AwsFrameInfo *info = (AwsFrameInfo*)arg;
		if((info->final) && (info->opcode == WS_TEXT)){
			// Terminate cast and print data:
			data[len] = '\0';
			String recvStr((char*)data);
			Serial.printf("WebSocket client #%d data: %s\n", client->id(), recvStr.c_str());
			
			// Create and deserialize into Json Object:
			DynamicJsonDocument recvData(JSON_DOC_SIZE);
			deserializeJson(recvData, recvStr);

			// Set LED:
			ledLevel = recvData["level"];
			analogWrite(ledPin, ledLevel);
		}
	}
		break;
	default:
		break;
	}
}
