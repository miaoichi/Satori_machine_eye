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

#include "AsyncWebCam.h"

AsyncWebCam::AsyncWebCam() : server(80) { }
SemaphoreHandle_t AsyncWebCam::mutex = NULL;
bool AsyncWebCam::newdata = false;
int AsyncWebCam::action = 0;
float AsyncWebCam::x = 0.f;
float AsyncWebCam::y = 0.f;
float AsyncWebCam::z = 0.f;

void AsyncWebCam::init_spiffs() {
  if (!SPIFFS.begin()) {
    Serial.println("Filesystem init failed");
    return;
  } else {
    Serial.println("Filesystem init secceeded");
    Serial.println("Listing files");

    File root = SPIFFS.open("/");
    while (File file = root.openNextFile()) { 
      Serial.printf("/%s, Size: %u\n", file.name(), file.size());
    }
    Serial.println();
    root.close(); 
  }
}

void AsyncWebCam::init_camera() {
  camera_config_t config;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;  // 视频流分辨率
  config.jpeg_quality = 10; // 帧压缩率
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println();
    return;
  } else {
    Serial.println("Camera init succeeded");
    Serial.println();
  }
}

void AsyncWebCam::init_mutex() {
  mutex = xSemaphoreCreateMutex(); 
  if (mutex == NULL) {
    Serial.println("Mutex creation failed");
    Serial.println();
    return;
  } else {
    Serial.println("Mutex inite succeeded");
    Serial.println();  
  }
}

void root_handler(AsyncWebServerRequest *request) {
  if (SPIFFS.exists("/index.html")) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html");
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    response->addHeader("X-Content-Type-Options", "nosniff");
    response->addHeader("Cache-Control", "no-cache");
    request->send(response);
  } else {
    Serial.println("/index.html not found");
    request->send(404, "text/plain", "/index.html not found");
  }
}

AsyncCallbackJsonWebHandler* data_handler = new AsyncCallbackJsonWebHandler("/data", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonObject jsonObj = json.as<JsonObject>();

  // 提取进public变量
  AsyncWebCam::newdata = true;
  AsyncWebCam::action = jsonObj["action"].as<int>();
  AsyncWebCam::x = jsonObj["mouseX"].as<float>();
  AsyncWebCam::y = jsonObj["mouseY"].as<float>();
  AsyncWebCam::z = jsonObj["mouseZ"].as<float>();

  // if (AsyncWebCam::mutex != NULL) {
  //   if (xSemaphoreTake(AsyncWebCam::mutex, portMAX_DELAY) == pdTRUE) {
      
  //     xSemaphoreGive(AsyncWebCam::mutex);
  //   }
  // }

  // 回应
  AsyncResponseStream *response = request->beginResponseStream("application/json; charset=utf-8");
  response->setCode(200);
  response->print("{\"status\": \"success\", \"message\": \"Action received\"}");
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "Content-Type");
  response->addHeader("X-Content-Type-Options", "nosniff");
  response->addHeader("Cache-Control", "no-cache");
  request->send(response);
});

void stream_handler(AsyncWebServerRequest *request){
    AsyncJpegStreamResponse *response = new AsyncJpegStreamResponse();
    if(!response){
        request->send(501);
        return;
    }
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
}

void AsyncWebCam::init_server() {
  WiFi.softAP(ssid, password);
  Serial.println("WiFi AP started");
  Serial.print("AP IP address: 'http://");
  Serial.print(WiFi.softAPIP());
  Serial.println("/ to view video stream");
  Serial.println();

  server.on("/", HTTP_GET, root_handler);
  server.serveStatic("/nipplejs.min.js", SPIFFS, "/nipplejs.min.js");
  server.addHandler(data_handler);
  server.on("/stream", HTTP_GET, stream_handler);
  server.begin();
}

void AsyncWebCam::start() {
  init_spiffs();
  init_camera();
  init_mutex();
  init_server();
}

