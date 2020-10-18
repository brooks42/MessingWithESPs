#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "router_deets.h"

#define LED 2 //Define blinking LED pin

#define HTTP_REST_PORT 8080
ESP8266WebServer server(HTTP_REST_PORT);

void setup() {
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
}

// the loop function runs over and over again forever
void loop() {
    server.handleClient();
    // digitalWrite(LED, LOW);  // Turn the LED on (Note that LOW is the voltage level)
    // delay(1000);             // Wait for a second
    // digitalWrite(LED, HIGH); // Turn the LED off by making the voltage HIGH
    // delay(1000);             // Wait for two seconds
}

void getHelloWord() {
    server.send(200, "text/json", "{\"name\": \"Hello world\"}");
}

void restServerRouting() {
    server.on("/", HTTP_GET, []() {
        server.send(200, F("text/html"),
                            F("Welcome to the REST Web Server"));
    });
    server.on(F("/helloWorld"), HTTP_GET, getHelloWord);
}

void handleNotFound() {
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