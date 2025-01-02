#ifndef ASYNCWEBCAM_H
#define ASYNCWEBCAM_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "SPIFFS.h"
#include "esp_camera.h"
#include "camera_pins.h"
#include "AsyncJpesStreamResponse.hpp"

class AsyncWebCam {
private:
    const char* ssid = "ESP32-AP"; 
    const char* password = "";  

    AsyncWebServer server;
    void init_spiffs();
    void init_camera();
    void init_mutex();
    void init_server();

public:
    static SemaphoreHandle_t mutex;
    static bool newdata;
    static int action;
    static float x, y, z;

    AsyncWebCam();
    void start();
};

#endif
