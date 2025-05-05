#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

// ================== WiFi Config ==================
const char* ssid = "Phuong Tran 3";               // Tên WiFi
const char* password = "24031972";   // Mật khẩu WiFi

// ================== MQTT Config ==================
const char* mqtt_server = "192.168.1.29";   // IP máy chạy Mosquitto (cùng mạng)
const int mqtt_port = 1883;                  // Cổng MQTT mặc định
const char* mqtt_topic_status = "home/door/status";
const char* mqtt_topic_sensor = "home/door/sensor";

WiFiClient espClient;
PubSubClient client(espClient);

// ================== Hardware Config ==================
#define RELAY_PIN D1     // GPIO5
#define SERVO_PIN D2     // GPIO4
#define SENSOR_PIN D6    // GPIO12

Servo myServo;
const unsigned long OPEN_TIME = 5000;

void setup_wifi() {
  delay(10);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("Kết nối WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Kết nối MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("thành công!");
    } else {
      Serial.print("Thất bại, mã lỗi = ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(0);
  digitalWrite(RELAY_PIN, LOW);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int sensorState = digitalRead(SENSOR_PIN);
  Serial.print("Trạng thái cảm biến: ");
  Serial.println(sensorState);

  // Gửi trạng thái cảm biến qua MQTT
  client.publish(mqtt_topic_sensor, sensorState == LOW ? "object_detected" : "clear");

  if (sensorState == LOW) {
    Serial.println("Phát hiện vật, mở cửa!");
    client.publish(mqtt_topic_status, "opening");
    openDoor();
    delay(OPEN_TIME);
    closeDoor();
    client.publish(mqtt_topic_status, "closed");
  } else {
    Serial.println("Không có vật.");
  }

  delay(200);
}

void openDoor() {
  digitalWrite(RELAY_PIN, HIGH);
  myServo.write(90);
  delay(500);
}

void closeDoor() {
  myServo.write(0);
  digitalWrite(RELAY_PIN, LOW);
  delay(500);
}
