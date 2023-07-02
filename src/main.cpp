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
  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200);
  }
  else
  {
    request->send(404, "application/json", "{\"message\":\"Not found\"}");
  }
}

void setupHttp()
{
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.serveStatic("/static/", SPIFFS, "/");

  server.on("/gps", HTTP_GET, [](AsyncWebServerRequest *request)
            {
        String out;
        serializeJson(doc, out);
        request->send(200, "text/json", out); });

  server.onNotFound(notFound);

  server.begin();
}

TinyGPSPlus gps;

void setup()
{
  Serial.begin(115200);
  Serial0.begin(57600);
  pinMode(A0, INPUT);
  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
  Serial.print(F("Testing TinyGPSPlus library v. "));
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();
  Serial.println("Configuring access point...");

  // No gateway so the connecting device can route to the internet independently
  WiFi.softAPConfig(IPAddress(192, 168, 123, 1), IPAddress(), IPAddress(255, 255, 255, 0));
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

float highestSpeedMph = 0.0;
double lastLat = 0.0;
double lastLng = 0.0;
double distance = 0.0;
void updateGps()
{

  if (gps.location.isValid())
  {
    doc["lat"] = gps.location.lat();
    doc["lng"] = gps.location.lng();
    if (gps.hdop.hdop() <= 10)
    {
      if (lastLat != 0.0 && lastLng != 0.0)
      {
        float deltaDist = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), lastLat, lastLng);
        distance += deltaDist;
        lastLat = gps.location.lat();
        lastLng = gps.location.lng();
      }
      else
      {
        lastLat = gps.location.lat();
        lastLng = gps.location.lng();
      }
    }
  }
  else
  {
    doc["lat"] = 0.0;
    doc["lng"] = 0.0;
  }

  if (gps.hdop.isValid())
  {
    doc["hdop"] = gps.hdop.hdop();
  }
  else
  {
    doc["hdop"] = 0.0;
  }

  if (gps.altitude.isValid())
  {
    doc["altitude"] = gps.altitude.meters();
  }
  else
  {
    doc["altitude"] = 0.0;
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
    doc["mph"] = gps.speed.mph();
    if (gps.speed.mph() > highestSpeedMph)
    {
      highestSpeedMph = gps.speed.mph();
    }
  }
  else
  {
    doc["mph"] = 0.0;
  }
  doc["maxMph"] = highestSpeedMph;

  if (gps.satellites.isValid())
  {
    doc["satellites"] = gps.satellites.value();
  }
  else
  {
    doc["satellites"] = 0;
  }

  if (gps.altitude.isValid())
  {
    doc["altitude"] = gps.altitude.meters();
  }
  else
  {
    doc["altitude"] = 0.0;
  }
  doc["distance"] = distance;
}

void updateGPS()
{
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial0.available() > 0)
    if (gps.encode(Serial0.read()))
    {
      updateGps();
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

  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++)
  {
    Vbatt = Vbatt + analogReadMilliVolts(A0); // ADC with correction
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0; // attenuation ratio 1/2, mV --> V
  doc["batt"] = round(Vbattf * 100) / 100.0;

  digitalWrite(LED_BUILTIN, millis() % 1000 < 500 ? HIGH : LOW);
}