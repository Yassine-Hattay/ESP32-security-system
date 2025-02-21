#ifndef CAMERA_v_H
#define CAMERA_v_H

#include "esp_camera.h"

#define ONBOADLED 12
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define GPIO_12 12

extern framesize_t framesize ;
extern int xclk_s ;
extern int jpeg_quality_v ;
extern int fb_count_v ;
extern int adjustment_time ;
extern int frames_skipped ;

void capturePhotoSaveLittleFS(void);
void takePhoto();
void initCamera();

#endif