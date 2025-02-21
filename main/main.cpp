#include "global_header.h"
#include "email_module.h"
#include "camera_var.h"
#include "server_v.h"
#include "tasks_v.h"

const char *ssid = "Orange-066C";
const char *password = "GMA6ABLMG87";

uint8_t mac[6] = {0xCC, 0x50, 0xE3, 0x42, 0x2A, 0x61};

WebServer server(80);

unsigned long checkInterval = 1800000; 
unsigned long timeout_F = 20000;

WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 3600, 3600000);

int adjustment_time = 1 * 1000;
int jpeg_quality_v = 10;
int frames_skipped = 10;
int xclk_s = 20000000;
int fb_count_v = 1;

extern "C" void app_main()
{

  Serial.begin(115200);
  Serial.println();

  initArduino();

  pinMode(GPIO_2, INPUT_PULLDOWN);
  pinMode(GPIO_12, OUTPUT);
  pinMode(ONBOADLED, OUTPUT);

  digitalWrite(ONBOADLED, LOW);

  Serial.print("\n Setup ! \n");

  digitalWrite(GPIO_12, HIGH);
  delay(1000);
  digitalWrite(GPIO_12, LOW);

  WiFi.mode(WIFI_MODE_APSTA);

  connected_router = connectToWiFi(ssid, password, 10000);

  while (!LittleFS.begin())
  {
    LittleFS.format();
  }

  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());

  InitESPNow();

  esp_now_register_send_cb(OnDataSent);

  Set_SLAVE_data(mac);

  initCamera();
  capturePhotoSaveLittleFS();

  isPaired = manageSlave(ESP_CHANNEL);

  // Configure timer wake-up (15 seconds)
  esp_sleep_enable_timer_wakeup(15 * 1000000);

  // Configure external wake-up on GPIO2 (HIGH = 1)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);

  manual_b_mutex = xSemaphoreCreateMutex();

  if (connected_router)
  {
    pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 160,
        .light_sleep_enable = false};

    esp_pm_configure(&pm_config);

    setup_server();

    connected_internet = checkSMTPService(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword); // final test3

    xTaskCreate(
      loop_f,
      "loop",
      3072,
      NULL,
      1,
      &loop_handle); 

    if (connected_internet) // change   if(connected_internet)

    {
      timeClient.begin();
      // Start NTP client to fetch time
      timeClient.update();
      unsigned long epochTime = timeClient.getEpochTime();
      setTime(epochTime); // Sets time for TimeLib functions

      timeClient.end();

      capturePhotoSaveLittleFS();
      sendPhoto();
    }
  }
  else
  {

    if (sending_photo_Handle != NULL)
    {
      vTaskDelete(sending_photo_Handle);
      sending_photo_Handle = NULL;
    }

    xTaskCreate(
        sending_photo_task,
        "send_photo",
        100008,
        NULL,
        2,
        &sending_photo_Handle);

    pm_config = {
        .max_freq_mhz = 40,
        .min_freq_mhz = 10,
        .light_sleep_enable = false};

    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.setAutoReconnect(false);

    if (esp_pm_configure(&pm_config) == ESP_OK)
    {
      Serial.print("\n light sleep enabled \n");
    }

    esp_light_sleep_start();
  }
}
