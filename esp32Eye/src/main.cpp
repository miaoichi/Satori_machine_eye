#include <AsyncWebCam.h>
#include <ESP32Servo.h>

AsyncWebCam asyncwebcam;
SemaphoreHandle_t xMutex;

Servo servoX, servoY, servoZ;
const int servoXPin = 1;
const int servoYPin = 2;
const int servoZPin = 4;

const struct { double min, max, def; } servoLimits[] = {
    {90, 170, 130}, {45, 100, 75}, {70, 170, 140}
};

Servo* servos[] = {&servoX, &servoY, &servoZ};

// 变量定义
volatile double targetAngle[3] = {130, 45, 170};
volatile double currentAngle[3] = {130, 45, 170};
volatile int action;
volatile bool newdata = false;

const int DEFAULT_STEPS = 100;      // 默认的最大步数
const int DEFAULT_TOTAL_TIME = 250; // 默认总时间（毫秒）

// 三个舵机同时平滑转动函数1
void servo_smooth(double x, double y, double z) {
  int steps;
  double delaytime, maxAngleDiff, stepAngles[3], tA[3] = {x, y, z};

  // 判断输入值是否超出范围
  if (x < servoLimits[0].min) x = servoLimits[0].min;
  if (x > servoLimits[0].max) x = servoLimits[0].max;
  if (y < servoLimits[1].min) y = servoLimits[1].min;
  if (y > servoLimits[1].max) y = servoLimits[1].max;
  if (z < servoLimits[2].min) z = servoLimits[2].min;
  if (z > servoLimits[2].max) z = servoLimits[2].max;

  // 计算最大角度差
  for (int i = 0; i < 3; i++) {
    double diff = abs(tA[i] - currentAngle[i]);
    if (diff > maxAngleDiff) maxAngleDiff = diff;
  }

  // 计算步数和每步延时
  steps = max(1, (int)(maxAngleDiff * DEFAULT_STEPS / 180.0)); // 步数计算
  delaytime = DEFAULT_TOTAL_TIME / steps; // 每步延时

  // 计算每步转动角度
  for (int i = 0; i < 3; i++) {
    stepAngles[i] = (tA[i] - currentAngle[i]) / steps;
  }

  // 平滑移动到目标角度
  for (int step = 0; step < steps; step++) {
    // 若有新数据立即中断当前平滑过程
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      if (asyncwebcam.newdata) {
        xSemaphoreGive(xMutex);
        break;
      }
    }
    xSemaphoreGive(xMutex);

    // 否则更新currentAngle并写入舵机
    for (int i = 0; i < 3; i++) {
      currentAngle[i] += stepAngles[i];
      servos[i] -> write(currentAngle[i]);
    }
    
    // 每步等待的时间
    delay(delaytime);
  }
}



void servo_task(void *pvParameters) {
  while (true) {
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
      if (asyncwebcam.newdata) {
        newdata = true;
        asyncwebcam.newdata = false;
        Serial.println("switch newdata status");
        
        // asyncwebcam.x y 是相对于中点坐标的百分比 需要映射为角度
        action = asyncwebcam.action;
        targetAngle[0] = servoLimits[0].def + asyncwebcam.x * abs(servoLimits[0].max - servoLimits[0].min);
        targetAngle[1] = servoLimits[1].def + asyncwebcam.y * abs(servoLimits[1].max - servoLimits[1].def);
        targetAngle[2] = asyncwebcam.z;
      }
    }
    xSemaphoreGive(xMutex);

    if (newdata) {
      newdata = false;

      // 鼠标跟随
      if (action == 0) {
        Serial.println("activate action 0");
        servo_smooth(targetAngle[0], targetAngle[1], targetAngle[2]);
      }

      // 眨眼
      else if (action == 1) {
        Serial.println("activate action 1");

        double temp[2][2];
        temp[0][0] = servoLimits[1].min;
        temp[0][1] = servoLimits[2].max;// + abs(currentAngle[0]-130) * 5 / 40;
        temp[1][0] = currentAngle[1];
        temp[1][1] = currentAngle[2];

        servo_smooth(currentAngle[0], temp[0][0], temp[0][1]);
        servo_smooth(currentAngle[0], temp[1][0], temp[1][1]);
        
        action = 0;

      }

      // 摇头
      else if (action == 2) {
        Serial.println("activate action 2");

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
      else if (action == 3) {
        Serial.println("activate action 3");

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
      else if (action == 4) {
        Serial.println("activate action 4");

        targetAngle[0] = servoLimits[0].def;
        targetAngle[1] = servoLimits[1].min;
        targetAngle[2] = servoLimits[2].max;
        servo_smooth(targetAngle[0], targetAngle[1], targetAngle[2]);
      }

      // 睁眼
      else if (action == 5) {
        Serial.println("activate action 5");

        targetAngle[0] = servoLimits[0].def;
        targetAngle[1] = servoLimits[1].def;
        targetAngle[2] = servoLimits[2].def;
        servo_smooth(targetAngle[0], targetAngle[1], targetAngle[2]);
      }  
    }
    delay(10);
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
  xMutex = asyncwebcam.mutex;

  xTaskCreate(servo_task, "ServoTask", 2048, NULL, 0, NULL);
}



void loop() {
  delay(1000);
}
