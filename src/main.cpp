#include <Arduino.h>
#include <Wifi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

#include <FastLED.h>

#include "gen/precompile.h"

FASTLED_USING_NAMESPACE
 
#define DATA_PIN    GPIO_NUM_5
#define NUM_LEDS    300
#define BRIGHTNESS         50
#define FRAMES_PER_SECOND  120

#define LED 2

#define SSID "NOWO-80EC"
#define WPAP "068E0EF8DC4DA6E4"

AsyncWebServer web_server(80);

CRGB leds[NUM_LEDS];
uint8_t gHue = 0;
uint8_t gPattern = 0;

uint8_t g_Hue = 0;
uint8_t g_Sat = 255;
uint8_t g_Val = 128;

void setupOTA()
{
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WPAP);

  pinMode(LED, OUTPUT);

  while(WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED, HIGH);
    delay(125);
    digitalWrite(LED, LOW);
    delay(375);
  }
  
  Serial.println(WiFi.localIP().toString().c_str());

  web_server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send(200, "text/html", INDEX_HTML); 
  });

  // TODO: Change for a json POST
  web_server.on("/update_value", HTTP_GET, [](AsyncWebServerRequest* req) {
    if(req->hasParam("name") && req->hasParam("value"))
    {
      String name = req->getParam("name")->value();
      String value = req->getParam("value")->value();

      if(name == "brightness")
      {
        FastLED.setBrightness(value.toInt());
      }
      else if(name == "hue")
      {
        g_Hue = (uint8_t)value.toInt();
        fill_solid(leds, NUM_LEDS, CHSV(g_Hue, g_Sat, g_Val)); // this is just a loop
      }
      else if(name == "sat")
      {
        g_Sat = (uint8_t)value.toInt();
        fill_solid(leds, NUM_LEDS, CHSV(g_Hue, g_Sat, g_Val)); // this is just a loop
      }
      else
      {
        req->send(200, "text/plain", "ERROR");
        return;
      }
      req->send(200, "text/plain", "OK");
      return;
    }
    req->send(200, "text/plain", "ERROR");
  });

  // Global set to red
  CHSV hsv(g_Hue, g_Sat, g_Val);
  for(int i = 0; i < NUM_LEDS; i++) leds[i] = hsv;

  // The leds are WS2812E, but the protocol is the same as the one in 2812B
  FastLED.addLeds<WS2812B,DATA_PIN,GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
 
  // Master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  setupOTA();
  
  web_server.begin();
}

void sinelon();
void matrix();

void loop()
{
  ArduinoOTA.handle();

  // fill_rainbow(leds, NUM_LEDS, gHue, 1);
  // fill_solid(leds, NUM_LEDS, CHSV(gHue + 100, 100, 128));
  // sinelon();
  // matrix();

  FastLED.show();
  FastLED.delay(1000/FRAMES_PER_SECOND);

  EVERY_N_MILLISECONDS( 50 ) { gHue++; }
}

void sinelon()
{
  uint16_t lpos = beatsin16(2, 0, NUM_LEDS-1);
  leds[lpos] += CHSV(gHue, 255, 128);
  fadeToBlackBy(leds, NUM_LEDS, 5);
}

void matrix()
{
  fadeToBlackBy(leds, NUM_LEDS, 20);
  static int pos[] = {0, 49, 99, 149, 199, 249, 299};
  static int hsv[] = {0, 37, 74, 112, 149, 186, 224};
  
  EVERY_N_MILLISECONDS( 40 )
  {
    for(int i = 0; i < 7; i++)
    {
      leds[pos[i]] += CHSV(hsv[i], 255, 192);
      pos[i] += 1;
      pos[i] %= 300;
    }
  }
}