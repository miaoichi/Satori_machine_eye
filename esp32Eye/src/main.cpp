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


/* 参调用 */
const int TOTAL_STEPS = 150;      // 默认的最大步数
const int TOTAL_DELAY = 250; // 默认总时间（毫秒）

volatile float targetAngle[3] = {130, 45, 170};
volatile float currentAngle[3] = {130, 45, 170};

/* 三个舵机同时平滑转动函数 */
void servo_smooth(float x, float y, float z) {
  int steps;
  float delaytime, maxAngleDiff, stepAngles[3], tA[3] = {x, y, z};

  // 判断输入值是否超出范围
  if (tA[0] < servoLimits[0].min) tA[0] = servoLimits[0].min;
  if (tA[0] > servoLimits[0].max) tA[0] = servoLimits[0].max;
  if (tA[1] < servoLimits[1].min) tA[1] = servoLimits[1].min;
  if (tA[1] > servoLimits[1].max) tA[1] = servoLimits[1].max;
  if (tA[2] < servoLimits[2].min) tA[2] = servoLimits[2].min;
  if (tA[2] > servoLimits[2].max) tA[2] = servoLimits[2].max;

  // 计算最大角度差
  for (int i = 0; i < 3; i++) {
    float diff = abs(tA[i] - currentAngle[i]);
    if (diff > maxAngleDiff) maxAngleDiff = diff;
  }

  // 计算步数和每步延时
  steps = max(1, (int)(maxAngleDiff * TOTAL_STEPS / 180.0)); // 步数计算
  delaytime = TOTAL_DELAY / steps; // 每步延时

  // 计算每步转动角度
  for (int i = 0; i < 3; i++) {
    stepAngles[i] = (tA[i] - currentAngle[i]) / steps;
  }

  // 平滑移动到目标角度
  for (int step = 0; step < steps; step++) {
    // 若有新数据立即中断当前平滑过程
    if (asyncwebcam.newdata) break;
    
    // 否则更新currentAngle并写入舵机
    for (int i = 0; i < 3; i++) {
      currentAngle[i] += stepAngles[i];
      servos[i] -> write(currentAngle[i]);
    }
    
    // 每步等待的时间
    delay(delaytime);
  }
}

void servo_smooth(float x, float y) {
  int steps;
  float z = y+65;
  float delaytime, maxAngleDiff, stepAngles[3], tA[3] = {x, y, z};

  // 判断输入值是否超出范围
  if (tA[0] < servoLimits[0].min) tA[0] = servoLimits[0].min;
  if (tA[0] > servoLimits[0].max) tA[0] = servoLimits[0].max;
  if (tA[1] < servoLimits[1].min) tA[1] = servoLimits[1].min;
  if (tA[1] > servoLimits[1].max) tA[1] = servoLimits[1].max;
  if (tA[2] < servoLimits[2].min) tA[2] = servoLimits[2].min;
  if (tA[2] > servoLimits[2].max) tA[2] = servoLimits[2].max;

  // 计算最大角度差
  for (int i = 0; i < 3; i++) {
    float diff = abs(tA[i] - currentAngle[i]);
    if (diff > maxAngleDiff) maxAngleDiff = diff;
  }

  // 计算步数和每步延时
  steps = max(1, (int)(maxAngleDiff * TOTAL_STEPS / 180.0)); // 步数计算
  delaytime = TOTAL_DELAY / steps; // 每步延时

  // 计算每步转动角度
  for (int i = 0; i < 3; i++) {
    stepAngles[i] = (tA[i] - currentAngle[i]) / steps;
  }

  // 平滑移动到目标角度
  for (int step = 0; step < steps; step++) {
    // 若有新数据立即中断当前平滑过程
    if (asyncwebcam.newdata) break;
    
    // 否则更新currentAngle并写入舵机
    for (int i = 0; i < 3; i++) {
      currentAngle[i] += stepAngles[i];
      servos[i] -> write(currentAngle[i]);
    }
    
    // 每步等待的时间
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
        
        // 提取所有变量
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
      // 摇杆
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

      //人脸跟踪
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

      // 摇头
      else if (action == -2) {
        Serial.println("activate action -2");

        double temp[3], xAdjustment=25; // 摇头偏移量
        temp[0] = currentAngle[0]-xAdjustment;
        temp[1] = currentAngle[0]+xAdjustment;
        temp[2] = currentAngle[0];

        servo_smooth(temp[0], currentAngle[1], currentAngle[2]);
        servo_smooth(temp[1], currentAngle[1], currentAngle[2]);
        servo_smooth(temp[0], currentAngle[1], currentAngle[2]);
        servo_smooth(temp[2], currentAngle[1], currentAngle[2]);

        action = 0;
      }

      // 点头
      else if (action == -3) {
        Serial.println("activate action -3");

        double temp[3], yAdjustment=25; // 点头偏移量
        temp[0] = currentAngle[1]-yAdjustment;
        temp[1] = currentAngle[1]+yAdjustment;
        temp[2] = currentAngle[1];

        servo_smooth(currentAngle[0], temp[0], currentAngle[2]);
        servo_smooth(currentAngle[0], temp[1], currentAngle[2]);
        servo_smooth(currentAngle[0], temp[0], currentAngle[2]);
        servo_smooth(currentAngle[0], temp[2], currentAngle[2]);

        action = 0;  
      }

      // 闭眼
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
  // 初始化舵机
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
