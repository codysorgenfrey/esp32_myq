#include <WiFi.h>
#include "secrets.h"
#include <MyQ.h>

#define LOG(message, ...) printf(">>> [%7d][%.2fkb] Test.ino: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)

bool statusOk = false;
MyQ myq;

void setup()
{
    Serial.begin(115200);
    while (!Serial) { ; }; // wait for serial
    LOG("Starting...");

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    LOG("Connected to %s.", WIFI_SSID);

    statusOk = myq.setup(MYQ_EMAIL, MYQ_PASS);
    if (statusOk) {
        int state = myq.getGarageState(MYQ_GARAGE_SERIAL);
        LOG("Garage state: %i", state);
    }

}

void loop(){
    if (statusOk) myq.loop();
}
