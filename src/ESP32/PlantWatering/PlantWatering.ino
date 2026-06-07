// ESP32 project for Automatic Plant Watering WebSever
// Alex Roberts

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "include/FileSystem.hpp"
#include "include/network.hpp"

#define UART2_TX 17
#define UART2_RX 16

AsyncWebServer webserver(80);
AsyncWebSocket ws("/ws");

String receivedUART = "";

void initWebSocket(){
  ws.onEvent(onEvent);
  webserver.addHandler(&ws);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len){
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->opcode == WS_TEXT){
    data[len] = 0;
    String msg = (char*)data;

    Serial.print("Printing to UART: ");
    Serial.println(msg);

    Serial2.print(msg);
    Serial2.print("\n");
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
 void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT: {
        String latestDataString = getLatestFromJSON();
        client->text(latestDataString); // send data immediately on connect
        break;
    }
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void processUART(String jsonStr){

  // ---- RAW JSON PRINT ----
  Serial.print("UART RAW JSON: ");
  Serial.println(jsonStr);
  Serial.println("------------------------");

  StaticJsonDocument<1024> incoming;
  DeserializationError err = deserializeJson(incoming, jsonStr);
  if (err) {
    Serial.print("JSON parse error: ");
    Serial.println(err.f_str());
    return;
  }

  if (incoming.containsKey("MoistureLevel")) {
      addJsonMoisture(incoming["MoistureLevel"]);
  }
  else if (incoming.containsKey("GrowLEDData")) {
      addJsonPowerReading(incoming["PowerData"]);
  }
  else if (incoming.containsKey("ManualWateringLog")) {
      addPumpWateringLog(true, &ws);
  }
  else if (incoming.containsKey("AutoWateringLog")) {
      addPumpWateringLog(false, &ws);
  }
  else {
      Serial.println("Unknown JSON object received");
  }

  String wsPayload;
  serializeJson(incoming, wsPayload);
  ws.textAll(wsPayload);
}

void setup() {
  Serial.begin(115200); // Serial Debugging
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX); // STM32 UART Connection

  initializeFilesystem();
  
  bool wifi_status = beginWiFiConnection();
  if (!wifi_status){
    Serial.println("ERROR: Unable to connect to a known network...");
    return;
  }

  Serial.println(WiFi.localIP());
  initWebSocket();

  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html", "text/html");
  });
  webserver.serveStatic("/", SPIFFS, "/");
  webserver.begin();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // NTP servers
  setenv("TZ", "CST6CDT", 1); // CST
  tzset();  
}

unsigned long lastWiFiCheck = 0;
void loop() {

  // Continuously check for incoming UART data
  while (Serial2.available()){
    char c = Serial2.read();
    if (c == '\n'){
      processUART(receivedUART);
      receivedUART = ""; // Clear
    }
    else receivedUART += c;
  }

  if (millis() - lastWiFiCheck > 5000) { // check every 5s
      lastWiFiCheck = millis();
      if(WiFi.status() != WL_CONNECTED) {
          Serial.println("WiFi disconnected, reconnecting...");
          beginWiFiConnection(); // attempt reconnect
      }
  }

  ws.cleanupClients();
}
