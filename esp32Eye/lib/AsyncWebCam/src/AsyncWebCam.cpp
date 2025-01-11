#include "AsyncWebCam.hpp"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART = "Content-Type: %s\r\nContent-Length: %u\r\n\r\n";

static const char * JPG_CONTENT_TYPE = "image/jpeg";
static const char * BMP_CONTENT_TYPE = "image/x-windows-bmp";


// 构造函数
AsyncJpegStreamResponse::AsyncJpegStreamResponse() {
  _callback = nullptr;
  _code = 200;
  _contentLength = 0;
  _contentType = STREAM_CONTENT_TYPE;
  _sendContentLength = false;
  _chunked = true;
  _index = 0;
  _jpg_buf_len = 0;
  _jpg_buf = NULL;
  lastAsyncRequest = 0;
  memset(&_frame, 0, sizeof(camera_frame_t));
}

// 析构函数
AsyncJpegStreamResponse::~AsyncJpegStreamResponse() {
  if (_frame.fb) {
    if (_frame.fb->format != PIXFORMAT_JPEG) {
      free(_jpg_buf);
    }
    esp_camera_fb_return(_frame.fb);  // 释放帧
  }
}

bool AsyncJpegStreamResponse::_sourceValid() const {
  return true;
}

size_t AsyncJpegStreamResponse::_fillBuffer(uint8_t *buf, size_t maxLen) {
  size_t ret = _content(buf, maxLen, _index);
  if (ret != RESPONSE_TRY_AGAIN) {
    _index += ret;
  }
  return ret;
}

size_t AsyncJpegStreamResponse::_content(uint8_t *buffer, size_t maxLen, size_t index) {
  if (!_frame.fb || _frame.index == _jpg_buf_len) {
    if (index && _frame.fb) {
      uint64_t end = (uint64_t)micros();
      int fp = (end - lastAsyncRequest) / 1000;
      float fps = 1000.0f / fp;
      lastAsyncRequest = end;
      if (_frame.fb->format != PIXFORMAT_JPEG) {
        free(_jpg_buf);  // 释放缓存
      }
      esp_camera_fb_return(_frame.fb);  // 释放帧
      _frame.fb = NULL;
      _jpg_buf_len = 0;
      _jpg_buf = NULL;
    }

    // 确保足够空间处理头部信息
    if (maxLen < (strlen(STREAM_BOUNDARY) + strlen(STREAM_PART) + strlen(JPG_CONTENT_TYPE) + 8)) {
      log_w("Not enough space for headers");
      return RESPONSE_TRY_AGAIN;
    }

    // 获取摄像头帧数据
    _frame.index = 0;
    _frame.fb = esp_camera_fb_get();
    if (_frame.fb == NULL) {
      log_e("Camera frame failed");
      return 0;
    } else {
      _jpg_buf_len = _frame.fb->len;
      _jpg_buf = _frame.fb->buf;
    }

    // 发送边界
    size_t blen = 0;
    if (index) {
      blen = strlen(STREAM_BOUNDARY);
      memcpy(buffer, STREAM_BOUNDARY, blen);
      buffer += blen;
    }

    // 发送头部
    size_t hlen = sprintf((char *)buffer, STREAM_PART, JPG_CONTENT_TYPE, _jpg_buf_len);
    buffer += hlen;

    // 发送帧数据
    hlen = maxLen - hlen - blen;
    if (hlen > _jpg_buf_len) {
      maxLen -= hlen - _jpg_buf_len;
      hlen = _jpg_buf_len;
    }
    memcpy(buffer, _jpg_buf, hlen);
    _frame.index += hlen;
    return maxLen;
  }

  size_t available = _jpg_buf_len - _frame.index;
  if (maxLen > available) {
    maxLen = available;
  }
  memcpy(buffer, _jpg_buf + _frame.index, maxLen);
  _frame.index += maxLen;

  vTaskDelay(pdMS_TO_TICKS(10));
  return maxLen;
}

AsyncWebCam::AsyncWebCam() : server(80) { }
SemaphoreHandle_t AsyncWebCam::mutex = NULL;
bool AsyncWebCam::is_mobile = false;
bool AsyncWebCam::newdata = false;
int AsyncWebCam::action = 0;
float AsyncWebCam::joystickX = 0.f;
float AsyncWebCam::joystickY = 0.f;
float AsyncWebCam::joystickZ = 0.f;

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
  config.jpeg_quality = 10;
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
    Serial.println("Mutex creation failed\n");
    Serial.println();
    return;
  } else {
    Serial.println("Mutex inite succeeded\n");
    Serial.println();  
  }
}

void root_handler(AsyncWebServerRequest *request) {
  String userAgent = request->header("User-Agent");

  // 是否是手机连接
  AsyncWebCam::is_mobile = userAgent.indexOf("Mobile") >= 0;
  Serial.printf("%s\n", AsyncWebCam::is_mobile?"mobile":"not mobile");
  
  // 打印出用户代理信息和设备类型
  Serial.println("User-Agent: " + userAgent);

  // 发送根页面HTML
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
  AsyncWebCam::joystickX = jsonObj["mouseX"].as<float>();
  AsyncWebCam::joystickY = jsonObj["mouseY"].as<float>();
  AsyncWebCam::joystickZ = jsonObj["mouseZ"].as<float>();

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

