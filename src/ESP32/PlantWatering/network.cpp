// Header file for network related functionality
// Alexander Roberts

#include "include/network.hpp"
#include "include/FileSystem.hpp"

bool beginWiFiConnection() {
    uint8_t network_count = getKnownNetworkCount();
    Serial.println("Total Network Count: " + String(network_count));

    for (int i = 1; i <= network_count; i++) {
        authCredentials creds = getNetworkAuth(i);
        if (creds.ssid.length() == 0) continue;

        Serial.println("Connecting to WiFi: " + creds.ssid);
        WiFi.begin(creds.ssid.c_str(), creds.pswd.c_str());

        uint8_t connection_count = 0;
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            if (++connection_count >= CONNECTION_RETRIES) {
                Serial.println("Unable to connect to network: " + creds.ssid);
                break; // try next network
            }
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected to network: " + creds.ssid);
            return true;
        }
    }

    Serial.println("Unable to connect to any known network.");
    return false;
}

String getDateString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "Unknown";

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", &timeinfo);
    return String(buffer);
}

String getTimeString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "Unknown";

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%I:%M %p", &timeinfo);
    return String(buffer);
}
