// Establish server on the host, that the browser connects to
var server = `ws://${window.location.hostname}/ws`;
// This server is the argument for the new WebSocket object "connection"
var connection = new WebSocket(server);

var ledSlider = document.getElementById("ledSlider");
var ledLevel = document.getElementById("ledLevel");
var dhtTempMeter = document.getElementById("dhtTempMeter");
var dhtTemp = document.getElementById("dhtTemp");
var dhtHumMeter = document.getElementById("dhtHumMeter");
var dhtHum = document.getElementById("dhtHum");

// Print messages on WebSocket events: 
connection.onopen = function () {
	console.log("Connected to server on: ", server);
};
connection.onclose = function() {
	console.log("Disconnected from server on: ", server);
};
connection.onerror = function (error) {
	console.log("WebSocket Error ", error);
	alert("WebSocket Error ", error);
};
// Handle received message from server:
connection.onmessage = function (msg) {
	console.log("Received: ", msg);
	// Parse the JSON string received to valid data:
	let data = JSON.parse(msg.data);

	// Pass data to HTML fields:
	dhtTempMeter.value = data.temp; 
	dhtTemp.innerHTML = data.temp.toFixed(1);
	dhtHumMeter.value = data.hum;
	dhtHum.innerHTML = data.hum.toFixed(0);
};

// On input to the slider, construct JSON and send it:
ledSlider.oninput = function () {
	// Set data in HTML:
	ledLevel.innerHTML = ledSlider.value;

	// Construct JSON data:
	let data = { level: ledSlider.value };

	// Send update:
	connection.send(JSON.stringify(data));
}