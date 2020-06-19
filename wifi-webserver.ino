#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include "config.h"

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const char* www_username = LOGIN;
const char* www_password = PASSWORD;

int value = LOW;
void updateValue(int val) {
  value = val;
}

int ledPin = 13; // GPIO13---D7 of NodeMCU

void setup() {
  Serial.begin(115200);
  WiFi.disconnect();  //Prevent connecting to wifi based on previous configuration
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);      // DHCP Hostname (useful for finding device for static lease)
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  ArduinoOTA.begin();
  pinMode(ledPin, OUTPUT);

  server.on("/", []() {
    if (!server.authenticate(www_username, www_password)) {
      return server.requestAuthentication();
    }
    server.send(200, "text/json", "{\"status\":\"Login OK\"}");
  });

  server.on("/on", []() {
    // turn on device
    digitalWrite(ledPin, HIGH);
    updateValue(HIGH);
    server.send(200, "text/json", "{\"updated\":\"success\",\"status\":1}");
  });

  server.on("/off", []() {
    // turn off device
    digitalWrite(ledPin, LOW);
    updateValue(LOW);
    server.send(200, "text/json", "{\"updated\":\"success\",\"status\":0}");
  });

  server.on("/lighter", []() {
    // more power to device
    notFound();
  });

  server.on("/darker", []() {
    // less power to device
    notFound();
  });

  server.on("/info", []() {
    // get device info
    server.send(200, "text/json", "{\"ip\":\"" + WiFi.localIP().toString() + "\"," +
      "\"status\":" + value + "," +
      "\"led_port\":" + ledPin + "," +
      "\"location\":\"" + LOCATION + "\"}");
  });

  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
}

void notFound() {
  return server.send(404, "{\"error\":\"not found\"}");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}
