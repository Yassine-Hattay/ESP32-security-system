#include "global_header.h"
#include "server_v.h"
#include "tasks_v.h"
#include "camera_var.h"

bool connected_internet = false;
bool connected_router = false;

bool photo_not_sent;
bool manual_b = true;

httpd_handle_t stream_httpd = NULL;
int32_t channel;

int32_t getWiFiChannel(const char *ssid)
{
  if (int32_t n = WiFi.scanNetworks())
  {
    for (uint8_t a = 0; a < n; a++)
    {
      if (!strcmp(ssid, WiFi.SSID(a).c_str()))
      {
        return WiFi.channel(a);
      }
    }
  }
  return 0;
}

String SendHTML() 
{

  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<meta http-equiv=\"refresh\" content=\"30\">\n"; // Adds auto-refresh every 30 seconds
  ptr += "<script>\n";
  // Use the 'pageshow' event to force a reload if loaded from the cache
  ptr += "window.addEventListener('pageshow', function(event) {\n";
  ptr += "  if (event.persisted) {\n";          // Check if the page is loaded from the cache
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
  ptr += "  xhr.send();\n";                                                                  // Send a GET request to the C++ server
  ptr += "  window.location.href = 'http://' + window.location.hostname + ':8000/video';\n"; // Redirect to the video stream
  ptr += "}\n";

  ptr += "function automated() {\n";
  ptr += "  window.location.href = 'http://' + window.location.hostname + '/automated';\n"; // Redirect to a confirmation page
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

bool connectToWiFi_mod(const char *ssid, const char *password, unsigned long timeout)
{

  WiFi.begin(ssid, password); // Start connecting to WiFi
  WiFi.setAutoReconnect(true);

  Serial.print("Connecting to WiFi");

  unsigned long startTime = millis(); // Record the start time

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    if (digitalRead(GPIO_2))
    {
      return false;
    }
    // Check if the timeout has been exceeded
    if (millis() - startTime >= timeout)
    {
      Serial.println("\nWiFi connection timeout.");
      return false; // Connection failed within the timeout
    }
  }

  channel = getWiFiChannel(ssid);
  Serial.println("\nWiFi connected!");
  return true; // Successfully connected
}



void handle_OnConnect()
{
  Serial.println("you are connected !");
  server.send(200, "text/html", SendHTML());

  if (sending_photo_Handle != NULL)
  {
    vTaskDelete(sending_photo_Handle);
    sending_photo_Handle = NULL;
  }

  if (live_f_handle != NULL)
  {
    vTaskDelete(live_f_handle);
    live_f_handle = NULL;
    if (httpd_stop(stream_httpd) == ESP_OK)
    {
      httpd_unregister_uri_handler(stream_httpd, "/video", HTTP_POST);
      Serial.print("\n live stream stopped .\n");
    }
  }
}

void wait_smiley()
{
  server.send(200, "text/html", R"(
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

void handleAutomatedMode()
{
  Serial.println("Automated Mode Activated");
  String ptr = "<!DOCTYPE html>";
  ptr += "<html>";
  ptr += "<head><meta charset='UTF-8'><title>Automated Mode</title></head>";
  ptr += "<body>";
  ptr += "<h1>Automated mode activated (❁´◡`❁)</h1>";
  ptr += "</body>";
  ptr += "</html>";
  server.send(200, "text/html", ptr);

  if (sending_photo_Handle != NULL)
  {
    vTaskDelete(sending_photo_Handle);
    sending_photo_Handle = NULL;
  }

  if (live_f_handle != NULL)
  {
    vTaskDelete(live_f_handle);
    live_f_handle = NULL;
    if (httpd_stop(stream_httpd) == ESP_OK)
    {
      httpd_unregister_uri_handler(stream_httpd, "/video", HTTP_POST);
      Serial.print("\n live stream stopped .\n");
    }
  }

  xTaskCreate(
      sending_photo_task,
      "send_photo",
      100008,
      NULL,
      2,
      &sending_photo_Handle);
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