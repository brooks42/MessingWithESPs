#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "router_deets.h"

// from https://github.com/bblanchon/ArduinoJson, you can install this
// w/ the ArduinoCli by doing `arduino-cli lib install ArduinoJson`
#include <ArduinoJson.h>

// if you clone this repo naked you'll need to make this `router_deets.h` file too it's just
/*
const char* ssid = "<ssid>";
const char* password = "<password>";
*/

#define HTTP_REST_PORT 8080
ESP8266WebServer server(HTTP_REST_PORT);

struct Vehicle {
    bool lightStatus = false;
    float steeringStatus = 0.0f;
    float speedStatus = 0.0f;
};

Vehicle car;

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Activate mDNS this is used to be able to connect to the server
    // with local DNS hostmane esp8266.local
    if (MDNS.begin("esp8266"))
    {
        Serial.println("MDNS responder started");
    }

    restServerRouting();
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

// the loop function runs over and over again forever
void loop()
{
    server.handleClient();
}

void getStatus() {

    DynamicJsonDocument doc(128);
    doc["lights"] = car.lightStatus;
    doc["speed"] = car.speedStatus;
    doc["steering"] = car.steeringStatus;

    Serial.print(F("Stream..."));
    String buf;
    serializeJson(doc, buf);
    server.send(200, "text/json", buf);
}

void getLights() {

    DynamicJsonDocument doc(24);
    doc["lights"] = car.lightStatus;

    Serial.print(F("Stream..."));
    String buf;
    serializeJson(doc, buf);
    server.send(200, "text/json", buf);
}

void setLights()
{
    String postBody = server.arg("plain");

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, postBody);
    JsonObject postObj = doc.as<JsonObject>();

    if (postObj.containsKey("lights")) {
        car.lightStatus = postObj["lights"];
        server.send(200, "text/plain", "");
    } else {
        server.send(400, "text/plain", "must define a `lights` boolean to set lights");
    }
}

void getSpeed() {
    
    DynamicJsonDocument doc(24);
    doc["speed"] = car.speedStatus;

    Serial.print(F("Stream..."));
    String buf;
    serializeJson(doc, buf);
    server.send(200, "text/json", buf);
}

void setSpeed()
{
    String postBody = server.arg("plain");

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, postBody);
    JsonObject postObj = doc.as<JsonObject>();

    if (postObj.containsKey("speed")) {
        car.speedStatus = postObj["speed"];
        server.send(200, "text/plain", "");
    } else {
        server.send(400, "text/plain", "must define a `speed` float between -1 and 1");
    }
}

void getSteering() {

    DynamicJsonDocument doc(24);
    doc["steering"] = car.steeringStatus;

    Serial.print(F("Stream..."));
    String buf;
    serializeJson(doc, buf);
    server.send(200, "text/json", buf);
}

void setSteering()
{
    String postBody = server.arg("plain");

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, postBody);
    JsonObject postObj = doc.as<JsonObject>();

    if (postObj.containsKey("steer")) {
        car.steeringStatus = postObj["steer"];
        server.send(200, "text/plain", "");
    } else {
        server.send(400, "text/plain", "must define a `steer` float between -1 and 1");
    }
}

void restServerRouting()
{
    // index just describes the calls you can make for convenience
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"), F("<html><body>Usage:</body></html>"));
    });

    server.on("/steer", HTTP_POST, setSteering);
    server.on("/lights", HTTP_POST, setLights);
    server.on("/speed", HTTP_POST, setSpeed);

    server.on("/steer", HTTP_GET, getSteering);
    server.on("/lights", HTTP_GET, getLights);
    server.on("/speed", HTTP_GET, getSpeed);
    server.on("/status", HTTP_GET, getStatus);
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}