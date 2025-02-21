#ifndef SERVER_H
#define SERVER_H

#include "global_header.h"

extern httpd_handle_t stream_httpd ;

extern int32_t channel;

bool connectToWiFi_mod(const char *ssid, const char *password, unsigned long timeout);
bool connectToWiFi(const char* ssid, const char* password, unsigned long timeout) ;
void setup_server();

#endif