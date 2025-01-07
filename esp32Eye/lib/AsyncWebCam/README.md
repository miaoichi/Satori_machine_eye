# AsyncWebCam
## 简介
这个库是基于 ***AsyncWebServer*** 和 ***esp_camera.h*** 二次包装的，实现了 ***WiFi AP*** 模式下，网页遥控觉之瞳和视频推流功能

## 作者
Cirnocon

## 联系方式
电子邮件： cirnocon5@gmail.com 或 reedscirno@gmail.com

GitHub：https://github.com/Cirnocon

b站：Cirnocon

## 项目结构
- `AsyncWebCaam/`
  - `src/`
    - `AsyncJpesStreamResponse.hpp` 定义了专门用于视频推流的AsyncWebServer类
    
    - `AsyncWebCam.cpp` AsyncWebCam实现
    
    - `calculator.h` AsyncWebCam头文件
    
    - `camera_pins.h` 摄像头引脚定义文件

## 类成员
- `private`
    - `const char* ssid` ***esp32 WiFi*** 名称

    - `const char* password` ***esp32 WiFi*** 密码，为空就是不需要密码就可以连接

    - `void init_spiffs()` 初始化 ***SPIFFS*** 文件储存系统

    - `void init_camera()` 初始化 ***OV2640*** 摄像头

    - `void init_mutex()` 初始化 ***互斥锁***（不建议用，用了就爆看门狗）

    - `void init_server()` 初始化 ***觉之瞳*** 服务器

- `public` 
    - `SemaphoreHandle_t mutex` 互斥锁

    - `bool newdata` 是否有新的遥控数据
 
    - `int action` 预设动作编号

    - `float x, y, z` 鼠标相对于浏览器页面中心点的位置和滚轮参数，用来控制舵机旋转的

    - `void start()` 启动服务器

## 可能会问的问题
### 为什么视频推流的实现专门写在了一个文件里？
> 因为我写不出来。这个代码是，有人和我一样不知道怎么AsyncWebCam这个库实现视频推流，就在github下面提issue。作者回复了实现代码。我不知道怎么集成，就随便吧改改然后放一个文件里直接调用了。详情请见 

>https://github.com/me-no-dev/ESPAsyncWebServer/issues/647 

>https://gist.github.com/me-no-dev/d34fba51a8f059ac559bf62002e61aa3

### 为什么不仿照 Arduino 官方的 WebCamera 示例，用 esp_http_server.h 这个库写？
> 最开始是用这个库写的，但是我的视频推流不知道怎么的会卡死线程，网页上其他元素比如按钮的数据就不能接收了。我也不知道为什么，技术力不够（悲）

### 为什么不使用 LITTLEFS 文件系统而是老旧的 SPIFFS
> 死活配置不好（即答）

> 而且 AsyncWebServer 对 SPIFFS 有支持。

### 有办法怎加视频推流质量和帧率吗？
> 你可以自己去 init_camera() 里改参数，注释已经标好了。但是 esp32 算力不够，再往上增加的话就不流畅了。而且现在这个配置能跑到60帧左右，还算不错了。

## 注意事项
摄像头引脚定义文件是根据 ***PCB*** 板子的连线写的

目前***esp32*** 的 ***WiFi*** 遥控和视频推流有些不稳定，网一卡，或者同时开着视频推流和遥控，***esp32*** 处理不过来，就会卡线程，然后就会触发看门狗。这应该是 ***AsyncWebServer*** 底层的问题，未来可能会用 ***vue3*** 重写。

只能支持一个设备打开遥控网页。可以写成支持多设备的，我没写，想改可以自己改。

我不关于网页设计，我不会 ***css*** ，全是 ***gpt*** 写的，会设计网页的可以帮忙设计一下。
