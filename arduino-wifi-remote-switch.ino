#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include "config-name.h"

#define DebugBegin(baud_rate)    Serial.begin(baud_rate)
#define DebugPrintln(message)    Serial.println(message)
#define DebugPrint(message)    Serial.print(message)
#define DebugPrintF(...) Serial.printf( __VA_ARGS__ )

ESP8266WebServer server(PORT);

int value = LOW;
void updateValue(int val) {
  value = val;
}

int ledPin = 13; // GPIO13---D7 of NodeMCU

void setup() {
  DebugBegin(115200);
  WiFi.hostname(HOSTNAME);      // DHCP Hostname (useful for finding device for static lease)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi_SSID, WiFi_Password);
  while(WiFi.status() != WL_CONNECTED){
		/*Note: if connection is established, and then lost for some reason, ESP will automatically reconnect. This will be done automatically by Wi-Fi library, without any user intervention.*/
		delay(1000);
		DebugPrint(".");
	}
	DebugPrint("\nConnected to " + String(WiFi_SSID) + " with IP Address: "); DebugPrint(WiFi.localIP()); DebugPrint("\n");

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    DebugPrintln("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  // ArduinoOTA.begin();

  // MDNS.begin("testing-simeon");
  if (!MDNS.begin(HOSTNAME)) {
    DebugPrintln("Error setting up MDNS responder!");
  }
  MDNS.addService(SERVICE_NAME, "tcp", PORT);

  pinMode(ledPin, OUTPUT);

  // ota handling

  ArduinoOTA.onStart([]() {
    String type;
  //判斷一下OTA內容
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    DebugPrintln("OTA Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    DebugPrintln("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DebugPrintF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DebugPrintF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      DebugPrintln("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      DebugPrintln("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      DebugPrintln("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      DebugPrintln("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      DebugPrintln("End Failed");
    }
  });

  // web handling

  server.on("/", []() {
    server.send(200, "text/json", "{\"status\":\"OK\"}");
  });

  server.on("/on", []() {
    // turn on device
    digitalWrite(ledPin, HIGH);
    updateValue(HIGH);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", "{\"updated\":\"success\",\"status\":1}");
  });

  server.on("/off", []() {
    // turn off device
    digitalWrite(ledPin, LOW);
    updateValue(LOW);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", "{\"updated\":\"success\",\"status\":0}");
  });

  server.on("/status", []() {
    server.send(200, "text", value == HIGH ? "1" : "0");
  });

  server.on("/info", []() {
    // get device info
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/json", "{\"ip\":\"" + WiFi.localIP().toString() + "\"," +
      "\"version\":\"" + VERSION + "\"," +
      "\"location\":\"" + LOCATION + "\"," +
      "\"hostname\":\"" + HOSTNAME + "\"," +
      "\"protocol\":\"http\"," +
      "\"devices\":{" +
        "\"light\":{" +
          "\"led_port\":" + ledPin + "," +
          "\"status\":" + value +
        "}" +
      "}," +
      "\"api\":[" +
        "{" +
          "\"label\": \"light\"," +
          "\"fullPath\": \"http://" + HOSTNAME + ".local/on\"," +
          "\"path\": \"/on\"," +
          "\"desc\": \"turn on kitchen light\"" +
        "}, " +
        "{" +
          "\"label\": \"light\"," +
          "\"fullPath\": \"http://" + HOSTNAME + ".local/off\"," +
          "\"path\": \"/off\"," +
          "\"desc\": \"turn off kitchen light\"" +
        "}, " +
        "{" +
          "\"label\": \"info\"," +
          "\"fullPath\": \"http://" + HOSTNAME + ".local/info\"," +
          "\"path\": \"/info\"," +
          "\"desc\": \"get info of this board\"" +
        "}" +
      "]" +
    "}");
  });

  server.begin();
  ArduinoOTA.begin();

  DebugPrint("Open http://");
  DebugPrint(WiFi.localIP());
  DebugPrintln("/ in your browser to see it working");
  DebugPrint(WiFi.hostname());
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}

// collect header example
// https://github.com/esp8266/Arduino/blob/4897e0006b5b0123a2fa31f67b14a3fff65ce561/libraries/ESP8266WebServer/examples/SimpleAuthentification/SimpleAuthentification.ino#L124
