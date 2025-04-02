# 機械覺之瞳-esp32eye
## 作者
Cirnocon -> miaoichi 

## 聯絡方式
電子郵件：miaoichi@gmail.com

GitHub：https://github.com/miaoichi


## 專案結構
- `ESP32EYE/`
 - `boards` 開發板設定文件，主要是啟用psram

 - `data` SPIFFS檔案儲存資料夾
 - `index.html` html文件

 - `nipplejs.min.js` 搖桿庫

 - `lib/` 自訂庫資料夾
 - `AsyncWebCam`

 - `src/` 原始碼資料夾
 - `main.cpp`

 - `partitions.csv` 分區表

 - `platformio.ini` ***PlatformIO*** 設定檔


## 食用方法
***vscode*** 下載 ***PlatformIO IDE*** 插件

> ***左側欄拓展圖示*** -> ***搜尋 PlatformIO IDE*** -> ***安裝***

打開 ***esp32eye*** 資料夾

> ***左上角檔案*** -> ***開啟資料夾*** -> ***找到 esp32eye 資料夾***

上傳 ***SPIFFS*** 系統文件

> ***左側欄小螞蟻圖示*** -> ***Bulid Filesystem Image*** -> ***Program Size*** -> ***Upload Filesystem Image***

編譯並燒錄文件

> ***左側欄小螞蟻圖示*** -> ***Build*** -> ***Upload***


## 注意事項
所有設定檔都是基於 ***ESP32-S3-N16R8*** 寫的

***platformio*** 的程式燒錄並不會自動更新 ***SPIFFS*** 系統文件，需要手動更新