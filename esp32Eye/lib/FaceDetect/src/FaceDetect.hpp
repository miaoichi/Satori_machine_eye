#ifndef FACEDETECT_H
#define FACEDETECT_H

#include <Arduino.h>
#include "esp_camera.h"
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"
#include "dl_tool.hpp"


class FaceDetect {
    private:
        const float is_two_stage = 0;  // 1 双阶段检测 0 单阶段检测 
        const float px_to_degree = 10;  // 距离画面中心点的像素长度与舵机角度的映射

    public:
        int* human_detect_position();
        int* fumo_detect_position();

};

#endif