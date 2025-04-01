#include <AsyncWebCam.hpp>
#include <FaceDetect.hpp>
#include <ESP32Servo.h>

AsyncWebCam asyncwebcam;
FaceDetect facedetect;

Servo servoX, servoY, servoZ;
const int servoXPin = 1;
const int servoYPin = 2;
const int servoZPin = 4;

const struct { double min, max, def; } servoLimits[] = {
    {90, 170, 130}, {45, 100, 75}, {70, 170, 140}
};

Servo* servos[] = {&servoX, &servoY, &servoZ};


/* 參調用 */
const int TOTAL_STEPS = 150;      // 預設的最大步數
const int TOTAL_DELAY = 250; // 預設總時間（毫秒）

volatile float targetAngle[3] = {130, 45, 170};
volatile float currentAngle[3] = {130, 45, 170};

/* 三个舵機同時平滑轉動函數 */
void servo_smooth(float x, float y, float z) {
  int steps;
  float delaytime, maxAngleDiff, stepAngles[3], tA[3] = {x, y, z};

  // 判斷輸入值是否超出範圍
  if (tA[0] < servoLimits[0].min) tA[0] = servoLimits[0].min;
  if (tA[0] > servoLimits[0].max) tA[0] = servoLimits[0].max;
  if (tA[1] < servoLimits[1].min) tA[1] = servoLimits[1].min;
  if (tA[1] > servoLimits[1].max) tA[1] = servoLimits[1].max;
  if (tA[2] < servoLimits[2].min) tA[2] = servoLimits[2].min;
  if (tA[2] > servoLimits[2].max) tA[2] = servoLimits[2].max;

  // 計算最大角度差
  for (int i = 0; i < 3; i++) {
    float diff = abs(tA[i] - currentAngle[i]);
    if (diff > maxAngleDiff) maxAngleDiff = diff;
  }

  // 計算步數和每步延時
  steps = max(1, (int)(maxAngleDiff * TOTAL_STEPS / 180.0)); // 步數計算
  delaytime = TOTAL_DELAY / steps; // 每步延遲時間

  // 計算每步轉動角度
  for (int i = 0; i < 3; i++) {
    stepAngles[i] = (tA[i] - currentAngle[i]) / steps;
  }

  // 平滑地移動到目標角度
  for (int step = 0; step < steps; step++) {
    // 若有新資料立即中斷目前平滑過程
    if (asyncwebcam.newdata) break;
    
    // 否則更新currentAngle並寫入舵機
    for (int i = 0; i < 3; i++) {
      currentAngle[i] += stepAngles[i];
      servos[i] -> write(currentAngle[i]);
    }
    
    // 每步等待的時間
    delay(delaytime);
  }
}

void servo_smooth(float x, float y) {
  int steps;
  float z = y+65;
  float delaytime, maxAngleDiff, stepAngles[3], tA[3] = {x, y, z};

  // 判斷輸入值是否超出範圍
  if (tA[0] < servoLimits[0].min) tA[0] = servoLimits[0].min;
  if (tA[0] > servoLimits[0].max) tA[0] = servoLimits[0].max;
  if (tA[1] < servoLimits[1].min) tA[1] = servoLimits[1].min;
  if (tA[1] > servoLimits[1].max) tA[1] = servoLimits[1].max;
  if (tA[2] < servoLimits[2].min) tA[2] = servoLimits[2].min;
  if (tA[2] > servoLimits[2].max) tA[2] = servoLimits[2].max;

  // 計算最大角度差
  for (int i = 0; i < 3; i++) {
    float diff = abs(tA[i] - currentAngle[i]);
    if (diff > maxAngleDiff) maxAngleDiff = diff;
  }

  // 計算步數和每步延時
  steps = max(1, (int)(maxAngleDiff * TOTAL_STEPS / 180.0)); // 步數計算
  delaytime = TOTAL_DELAY / steps; // 每步延遲時

  // 計算每步轉動角度
  for (int i = 0; i < 3; i++) {
    stepAngles[i] = (tA[i] - currentAngle[i]) / steps;
  }

  // 平滑地移動到目標角度
  for (int step = 0; step < steps; step++) {
    // 若有新資料立即中斷目前平滑過程
    if (asyncwebcam.newdata) break;
    
    // 否則更新currentAngle並寫入舵機
    for (int i = 0; i < 3; i++) {
      currentAngle[i] += stepAngles[i];
      servos[i] -> write(currentAngle[i]);
    }
    
    // 每步等待的時間
    delay(delaytime);
  }
}


SemaphoreHandle_t Mutex;
bool is_mobile = false;
bool newdata = false;
int action;
float joystickX, joystickY, joystickZ;

TaskHandle_t servoTaskHandle; 
TaskHandle_t faceTracingTaskHandle; 

void faceTracingTask(void *pvParameters);

void servo_task(void *pvParameters) {
  Serial.println("Servo task on");  
  while (true) {
    if (xSemaphoreTake(Mutex, portMAX_DELAY) == pdTRUE) {
      if (asyncwebcam.newdata) {
        Serial.println("newdata");
        
        // 提取所有變數
        newdata = true;
        action = asyncwebcam.action;
        joystickX = asyncwebcam.joystickX;
        joystickY = asyncwebcam.joystickY;
        joystickZ = asyncwebcam.joystickZ;
        is_mobile = asyncwebcam.is_mobile;
        
        asyncwebcam.newdata = false;
      }
    }
    xSemaphoreGive(Mutex);
    

    if (newdata) {
      newdata = false;
      // 搖桿
      if (action == 0) {
        Serial.println("joystick");
        
        targetAngle[0] = servoLimits[0].def + joystickX * fabs(servoLimits[0].max - servoLimits[0].min);
        targetAngle[1] = servoLimits[1].def + joystickY * fabs(servoLimits[1].max - servoLimits[1].def);
        targetAngle[2] = joystickZ;
        
        if (is_mobile) {
          servo_smooth(targetAngle[0], targetAngle[1]);
        } else {
          servo_smooth(targetAngle[0], targetAngle[1], targetAngle[2]);
        }  
      }

      //人臉跟蹤
      else if (action == 1) {
        Serial.println("Servo task off, creating face tracing task"); 
        xTaskCreate(faceTracingTask, "FaceTracingTask", 32768, NULL, 1, &faceTracingTaskHandle);
        vTaskDelete(NULL); 
        
      }


      // 眨眼
      else if (action == -1) {
        Serial.println("activate action -1");

        double temp[2][2];
        temp[0][0] = servoLimits[1].min;
        temp[0][1] = servoLimits[2].max;// + fabs(currentAngle[0]-130) * 5 / 40;
        temp[1][0] = currentAngle[1];
        temp[1][1] = currentAngle[2];

        servo_smooth(currentAngle[0], temp[0][0], temp[0][1]);
        servo_smooth(currentAngle[0], temp[1][0], temp[1][1]);
        
        action = 0;
      }

      // 搖頭
      else if (action == -2) {
        Serial.println("activate action -2");

        double temp[3], xAdjustment=25; // 搖頭偏移量
        temp[0] = currentAngle[0]-xAdjustment;
        temp[1] = currentAngle[0]+xAdjustment;
        temp[2] = currentAngle[0];

        servo_smooth(temp[0], currentAngle[1], currentAngle[2]);
        servo_smooth(temp[1], currentAngle[1], currentAngle[2]);
        servo_smooth(temp[0], currentAngle[1], currentAngle[2]);
        servo_smooth(temp[2], currentAngle[1], currentAngle[2]);

        action = 0;
      }

      // 點頭
      else if (action == -3) {
        Serial.println("activate action -3");

        double temp[3], yAdjustment=25; // 點頭偏移量
        temp[0] = currentAngle[1]-yAdjustment;
        temp[1] = currentAngle[1]+yAdjustment;
        temp[2] = currentAngle[1];

        servo_smooth(currentAngle[0], temp[0], currentAngle[2]);
        servo_smooth(currentAngle[0], temp[1], currentAngle[2]);
        servo_smooth(currentAngle[0], temp[0], currentAngle[2]);
        servo_smooth(currentAngle[0], temp[2], currentAngle[2]);

        action = 0;  
      }

      // 閉眼
      else if (action == -4) {
        Serial.println("activate action -4");

        targetAngle[0] = servoLimits[0].def;
        targetAngle[1] = servoLimits[1].min;
        targetAngle[2] = servoLimits[2].max;
        servo_smooth(targetAngle[0], targetAngle[1], targetAngle[2]);

      }

      // 睁眼
      else if (action == -5) {
        Serial.println("activate action -5");

        targetAngle[0] = servoLimits[0].def;
        targetAngle[1] = servoLimits[1].def;
        targetAngle[2] = servoLimits[2].def;
        servo_smooth(targetAngle[0], targetAngle[1], targetAngle[2]);

      }  
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void faceTracingTask(void *pvParameters) {
  Serial.println("Face tracing task on");
  while (true) {
    if (xSemaphoreTake(Mutex, portMAX_DELAY) == pdTRUE) {
      if (asyncwebcam.newdata) {
      xSemaphoreGive(Mutex);
      Serial.println("Face tracking task off, creating servo task");
      xTaskCreate(servo_task, "ServoTask", 2048, NULL, 0, &servoTaskHandle);
      vTaskDelete(NULL);
      }
    }
    xSemaphoreGive(Mutex);
    
    Serial.println("!");
    
    facedetect.human_detect_position();

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}


void setup() {
  Serial.begin(115200);
  // 初始化舵機
  servoX.attach(servoXPin);
  servoY.attach(servoYPin);
  servoZ.attach(servoZPin);  

  servoX.write(125);
  servoY.write(45);
  servoZ.write(170);
  
  asyncwebcam.start();
  Mutex = asyncwebcam.mutex;

  xTaskCreate(servo_task, "ServoTask", 2048, NULL, 0, NULL);

}


void loop() {  
  delay(1000);
}
