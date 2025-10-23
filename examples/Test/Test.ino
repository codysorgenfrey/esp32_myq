#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include <MyQ.h>

#define LOG(message, ...) printf(">>> [%7d][%.2fkb] Test.ino: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)

bool statusOk = false;
MyQ myq;

void setup()
{
  delay(2000);                  // give USB time to enumerate
  Serial.begin(115200);
  // Optional: wait for USB-CDC connection (works on many hosts)
  unsigned long start = millis();
  while (!Serial && millis() - start < 3000) { /* wait up to 3s */ }
  LOG("Starting...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  LOG("Connected to %s.", WIFI_SSID);

  statusOk = myq.setup();
  if (statusOk) {
    int state = myq.getGarageState(MYQ_GARAGE_SERIAL);
    LOG("Garage state: %i", state);

    state = myq.setGarageState(MYQ_GARAGE_SERIAL, MYQ_DOOR_SETSTATE_CLOSE);
    LOG("Told garage to close. Door state: %i", state);
  }
}

void loop() {
  if (statusOk) myq.loop();
}
