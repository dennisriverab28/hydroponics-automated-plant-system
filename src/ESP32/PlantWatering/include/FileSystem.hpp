// Header file for File System Operations
// Alex Roberts

#include <Arduino.h>
#include <string.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

typedef struct authCredentials{
    String ssid = "";
    String pswd = "";
} authCredentials;


bool initializeFilesystem();
String getLatestFromJSON();
uint8_t getKnownNetworkCount();
authCredentials getNetworkAuth(uint8_t network_index);

// Helpers for Updating SPIFFS
void addJsonMoisture(uint8_t value);
void addJsonPowerReading(uint16_t value);
void appendIncomingWateringLog(String jsonStr);
void addPumpWateringLog(bool isManual, AsyncWebSocket* ws = nullptr);
