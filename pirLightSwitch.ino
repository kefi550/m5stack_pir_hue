#include <M5Atom.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "config.h"

enum STATE_CODE {
  LIGHT_OFF,
  LIGHT_ON,
  WIFI_ERROR
};

HTTPClient http;
const int LOOP_MILLI_SECOND = 100;
float timer_milli_second = 0.0;
int state = LIGHT_OFF;
int old_state;
char url[200];

void waitWifiConnected(){
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    count++;
//    int n = WiFi.scanNetworks();
//    for(int i=0; i<3; i++){
//      Serial.printf("%s\n",WiFi.SSID(i));
//    }
    if (count == 10) {
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
      delay(1000);
      Serial.println("wifi restart");
    }
    if (count > 15) {
      Serial.println("restart");
      ESP.restart();
    }
    Serial.print(".");
    delay(500);
  }
}

void setup() {
  M5.begin(true, false, true);
  M5.dis.drawpix(0, 0xf00000);
  pinMode(PIR_MOTION_SENSOR, INPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  sprintf(url, "http://%s/api/%s/groups/1/action", HUE_BRIDGE_HOST, HUE_USERNAME);
  Serial.printf("url: %s\n", url);
  Serial.print("Connecting Wifi...");
  waitWifiConnected();
  Serial.println("connected");
  M5.dis.drawpix(0, 0x00ff00);
}

void changeLight(int new_state) {
  if (WiFi.status() == WL_CONNECTED) {
    old_state = state;
    DynamicJsonDocument doc(2048);
    String json;
    doc["on"] = bool(new_state);
    http.begin(url);
    serializeJson(doc, json);
    int httpCode = http.PUT(json);
    Serial.println(httpCode);
    if (httpCode == HTTP_CODE_OK) {
      int res = http.getString().toInt();
      Serial.println(res);
      state = new_state;
      M5.dis.drawpix(0, 0xffff00);
    } else {
      Serial.printf("[HTTP] failed, error: %s\n", http.errorToString(httpCode).c_str());
      state = old_state;
      M5.dis.drawpix(0, 0xff0000);
    }
    http.end();
  }else{
    old_state = state;
    state = WIFI_ERROR;
  }
}

bool motion() {
  return digitalRead(PIR_MOTION_SENSOR);
}

void loop() {
  if (state == WIFI_ERROR){
    waitWifiConnected();
    if (WiFi.status() == WL_CONNECTED) {
      state = old_state;
      return;
    }
    M5.dis.drawpix(0, 0xff0000);
  }else{
    M5.dis.drawpix(0, 0x00ff00);
    if (motion()) {
      M5.dis.drawpix(0, 0x0000ff);
      Serial.println("sensed");
      if (state == LIGHT_OFF) {
        Serial.println("on");
        changeLight(LIGHT_ON);
      }
      timer_milli_second = 0;
    } 
    if ((state == LIGHT_ON && timer_milli_second > 10000) || timer_milli_second > 60000) {
      Serial.println("off");
      changeLight(LIGHT_OFF);
      timer_milli_second = 0;
    }
    timer_milli_second += LOOP_MILLI_SECOND;
  }
  delay(LOOP_MILLI_SECOND);
}
