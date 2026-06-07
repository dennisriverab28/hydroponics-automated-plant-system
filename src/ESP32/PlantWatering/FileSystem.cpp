// Implementation for Filesystem Related Functions
// Alex Roberts

#include "include/FileSystem.hpp"
#include "include/network.hpp"

#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>


bool initializeFilesystem(){
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return false;
    }
    return true;
}

// Helper function to return JSON in string format
String getLatestFromJSON(){

    File latest_file = SPIFFS.open("/watering-data.json", "r");
    if (!latest_file){
        Serial.println("Unable to locate latest data");
        return "{}";
    }

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, latest_file);
    latest_file.close();
    
    if (error) {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.f_str());
        return "{}";
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

uint8_t getKnownNetworkCount(){
  
    if (!SPIFFS.exists("/auth.json")) {
        Serial.println("Unable to locate Auth File");
        return 0;
    }

    File file = SPIFFS.open("/auth.json", "r");
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) {
        Serial.println("Failed to parse auth JSON");
        return 0;
    }
    
    return doc["network_count"].as<uint8_t>();
}

authCredentials getNetworkAuth(uint8_t network_index){

  authCredentials auth; // Empty Default
  File file = SPIFFS.open("/auth.json", "r");
  if (!file) return auth;
    
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) return auth;

  String networkKey = "network" + String(network_index);
  auth.ssid = String(doc[networkKey]["ssid"].as<const char*>());
  auth.pswd = String(doc[networkKey]["password"].as<const char*>());
  return auth;
}


void addJsonMoisture(uint8_t value){
    File file = SPIFFS.open("/watering-data.json", "r");
    if (!file) return;

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, file);
    file.close();

    // Handle No Data Yet
    JsonArray arr = doc["MoistureLevel"].as<JsonArray>();
    if (arr.isNull()) {
        arr = doc.createNestedArray("MoistureLevel");
    }
    arr.add(value);


    file = SPIFFS.open("/watering-data.json", "w");
    serializeJson(doc, file);
    file.close();
}

void addJsonPowerReading(uint16_t value){
    File file = SPIFFS.open("/watering-data.json", "r");
    if (!file) return;

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, file);
    file.close();

    // Handle No Data Yet
    JsonArray arr = doc["GrowLEDPowerData"].as<JsonArray>();
    if (arr.isNull()) {
        arr = doc.createNestedArray("GrowLEDPowerData");
    }
    arr.add(value);

    file = SPIFFS.open("/watering-data.json", "w");
    serializeJson(doc, file);
    file.close();
}

void appendIncomingWateringLog(String jsonStr){

    DynamicJsonDocument incomingDoc(512);
    DeserializationError err = deserializeJson(incomingDoc, jsonStr);
    if (err) {
        Serial.println("Failed to parse incoming log JSON");
        return;
    }

    JsonObject newLog = incomingDoc["WateringLog"]["Log"];
    if (newLog.isNull()) {
        Serial.println("No Log object in incoming JSON");
        return;
    }

    // Load existing and Append
    File file = SPIFFS.open("/watering-data.json", "r");
    DynamicJsonDocument doc(4096);
    err = deserializeJson(doc, file);
    file.close();

    JsonObject logs = doc["WateringLogs"].as<JsonObject>();
    if (logs.isNull()) {
        logs = doc.createNestedObject("WateringLogs");
        logs["NewLogIndex"] = 0;
    }

    int index = logs["NewLogIndex"].as<int>();
    String logKey = "Log" + String(index);

    JsonObject logEntry = logs.createNestedObject(logKey);
    logEntry["Date"] = newLog["Date"].as<const char*>();
    logEntry["Time"] = newLog["Time"].as<const char*>();
    logEntry["Amount"] = newLog["Amount"].as<int>();
    logEntry["IsManual"] = newLog["IsManual"].as<bool>();

    logs["NewLogIndex"] = index + 1;

    // Save JSON back to SPIFFS
    file = SPIFFS.open("/watering-data.json", "w");
    serializeJson(doc, file);
    file.close();
    Serial.println("New watering log added successfully");
}


void addPumpWateringLog(bool isManual, AsyncWebSocket* ws) {
  
    StaticJsonDocument<256> logDoc;
    JsonObject root = logDoc.createNestedObject("WateringLog");
    JsonObject entry = root.createNestedObject("Log");

    entry["Date"] = getDateString();
    entry["Time"] = getTimeString();
    entry["Amount"] = 1;
    entry["IsManual"] = isManual;

    // Save to SPIFFS
    String jsonStr;
    serializeJson(logDoc, jsonStr);
    appendIncomingWateringLog(jsonStr);

    // Push to Websocket Clients in real time
    if (ws != nullptr) {
        DynamicJsonDocument wsDoc(256);
        JsonObject wsRoot = wsDoc.createNestedObject("WateringLogs");
        JsonObject logCopy = wsRoot.createNestedObject("Log0");
        logCopy["Date"] = entry["Date"];
        logCopy["Time"] = entry["Time"];
        logCopy["Amount"] = entry["Amount"];
        logCopy["IsManual"] = entry["IsManual"];

        String wsPayload;
        serializeJson(wsDoc, wsPayload);
        ws->textAll(wsPayload);
    }
}
