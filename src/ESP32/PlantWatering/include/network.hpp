// Header file for network related functionality
// Alexander Roberts

#include "Arduino.h"
#include <WiFi.h>

const uint8_t CONNECTION_RETRIES = 10;

bool beginWiFiConnection();
String getTimeString();
String getDateString();