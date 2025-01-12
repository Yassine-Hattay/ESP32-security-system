#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <espnow.h>
#include <SD.h>
#include "LittleFS.h"
#include <NTPClient.h>
#include <TimeLib.h> // Include the TimeLib library

#define SPI_20MHZ_SPEED SD_SCK_MHZ(20)
#define BUFFER_SIZE 15720  

// WiFi credentials
const char* ssid = "Orange-066C";
const char* password = "GMA6ABLMG87";

// NTP settings
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 3600, 3600000);  // Offset: UTC+1 (3600 seconds)

// File and async web server setup
File file;
AsyncWebServer server(80);

// Transmit status variables
int currentTransmitCurrentPosition = 0;
int currentTransmitTotalPackages = 0;
bool fileReceivingStarted = false; // Flag to check if receiving has started
bool fileTransmissionComplete = false; // Flag to check if transmission is complete
unsigned long startTime = 0; // Declare the startTime variable

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

// Initialize ESP-NOW
void InitESPNow() {
  if (esp_now_init() == 0) {
    Serial.println("ESPNow Initialized Successfully");
  } else {
    Serial.println("ESPNow Initialization Failed. Restarting...");
    ESP.restart();
  }
}

String processor(const String& var) {
  if (var == "IMAGE_URL") {
    return F("/moon.jpg"); // Dynamically set the image URL
  }
  return String();
}

void handleHome(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server", "ESP Async Web Server");

  // Start HTML document
  response->print("<!DOCTYPE html><html><head><title>Select Date</title>");
  response->print("<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  response->print("<style>body { font-family: Arial, sans-serif; }</style>");
  response->print("</head><body>");
  response->print("<h1>Available Dates for Photos</h1>");

  // Get the available dates by scanning SD card for folders
  File root = SD.open("/photos");  // Open the directory for scanning
  if (root && root.isDirectory()) {
    File dir = root.openNextFile();
    while (dir) {  // Iterate through the directory
      if (dir.isDirectory()) {
        String date = dir.name();
        response->printf("<a href=\"/photos/%s\">%s</a><br>", date.c_str(), date.c_str());
      }
      dir = root.openNextFile();
    }
  }
  root.close();

  response->print("</body></html>");
  request->send(response);
}

// Serve images for a selected date
void handleDatePhotos(AsyncWebServerRequest *request) {
  String date = request->pathArg(0);
  String folderPath = "/photos/" + date;
  String html = "<!DOCTYPE html><html><head><title>Photos from " + date + "</title>";
  html += "<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<style>body { font-family: Arial, sans-serif; }</style>";
  html += "</head><body>";
  html += "<h1>Photos from " + date + "</h1>";

  File root = SD.open(folderPath);  // Open the directory for scanning
bool foundFiles = false;  // Flag to check if files were found

if (root && root.isDirectory()) {
  File dir = root.openNextFile();
  while (dir) {  // Iterate through the files in the directory
    if (!dir.isDirectory()) {
      String imageUrl = "/photos/" + date + "/" + dir.name();
      html += "<div><a href=\"" + imageUrl + "\" target=\"_blank\">";
      html += "<img src=\"" + imageUrl + "\" width=\"200\" alt=\"" + dir.name() + "\"/></a></div>";
      
      // Mark that a file was found
      foundFiles = true;
    }
    dir = root.openNextFile();
  }
}

root.close();

// Print message to serial terminal based on whether files were found
if (foundFiles) {
  Serial.println("Files found for date: " + date);
} else {
  Serial.println("No files found for date: " + date);
}

  html += "</body></html>";
  
  request->send(200, "text/html", html);
}

// Serve the photo file
void handlePhotoFile(AsyncWebServerRequest *request) {
  String filePath = "/photos" + request->url();
  String littlefsPath = "/moon_littlefs.jpg";  // Set destination in LittleFS

  // Open SD file
  File sdFile = SD.open(filePath, "r");
  if (!sdFile) {
    request->send(404, "text/plain", "File not found on SD card");
    return;
  }

  // Create or open the file in LittleFS
  File littlefsFile = LittleFS.open(littlefsPath, "w");
  if (!littlefsFile) {
    request->send(500, "text/plain", "Failed to write to LittleFS");
    sdFile.close();
    return;
  }

  // Copy file from SD to LittleFS in chunks
  byte buffer[512];
  while (sdFile.available()) {
    size_t bytesRead = sdFile.read(buffer, sizeof(buffer));
    littlefsFile.write(buffer, bytesRead);
  }

  sdFile.close();
  littlefsFile.close();

  // Serve the image from LittleFS
  request->send(LittleFS, littlefsPath, "image/jpeg");
}

// Event handler to send status updates to the client
void handleEventSource(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/event-stream");

  // Notify client when receiving has started
  if (!fileReceivingStarted) {
    response->print("data: Currently receiving file...\n\n");
    fileReceivingStarted = true; // Set flag to prevent further updates
  }

  // Notify client when file transfer is complete
  if (!fileTransmissionComplete && currentTransmitCurrentPosition == currentTransmitTotalPackages) {
    response->print("data: File transmission complete!\n\n");
    fileTransmissionComplete = true; // Set flag to prevent further updates
  }

  request->send(response);
}

// Validate folder name format as "YYYY-MM-DD"
bool isValidDateFolder(String folderName) {
  if (folderName.length() == 10 && folderName[4] == '-' && folderName[7] == '-') {
    return true;
  }
  return false;
}

// Delete all files and subdirectories in a folder
void deleteFolderContents(String folderPath) {
  File dir = SD.open(folderPath);
  if (dir && dir.isDirectory()) {
    File entry = dir.openNextFile();
    while (entry) {
      if (entry.isDirectory()) {
        deleteFolderContents(folderPath + "/" + entry.name()); // Recursive deletion
        SD.rmdir(folderPath + "/" + entry.name());
      } else {
        SD.remove(folderPath + "/" + entry.name()); // Remove file
      }
      entry = dir.openNextFile();
    }
  }
}

void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len) {
  static byte buffer[BUFFER_SIZE];  // Use larger buffer for faster data handling
  int bufferIndex = 0;

  // Update current date using NTP
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  setTime(epochTime); // Sets time for TimeLib functions

  String currentDate = String(year()) + "-" + String(month()) + "-" + String(day());
  String folderPath = "/photos/" + currentDate;

  

  switch (*data++) {
    case 0x01: // Start of file transmission
    { 
      if (!SD.exists(folderPath)) {
      if (SD.mkdir(folderPath)) {
        Serial.println("Folder created: " + folderPath);
      } else {
        Serial.println("Failed to create folder: " + folderPath);
      }
      } else {
        Serial.println("Folder already exists: " + folderPath);
      }
      server.end();

      WiFi.disconnect();
      WiFi.mode(WIFI_STA);
      wifi_set_channel(1); // WiFi channel switching

      currentTransmitCurrentPosition = 0; 
      currentTransmitTotalPackages = (*data++) << 8 | *data;

      // Open the file for writing with the current date in folder
      file = SD.open(folderPath + "/moon_" + String(millis()) + ".jpg", "w");
      if (!file) {
        Serial.println("Error opening file for writing!");
        return;
      }

      // Stop the server when file reception starts
      Serial.println("Server stopped to receive file.");

      // Record the start time of the transfer
      break;
    }
    case 0x02: // Data pack
    if(currentTransmitCurrentPosition == 0)
      startTime = millis();

      byte highByte = *data++;
      byte lowByte = *data++;
      currentTransmitCurrentPosition = (highByte << 8) | lowByte;

      // Store data in the buffer
      for (int i = 0; i < (data_len - 3); i++) {
        buffer[bufferIndex++] = *data++;  // Fill buffer with incoming data
        if (bufferIndex >= BUFFER_SIZE) {
          file.write(buffer, bufferIndex); // Write buffer to file once full
          bufferIndex = 0; // Reset buffer index
        }
      }

      // Handle any remaining data in the buffer after the loop
      if (bufferIndex > 0) {
        file.write(buffer, bufferIndex);
        bufferIndex = 0; // Reset buffer
      }

      Serial.printf("Received packet %d of %d\n", currentTransmitCurrentPosition, currentTransmitTotalPackages);
      break;
  }

  if (currentTransmitCurrentPosition == currentTransmitTotalPackages) {
    Serial.printf("File transfer complete! Size: %d bytes\n", file.size());

    // Calculate the time taken for the transfer
    int transferTime = millis() - startTime;
    float transferTimeInSeconds = transferTime / 1000.0;  // Convert milliseconds to seconds
    Serial.printf("Total transfer time: %.2f seconds\n", transferTimeInSeconds);  // Print total time in seconds

    file.close();  // Close the file once the transfer is complete
    fileReceivingStarted = false;
    fileTransmissionComplete = false; // Reset flags for next transfer

    // Restart the server after the file is fully received
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);

    server.begin();
    Serial.println("Server restarted after file transfer.");
  }
}

void startWebServer() {
  server.on("/", HTTP_GET, handleHome); // Show home page
  server.on("/photos/:date", HTTP_GET, handleDatePhotos); // Show photos from selected date
  server.on("/photos/*", HTTP_GET, handlePhotoFile); // Serve photo file
  server.on("/events", HTTP_GET, handleEventSource); // Event source for updates
  server.begin();
  Serial.println("Async Web server initialized.");
}

void setup() {
  Serial.begin(115200);

  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("\n Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi.");

  InitESPNow();
  esp_now_register_recv_cb(OnDataRecv);

  if (!SD.begin(D8, SPI_20MHZ_SPEED)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized successfully.");

  timeClient.begin();  // Start NTP client to fetch time

  startWebServer();


}

void loop() {
  timeClient.update(); // Update NTP time periodically
}
