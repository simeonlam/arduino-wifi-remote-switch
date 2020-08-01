#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include "config-name.h"

ESP8266WebServer server(PORT);

int value = LOW;
void updateValue(int val) {
  value = val;
}

int ledPin = 13; // GPIO13---D7 of NodeMCU

void setup() {
  Serial.begin(115200);
  WiFi.hostname(HOSTNAME);      // DHCP Hostname (useful for finding device for static lease)
  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi_SSID, WiFi_Password);
  while(WiFi.status() != WL_CONNECTED){
		/*Note: if connection is established, and then lost for some reason, ESP will automatically reconnect. This will be done automatically by Wi-Fi library, without any user intervention.*/
		delay(1000);
		Serial.print(".");
	}
	Serial.print("\nConnected to " + String(WiFi_SSID) + " with IP Address: "); Serial.print(WiFi.localIP()); Serial.print("\n");

  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  // ArduinoOTA.begin();

  // MDNS.begin("testing-simeon");
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("Error setting up MDNS responder!");
  }
  MDNS.addService(SERVICE_NAME, "tcp", PORT);

  pinMode(ledPin, OUTPUT);

  server.on("/", []() {
    server.send(200, "text/json", "{\"status\":\"OK\"}");
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

  server.on("/info", []() {
    // get device info
    server.send(200, "text/json", "{\"ip\":\"" + WiFi.localIP().toString() + "\"," +
      "\"status\":" + value + "," +
      "\"led_port\":" + ledPin + "," +
      "\"location\":\"" + LOCATION + "\"," +
      "\"hostname\":\"" + HOSTNAME + "\"," +
      "\"api\":[" +
        "{" +
          "\"path\": \"http://" + HOSTNAME + ".local/on\"," +
          "\"desc\": \"turn on kitchen light\"" +
        "}, " +
        "{" +
          "\"path\": \"http://" + HOSTNAME + ".local/off\"," +
          "\"desc\": \"turn off kitchen light\"" +
        "}, " +
        "{" +
          "\"path\": \"http://" + HOSTNAME + ".local/info\"," +
          "\"desc\": \"get info of this board\"" +
        "}" +
      "]" +
    "}");
  });

  server.begin();

  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see it working");
  Serial.print(WiFi.hostname());
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}

// collect header example
// https://github.com/esp8266/Arduino/blob/4897e0006b5b0123a2fa31f67b14a3fff65ce561/libraries/ESP8266WebServer/examples/SimpleAuthentification/SimpleAuthentification.ino#L124
