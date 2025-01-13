#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <espnow.h>
#include <SdFat.h>
#include <SdBase.h>  
#include "LittleFS.h"
#include <NTPClient.h>
#include <TimeLib.h> // Include the TimeLib library

#define SPI_20MHZ_SPEED SD_SCK_MHZ(20)
#define BUFFER_SIZE 15720  

// WiFi credentials
const char* ssid = "Orange-066C";
const char* password = "GMA6ABLMG87";

SdBase SD; 

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
  response->print(R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Select Date</title>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <style>
        body {
          font-family: 'Arial', sans-serif;
          background-color: #f4f4f9;
          color: #333;
          margin: 0;
          padding: 0;
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          height: 100vh;
        }
        h1 {
          font-size: 2em;
          margin-bottom: 20px;
          color: #444;
        }
        .container {
          width: 90%;
          max-width: 600px;
          background: white;
          border-radius: 8px;
          box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
          padding: 20px;
        }
        .date-list {
          list-style: none;
          padding: 0;
          margin: 0;
        }
        .date-list li {
          margin: 10px 0;
        }
        .date-list a {
          text-decoration: none;
          color: #007BFF;
          font-weight: bold;
          transition: color 0.3s ease;
        }
        .date-list a:hover {
          color: #0056b3;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>Available Dates for Photos</h1>
        <ul class="date-list">
  )rawliteral");

  // Get the available dates by scanning SD card for folders
  File root = SD.open("/photos");  // Open the directory for scanning
  if (root && root.isDirectory()) {
    File dir = root.openNextFile();
    while (dir) {  // Iterate through the directory
      if (dir.isDirectory()) {
        String date = dir.name();
        response->printf("<li><a href=\"/photos/%s\">%s</a></li>", date.c_str(), date.c_str());
      }
      dir = root.openNextFile();
    }
  }
  root.close();

  // Close HTML document
  response->print(R"rawliteral(
        </ul>
      </div>
    </body>
    </html>
  )rawliteral");

  request->send(response);
}


bool loadFileToLittleFS(const String &sourcePath, const String &destPath) {
  // Open SD file
  File sdFile = SD.open(sourcePath, "r");
  if (!sdFile) {
    Serial.println("Failed to open source file on SD card: " + sourcePath);
    return false;
  }

  // Create or open the file in LittleFS
  File littlefsFile = LittleFS.open(destPath, "w");
  if (!littlefsFile) {
    Serial.println("Failed to open destination file on LittleFS: " + destPath);
    sdFile.close();
    return false;
  }

  // Copy file from SD to LittleFS in chunks
  byte buffer[512];
  while (sdFile.available()) {
    size_t bytesRead = sdFile.read(buffer, sizeof(buffer));
    littlefsFile.write(buffer, bytesRead);
  }

  sdFile.close();
  littlefsFile.close();
  Serial.println("File successfully copied to LittleFS: " + destPath);
  return true;
}

void handleDatePhotos(AsyncWebServerRequest *request) {
  String url = request->url(); // Example: "/photos/2025-1-12/"
  int firstSlash = url.indexOf('/', 1); // Find the first slash after the initial '/'
  String date = "";

  if (firstSlash != -1) {
    date = url.substring(firstSlash + 1);
    if (date.endsWith("/")) {
      date = date.substring(0, date.length() - 1);
    }
  }

  String folderPath = "/photos/" + date;
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->print("<!DOCTYPE html><html><head><title>Photos from ");
  response->print(date);
  response->print("</title>");
  response->print("<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  response->print("<style>");
  response->print("body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #f4f4f9; margin: 0; padding: 0; }");
  response->print("h1 { text-align: center; color: #333; margin-top: 20px; font-size: 2rem; }");
  response->print("p { text-align: center; color: #777; font-size: 1.1rem; margin: 20px; }");
  response->print(".gallery { display: flex; flex-wrap: wrap; justify-content: center; gap: 20px; padding: 20px; }");
  response->print(".gallery div { width: 200px; height: auto; text-align: center; border-radius: 8px; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); transition: transform 0.3s ease; }");
  response->print(".gallery img { width: 100%; height: auto; border-radius: 8px; object-fit: cover; }");
  response->print(".gallery div:hover { transform: scale(1.05); }");
  response->print(".file-name { margin-top: 10px; font-size: 1rem; color: #333; }");
  response->print("</style></head><body>");
  response->print("<h1>Photos from ");
  response->print(date);
  response->print("</h1>");

  File root = SD.open(folderPath); // Open the directory for scanning
  bool foundFiles = false;         // Flag to check if files were found

  if (root && root.isDirectory()) {
    File dir = root.openNextFile();
    response->print("<div class=\"gallery\">");

    while (dir) { // Iterate through the files in the directory
      if (!dir.isDirectory()) {
        String filePath = String(folderPath + "/" + dir.name());
        String littlefsPath = "/moon_littlefs_" + String(millis()) + ".jpg"; // Unique path in LittleFS

        // Extract the file name without the extension
        String fileName = dir.name();
        fileName = fileName.substring(0, fileName.lastIndexOf('.')); // Remove .jpg extension

        // Load file into LittleFS
        if (loadFileToLittleFS(filePath, littlefsPath)) {
          // Serve the image directly when requested
          server.on(littlefsPath.c_str(), HTTP_GET, [littlefsPath](AsyncWebServerRequest *req) {
            req->send(LittleFS, littlefsPath, "image/jpeg");
          });

          // Add the image and file name to the HTML
          response->print("<div>");
          response->print("<div class=\"file-name\">");
          response->print(fileName); // Display file name without extension
          response->print("</div>");
          response->print("<a href=\"");
          response->print(littlefsPath);
          response->print("\" target=\"_blank\">");
          response->print("<img src=\"");
          response->print(littlefsPath);
          response->print("\" alt=\"");
          response->print(fileName);
          response->print("\"/></a>");
          response->print("</div>");
          foundFiles = true;
        } else {
          Serial.println("Failed to load file to LittleFS: " + filePath);
        }
      }
      dir = root.openNextFile();
    }
    response->print("</div>");
  }

  root.close();

  // Message if no files were found
  if (!foundFiles) {
    response->print("<p>No files found for the selected date.</p>");
    Serial.println("No files found for date: " + date);
  } else {
    Serial.printf("\nFiles found for date: %s\n", date.c_str());
  }

  response->print("</body></html>");
  request->send(response);
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

      // Get current time in hours and minutes for file naming
      String currentTime = String(hour()) + "-" + String(minute());
      // Open the file for writing with the current time in the filename
      file = SD.open(folderPath + "/" + currentTime + ".jpg", "w");
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
    if(currentTransmitCurrentPosition == 0) {
      startTime = millis();
    }

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
  server.on("/photos/*", HTTP_GET, handleDatePhotos); // Serve photo file
  server.on("/events", HTTP_GET, handleEventSource); // Event source for updates
  server.begin();
  Serial.println("Async Web server initialized.");
}

bool checkSDCardSpace() {
    uint64_t totalSize = SD.card()->sectorCount() * 512;  // Get total size in bytes
    uint64_t usedBytes = SD.totalBytes() - SD.freeBytes();  // Calculate used space
    uint64_t availableBytes = totalSize - usedBytes;  // Calculate available space

    // Convert bytes to gigabytes for easier readability
    float totalSizeGB = totalSize / (1024.0 * 1024.0 * 1024.0);
    float availableSizeGB = availableBytes / (1024.0 * 1024.0 * 1024.0);

    // Print the sizes in GB
    Serial.print("Total Size: ");
    Serial.print(totalSizeGB);
    Serial.println(" GB");

    Serial.print("Available Size: ");
    Serial.print(availableSizeGB);
    Serial.println(" GB");

    // Threshold for available space (e.g., 1MB = 1024 * 1024 bytes)
    size_t thresholdBytes = 1024 * 1024; // 1 MB in bytes
    
    if (availableBytes >= thresholdBytes) {
        Serial.println("SD Card has enough space.");
        return true;
    } else {
        Serial.println("SD Card is running low on space.");
        return false;
    }
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

  if (!SD.begin(D8)) {
    Serial.println("SD card initialization failed!");
    return;
  }

  checkSDCardSpace();

  Serial.println("SD card initialized successfully.");

  timeClient.begin();  // Start NTP client to fetch time

  startWebServer();


}

void loop() {
  timeClient.update(); // Update NTP time periodically
}
