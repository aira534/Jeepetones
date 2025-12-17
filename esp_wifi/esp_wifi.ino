#include <ArduinoJson.h>
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoMqttClient.h>
#include "esp_eap_client.h"

const char* ssid = "Rui";      // wifi name (with phone hotspot)
const char* password = "ruicontrasenha123"; // password

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "193.147.79.118"; // where to send the messages
int port = 21883;

#define RXD2 33
#define TXD2 4

String team_name = "Jeepetones";
int id = 15;

const char topic[] = "/SETR/2025/15/";

String rcv_msg;

/*-----------------------------------------------------*/
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  delay(3000);

  Serial.println("Connecting to wifi...");

  WiFi.disconnect(true);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());


  // MQTT connect
  while (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed, error = ");
    Serial.println(mqttClient.connectError());
    delay(1000);
  }

  Serial.println("MQTT connected");
  Serial2.print("r");
}

/*-----------------------------------------------------*/
// Message functions
void lost_line() {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "LINE_LOST";

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}


void visible_line(float val) {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "VISIBLE_LINE";
  doc["value"] = val;

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}


void end_lap(unsigned long val) {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "END_LAP";
  doc["time"] = val;

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}


void obst_det(float val) {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "OBSTACLE_DETECTED";
  doc["distance"] = val;

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}


void line_search() {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "INIT_LINE_SEARCH";

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}


void stop_search() {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "STOP_LINE_SEARCH";

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}


void line_found() {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "LINE_FOUND";

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}


void ping_message(unsigned long val) {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "PING";
  doc["time"] = val;

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}


void start_lap() {
  StaticJsonDocument<200> doc;

  doc["team_name"] = team_name;
  doc["id"] = id;
  doc["action"] = "START_LAP";

  mqttClient.beginMessage(topic);
  serializeJson(doc, mqttClient);
  mqttClient.endMessage();
}

/*-----------------------------------------------------*/
void loop() {
  mqttClient.poll();
  
  if (Serial2.available()) {  // Read the serial port and retreive the received signs to send the adequate message to the broker
    char c = Serial2.read();
    rcv_msg += c;
    if (c == '}') {
      Serial.print("ESP32 received: ");
      Serial.println(rcv_msg);

      if (rcv_msg[1] == 'l' && rcv_msg[2] == 'l') { // lost_line
        lost_line();

      } else if (rcv_msg[1] == 'v' && rcv_msg[2] == 'l') {  // visible_line
        int value = rcv_msg.substring(3).toInt();
        visible_line(value);

      } else if (rcv_msg[1] == 'e' && rcv_msg[2] == 'l') { //end_lap
        int value = rcv_msg.substring(3).toInt();
        end_lap(value);

      } else if (rcv_msg[1] == 'o' && rcv_msg[2] == 'd') { //obst_det
        float dist = rcv_msg.substring(3).toFloat();
        obst_det(dist);

      } else if (rcv_msg[1] == 'l' && rcv_msg[2] == 's') { //line_search
        line_search();

      } else if (rcv_msg[1] == 's' && rcv_msg[2] == 's') { //stop_search
        stop_search();

      } else if (rcv_msg[1] == 'l' && rcv_msg[2] == 'f') { //line_found
        line_found();

      } else if (rcv_msg[1] == 'p' && rcv_msg[2] == 'm') { //ping_message
        int time = rcv_msg.substring(3).toInt();
        ping_message(time);

      } else if (rcv_msg[1] == 's' && rcv_msg[2] == 'l') { //start_lap
        Serial.print("PAT");
        start_lap();
      }
      // Clean the msg to recive the new one
      rcv_msg.remove(0, rcv_msg.indexOf('}') + 1 );

    }
  }
}
