#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "EWS";
const char* wifi_password = "00000000";

// Broker MQTT HiveMQ
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "MySkill/LEDcontrol"; // Topic yang digunakan untuk mengontrol LED

const char* mqtt_user = "hivemq.webclient.1731876506591";
const char* mqtt_pass = "!ncToS$W7wue%JVR648&"; 

WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin = 2; // PIN LED

// Fungsi untuk menghubungkan ke Wi-Fi
void setup_wifi() {
  delay(10);
  // Menyambungkan ke Wi-Fi
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk reconnect ke broker MQTT
void reconnect() {
  // Loop sampai terkoneksi ke broker MQTT
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Melakukan koneksi dengan username dan password
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Subscribe ke topic ledControl
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Fungsi callback ketika pesan diterima dari broker MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Menampilkan payload pesan yang diterima
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);
  
  // Jika pesan "ON", hidupkan LED
  if (message == "true") {
    digitalWrite(ledPin, HIGH); // LED ON
    Serial.println("LED ON");
  }
  // Jika pesan "OFF", matikan LED
  else if (message == "false") {
    digitalWrite(ledPin, LOW); // LED OFF
    Serial.println("LED OFF");
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  
  Serial.begin(115200);
  
  setup_wifi();
  
  // Sambungkan ke broker MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  // Jika belum terhubung ke MQTT, lakukan reconnect
  if (!client.connected()) {
    reconnect();
  }
  
  // Menjaga koneksi dengan broker MQTT
  client.loop();
}
