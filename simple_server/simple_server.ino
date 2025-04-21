#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
#include "router_deets.h"
// if you clone this repo naked you'll need to make this `router_deets.h` file too it's just
/*
const char* ssid = "<ssid>";
const char* password = "<password>";
*/

// this file was adapted from https://www.mischianti.org/2020/05/16/how-to-create-a-rest-server-on-esp8266-and-esp32-startup-part-1/
// so shoutouts to Renzo Mischianti

#define LED 2 //Define blinking LED pin

#define HTTP_REST_PORT 8080
ESP8266WebServer server(HTTP_REST_PORT);

void(* resetFunc) (void) = 0;

void setup()
{
    pinMode(LED, OUTPUT); // Initialize the LED pin as an output

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

    // Set server routing
    restServerRouting();
    // Set not found response
    server.onNotFound(handleNotFound);
    // Start server
    server.begin();
    Serial.println("HTTP server started");

    toggleOn();
}

// the loop function runs over and over again forever
void loop()
{
    server.handleClient();
}

void getHelloWord()
{
    server.send(200, "text/json", "{\"name\": \"Hello world\"}");
}

void blinkOn()
{
    toggleOn();
    server.send(200, F("text/html"), F(""));
}

void toggleOn() 
{
    digitalWrite(LED, LOW);  // Turn the LED on (Note that LOW is the voltage level)
}

void blinkOff()
{
    toggleOff();
    server.send(200, F("text/html"), F(""));
}

void toggleOff() 
{
    digitalWrite(LED, HIGH); // Turn the LED off by making the voltage HIGH
}

void restServerRouting()
{
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"), F("Welcome to the REST Web Server"));
    });
    server.on("/on", HTTP_POST, blinkOn);
    server.on("/off", HTTP_POST, blinkOff);
    server.on(F("/helloWorld"), HTTP_GET, getHelloWord);
    server.on(F("/update"), HTTP_PUT, performOtaUpdate);
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

void performOtaUpdate()
{
    const char *address = "10.0.0.220";
    int port = 8081;
    const char *resource = "/binary.bin";

    toggleOff();

    ESPhttpUpdate.closeConnectionsOnUpdate(false);
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return ret = ESPhttpUpdate.update(address, port, resource);
    ESPhttpUpdate.onStart(updateStart);
    ESPhttpUpdate.onEnd(updateEnd);
    ESPhttpUpdate.onError(updateError);
    ESPhttpUpdate.onProgress(updateProgress);

    switch(ret) {
        case HTTP_UPDATE_FAILED:
            toggleOn();
            Serial.println("[update] Update failed.");
            server.send(200, "text/plain", "Update failed.");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            toggleOn();
            Serial.println("[update] Update no Update.");
            server.send(200, "text/plain", "No update needed");
            break;
        case HTTP_UPDATE_OK:
            Serial.println("[update] Update ok.");
            server.send(200, "text/plain", "Update OK");

            // wait for a bit, then restart the chip
            delay(500);
            resetFunc();
            break;
    }
}

void updateStart() {
    Serial.println("Update started."); 
}

void updateEnd() {
    Serial.println("Update ended."); 
}

void updateError(int err) {
    Serial.print("Update ERROR ("); 
    Serial.print(err); 
    Serial.println(")"); 
    Serial.println(ESPhttpUpdate.getLastErrorString());
}

void updateProgress(int one, int two) {
    Serial.print("Update progress ("); 
    Serial.print(one); 
    Serial.print(", ");
    Serial.print(two); 
    Serial.println(")"); 
}