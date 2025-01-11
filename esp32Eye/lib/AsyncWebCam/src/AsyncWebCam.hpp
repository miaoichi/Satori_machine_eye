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

typedef struct {
    camera_fb_t * fb;
    size_t index;
} camera_frame_t;

class AsyncJpegStreamResponse : public AsyncAbstractResponse {
    private:
        camera_frame_t _frame;
        size_t _index;
        size_t _jpg_buf_len;
        uint8_t * _jpg_buf;
        uint64_t lastAsyncRequest;

    public:
        AsyncJpegStreamResponse();
        ~AsyncJpegStreamResponse();

        bool _sourceValid() const override;
        virtual size_t _fillBuffer(uint8_t* buf, size_t maxLen) override;
        size_t _content(uint8_t* buffer, size_t maxLen, size_t index);
};

class AsyncWebCam {
    private:
        const char* ssid = "觉之瞳"; 
        const char* password = "";  
        
        AsyncWebServer server;
        void init_spiffs();
        void init_camera();
        void init_mutex();
        void init_server();

    public:
        static SemaphoreHandle_t mutex;
        static bool is_mobile;
        static bool newdata;
        static int action;
        static float joystickX, joystickY, joystickZ;
        
        AsyncWebCam();
        void start();
};


#endif
