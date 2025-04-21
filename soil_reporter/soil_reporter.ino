#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <ArduinoOTA.h>
#include <sstream>

#include <NTPClient.h>
#include <WiFiUdp.h>

/*
if you clone this repo naked you'll need to make this `router_deets.h` file too it's just:

const char* ssid = "<ssid>";
const char* password = "<password>";
*/
#include "router_deets.h"

#define LED 2 //Define blinking LED pin

#define HTTP_REST_PORT 8080
ESP8266WebServer server(HTTP_REST_PORT);

// infra for internal timer
#define NTP_OFFSET 3600 * -7 // -7 hours UTC time
#define NTP_INTERVAL 60 * 5000 // milliseconds, update every 5 seconds
#define NTP_ADDRESS "north-america.pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// soil data management
struct SoilData {
    unsigned long timestamp;
    short humidity;
};

#define SOIL_DATAPOINTS_COUNT 100
int soilDataIndex = 0;
SoilData *soilData[SOIL_DATAPOINTS_COUNT];

void(* resetFunc) (void) = 0;

/*
Writes soil humidity data to RAM
*/
void writeSoilDataToIndex() {

    SoilData *point = new SoilData();
    point->humidity = getHumidityData();
    point->timestamp = timeClient.getEpochTime();
    soilData[soilDataIndex] = point;

    soilDataIndex++;

    // reset the soil data count to loop, if we overflow
    if (soilDataIndex >= SOIL_DATAPOINTS_COUNT) {
        soilDataIndex = 0;
    }
}

short getHumidityData() {
    return timeClient.getSeconds();
}

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

    timeClient.begin();

    for (int i = 0; i < SOIL_DATAPOINTS_COUNT; i++) {
        soilData[i] = new SoilData();
        soilData[i]->timestamp = timeClient.getEpochTime();
        soilData[i]->humidity = 100;
    }
}

// the loop function runs over and over again forever
void loop()
{
    server.handleClient();
    timeClient.update();
}

void restServerRouting()
{
    server.on(F("/"), HTTP_GET, []() {
        server.send(200, F("text/html"), F("Welcome to the REST Web Server"));
    });
    server.on(F("/on"), HTTP_POST, blinkOn);
    server.on(F("/off"), HTTP_POST, blinkOff);
    server.on(F("/helloWorld"), HTTP_GET, getHelloWord);
    server.on(F("/time"), HTTP_GET, getTime);
    server.on(F("/soildata"), HTTP_GET, getSoilData);

    // server.on(F("/update"), HTTP_PUT, performOtaUpdate);
}

/**

*/
void getHelloWord()
{
    server.send(200, "text/json", "{\"name\": \"Hello world\"}");
}

/**

*/
void blinkOn()
{
    toggleOn();
    server.send(200, F("text/html"), F(""));
}

/**

*/
void toggleOn() 
{
    digitalWrite(LED, LOW);  // Turn the LED on (Note that LOW is the voltage level)
}

/**

*/
void blinkOff()
{
    toggleOff();
    server.send(200, F("text/html"), F(""));
}

/**

*/
void toggleOff() 
{
    digitalWrite(LED, HIGH); // Turn the LED off by making the voltage HIGH
}

/**

*/
void getTime() {

  // String formattedTime = 
  // unsigned long epcohTime =  timeClient.getEpochTime();
    server.send(200, F("text/html"), timeClient.getFormattedTime());
}

/**

*/
void getSoilData() {
    
    std::ostringstream oss;

    for (int i = 0; i < SOIL_DATAPOINTS_COUNT; i++) {
        oss << soilData[i]->timestamp << "," << soilData[i]->humidity;

        if (i < SOIL_DATAPOINTS_COUNT - 1) {
            oss << ","; 
        }
    }

    server.send(200, "text/csv", oss.str().c_str());
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

// void performOtaUpdate()
// {
//     const char *address = "10.0.0.220";
//     int port = 8081;
//     const char *resource = "/binary.bin";

//     toggleOff();

//     ESPhttpUpdate.closeConnectionsOnUpdate(false);
//     ESPhttpUpdate.rebootOnUpdate(false);
//     t_httpUpdate_return ret = ESPhttpUpdate.update(address, port, resource);
//     ESPhttpUpdate.onStart(updateStart);
//     ESPhttpUpdate.onEnd(updateEnd);
//     ESPhttpUpdate.onError(updateError);
//     ESPhttpUpdate.onProgress(updateProgress);

//     switch(ret) {
//         case HTTP_UPDATE_FAILED:
//             toggleOn();
//             Serial.println("[update] Update failed.");
//             server.send(200, "text/plain", "Update failed.");
//             break;
//         case HTTP_UPDATE_NO_UPDATES:
//             toggleOn();
//             Serial.println("[update] Update no Update.");
//             server.send(200, "text/plain", "No update needed");
//             break;
//         case HTTP_UPDATE_OK:
//             Serial.println("[update] Update ok.");
//             server.send(200, "text/plain", "Update OK");

//             // wait for a bit, then restart the chip
//             delay(500);
//             resetFunc();
//             break;
//     }
// }

// void updateStart() {
//     Serial.println("Update started."); 
// }

// void updateEnd() {
//     Serial.println("Update ended."); 
// }

// void updateError(int err) {
//     Serial.print("Update ERROR ("); 
//     Serial.print(err); 
//     Serial.println(")"); 
//     Serial.println(ESPhttpUpdate.getLastErrorString());
// }

// void updateProgress(int one, int two) {
//     Serial.print("Update progress ("); 
//     Serial.print(one); 
//     Serial.print(", ");
//     Serial.print(two); 
//     Serial.println(")"); 
// }