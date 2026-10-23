#include <Arduino.h>
#include "secrets.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ==========================================
// 1. ตั้งค่า WiFi, MQTT และ LINE API
// ==========================================

WiFiClient espClient;
PubSubClient client(espClient);

// ขาพินที่ปลอดภัยสำหรับบอร์ด ESP32
Servo servos[4];
const int pins[4] = {13, 14, 25, 26}; 

// ==========================================
// 🌟 ตัวแปร: ระบบควบคุมเซอร์โวแบบไม่ใช้ Delay
// ==========================================
int activeServoIndex = -1;             
unsigned long servoOpenTime = 0;       
const long servoKeepOpenMs = 7000;     
String pendingCommand = ""; 

// ==========================================
// 🔥 ฟังก์ชันเกราะป้องกัน: ล็อกขอบเขต 0-90 (แบบไม่ตัดสัญญาณ)
// ==========================================
void safeServoWrite(int index, int angle) {
    // กำแพงซอฟต์แวร์: ดักจับคำสั่งที่ผิดพลาด ป้องกันเฟืองรูด
    if (angle > 90) angle = 90;
    if (angle < 0) angle = 0;
    
    // สั่งหมุนอย่างเดียว ไม่ต้อง detach/attach สลับไปมาแล้ว!
    servos[index].write(angle);        
}

// ==========================================
// ตั้งค่าเซนเซอร์ HC-SR04
// ==========================================
const int trigPin = 5;  
const int echoPin = 18; 
unsigned long previousMillis = 0;
const long interval = 5000;    
const int full_threshold = 10; 
bool isLineNotified = false; 

// ==========================================
// ฟังก์ชันสำหรับส่ง LINE Messaging API
// ==========================================
void sendLineNotification(String binName, int binId) {
    WiFiClientSecure secureClient;
    secureClient.setInsecure(); 
    HTTPClient https;
    String url = "https://api.line.me/v2/bot/message/push";

    Serial.println("✉️ กำลังส่งแจ้งเตือนไปที่ LINE...");

    if (https.begin(secureClient, url)) {
        https.addHeader("Content-Type", "application/json");
        https.addHeader("Authorization", "Bearer " + String(lineToken));

        String payload = R"({
            "to": ")" + String(userId) + R"(",
            "messages": [
                {
                    "type": "template",
                    "altText": "🚨 แจ้งเตือน: ถังขยะเต็ม!",
                    "template": {
                        "type": "buttons",
                        "text": "🚨 ถังขยะ )" + binName + R"( เต็มแล้ว!\nกรุณาเข้าเคลียร์พื้นที่",
                        "actions": [
                            {
                                "type": "postback",
                                "label": "✅ รับงานนี้",
                                "data": "action=accept_job&bin_id=)" + String(binId) + R"("
                            }
                        ]
                    }
                }
            ]
        })";

        int httpCode = https.POST(payload);
        if (httpCode == 200) Serial.println("✅ ส่ง LINE สำเร็จเรียบร้อย!");
        else Serial.printf("❌ ส่ง LINE ผิดพลาด! HTTP Code: %d\n", httpCode);
        https.end();
    } else {
        Serial.println("❌ ไม่สามารถเชื่อมต่อเซิร์ฟเวอร์ LINE ได้");
    }
}

// ==========================================
// 2. ฟังก์ชันรับคำสั่งเปิดฝาถังขยะ (MQTT CallBack)
// ==========================================
void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) message += (char)payload[i];
    Serial.println("Message arrived: " + message);
    pendingCommand = message; 
}

// ==========================================
// 3. ฟังก์ชันเริ่มต้นระบบ (Setup)
// ==========================================
void setup() {
    Serial.begin(115200);
    
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    Serial.println("⚙️ System Booting: Locking all servos strictly to 0 degree...");
    for(int i = 0; i < 4; i++) {
        servos[i].setPeriodHertz(50);         
        
        // พอจ่ายไฟปุ๊บ มอเตอร์จะดึงตัวเองกลับมาล็อกที่ 0 ทันที โดยไม่กระตุกไป 90
        servos[i].attach(pins[i], 500, 2400);  

        // 🔥 นี่คือหัวใจหลักของความปลอดภัย:
        // สั่งโปรแกรมล่วงหน้าไว้ที่ 0 องศา "ก่อน" ที่จะเปิดสวิตช์จ่ายไฟเข้ามอเตอร์
        servos[i].write(0); 
        
        // delay(500); // ให้เวลามอเตอร์ได้ตั้งตัวและล็อกที่ 0 องศาอย่างมั่นคงก่อนจะไปสั่งตัวถัดไป
    }
    Serial.println("✅ All Servos are safely locked at 0 degree.");
    delay(1500);
    
    // ไม่มีการสั่ง detach() อีกต่อไป มอเตอร์จะถูกล็อกค้างไว้ที่ 0 องศาอย่างมั่นคง

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500); 
        Serial.print("."); 
    }
    Serial.println("\nWiFi Connected!");
    
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

// ==========================================
// 4. ลูปหลัก (ทำงานวนรอบ)
// ==========================================
void loop() {
    if (!client.connected()) {
        while (!client.connected()) {
            Serial.print("Attempting MQTT connection...");
            if (client.connect("ESP32_Bin")) {
                Serial.println("Connected!");
                client.subscribe("smartbin/command"); 
            } else { 
                Serial.print("Failed, rc=");
                Serial.print(client.state());
                Serial.println(" -> Try again in 5 seconds");
                delay(5000); 
            }
        }
    }
    client.loop(); 

    unsigned long currentMillis = millis();

    // ==========================================
    // 🔥 ระบบสั่งมอเตอร์เปิด (รับประกันว่าหมุนชัวร์)
    // ==========================================
    if (pendingCommand != "") {
        if (pendingCommand == "Organic") {
            Serial.println(">>> เปิดฝา 1: Organic");
            safeServoWrite(0, 90); 
            activeServoIndex = 0;
            servoOpenTime = currentMillis;
        } 
        else if (pendingCommand == "Recycle") {
            Serial.println(">>> เปิดฝา 2: Recycle");
            safeServoWrite(1, 90);
            activeServoIndex = 1;
            servoOpenTime = currentMillis;
        }
        else if (pendingCommand == "General") {
            Serial.println(">>> เปิดฝา 3: General");
            safeServoWrite(2, 90);
            activeServoIndex = 2;
            servoOpenTime = currentMillis;
        }
        else if (pendingCommand == "Hazardous") {
            Serial.println(">>> เปิดฝา 4: Hazardous");
            safeServoWrite(3, 90);
            activeServoIndex = 3;
            servoOpenTime = currentMillis;
        }
        else if (pendingCommand == "CloseAll") {
            Serial.println(">>> รีเซ็ตระบบ: ปิดฝาทุกตัว");
            for(int i=0; i<4; i++) {
                safeServoWrite(i, 0);
            }
            activeServoIndex = -1; 
        }
        
        pendingCommand = ""; 
    }

    // ==========================================
    // 🌟 ระบบนับเวลาปิด (7 วินาที)
    // ==========================================
    if (activeServoIndex != -1) {
        if (currentMillis - servoOpenTime >= servoKeepOpenMs) {
            Serial.println(">>> หมดเวลา 7 วินาที: ปิดฝาถังกลับไปที่ 0");
            safeServoWrite(activeServoIndex, 0); // สั่งปิด 
            activeServoIndex = -1;             
        }
    }

    // ==========================================
    // ระบบตรวจจับขยะเต็ม 
    // ==========================================
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        long duration = pulseIn(echoPin, HIGH, 30000); 
        
        if (duration > 0) {
            int distance = duration * 0.034 / 2;
            Serial.printf("📏 ตรวจสอบระยะ: %d cm\n", distance);

            if (distance <= full_threshold) {
                if (!isLineNotified) { 
                    Serial.println("🚨 ตรวจพบขยะเต็มระดับวิกฤต!");
                    client.publish("smartbin/status", "Organic_Full"); 
                    sendLineNotification("ขยะอินทรีย์ (Organic)", 1); 
                    isLineNotified = true; 
                }
            } 
            else {
                if (isLineNotified) {
                    Serial.println("♻️ ถังขยะว่างแล้ว! รีเซ็ตสถานะ");
                    isLineNotified = false; 
                }
            }
        }
    }
}