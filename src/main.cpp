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
#define BUFFER_SIZE 8192  

// WiFi credentials
const char ssid[] = "Orange-066C";  
const char password[] = "GMA6ABLMG87"; 

// NTP settings
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 3600, 3600000);  // Offset: UTC+1 (3600 seconds)

// File and async web server setup
File dir;
File file;
File root;
FSInfo fs_info;
char date[13] = "";

AsyncWebServer server(80);


uint16_t currentTransmitCurrentPosition = 0;
uint16_t currentTransmitTotalPackages = 0;
bool fileReceivingStarted = false; // Flag to check if receiving has started
bool fileTransmissionComplete = false; // Flag to check if transmission is complete
unsigned long startTime = 0; // Declare the startTime variable
bool moreFiles ;

void printMemoryAndFileSystemStats() {
  Serial.println("Memory and LittleFS Statistics:");
  // LittleFS Flash Information
  if (LittleFS.info(fs_info)) {
    Serial.printf("Total LittleFS Space: %u bytes\n", fs_info.totalBytes);
    Serial.printf("Used LittleFS Space: %u bytes\n", fs_info.usedBytes);
    Serial.printf("Free LittleFS Space: %u bytes\n", fs_info.totalBytes - fs_info.usedBytes);
  } else {
    Serial.println("Failed to retrieve LittleFS information.");
  }
    Serial.printf("Free RAM: %u bytes\n", ESP.getFreeHeap());

  
}

uint8_t getWiFiChannel(const char *ssid) {
  if (uint8_t n = WiFi.scanNetworks()) {
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

void handleHome(AsyncWebServerRequest *request) {
  
  moreFiles = false;
  LittleFS.format();
  date[0] = '\0';
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
  File root_l = SD.open("/photos");  // Open the directory for scanning
  if (root_l && root_l.isDirectory()) {
    File dir_l = root_l.openNextFile();
    while (dir_l) {  // Iterate through the directory
      if (dir_l.isDirectory()) {
        String date = dir_l.name();
        response->printf("<li><a href=\"/photos/%s\">%s</a></li>", date.c_str(), date.c_str());
      }
      dir_l = root_l.openNextFile();
    }
      dir_l.close();
  }
  root_l.close();
  root.close();
  dir.close();
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
  byte buffer[2048];
  while (sdFile.available()) {
    size_t bytesRead = sdFile.read(buffer, sizeof(buffer));
    littlefsFile.write(buffer, bytesRead);
    Serial.printf("Free RAM: %u bytes\n", ESP.getFreeHeap());
  }

  sdFile.close();
  littlefsFile.close();
  Serial.println("File successfully copied to LittleFS: " + destPath);
  return true;
}

void handleDatePhotos(AsyncWebServerRequest *request) {
    char url[20];

    LittleFS.format();

    strcpy(url, request->url().c_str());

    char* firstSlash = strchr(url + 1, '/');  // Find the first slash after the initial '/'

    if (firstSlash != nullptr && strlen(date) == 0) {
        firstSlash++;  // Move past the initial slash
        strcpy(date, firstSlash);  // Copy the substring into date

        // Remove trailing slash if present
        size_t len = strlen(date);
        while (len > 0 && date[len - 1] == '/') {
            date[len - 1] = '\0';  // Remove the last character (slash)
            len--;  // Decrease length to check the next character
        }

        // Remove leading slashes if present
        while (date[0] == '/') {
            // Shift all characters to the left by one (overwrite the first character)
            memmove(date, date + 1, strlen(date));
        }
    }

    char folderPath[21];  
    snprintf(folderPath, sizeof(folderPath), "/photos/%s", date);

    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>Photos from ");
    response->print(date);
    response->print("</title><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><style>");
    response->print("body{font-family:sans-serif;background:#f4f4f9;margin:0;padding:0;color:#333}h1{text-align:center;margin-top:20px;font-size:2.5rem}p{text-align:center;margin:20px}.gallery{display:flex;flex-wrap:wrap;justify-content:center;gap:20px;padding:20px}.gallery div{width:220px;height:auto;text-align:center;border-radius:10px;box-shadow:none;background:none}.gallery img{width:100%;height:auto;border-radius:8px;object-fit:cover}.gallery div:hover{transform:scale(1.05);box-shadow:none}.file-name{margin-top:10px;font-size:1rem;color:#333;padding:0 10px;background:none}.button{padding:12px 24px;font-size:1.2rem;border:none;border-radius:8px;background:#007BFF;color:white;cursor:pointer}.home-button{background:#28a745;position:fixed;top:20px;right:20px}.home-button:hover{background:#218838}</style>");
    response->print("<script>");
    response->print("function loadMorePhotos(){fetch('/more').then(r=>r.text()).then(d=>{document.body.innerHTML=d}).catch(e=>console.error(e))}");
    response->print("function goHome(){");
    response->print("  fetch('/').then(r => r.text())");
    response->print("    .then(d => {");
    response->print("      document.body.innerHTML = d;"); // This replaces the body content
    response->print("      window.location.href = '/';"); // This redirects the user to the root URL after content is fetched
    response->print("    })");
    response->print("    .catch(e => console.error(e));");
    response->print("}");    response->print("</script></head><body>");
    response->print("<button class=\"home-button\" onclick=\"goHome()\">Go Home</button>");
    response->print("<h1>Photos from ");
    response->print(date);
    response->print("</h1><div class=\"gallery\">");

    uint8_t counter = 0;
    if (!moreFiles) {
        root = SD.open(folderPath); // Open the directory for scanning
    }

    if (root && root.isDirectory()) {
        if (!moreFiles) {
            dir = root.openNextFile();
        }
        moreFiles = false;

        while (dir) {
            if (!dir.isDirectory()) {
                char filePath[100];  // Ensure filePath is large enough

                const char* dirName = dir.name();  // Example directory name

                // Clear tempPath and filePath before using them
                memset(filePath, 0, sizeof(filePath));

                // Concatenate folderPath and dirName into tempPath
                snprintf(filePath, sizeof(filePath), "%s/%s", folderPath, dirName);

                // Check if the file ends with ".jpg"
                if (strlen(filePath) > 4 && strcmp(&filePath[strlen(filePath) - 4], ".jpg") == 0) {
                    counter++;

                    char littlefsPath[23];
                    snprintf(littlefsPath, sizeof(littlefsPath), "/moon_littlefs_%d.jpg", counter);

                    // Extract the file name without the extension
                    char fileName[100];  // Ensure fileName is large enough
                    // Copy the directory name to fileName
                    strncpy(fileName, dirName, sizeof(fileName) - 1);    

                    // Find the last occurrence of the period (.) character
                    char* dotPosition = strrchr(fileName, '.');

                    // If a dot is found and it's not the last character
                    if (dotPosition != NULL) {
                        *dotPosition = '\0';  // Terminate the string at the dot
                    }
                    // Check LittleFS space
                    LittleFS.info(fs_info);
                    unsigned long freeSpace = fs_info.totalBytes - fs_info.usedBytes;
                    if (freeSpace > 300000) { // Only load the file if there's enough space
                        if (loadFileToLittleFS(filePath, littlefsPath)) {
                            server.on(littlefsPath, HTTP_GET, [littlefsPath](AsyncWebServerRequest *req) {
                                req->send(LittleFS, littlefsPath, "image/jpeg");
                            });

                            for (int i = 0; fileName[i] != '\0'; i++) {
                                if (fileName[i] == '-') {
                                    fileName[i] = ':';  // Replace '-' with ':'
                                }
                            }                                              
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
                        } else {
                            Serial.print("Failed to load file to LittleFS: ");
                            Serial.println(filePath);
                        }
                    } else {
                        moreFiles = true;
                        break; // Stop if there's not enough space for another file
                    }
                }
            }
            dir = root.openNextFile();
        }
    }

    response->print("</div>");

    if (moreFiles) {
        response->print("<div style=\"text-align: center; margin-top: 20px;\">");
        response->print("<button onclick=\"loadMorePhotos()\">Load More Photos</button>");
        response->print("</div>");
    }
    response->print("</body></html>");
    request->send(response);
    printMemoryAndFileSystemStats();
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
        file.write(buffer, bufferIndex);
        Serial.printf("\n Buffer index: %d \n" ,bufferIndex) ;// Write buffer to file once full
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
    int transferTimeInSeconds = transferTime / 1000;  // Convert milliseconds to seconds
    Serial.printf("Total transfer time: %d seconds\n", transferTimeInSeconds);  // Print total time in seconds

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
  server.on("/more", HTTP_GET, handleDatePhotos); // Serve photo file

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

  // Initialize SD card
  while(!SD.begin(D8)) {
    Serial.println("SD card initialization failed!");
    delay(100);
    Serial.println("Retrying!");
    SD.end();
  }

  Serial.println("SD card initialized successfully.");

  timeClient.begin();  // Start NTP client to fetch time

  startWebServer();
  
if (LittleFS.format()) {
  Serial.println("LittleFS formatted successfully!");
} else {
  Serial.println("Failed to format LittleFS!");
} 
  printMemoryAndFileSystemStats();
}

void loop() {
  timeClient.update(); 
  Serial.printf("Free RAM: %u bytes\n", ESP.getFreeHeap());
}
