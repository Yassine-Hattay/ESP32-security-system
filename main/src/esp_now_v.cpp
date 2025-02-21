#include "global_header.h"
#include "camera_var.h"
#include "email_module.h"

bool isPaired = 0;

// for photo name
byte takeNextPhotoFlag = 1;

esp_pm_config_t pm_config;
SemaphoreHandle_t manual_b_mutex;

esp_now_peer_info_t slave;
const esp_now_peer_info_t *peer = &slave;
volatile bool wait_b = true;

// for photo transmit
int currentTransmitCurrentPosition = 0;
int currentTransmitTotalPackages = 0;
byte sendNextPackageFlag = 0;

/* ***************************************************************** */
/* callback when data is sent from Master to Slave                   */
/* ***************************************************************** */
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{

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
void InitESPNow()
{
  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESPNow Init Success");
  }
  else
  {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}



void Set_SLAVE_data(uint8_t mac[6])
{

  memset(&slave, 0, sizeof(slave));

  for (int ii = 0; ii < 6; ++ii)
  {
    slave.peer_addr[ii] = mac[ii];
  }

  slave.encrypt = 0;
}


void sendData(uint8_t *dataArray, uint8_t dataArrayLength)
{
  const uint8_t *peer_addr = slave.peer_addr;

  esp_err_t result = esp_now_send(peer_addr, dataArray, dataArrayLength);
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
      (uint8_t)(currentTransmitTotalPackages >> 8), // Cast to uint8_t after shifting
      (uint8_t)currentTransmitTotalPackages         // Explicit cast to uint8_t
  };
  sendData(message, sizeof(message));
}

void sendNextPackage()
{
  // claer the flag
  sendNextPackageFlag = 0;
  Serial.printf("\n currentTransmitCurrentPosition: %d  \n currentTransmitTotalPackages : %d", currentTransmitCurrentPosition, currentTransmitTotalPackages);
  // if got to AFTER the last package
  if (currentTransmitCurrentPosition == currentTransmitTotalPackages)
  {
    currentTransmitCurrentPosition = 0;
    currentTransmitTotalPackages = 0;
    Serial.print("\n Done submiting files .\n");

    if (connected_router)
    {
      WiFi.mode(WIFI_MODE_APSTA);
      WiFi.reconnect();

      pm_config = {
          .max_freq_mhz = 240,
          .min_freq_mhz = 160,
          .light_sleep_enable = false};

      esp_pm_configure(&pm_config);
    }
    else
    {
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
    takeNextPhotoFlag = 1;
    photo_not_sent = false;
    return;
  } // end if

  // first read the data.
  File file = LittleFS.open(FILE_PHOTO_PATH, FILE_READ);
  if (!file)
  {
    Serial.println("Failed to open file in writing mode");
    photo_not_sent = false;
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
    Serial.println((currentTransmitTotalPackages - 1) * fileDatainMessage);
    fileDataSize = file.size() - ((currentTransmitTotalPackages - 1) * fileDatainMessage);
  }

  Serial.printf(" \n fileDataSize= %d \n", fileDataSize);

  // define message array
  uint8_t messageArray[fileDataSize + 3];
  messageArray[0] = 0x02;

  file.seek(currentTransmitCurrentPosition * fileDatainMessage);
  currentTransmitCurrentPosition++; // set to current (after seek!!!)
  // Serial.println("PACKAGE - " + String(currentTransmitCurrentPosition));

  messageArray[1] = currentTransmitCurrentPosition >> 8;
  messageArray[2] = (byte)currentTransmitCurrentPosition;
  int a = 0; // Initialize the index
  while (a < fileDataSize && file.available())
  {
    messageArray[3 + a] = file.read();
    a++; // Increment the index after reading a byte
  }

  sendData(messageArray, sizeof(messageArray));

  file.close();
}



void deletePeer()
{
  const uint8_t *peer_addr = slave.peer_addr;
  esp_err_t delStatus = esp_now_del_peer(peer_addr);
  Serial.print("Slave Delete Status: ");
  if (delStatus == ESP_OK)
  {
    // Delete success
    Serial.println("Success");
  }
}

bool manageSlave(int channel_v)
{

  if (DELETEBEFOREPAIR)
  {
    deletePeer();
  }

  slave.channel = channel_v;

  Serial.print("Slave Status: ");
  const uint8_t *peer_addr = slave.peer_addr;
  // check if the peer exists
  bool exists = esp_now_is_peer_exist(peer_addr);
  if (exists)
  {
    // Slave already paired.
    Serial.println("Already Paired");
    return true;
  }
  else
  {
    // Slave not paired, attempt pair
    esp_err_t addStatus1 = esp_now_add_peer(peer);
    if (addStatus1 == ESP_OK)
    {
      // Pair success
      Serial.println("Pair success");
      return true;
    }
  }
  return true;
}



void takePhoto()
{

  pm_config = {
      .max_freq_mhz = 240,
      .min_freq_mhz = 160,
      .light_sleep_enable = false};

  esp_pm_configure(&pm_config);

  start_trans_time = millis();

  WiFi.disconnect();
  WiFi.mode(WIFI_MODE_STA);

  WiFi.printDiag(Serial); // Uncomment to verify channel number before
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(ESP_CHANNEL, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  WiFi.printDiag(Serial);

  takeNextPhotoFlag = 0;

  if ((hour() > 17) || (hour() < 7))
  {
    digitalWrite(ONBOADLED, HIGH);
  }

  delay(adjustment_time);
  camera_fb_t *fb = NULL;

  for (int a = 0; a < frames_skipped; a++)
  {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
  fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    ESP.restart();
  }

  digitalWrite(ONBOADLED, LOW);

  Serial.printf("Picture file name: %s\n", FILE_PHOTO_PATH);
  File file = LittleFS.open(FILE_PHOTO_PATH, FILE_WRITE);

  Serial.printf("Picture file name: %s\n", FILE_PHOTO_PATH);

  if (!file)
  {
    Serial.println("Failed to open file in writing mode");
    photo_not_sent = false;
    return;
  }
  else
  {
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
