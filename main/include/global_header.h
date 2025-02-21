#ifndef GLOBAL_VAR_H
#define GLOBAL_VAR_H

#include "esp_system.h"
#include <esp_wifi.h>
#include "esp_pm.h" 
#include "Arduino.h" 
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <TimeLib.h> 
#include <esp_now.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "esp_http_server.h"

#define GPIO_2 2

#define FILE_PHOTO "photo.jpg"
#define FILE_PHOTO_PATH "/photo.jpg"


#define PART_BOUNDARY "123456789000000000000987654321"
#define _STREAM_CONTENT_TYPE "multipart/x-mixed-replace;boundary=" PART_BOUNDARY
#define _STREAM_BOUNDARY "\r\n--" PART_BOUNDARY "\r\n"
#define _STREAM_PART "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"

#define fileDatainMessage 240.0
#define DELETEBEFOREPAIR 1
#define ESP_CHANNEL 13

extern WebServer server;

extern bool connected_internet;

extern unsigned long timeout_F;
extern unsigned long checkInterval ; // Interval for checking (30 minutes = 1800000 ms)
extern unsigned long lastCheckTime ;       // Last time check was done
extern unsigned long lastCheckTime_h;     // Last time check was done
extern unsigned long start_trans_time;

extern const char *ssid ;
extern const char *password ;

extern int currentTransmitTotalPackages ;
extern int currentTransmitCurrentPosition ;
extern byte sendNextPackageFlag ;
extern esp_now_peer_info_t slave;

extern bool connected_router;
extern esp_pm_config_t pm_config;
extern byte takeNextPhotoFlag ;

extern bool photo_not_sent ;
extern volatile bool wait_b ;
extern const esp_now_peer_info_t *peer;

extern bool isPaired ;
extern TaskHandle_t sending_photo_Handle ;
extern TaskHandle_t live_f_handle;
extern TaskHandle_t loop_handle ;
extern SemaphoreHandle_t manual_b_mutex;

extern bool manual_b;

void sendPhoto();
void sendNextPackage();
void InitESPNow();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void Set_SLAVE_data(uint8_t mac[6]);
bool manageSlave(int channel_v);

#endif

