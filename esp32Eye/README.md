# 机械觉之瞳-esp32eye
## 作者
Cirnocon

## 联系方式
电子邮件：cirnocon5@gmail.com 或 reedscirno@gmail.com

GitHub：https://github.com/Cirnocon

b站：Cirnocon

## 项目结构
- `ESP32EYE/`
    - `boards` 开发板配置文件，主要是启用psram

    - `data` SPIFFS文件储存文件夹
        - `index.html` html文件

        - `nipplejs.min.js` 摇杆库

    - `lib/` 自定义库文件夹
        - `AsyncWebCam` 

    - `src/` 源代码文件夹
        - `main.cpp` 
        
    - `partitions.csv` 分区表
    
    - `platformio.ini` ***PlatformIO*** 配置文件


## 食用方法
***vscode*** 下载 ***PlatformIO IDE*** 插件

> ***左侧栏拓展图标*** -> ***搜索 PlatformIO IDE*** -> ***安装***

打开 ***esp32eye*** 文件夹

> ***左上角文件*** -> ***打开文件夹*** -> ***找到 esp32eye 文件夹***

上传 ***SPIFFS*** 系统文件

> ***左侧栏小蚂蚁图标*** -> ***Bulid Filesystem Image*** -> ***Program Size*** -> ***Upload Filesystem Image***

编译并烧录文件

> ***左侧栏小蚂蚁图标*** -> ***Build*** -> ***Upload*** 


## 注意事项
所有配置文件都是基于 ***ESP32-S3-N16R8*** 写的

***platformio*** 的程序烧录并不会自动更新 ***SPIFFS*** 系统文件，需要手动更新


