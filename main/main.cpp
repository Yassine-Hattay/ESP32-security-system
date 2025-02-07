#include <ESP_Mail_Client.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include <WebServer.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <FS.h>  
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>  
#include "esp_system.h"
#include "esp_pm.h"
#include "Arduino.h"


#define emailSenderAccount    "yassinebeebotte@gmail.com"
#define emailSenderPassword   "gsti ffmp djsg kfhf"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "Mouvement détecté"
#define emailRecipient1        "hattayyassine519@gmail.com"
#define emailRecipient2      "yassine.hattay@etudiant-fst.utm.tn"

#define ONBOADLED 4

#define ESP_CHANNEL 13
// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define GPIO_2  2
#define GPIO_12  12
#define PART_BOUNDARY "123456789000000000000987654321"

#define FILE_PHOTO "photo.jpg"
#define FILE_PHOTO_PATH "/photo.jpg"

#define fileDatainMessage 240.0
// Global copy of slave
#define DELETEBEFOREPAIR 1


const char* ssid = "Orange-066C";
const char* password = "GMA6ABLMG87";


static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

unsigned long timeout_F = 20000;
unsigned long checkInterval = 1800000;  // Interval for checking (30 minutes = 1800000 ms)
unsigned long lastCheckTime = 0;  // Last time check was done
unsigned long lastCheckTime_h = 0;  // Last time check was done
unsigned long start_trans_time ;

esp_pm_config_t pm_config ;
SemaphoreHandle_t manual_b_mutex;


uint8_t mac[6] = {0xCC, 0x50, 0xE3, 0x42, 0x2A, 0x61};  

WebServer server(80);

SMTPSession smtp;
wifi_mode_t mode;

httpd_handle_t stream_httpd = NULL;

TaskHandle_t sending_photo_Handle = NULL ;
TaskHandle_t loop_handle = NULL;
TaskHandle_t live_f_handle = NULL;

WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 3600, 3600000);

framesize_t framesize = framesize_t::FRAMESIZE_SXGA;
int adjustment_time = 1 * 1000 ;
int jpeg_quality_v = 10;
int frames_skipped = 10 ;
int xclk_s = 20000000;
int fb_count_v = 1 ;
int8_t max_tx_power ;

int32_t channel;
esp_now_peer_info_t slave;
const esp_now_peer_info_t *peer = &slave;
esp_err_t addStatus ;

bool ALS_b = false ;
bool connected_internet = false ;
bool connected_router = false ;
volatile bool wait_b = true;
bool photo_not_sent;
bool manual_b = true;
// for esp now connect
bool isPaired = 0;
bool send_error_b = true ;

// for photo name
byte takeNextPhotoFlag = 1;

// for photo transmit
int currentTransmitCurrentPosition = 0;
int currentTransmitTotalPackages = 0;
byte sendNextPackageFlag = 0;

// for connection type
int i = 0;

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

void deletePeer() {
  const esp_now_peer_info_t *peer = &slave;
  const uint8_t *peer_addr = slave.peer_addr;
  esp_err_t delStatus = esp_now_del_peer(peer_addr);
  Serial.print("Slave Delete Status: ");
  if (delStatus == ESP_OK) {
    // Delete success
    Serial.println("Success");
  } 
}

bool manageSlave(int channel_v) {

    if (DELETEBEFOREPAIR) {
      deletePeer();
    }

    slave.channel = channel_v;  

    Serial.print("Slave Status: ");
    const uint8_t *peer_addr = slave.peer_addr;
    // check if the peer exists
    bool exists = esp_now_is_peer_exist(peer_addr);
    if ( exists) {
      // Slave already paired.
      Serial.println("Already Paired");
      return true;
    } else {
      // Slave not paired, attempt pair
      esp_err_t addStatus = esp_now_add_peer(peer);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
        return true;
      } 
    }
    return true ;
  }

/* ***************************************************************** */
/* SEND NEXT PACKAGE                                                 */
/* ***************************************************************** */

void sendData(uint8_t * dataArray, uint8_t dataArrayLength) {
  const uint8_t *peer_addr = slave.peer_addr;

  esp_err_t result = esp_now_send(peer_addr, dataArray, dataArrayLength);
  if(result != ESP_OK)
  {
    send_error_b = true ;
  }
  
}

void sendNextPackage()
{
  // claer the flag
  sendNextPackageFlag = 0;
  Serial.printf("\n currentTransmitCurrentPosition: %d  \n currentTransmitTotalPackages : %d",currentTransmitCurrentPosition,currentTransmitTotalPackages);
  // if got to AFTER the last package
  if (currentTransmitCurrentPosition == currentTransmitTotalPackages)
  {
    currentTransmitCurrentPosition = 0;
    currentTransmitTotalPackages = 0;
    Serial.print("\n Done submiting files .\n");
    
    if(connected_router)
    {
      WiFi.mode(WIFI_MODE_APSTA);
      pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 160,
        .light_sleep_enable = false
            };
  

      esp_pm_configure(&pm_config);

    }
    else{
    pm_config = {
          .max_freq_mhz = 40,
          .min_freq_mhz = 10,
          .light_sleep_enable = false
              };
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.setAutoReconnect(false);

    if(esp_pm_configure(&pm_config) == ESP_OK)
    {
    Serial.print("\n light sleep enabled \n");
    } 

    esp_light_sleep_start();

    }
    takeNextPhotoFlag = 1 ;
    photo_not_sent = false ;
    return ;
  } //end if

  //first read the data.
  File file = LittleFS.open(FILE_PHOTO_PATH, FILE_READ);
  if (!file) {
    Serial.println("Failed to open file in writing mode");
    photo_not_sent = false ;
    return;
  }

  // set array size.
  int fileDataSize = fileDatainMessage;
  // if its the last package - we adjust the size !!!
  if (currentTransmitCurrentPosition == currentTransmitTotalPackages - 1)
  {
    Serial.println("*************************");
    Serial.println(file.size());
    Serial.println(currentTransmitTotalPackages - 1);
    Serial.println((currentTransmitTotalPackages - 1)*fileDatainMessage);
    fileDataSize = file.size() - ((currentTransmitTotalPackages - 1) * fileDatainMessage);
  }

  Serial.printf(" \n fileDataSize= %d \n" ,fileDataSize);

  // define message array
  uint8_t messageArray[fileDataSize + 3];
  messageArray[0] = 0x02;


  file.seek(currentTransmitCurrentPosition * fileDatainMessage);
  currentTransmitCurrentPosition++; // set to current (after seek!!!)
  //Serial.println("PACKAGE - " + String(currentTransmitCurrentPosition));

  messageArray[1] = currentTransmitCurrentPosition >> 8;
  messageArray[2] = (byte) currentTransmitCurrentPosition;
  int i = 0; // Initialize the index
    while (i < fileDataSize && file.available()) {
        messageArray[3 + i] = file.read();
        i++; // Increment the index after reading a byte
    }

  sendData(messageArray, sizeof(messageArray));
  
  file.close();

}

void startTransmit()
{
  Serial.println("Starting transmit");
  File file = LittleFS.open(FILE_PHOTO_PATH, FILE_READ);
  
  Serial.println(file.size());
  int fileSize = file.size();
  file.close();
  wait_b = true;

  currentTransmitTotalPackages = ceil(fileSize / fileDatainMessage);
  Serial.println(currentTransmitTotalPackages);
    uint8_t message[] = {
        0x01, 
        (uint8_t)(currentTransmitTotalPackages >> 8),  // Cast to uint8_t after shifting
        (uint8_t)currentTransmitTotalPackages  // Explicit cast to uint8_t
    };
  sendData(message, sizeof(message));
}

void takePhoto()
{ 

  pm_config = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 160,
    .light_sleep_enable = false
        };


  esp_pm_configure(&pm_config);

  send_error_b = false ;
  start_trans_time = millis();

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESP_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial);

  isPaired = manageSlave(ESP_CHANNEL);

  takeNextPhotoFlag = 0;
  if ((hour() > 17) || ( hour() < 7) ) 
  {  digitalWrite(ONBOADLED,HIGH);
  }

  delay(adjustment_time);
  camera_fb_t * fb = NULL;

  for (int i = 0; i < frames_skipped; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  fb = NULL;  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
ESP.restart();
  }  

  digitalWrite(ONBOADLED,LOW);

  Serial.printf("Picture file name: %s\n", FILE_PHOTO_PATH);
  File file = LittleFS.open(FILE_PHOTO_PATH, FILE_WRITE);

  Serial.printf("Picture file name: %s\n",FILE_PHOTO_PATH);

  if (!file) {
    Serial.println("Failed to open file in writing mode");
    photo_not_sent = false ;
    return;
  }
  else {
    file.write(fb->buf, fb->len); 
    Serial.print("The picture has been saved in ");
    Serial.print(FILE_PHOTO_PATH);
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  file.close();
  esp_camera_fb_return(fb);


  if (isPaired)
    startTransmit();


}

void smtpCallback(SMTP_Status status);

static esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t * _jpg_buf = NULL;
  char * part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if(res != ESP_OK){
    return ESP_FAIL;
  }

  while(true){
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      ESP.restart();
    } else {
      if(fb->width > 400){
        if(fb->format != PIXFORMAT_JPEG){
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if(!jpeg_converted){
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
            ESP.restart();
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if(res == ESP_OK){
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if(res == ESP_OK){
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if(fb){
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if(_jpg_buf){
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if(res != ESP_OK){
      return ESP_FAIL;
    }
  }
  return res;
}

void live_f(void * parameter){

  lastCheckTime_h = millis();
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 8000;
  config.recv_wait_timeout = 10 ;
  config.send_wait_timeout = 10 ;

  httpd_uri_t index_uri = {
    .uri       = "/video",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
 

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &index_uri);
  }
   while (true)
  {
  vTaskDelay(5);
  if (( millis() - lastCheckTime_h >= 900000 )) {
      if(httpd_stop(stream_httpd) == ESP_OK)
      httpd_unregister_uri_handler(stream_httpd,"/video",HTTP_POST);
  
      if(sending_photo_Handle != NULL) {
        vTaskDelete(sending_photo_Handle);
        sending_photo_Handle = NULL;
      }

      xTaskCreate(
          sending_photo_task,    
          "send_photo",  
          100008 ,           
          NULL,           
          2,              
        &sending_photo_Handle            
        );
  
      vTaskDelete(NULL);
  }
  }
  return;
}

String SendHTML(){
  
String ptr = "<!DOCTYPE html> <html>\n";
ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
ptr += "<meta http-equiv=\"refresh\" content=\"30\">\n"; // Adds auto-refresh every 30 seconds
ptr += "<script>\n";
// Use the 'pageshow' event to force a reload if loaded from the cache
ptr += "window.addEventListener('pageshow', function(event) {\n";
ptr += "  if (event.persisted) {\n"; // Check if the page is loaded from the cache
ptr += "    window.location.reload(true);\n"; // Force a full reload
ptr += "  }\n";
ptr += "});\n";
ptr += "</script>\n";
ptr += "<title>Control Panel</title>\n";
ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
ptr += ".button {display: inline-block;width: 150px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 20px;margin: 10px auto;cursor: pointer;border-radius: 4px;}\n";
ptr += ".button-on {background-color: #3498db;}\n";
ptr += ".button-on:active {background-color: #2980b9;}\n";
ptr += ".button-off {background-color: #34495e;}\n";
ptr += ".button-off:active {background-color: #2c3e50;}\n";
ptr += "label {font-size: 18px;color: #444;margin-right: 10px; display: inline-block;}\n";
ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
ptr += ".bottom-container {margin-top: 50px;}\n"; // New class to control spacing for the bottom button
ptr += "</style>\n";

ptr += "<script>\n";
ptr += "function liveFeed() {\n";
ptr += "  var xhr = new XMLHttpRequest();\n";
ptr += "  xhr.open('GET', '/manual', true);\n";
ptr += "  xhr.send();\n";  // Send a GET request to the C++ server
ptr += "  window.location.href = 'http://' + window.location.hostname + ':8000/video';\n";  // Redirect to the video stream
ptr += "}\n";

ptr += "function automated() {\n";
ptr += "  window.location.href = 'http://' + window.location.hostname + '/automated';\n";  // Redirect to a confirmation page
ptr += "}\n"; 

ptr += "</script>\n";

ptr += "</head>\n";
ptr += "<body>\n";

// Title
ptr += "<h1>Automated Mode deactivated</h1>\n";

// Container div for Live Feed button and resolution dropdown
ptr += "<div style=\"display: inline-block;\">\n";

// Live Feed Button
ptr += "<button class=\"button button-on\" onclick=\"liveFeed()\">Live Feed</button>\n";

ptr += "</div>\n";
// Small margin to separate from previous section
ptr += "<div class=\"bottom-container\">\n";
ptr += "<button class=\"button button-on\" onclick=\"automated()\">Automated Mode</button>\n";

// Automated Mode Button closer to the content but still at the bottom

ptr += "</div>\n";

ptr += "</body>\n";
ptr += "</html>\n";


return ptr;
}

void capturePhotoSaveLittleFS( void ) {

  Serial.printf("\n time : %d:%d",hour(),minute());

  if ((hour() > 17) || ( hour() < 7) ) 
  {  digitalWrite(ONBOADLED,HIGH);
  }

  delay(adjustment_time);
  camera_fb_t* fb = NULL;
  for (int i = 0; i < frames_skipped; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  fb = NULL;  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
  ESP.restart();
  }  

  digitalWrite(ONBOADLED,LOW);
  
  Serial.printf("Picture file name: %s\n", FILE_PHOTO_PATH);
  File file = LittleFS.open(FILE_PHOTO_PATH, FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file in writing mode");
    return ;
  }
  else {
    file.write(fb->buf, fb->len); 
    Serial.print("The picture has been saved in ");
    Serial.print(FILE_PHOTO_PATH);
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  file.close();
  esp_camera_fb_return(fb);
}

void smtpCallback(SMTP_Status status){
  Serial.println(status.info());

  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++){
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

   smtp.sendingResult.clear();
  }
}
void sendPhoto( void ) {

  
  smtp.debug(1);

  smtp.callback(smtpCallback);

  Session_Config config;
  
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 0;
  config.time.day_light_offset = 1;

  config.server.host_name = smtpServer;
  config.server.port = smtpServerPort;
  config.login.email = emailSenderAccount;
  config.login.password = emailSenderPassword;
  config.login.user_domain = "";

  SMTP_Message message;

  message.enable.chunking = true;

  message.sender.name = "ESP32-CAM";
  message.sender.email = emailSenderAccount;

  message.subject = emailSubject;
  message.addRecipient("yassine",emailRecipient1);
  message.addRecipient("yassine",emailRecipient2);

  String htmlMsg = "<h2>Photo captured with ESP32-CAM and attached in this email.</h2>";
  message.html.content = htmlMsg.c_str();
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_qp;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_high;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  SMTP_Attachment att;


  att.descr.filename = FILE_PHOTO;
  att.descr.mime = "image/png"; 
  att.file.path = FILE_PHOTO_PATH;
  att.file.storage_type = esp_mail_file_storage_type_flash;
  att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

  message.addAttachment(att);

  if(!smtp.connect(&config))
     {connected_internet = false ;}                                      //  mdse
  if(!MailClient.sendMail(&smtp, &message, false))
     {connected_internet = false ;}                                      //  mdse
}

void handleManualMode() 
{ 
   // Suspend interfering task

  Serial.println("Manual Mode Activated");
  server.send(200); 

  lastCheckTime_h = millis();

  if (xSemaphoreTake(manual_b_mutex, portMAX_DELAY)) {
    // Access manual_b safely
    manual_b = true;

    xSemaphoreGive(manual_b_mutex);
  }

  if(live_f_handle != NULL) 
  {
  vTaskDelete(live_f_handle);
  live_f_handle = NULL;
  if(httpd_stop(stream_httpd) == ESP_OK)
    {
      httpd_unregister_uri_handler(stream_httpd,"/video",HTTP_POST);
      Serial.print("\n live stream stopped .\n");    
    }
  }
  
  if(sending_photo_Handle != NULL) {
    vTaskDelete(sending_photo_Handle);
    sending_photo_Handle = NULL;
  }

  xTaskCreate(
    live_f,    
    "live_feed",  
    100008 ,           
    NULL,           
    2,              
  &live_f_handle            
  );
  
}

bool connectToWiFi(const char* ssid, const char* password, unsigned long timeout) {
  
  WiFi.begin(ssid, password); // Start connecting to WiFi
  WiFi.setAutoReconnect(true);

  Serial.print("Connecting to WiFi");

  unsigned long startTime = millis(); // Record the start time

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    // Check if the timeout has been exceeded
    if (millis() - startTime >= timeout) {
      Serial.println("\nWiFi connection timeout.");
      return false; // Connection failed within the timeout
    }
  }

  channel = getWiFiChannel(ssid);
  Serial.println("\nWiFi connected!");
  return true; // Successfully connected
}

bool checkSMTPService(const char* server, uint16_t port, const char* email, const char* password) {
  Session_Config sessionConfig;
  sessionConfig.server.host_name = server;
  sessionConfig.server.port = port;
  sessionConfig.login.email = email;
  sessionConfig.login.password = password;

  // Attempt to connect to the SMTP server
  Serial.printf("Checking SMTP server %s on port %d...\n", server, port);
  if (smtp.connect(&sessionConfig, false)) { // 'false' means don't log in
    Serial.println("SMTP server is available!");
    smtp.closeSession(); // Close the connection
    return true;
  } else {
    Serial.printf("Failed to connect to SMTP server. Error: %s\n", smtp.errorReason().c_str());
    return false;
  }
}

bool connectToWiFi_mod(const char* ssid, const char* password, unsigned long timeout) {
  
  WiFi.begin(ssid, password); // Start connecting to WiFi
  WiFi.setAutoReconnect(true);

  Serial.print("Connecting to WiFi");

  unsigned long startTime = millis(); // Record the start time

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if(digitalRead(GPIO_2))
    {return false ;}
    // Check if the timeout has been exceeded
    if (millis() - startTime >= timeout) {
      Serial.println("\nWiFi connection timeout.");
      return false; // Connection failed within the timeout
    }
  }

  channel = getWiFiChannel(ssid);
  Serial.println("\nWiFi connected!");
  return true; // Successfully connected
}

void sending_photo_task(void * parameter)
{
  for(;;){
    
        vTaskDelay(5);

        if(digitalRead(GPIO_2))
                {
            if(connected_internet)
                    { 
                      

                      long int t1 = millis();
                      capturePhotoSaveLittleFS();

                      sendPhoto();
                      long int t2 = millis();
                      Serial.print("Time taken: "); 
                      Serial.print((t2-t1)/1000); 
                      Serial.println(" seconds");
                      

                    }
            else { 
                   
                    photo_not_sent = true ;
                    takeNextPhotoFlag = 1 ;
                    sendNextPackageFlag = 0 ;

                    while(photo_not_sent)
                  { 
                        if (takeNextPhotoFlag)
                              {takePhoto();}
                        
                        if (sendNextPackageFlag)
                              {sendNextPackage();}   

                        if ( (millis() - start_trans_time >= timeout_F)  || send_error_b) 
                        {
                          Serial.println("ESP NOW Timed out or an error ocurred !");

                          if(!connected_router)
                          {                         
                            connected_router = connectToWiFi_mod(ssid,password,10000);
                            if(connected_router){

                            WiFi.mode(WIFI_MODE_APSTA);
                            if(loop_handle != NULL) {
                              vTaskDelete(loop_handle);
                              loop_handle = NULL;
                            }

                            xTaskCreate(
                              loop_f,    
                              "loop",   
                              3072  ,            
                              NULL,            
                              1,              
                            &loop_handle            
                            );
 
                            connected_internet = checkSMTPService(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword) ;
                            }
                          }
                          else{  

                            if(loop_handle != NULL) {
                              vTaskDelete(loop_handle);
                              loop_handle = NULL;
                            }

                            xTaskCreate(
                              loop_f,    
                              "loop",  
                              3072  ,           
                              NULL,           
                              1,              
                            &loop_handle            
                            );

                            connected_internet = checkSMTPService(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword) ;
                          }



                          break;
                        }
                    }    
            }
      }

      if ((millis() - lastCheckTime >= checkInterval) && (!connected_internet) ) 
                      {
                        lastCheckTime = millis();

                        pm_config = {
                              .max_freq_mhz = 240,
                              .min_freq_mhz = 160,
                              .light_sleep_enable = false
                                  };
                        

                        esp_pm_configure(&pm_config);
                      

                        connected_router = connectToWiFi_mod(ssid,password,10000);
                        
                        if(connected_router)
                          { 
                            setup_server();

                            if(loop_handle != NULL) {
                              vTaskDelete(loop_handle);
                              loop_handle = NULL;
                            }

                            xTaskCreate(
                              loop_f,    
                              "loop",  
                              3072  ,           
                              NULL,           
                              1,              
                            &loop_handle            
                            );

                            WiFi.mode(WIFI_MODE_APSTA);
                            connected_internet = checkSMTPService(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword) ;
                          }
                          else{

                            if(loop_handle != NULL) {
                              vTaskDelete(loop_handle);
                              loop_handle = NULL;
                            }


                                    pm_config = {
                                      .max_freq_mhz = 40,
                                      .min_freq_mhz = 10,
                                      .light_sleep_enable = false
                                          };

                                WiFi.disconnect(true);
                                WiFi.mode(WIFI_MODE_STA);
                                WiFi.setAutoReconnect(false);

                                if(esp_pm_configure(&pm_config) == ESP_OK)
                                {
                                Serial.print("\n light sleep enabled \n");
                                } 

                              esp_light_sleep_start();
                              }
                      }
  

}
return ;
}

void handleAutomatedMode() {
  Serial.println("Automated Mode Activated");
  String ptr = "<!DOCTYPE html>";
    ptr += "<html>";
    ptr += "<head><meta charset='UTF-8'><title>Automated Mode</title></head>";
    ptr += "<body>";
    ptr += "<h1>Automated mode activated (❁´◡`❁)</h1>";
    ptr += "</body>";
    ptr += "</html>";
  server.send(200, "text/html", ptr);

  if(sending_photo_Handle != NULL) {
      vTaskDelete(sending_photo_Handle);
      sending_photo_Handle = NULL;
    }

  if(live_f_handle != NULL) 
  {
  vTaskDelete(live_f_handle);
  live_f_handle = NULL;
  if(httpd_stop(stream_httpd) == ESP_OK)
    {
      httpd_unregister_uri_handler(stream_httpd,"/video",HTTP_POST);
      Serial.print("\n live stream stopped .\n");    
    }
  }

   xTaskCreate(
    sending_photo_task,    
    "send_photo",  
    100008 ,           
    NULL,           
    2,              
  &sending_photo_Handle            
  );

}

void handle_OnConnect() 
 {
  Serial.println("you are connected !");
  server.send(200, "text/html", SendHTML()); 

if(sending_photo_Handle != NULL) {
    vTaskDelete(sending_photo_Handle);
    sending_photo_Handle = NULL;
  }

if(live_f_handle != NULL) 
{
  vTaskDelete(live_f_handle);
  live_f_handle = NULL;
  if(httpd_stop(stream_httpd) == ESP_OK)
    {
      httpd_unregister_uri_handler(stream_httpd,"/video",HTTP_POST);
      Serial.print("\n live stream stopped .\n");    
    }
  }

  }

void wait_smiley()
{ server.send(200, "text/html", R"(
<html>
  <head>
    <meta charset='UTF-8'>
    <meta http-equiv='refresh' content='1'>
    <style>
      .dots {
        display: inline-block;
        font-size: 24px;
        letter-spacing: 2px;
      }
      .dots::after {
        content: ' .';
        animation: dots 1.5s steps(5, end) infinite;
      }
      @keyframes dots {
        0%, 20% {
          content: ' .';
        }
        40% {
          content: ' ..';
        }
        60% {
          content: ' ...';
        }
        80%, 100% {
          content: ' ....';
        }
      }
    </style>
  </head>
  <body>
    Plz wait for a few seconds (●'◡'●)<span class="dots"></span>
  </body>
</html>
)");
}


void Set_SLAVE_data(uint8_t mac[6]) {
  
    memset(&slave, 0, sizeof(slave));

    for (int ii = 0; ii < 6; ++ii ) {
      slave.peer_addr[ii] = mac[ii];  
    }

    slave.encrypt = 0;      

} 

void initCamera()
{
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = xclk_s;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  config.frame_size = framesize;
  config.jpeg_quality = jpeg_quality_v;
  config.fb_count = fb_count_v;
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
 
 sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);    
  s->set_contrast(s, 0);      
  s->set_saturation(s, 0);    
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);      
  s->set_awb_gain(s, 1);      
  s->set_wb_mode(s, 0);       
  s->set_exposure_ctrl(s, 1); 
  s->set_aec2(s, 0);          
  s->set_ae_level(s, 0);     
  s->set_aec_value(s, 300);   
  s->set_gain_ctrl(s, 1);     
  s->set_agc_gain(s, 0);      
  s->set_gainceiling(s, (gainceiling_t)0);  
  s->set_bpc(s, 0);           
  s->set_wpc(s, 1);           
  s->set_raw_gma(s, 1);       
  s->set_lenc(s, 1);          
  s->set_hmirror(s, 0);       
  s->set_vflip(s, 0);         
  s->set_dcw(s, 1);           
  s->set_colorbar(s, 0);
}




/* ***************************************************************** */
/* callback when data is sent from Master to Slave                   */
/* ***************************************************************** */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

 if (currentTransmitTotalPackages)
  { 
    sendNextPackageFlag = 1;
    if (status != ESP_NOW_SEND_SUCCESS)
      currentTransmitCurrentPosition--;
  }

}






/* ***************************************************************** */
/* Init ESP Now with fallback                                        */
/* ***************************************************************** */
void InitESPNow() {
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

void setup_server()
{ 
  
  Serial.print("Go to: http://");
  Serial.print(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/manual", handleManualMode);
  server.on("/automated", handleAutomatedMode);

  server.onNotFound(wait_smiley);
  server.begin();

  
  }


void loop_f(void * parameter)
{
  for(;;)
  {      
        server.handleClient();
        vTaskDelay(5);

        if (xSemaphoreTake(manual_b_mutex, portMAX_DELAY)) {
          // Access manual_b safely 
          if( (manual_b == true) && ( millis() - lastCheckTime_h >= 900000 ) && (sending_photo_Handle == NULL) )
          {
          manual_b = false ; 
          xTaskCreate(
              sending_photo_task,     
              "send_photo",  
              100008 ,           
              NULL,           
              2,              
            &sending_photo_Handle            
            );
          }
      
          xSemaphoreGive(manual_b_mutex);
        }
  }
  return ;
}

extern "C" void app_main()
{  

  Serial.begin(115200);
  Serial.println();
  
  initArduino();

  pinMode(GPIO_2,INPUT_PULLDOWN);
  pinMode(GPIO_12,OUTPUT);
  pinMode(ONBOADLED,OUTPUT);


  digitalWrite(ONBOADLED,LOW);

  Serial.print("\n Setup ! \n");

  
  digitalWrite(GPIO_12,HIGH);
  delay(1000);
  digitalWrite(GPIO_12,LOW);

  WiFi.mode(WIFI_MODE_APSTA);

  connected_router = connectToWiFi(ssid,password,10000);
  
  connected_router = true ; //remove

  connected_internet = false ; //remove

  while(!LittleFS.begin()) {
    LittleFS.format();
  }

  Serial.print("STA MAC: "); 
  Serial.println(WiFi.macAddress());

  InitESPNow();

  esp_now_register_send_cb(OnDataSent);


  Set_SLAVE_data(mac);

  initCamera();
  capturePhotoSaveLittleFS();

  // Configure timer wake-up (15 seconds)
  esp_sleep_enable_timer_wakeup(15 * 1000000);

  // Configure external wake-up on GPIO2 (HIGH = 1)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);

  manual_b_mutex = xSemaphoreCreateMutex();

  if( connected_router)
    {
      pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 160,
        .light_sleep_enable = false
            };


  esp_pm_configure(&pm_config);
  

  setup_server();

  xTaskCreate(
    loop_f,    
    "loop",  
    3072  ,           
    NULL,           
    1,              
  &loop_handle            
  );

    if(false) //remove
  connected_internet = checkSMTPService(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword) ;

  if(connected_internet) // change   if(connected_internet)

  {
    
    timeClient.begin(); 
    //Start NTP client to fetch time
    timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    setTime(epochTime); // Sets time for TimeLib functions

    timeClient.end();

    capturePhotoSaveLittleFS();
    sendPhoto();
    }

  }
  else{

    if(sending_photo_Handle != NULL) {
      vTaskDelete(sending_photo_Handle);
      sending_photo_Handle = NULL;
    }

   xTaskCreate(
    sending_photo_task,    
    "send_photo",  
    100008 ,           
    NULL,           
    2,              
  &sending_photo_Handle            
  );

pm_config = {
          .max_freq_mhz = 40,
          .min_freq_mhz = 10,
          .light_sleep_enable = false
              };

    WiFi.disconnect(true);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.setAutoReconnect(false);

    if(esp_pm_configure(&pm_config) == ESP_OK)
    {
    Serial.print("\n light sleep enabled \n");
    } 

  esp_light_sleep_start();
    
  }



}

