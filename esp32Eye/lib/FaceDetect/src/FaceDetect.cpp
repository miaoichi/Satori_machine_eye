#include "FaceDetect.hpp"
#include "image.hpp"

int* FaceDetect::human_detect_position() {
  dl::tool::Latency latency;
  HumanFaceDetectMSR01 s1(0.3F, 0.5F, 10, 0.2F);
  camera_fb_t *fb = esp_camera_fb_get();

  latency.start();

  std::list<dl::detect::result_t> &results = s1.infer((uint8_t *)IMAGE_ELEMENT, {IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_CHANNEL});
  // uint8_t *out_buf = (uint8_t*)malloc(fb->width * fb->height * 3);
  // fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
  // std::list<dl::detect::result_t> &results = s1.infer((uint8_t *)fb->buf, {(int)fb->height, (int)fb->width, 3});
  // free(out_buf);

  latency.end();
  latency.print("Inference latency");

  int i = 0;
  for (std::list<dl::detect::result_t>::iterator prediction = results.begin(); prediction != results.end(); prediction++, i++) {
    Serial.printf("[%d] score: %f, box: [%d, %d, %d, %d]\n", i, prediction->score, prediction->box[0], prediction->box[1], prediction->box[2], prediction->box[3]);
  }

  int *a;
  return a;
}
