#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DE D3
#define RE D4

const char* ssid = "honor"; // Replace with your WiFi SSID
const char* password = "51229216"; // Replace with your WiFi password
const char* serverName = "http://54.211.116.192/add-values"; // Replace with your server IP and endpoint

byte n, p, k;

const byte nitro[] = { 0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c };
const byte phos[] = { 0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc };
const byte pota[] = { 0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0 };

byte values[11];
SoftwareSerial mod(14, 12);

byte nitrogen() {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(150);
    if (mod.write(nitro, sizeof(nitro)) == 8) {
        digitalWrite(DE, LOW);
        digitalWrite(RE, LOW);
        for (byte i = 0; i < 7; i++) {
            values[i] = mod.read();
        }
    }
    return values[4];
}

byte phosphorous() {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(150);
    if (mod.write(phos, sizeof(phos)) == 8) {
        digitalWrite(DE, LOW);
        digitalWrite(RE, LOW);
        for (byte i = 0; i < 7; i++) {
            values[i] = mod.read();
        }
    }
    return values[4];
}

byte potassium() {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(150);
    if (mod.write(pota, sizeof(pota)) == 8) {
        digitalWrite(DE, LOW);
        digitalWrite(RE, LOW);
        for (byte i = 0; i < 7; i++) {
            values[i] = mod.read();
        }
    }
    return values[4];
}

void showResult() {
    if (!((n == 0 && p == 0 && k == 0) || (n > 0 && p > 0 && k > 0))) {
        return;
    }
    Serial.print("Nitrogen: ");
    Serial.print(n);
    Serial.println(" mg/kg");
    Serial.print("Phosphorous: ");
    Serial.print(p);
    Serial.println(" mg/kg");
    Serial.print("Potassium: ");
    Serial.print(k);
    Serial.println(" mg/kg");

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(15, 5);
    display.print("N: ");
    display.print(n);
    display.setTextSize(1);
    display.print(" mg/kg");
    display.setCursor(15, 25);
    display.setTextSize(2);
    display.print("P: ");
    display.print(p);
    display.setTextSize(1);
    display.print(" mg/kg");
    display.setCursor(15, 45);
    display.setTextSize(2);
    display.print("K: ");
    display.print(k);
    display.setTextSize(1);
    display.print(" mg/kg");
    display.display();
}

void sendNPKData() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, serverName);
        http.addHeader("Content-Type", "application/json");
        String jsonPayload = "{\"nitrogen\":" + String(n) + ", \"phosphorous\":" + String(p) + ", \"potassium\":" + String(k) + "}";
        int httpResponseCode = http.POST(jsonPayload);
        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println(httpResponseCode);
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("Error in WiFi connection");
    }
}

void setup() {
    Serial.begin(9600);
    mod.begin(9800);
    pinMode(RE, OUTPUT);
    pinMode(DE, OUTPUT);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(10, 3);
    display.setTextSize(2);
    display.println("AgroSense");
    display.setCursor(5, 28);
    display.setTextSize(2);
    display.println("NPK Sensor");
    display.setCursor(25, 50);
    display.setTextSize(1);
    display.print("Initializing");
    display.display();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("Press any key to start test...");
    display.display();
}

void loop() {
    if (Serial.available()) {
        Serial.read(); // Clear the input buffer

        // Conduct the test
        n = nitrogen();
        delay(350);
        p = phosphorous();
        delay(350);
        k = potassium();
        
        // Show and send the result
        showResult();
        sendNPKData();
        
        // Ask for a new test
        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println("Press any key to start a new test...");
        display.display();
    }
}