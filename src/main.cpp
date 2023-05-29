#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <TinyGPSPlus.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

const char *ssid = "RedTrack";
const char *password = "3th3rn3t";

StaticJsonDocument<1024> doc;

AsyncWebServer server(80);

const char *PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void setupHttp()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        File file = SPIFFS.open("/index.html");
        if(!file){
            Serial.println("Failed to open file for reading");
            return;
        }
        
        request->send(200, "text/html", file.readString()); 
        file.close();
    });

    server.on("/gps", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String out;
        serializeJson(doc, out);
        request->send(200, "text/json", out); });

    // // Send a POST request to <IP>/post with a form field message set to <message>
    // server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request)
    //           {
    //     String message;
    //     if (request->hasParam(PARAM_MESSAGE, true)) {
    //         message = request->getParam(PARAM_MESSAGE, true)->value();
    //     } else {
    //         message = "No message sent";
    //     }
    //     request->send(200, "text/plain", "Hello, POST: " + message); });

    server.onNotFound(notFound);

    server.begin();
}

static const uint32_t GPSBaud = 57600;

// The TinyGPSPlus object
TinyGPSPlus gps;

void setup()
{
    Serial.begin(115200);
    Serial0.begin(GPSBaud);

    Serial.println(F("DeviceExample.ino"));
    Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
    Serial.print(F("Testing TinyGPSPlus library v. "));
    Serial.println(TinyGPSPlus::libraryVersion());
    Serial.println(F("by Mikal Hart"));
    Serial.println();

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    Serial0.begin(57600);
    Serial.println();
    Serial.println("Configuring access point...");

    // You can remove the password parameter if you want the AP to be open.
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    setupHttp();

    Serial.println("Server started");
}

void displayInfo()
{

    if (gps.location.isValid())
    {
        doc["lat"] = gps.location.lat();
        doc["lng"] = gps.location.lng();
    }
    else
    {
        doc["lat"] = 0.0;
        doc["lng"] = 0.0;
    }

    if (gps.date.isValid())
    {
        doc["day"] = gps.date.day();
        doc["month"] = gps.date.month();
        doc["year"] = gps.date.year();
    }
    else
    {
        doc["day"] = 0;
        doc["month"] = 0;
        doc["year"] = 0;
    }

    if (gps.time.isValid())
    {
        doc["hour"] = gps.time.hour();
        doc["minute"] = gps.time.minute();
        doc["second"] = gps.time.second();
        doc["centisecond"] = gps.time.centisecond();
    }
    else
    {
        doc["hour"] = 0;
        doc["minute"] = 0;
        doc["second"] = 0;
        doc["centisecond"] = 0;
    }

    if (gps.speed.isValid())
    {
        doc["kmph"] = gps.speed.kmph();
        doc["mph"] = gps.speed.mph();
        doc["mps"] = gps.speed.mps();
    }
    else
    {
        doc["kmph"] = 0.0;
        doc["mph"] = 0.0;
        doc["mps"] = 0.0;
    }

    if (gps.satellites.isValid())
    {
        doc["satellites"] = gps.satellites.value();
    }
    else
    {
        doc["satellites"] = 0;
    }
}

void updateGPS()
{
    // This sketch displays information every time a new sentence is correctly encoded.
    while (Serial0.available() > 0)
        if (gps.encode(Serial0.read()))
        {
            displayInfo();
        }

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
        Serial.println(F("No GPS detected: check wiring."));
        while (true)
            ;
    }
}

unsigned long lastTime = 0;
void loop()
{
    updateGPS();

    if (millis() - lastTime > 1000)
    {
        lastTime = millis();
        String out;
        serializeJson(doc, out);
        Serial.println(out);
    }

    digitalWrite(LED_BUILTIN, millis() % 1000 < 500 ? HIGH : LOW);
}