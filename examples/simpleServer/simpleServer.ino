#include <WiFi.h> // WiFi functions
#include <WiFiClient.h> // WiFi Client functions
#include <ESPAsyncWebServer.h> // Async Webserver
#include <ESPmDNS.h> // mDNS, to be visible under friendly name
#include <SPIFFS.h> // File system to load webpage from

// Enter WiFi credentials (SSID, password):
const char* ssid = "IDA-Public";
const char* password = "";

// Declare a webserver listening on port 80:
const char* hostname = "simpleServer";
AsyncWebServer server(80);

// LED we control is on pin 25:
const int ledPin = 25;
bool ledState = 0;

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
	digitalWrite(ledPin, ledState);

	// Mount the SPIFFS file system:
	SPIFFS.begin();

	// Define Server responses:
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/simpleServer.html", "text/html", false, preprocessor);
	});
	
	server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/style.css", "text/css");
	});

	server.on("/setled", HTTP_POST, [](AsyncWebServerRequest *request){
		ledState = request->getParam("state", true)->value().toInt();
		digitalWrite(ledPin, ledState);
		request->send(SPIFFS, "/simpleServer.html", "text/html", false, preprocessor);
	});
	
	// Start server:
  	server.begin();
}

void loop() {
	// Delay to give background processes (WiFi handling etc.) more processing time.
	delay(2);
}

// Preprocessor function for page:
String preprocessor(const String& var){
	if(var == "STATE"){
		return String(ledState);
	}
	else{
		return "";
	}
}
