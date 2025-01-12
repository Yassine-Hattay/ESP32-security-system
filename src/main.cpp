#include "Arduino.h" 
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <espnow.h>
#include <SD.h>
#include "LittleFS.h"

#define SPI_20MHZ_SPEED SD_SCK_MHZ(20)
#define BUFFER_SIZE 30720  

// WiFi credentials
const char* ssid = "Orange-066C";
const char* password = "GMA6ABLMG87";

// File and async web server setup
File file;
AsyncWebServer server(80);

// Transmit status variables
int currentTransmitCurrentPosition = 0;
int currentTransmitTotalPackages = 0;
bool fileReceivingStarted = false; // Flag to check if receiving has started
bool fileTransmissionComplete = false; // Flag to check if transmission is complete


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

// Serve the index page with dynamic template processing
void handleRoot(AsyncWebServerRequest *request) {
  AsyncResponseStream *response = request->beginResponseStream("text/html");
  response->addHeader("Server", "ESP Async Web Server");

  // Start HTML document
  response->printf("<!DOCTYPE html><html><head><title>Webpage at %s</title>", request->url().c_str());
  response->print("<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");

  // Include JavaScript for EventSource and image refresh
  response->print("<script>");
  response->print("var source = new EventSource('/events');");
  response->print("var refreshTimeout;");
  response->print("source.onmessage = function(event) {");
  response->print("  var statusText = event.data;");
  response->print("  document.getElementById(\"status\").innerHTML = statusText;");
  response->print("  if (event.data.includes(\"File transfer complete\")) {");
  response->print("    clearTimeout(refreshTimeout);");
  response->print("    refreshTimeout = setTimeout(function() {");
  response->print("      document.getElementById(\"image\").src = \"/moon.jpg?\" + new Date().getTime();");
  response->print("    }, 3000);");
  response->print("  }");
  response->print("};");
  response->print("</script>");
  response->print("</head><body>");

  // Page Header with dynamic content
  response->printf("<h1>ESP8266 Web Server at %s</h1>", request->url().c_str());

  // Dynamically include image
  response->print("<h2>File Transfer Status</h2>");
  response->print("<p id=\"status\">Waiting for updates...</p>");
  response->print("<img id=\"image\" src=\"/moon.jpg\" alt=\"Moon Image\"/>");

  // End of HTML document
  response->print("</body></html>");

  // Send the response with template processor applied
  request->send(response);
}

// Serve the image file from LittleFS (after copying it from SD)
void handleImage(AsyncWebServerRequest *request) {
  // Always overwrite the image in LittleFS
  File sdFile = SD.open("/moon.jpg", "r");
  if (!sdFile) {
    request->send(404, "text/plain", "File not found on SD card");
    return;
  }

  // Create or open the file in LittleFS for writing with a different name
  File littlefsFile = LittleFS.open("/moon_littlefs.jpg", "w");
  if (!littlefsFile) {
    request->send(500, "text/plain", "Failed to write to LittleFS");
    sdFile.close();
    return;
  }

  // Copy file from SD to LittleFS in chunks
  byte buffer[512];  // Buffer to hold data
  while (sdFile.available()) {
    size_t bytesRead = sdFile.read(buffer, sizeof(buffer));
    littlefsFile.write(buffer, bytesRead);
  }

  sdFile.close();
  littlefsFile.close();

  // Now serve the image from LittleFS with the new name
  request->send(LittleFS, "/moon_littlefs.jpg", "image/jpeg");
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

// Configure Async Web Server
void startWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/moon.jpg", HTTP_GET, handleImage);
  server.on("/events", HTTP_GET, handleEventSource);

  server.begin();
  Serial.println("Async Web server initialized.");
}


unsigned long startTime = 0;  // Variable to store the start time
unsigned long transferTime = 0;  // Variable to store the time taken for transfer
  
void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len) {
  static byte buffer[BUFFER_SIZE];  // Use larger buffer for faster data handling
  int bufferIndex = 0;

  switch (*data++) {
    case 0x01: // Start of file transmission
      server.end();
      wifi_set_channel(1);

      currentTransmitCurrentPosition = 0; 
      currentTransmitTotalPackages = (*data++) << 8 | *data;
      SD.remove("/moon.jpg");

      // Open the file for writing just once when the transfer starts
      file = SD.open("/moon.jpg", "w");
      if (!file) {
        Serial.println("Error opening file for writing!");
        return;
      }

      // Stop the server when file reception starts
      Serial.println("Server stopped to receive file.");

      // Record the start time of the transfer
      startTime = millis();
      break;

    case 0x02: // Data pack
      currentTransmitCurrentPosition = (*data++) << 8 | *data++;
      wifi_set_channel(1);
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
    transferTime = millis() - startTime;
    float transferTimeInSeconds = transferTime / 1000.0;  // Convert milliseconds to seconds
    Serial.printf("Total transfer time: %.2f seconds\n", transferTimeInSeconds);  // Print total time in seconds

    file.close();  // Close the file once the transfer is complete
    fileReceivingStarted = false;
    fileTransmissionComplete = false; // Reset flags for next transfer

    // Restart the server after the file is fully received
    WiFi.channel(getWiFiChannel(ssid));
    server.begin();
    Serial.println("Server restarted after file transfer.");
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

  startWebServer();

  

}

void loop() {

  }
