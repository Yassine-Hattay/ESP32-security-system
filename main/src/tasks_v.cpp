#include "global_header.h"
#include "camera_var.h"
#include "server_v.h"
#include "email_module.h"
#include "tasks_v.h"

unsigned long start_trans_time;

unsigned long lastCheckTime_h = 0;
unsigned long lastCheckTime = 0; 

TaskHandle_t loop_handle = NULL;
TaskHandle_t live_f_handle = NULL;
TaskHandle_t sending_photo_Handle =NULL; 

void sending_photo_task(void *parameter)
{
  for (;;)
  {

    vTaskDelay(5);
    if (digitalRead(GPIO_2))
    {
      if (connected_internet)
      {

        long int t1 = millis();
        capturePhotoSaveLittleFS();

        sendPhoto();
        long int t2 = millis();
        Serial.print("Time taken: ");
        Serial.print((t2 - t1) / 1000);
        Serial.println(" seconds");
      }
      else
      {

        photo_not_sent = true;
        takeNextPhotoFlag = 1;
        sendNextPackageFlag = 0;

        while (photo_not_sent)
        {
          if (takeNextPhotoFlag)
          {
            takePhoto();
          }

          if (sendNextPackageFlag)
          {
            sendNextPackage();
          }

          if ((millis() - start_trans_time >= timeout_F))
          {
            Serial.println("ESP NOW Timed out or an error ocurred !");

            currentTransmitCurrentPosition = 0;
            currentTransmitTotalPackages = 0;

            if (!connected_router)

            {
              connected_router = connectToWiFi_mod(ssid, password, 10000);
              if (connected_router)
              {

                WiFi.mode(WIFI_MODE_APSTA);
                if (loop_handle != NULL)
                {
                  vTaskDelete(loop_handle);
                  loop_handle = NULL;
                }

                xTaskCreate(
                    loop_f,
                    "loop",
                    3072,
                    NULL,
                    1,
                    &loop_handle);

                connected_internet = checkSMTPService(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
              }
            }
            else
            {

              if (loop_handle != NULL)
              {
                vTaskDelete(loop_handle);
                loop_handle = NULL;
              }

              xTaskCreate(
                  loop_f,
                  "loop",
                  3072,
                  NULL,
                  1,
                  &loop_handle);

              connected_internet = checkSMTPService(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
            }

            break;
          }
        }
      }
    }

    if ((millis() - lastCheckTime >= checkInterval) && (!connected_internet))
    {
      lastCheckTime = millis();

      pm_config = {
          .max_freq_mhz = 240,
          .min_freq_mhz = 160,
          .light_sleep_enable = false};

      esp_pm_configure(&pm_config);

      connected_router = connectToWiFi_mod(ssid, password, 10000);

      if (connected_router)
      {
        setup_server();

        if (loop_handle != NULL)
        {
          vTaskDelete(loop_handle);
          loop_handle = NULL;
        }

        xTaskCreate(
            loop_f,
            "loop",
            3072,
            NULL,
            1,
            &loop_handle);

        WiFi.mode(WIFI_MODE_APSTA);
        connected_internet = checkSMTPService(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
      }
      else
      {

        if (loop_handle != NULL)
        {
          vTaskDelete(loop_handle);
          loop_handle = NULL;
        }

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
  }
  return;
}

void loop_f(void *parameter)
{
  for (;;)
  {
    server.handleClient();
    vTaskDelay(5);

    if (xSemaphoreTake(manual_b_mutex, portMAX_DELAY))
    {
      // Access manual_b safely
      if ((manual_b == true) && (millis() - lastCheckTime_h >= 900000) && (sending_photo_Handle == NULL))
      {
        manual_b = false;
        xTaskCreate(
            sending_photo_task,
            "send_photo",
            100008,
            NULL,
            2,
            &sending_photo_Handle);
      }

      xSemaphoreGive(manual_b_mutex);
    }
  }
  return;
}



static esp_err_t stream_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK)
  { 
    return ESP_FAIL;
  }

  while (true) 
  {
    fb = esp_camera_fb_get();
    if (!fb)
    {
      Serial.println("Camera capture failed");
      ESP.restart();
    }
    else
    {
      if (fb->width > 400)
      {
        if (fb->format != PIXFORMAT_JPEG)
        {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted)
          {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
            ESP.restart();
          }
        }
        else
        {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if (res == ESP_OK)
    {
      size_t hlen = snprintf(reinterpret_cast<char *>(part_buf), 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, reinterpret_cast<const char *>(part_buf), hlen);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, reinterpret_cast<const char *>(_jpg_buf), _jpg_buf_len);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb)
    {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    }
    else if (_jpg_buf)
    {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK)
    {
      return ESP_FAIL;
    }
  }
  return res;
}


void live_f(void *parameter)
{

  lastCheckTime_h = millis();
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 8000;
  config.recv_wait_timeout = 10;
  config.send_wait_timeout = 10;

  httpd_uri_t index_uri = {
      .uri = "/video",
      .method = HTTP_GET,
      .handler = stream_handler,
      .user_ctx = NULL};

  if (httpd_start(&stream_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(stream_httpd, &index_uri);
  }
  while (true)
  {
    vTaskDelay(5);
    if ((millis() - lastCheckTime_h >= 900000))
    {
      if (httpd_stop(stream_httpd) == ESP_OK)
        httpd_unregister_uri_handler(stream_httpd, "/video", HTTP_POST);

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

      vTaskDelete(NULL);
    }
  }
  return;
}
