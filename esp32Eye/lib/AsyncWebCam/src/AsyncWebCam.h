/* 
 * Satori_machine_eye - Toho Satori third eye
 * Copyright (C) 2025  Cirnocon
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
