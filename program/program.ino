#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6EUkpiPrX"
#define BLYNK_TEMPLATE_NAME "Ruang 5"
#define BLYNK_AUTH_TOKEN "OUhnoUFYECNfKW9T5pvhRLbGJLha8m6Q"

#include "DHT.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEsp32.h>
#include <MQUnifiedsensor.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define placa "ESP32"
#define Voltage_Resolution 3.3
#define pin 32
#define type "MQ-135"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6  //RS / R0 = 3.6 ppm

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

#define led_R 25
#define led_G 26
#define led_B 27
#define buzzer 4

#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

float t, h;
char ssid[] = "MySkill";
char pass[] = "00000000";

String statTemp;
String statHumi;
String statAQI;

unsigned long previousMillis = 0;
const long interval = 500;
bool buzzerState = LOW;

unsigned long lcdPreviousMillis = 0; 
const unsigned long lcdInterval = 3000; 
int lcdScreenIndex = 0;

void sendSensor() {
  MQ135.update();
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  //konsentrasi CO2
  MQ135.setA(110.47);
  MQ135.setB(-2.862);
  float CO2 = MQ135.readSensor() + 400;

  int AQI = calculateAQI(CO2);

  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca DHT!");
    return;
  }
  if (AQI >= 0 && AQI <= 50) {
    statAQI = "Good";
  } else if (AQI >= 51 && AQI <= 100) {
    statAQI = "Moderate";
  } else if (AQI >= 101) {
    statAQI = "Bad";
  }

  if (t >= 32 && t <= 34) {
    statTemp = "Good";
  } else if ((t >= 30 && t <= 31) || (t >= 35 && t <= 36)) {
    statTemp = "Moderate";
  } else {
    statTemp = "Bad";
  }

  if (h >= 40 && h <= 60) {
    statTemp = "Good";
  } else if ((h >= 35 && h <= 39) || (h >= 61 && h <= 65)) {
    statHumi = "Moderate";
  } else {
    statHumi = "Bad";
  }

  if (statAQI == "Bad") {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      buzzerState = !buzzerState;
      if (buzzerState) {
        tone(buzzer, 1000);
      } else {
        noTone(buzzer);
      }
    }
  } else {
    noTone(buzzer);
  }

  if (statHumi == "Bad" && statTemp == "Bad"){
    setRGB(255, 0, 0);
  }else if (statHumi == "Moderate" || statTemp == "Moderate") {
    setRGB(0, 0, 255);
  }else if (statHumi == "Good" && statTemp == "Good") {
    setRGB(0, 255, 0);
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lcdPreviousMillis >= lcdInterval) {
    lcdPreviousMillis = currentMillis;
    lcdScreenIndex = (lcdScreenIndex + 1) % 3; 
    lcd.clear(); 
  }
  switch(lcdScreenIndex){
    case 0:
      lcd.setCursor(0, 0);
      lcd.print(String(t) + String(char(223)) + "C");
      lcd.setCursor(8, 0);
      lcd.print("|" + String(h) + "%");
      lcd.setCursor(0, 1);
      lcd.print(String(CO2));
      lcd.setCursor(8, 1);
      lcd.print("|AQI:" + String(AQI));
      break;
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("Temp:" + statTemp);
      lcd.setCursor(0, 1);
      lcd.print("Humi:" + statHumi);
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print("AQI:" + statAQI);
      break;
  }


  Serial.println(String(h) + "%; " + String(t) + "°C; " + String(CO2) + ";" + String(AQI));
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, AQI);
  Blynk.virtualWrite(V3, "Temp:" + statTemp + "   | Humi:" + statHumi);
  Blynk.virtualWrite(V4, statAQI);
}

void setup() {
  Serial.begin(115200);
  lcd.backlight();
  lcd.begin(16, 2);
  lcd.init();

  lcd.setCursor(0, 0);
  lcd.print("Conneting WiFi");
  lcd.setCursor(0, 1);
  lcd.print(String(ssid) + "...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lcd.clear();

  dht.begin();  // Inisialisasi sensor DHT
  pinMode(buzzer, OUTPUT);
  pinMode(led_R, OUTPUT);
  pinMode(led_G, OUTPUT);
  pinMode(led_B, OUTPUT);

  // Inisialisasi MQ-135
  MQ135.setRegressionMethod(1);
  MQ135.init();

  // Kalibrasi MQ-135
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update();
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1)
      ;
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1)
      ;
  }

  // Serial.print("Nilai R0: ");
  // Serial.println(MQ135.getR0());
}

void loop() {
  Blynk.run();
  sendSensor();
  delay(2000);
}

int calculateAQI(float CO2) {
  // Referensi AQI: https://en.wikipedia.org/wiki/Air_quality_index
  if (CO2 <= 400) return 0;  // Udara bersih
  if (CO2 <= 1000) return map(CO2, 400, 1000, 0, 50);
  if (CO2 <= 2000) return map(CO2, 1000, 2000, 51, 100);
  if (CO2 <= 5000) return map(CO2, 2000, 5000, 101, 150);
  if (CO2 <= 10000) return map(CO2, 5000, 10000, 151, 200);
}

void setRGB(int red, int green, int blue) {
  analogWrite(led_R, red);
  analogWrite(led_G, green);
  analogWrite(led_B, blue);
}
