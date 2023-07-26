#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

char auth[] = BLYNK_AUTH_TOKEN;
char id[] = "Widjaya Cell 1";
char pass[] = "275351928";

const char *ssid = "Widjaya Cell 1";
const char *password = "275351928";
const uint16_t port = 80;
const char *host = "192.168.0.55";

#define S0 2
#define S1 14
#define S2 12
#define S3 15
#define sensorOut 16 // D0 Sensor Out
#define LedMerah 13  // D8 pin led merah
#define LedBiru 0    // D3 pin led hijau

int redFrequency = 0;
int greenFrequency = 0;
int blueFrequency = 0;

int redColor = 0;
int greenColor = 0;
int blueColor = 0;

float Red;
float Green;
float Blue;
int ColorMax = 255;

bool colorDetectionMode = false;

WiFiClient client;
WidgetLCD LCD(V1);
WidgetLED ledMode(V2);
WidgetLED ledColorDetected(V3);

void connect_wifi()
{
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  delay(250);
}

void connect_server()
{
  while (!client.connect(host, port))
  {
    Serial.printf("\n[Connecting to %s ... ", host);
    delay(1000);
    return;
  }
  Serial.println("connected]");
  delay(1000);
}

void setup()
{
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  connect_wifi();
  connect_server();

  Blynk.begin(auth, id, pass);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  pinMode(LedMerah, OUTPUT);
  pinMode(LedBiru, OUTPUT);

  // Skala frekuensi Output 100%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
}

void visualizeDataOnBlynk()
{
  Blynk.virtualWrite(V4, redColor);
  Blynk.virtualWrite(V5, greenColor);
  Blynk.virtualWrite(V6, blueColor);
}

void colorDetection()
{
  if (colorDetectionMode)
  {
    ledColorDetected.on();
    // Setting Merah (R)
    digitalWrite(S2, LOW);
    digitalWrite(S3, LOW);
    redFrequency = pulseIn(sensorOut, LOW);
    redColor = map(redFrequency, 25, 60, 255, 0);

    // Setting Hijau (G)
    digitalWrite(S2, HIGH);
    digitalWrite(S3, HIGH);
    greenFrequency = pulseIn(sensorOut, LOW);
    greenColor = map(greenFrequency, 50, 150, 255, 0);

    // Setting Biru (B)
    digitalWrite(S2, LOW);
    digitalWrite(S3, HIGH);
    blueFrequency = pulseIn(sensorOut, LOW);
    blueColor = map(blueFrequency, 40, 125, 255, 0);

    // Deteksi Warna Tertentu berdasarkan Mode
    if (redColor > greenColor && redColor > blueColor)
    {
      greenColor = (blueColor - redColor) / (blueColor - greenColor);
      blueColor = (redColor - greenColor) / (blueColor - greenColor);
      Red = (blueColor * (redColor * ColorMax)) + (greenColor * (redColor * ColorMax)) / (greenColor + blueColor);
    }
    else if (greenColor > redColor && greenColor > blueColor)
    {
      redColor = (blueColor - greenColor) / (blueColor - redColor);
      blueColor = (greenColor - redColor) / (blueColor - redColor);
      Green = (blueColor * (greenColor * ColorMax)) + (redColor * (greenColor * ColorMax)) / (redColor + blueColor);
    }
    else if (blueColor > greenColor && blueColor > redColor)
    {
      greenColor = (redColor - blueColor) / (redColor - greenColor);
      redColor = (blueColor - greenColor) / (redColor - greenColor);
      Blue = (redColor * (blueColor * ColorMax)) + (greenColor * (blueColor * ColorMax)) / (greenColor + redColor);
    }
    else
    {
      // Warna tidak terdeteksi
      ledColorDetected.off();
    }
    visualizeDataOnBlynk();
  }
}

BLYNK_WRITE(V7)
{
  // Mode deteksi warna diaktifkan atau dinonaktifkan melalui tombol di aplikasi Blynk
  colorDetectionMode = param.asInt();
  if (colorDetectionMode)
  {
    ledMode.on();
  }
  else
  {
    ledMode.off();
  }
}

void loop()
{
  if (client.connected())
  {
    Serial.print("[Response:]");
    String line = client.readStringUntil('\n');
    Serial.println(line);

    if (line.equalsIgnoreCase("ON"))
    {
      colorDetection();
    }
    else if (line.equalsIgnoreCase(""))
    {
      Serial.println("Tidak Ada Respon");
    }
  }
  else
  {
    connect_server();
  }

  delay(500);
  Blynk.run();
}
