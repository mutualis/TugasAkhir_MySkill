#include <WiFi.h>
#include <PubSubClient.h>

// Ganti dengan kredensial Wi-Fi Anda
const char* ssid = "EWS";
const char* password = "00000000";

// Server MQTT HiveMQ (gunakan broker publik HiveMQ di sini)
const char* mqtt_server = "broker.hivemq.com";

// Username dan Password untuk autentikasi
const char* mqtt_username = "hivemq.webclient.1731876506591";
const char* mqtt_password = "!ncToS$W7wue%JVR648&";

// Topik yang digunakan untuk publikasi
const char* mqtt_topic = "MySkill/msg";

// Deklarasi objek Wi-Fi dan MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi untuk menyambung ke Wi-Fi
void setup_wifi() {
  Serial.print("Menyambung ke WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Terhubung ke WiFi!");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk menyambung ke broker MQTT
void reconnect() {
  // Coba sambung ke broker sampai berhasil
  while (!client.connected()) {
    Serial.print("Mencoba sambung ke MQTT...");
    
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {  // Gunakan username dan password untuk autentikasi
      Serial.println("Terhubung ke broker MQTT!");
    } else {
      Serial.print("Gagal, coba lagi dalam 5 detik...");
      delay(5000);
    }
  }
}

// Fungsi setup() untuk inisialisasi
void setup() {
  Serial.begin(115200);

  // Sambungkan ke Wi-Fi
  setup_wifi();

  // Sambungkan ke broker MQTT
  client.setServer(mqtt_server, 1883);  // Port default MQTT adalah 1883
  
  // Tunggu sambungan MQTT berhasil
  while (!client.connected()) {
    reconnect();
  }

  // Publikasikan pesan ke topik tertentu
  String message = "Hello from ESP32!";
  if (client.publish(mqtt_topic, message.c_str())) {
    Serial.println("Pesan terkirim!");
  } else {
    Serial.println("Gagal mengirim pesan.");
  }
}

// Fungsi loop() untuk mengatur komunikasi MQTT
void loop() {
  // Pastikan client tetap terhubung
  if (!client.connected()) {
    reconnect();
  }
  
  // Proses pesan MQTT (subscriber) jika ada
  client.loop();

  // Kirimkan pesan setiap 5 detik
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 5000) {
    lastTime = millis();
    String message = "Nama: Muhammad Ridho Assidiqi";
    if (client.publish(mqtt_topic, message.c_str())) {
      Serial.println("Pesan terkirim!");
    } else {
      Serial.println("Gagal mengirim pesan.");
    }
  }
}
