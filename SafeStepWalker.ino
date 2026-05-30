// ================= BLYNK =================

#define BLYNK_TEMPLATE_ID "TMPL6W4w61laD"
#define BLYNK_TEMPLATE_NAME "Alert Notification"
#define BLYNK_AUTH_TOKEN "w6ehE79SPRoayitUdL7mS6-Z6Ye0T7FG"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// ================= WIFI =================

char ssid[] = "Zubair";
char pass[] = "zubair01407";

char auth[] = BLYNK_AUTH_TOKEN;

String Message = "🚨 SOS ALERT FROM Walker";

// ================= MOTOR PINS =================

#define IN1 26
#define IN2 27
#define ENA 25

#define IN3 14
#define IN4 12
#define ENB 33

// ================= BUTTONS =================

// Walker ON/OFF Button
#define BUTTON_PIN 4

// Blynk SOS Button
#define SOS_BUTTON 13

// ================= BUZZERS =================

#define LEFT_BUZZER 21
#define RIGHT_BUZZER 22

// ================= WATER SENSOR =================

#define WATER_SENSOR 34

// ================= ULTRASONIC =================

#define FRONT_TRIG 18
#define FRONT_ECHO 19

#define LEFT_TRIG 5
#define LEFT_ECHO 17

#define RIGHT_TRIG 16
#define RIGHT_ECHO 15

// ================= VARIABLES =================

bool systemEnabled = false;

int lastButtonReading = HIGH;
int buttonState = HIGH;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

int obstacleDistance = 20;
int waterThreshold = 200;

// SOS timer
unsigned long oldTime = 0;

// ================= SETUP =================

void setup() {

  Serial.begin(115200);

  // ================= WIFI =================

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");

  Blynk.begin(auth, ssid, pass);

  // ================= MOTOR PINS =================

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  // ================= BUTTONS =================

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SOS_BUTTON, INPUT_PULLUP);

  // ================= BUZZERS =================

  pinMode(LEFT_BUZZER, OUTPUT);
  pinMode(RIGHT_BUZZER, OUTPUT);

  // ================= WATER SENSOR =================

  pinMode(WATER_SENSOR, INPUT);

  // ================= ULTRASONIC =================

  pinMode(FRONT_TRIG, OUTPUT);
  pinMode(FRONT_ECHO, INPUT);

  pinMode(LEFT_TRIG, OUTPUT);
  pinMode(LEFT_ECHO, INPUT);

  pinMode(RIGHT_TRIG, OUTPUT);
  pinMode(RIGHT_ECHO, INPUT);

  stopMotors();
  stopBuzzers();
}

// ================= LOOP =================

void loop() {

  Blynk.run();

  handleSOS();

  handleButton();

  if (!systemEnabled) {

    stopMotors();
    stopBuzzers();
    return;
  }

  // ================= WATER DETECTION =================

  int waterValue = analogRead(WATER_SENSOR);

  if (waterValue > waterThreshold) {

    stopMotors();

    waterAlarm();

    return;
  }

  // ================= OBSTACLE DETECTION =================

  float frontDistance = getDistance(FRONT_TRIG, FRONT_ECHO);

  // CLEAR PATH
  if (frontDistance > obstacleDistance || frontDistance == -1) {

    stopBuzzers();

    moveForward();
  }

  // OBSTACLE DETECTED
  else {

    stopMotors();

    delay(200);

    float leftDistance = getDistance(LEFT_TRIG, LEFT_ECHO);
    float rightDistance = getDistance(RIGHT_TRIG, RIGHT_ECHO);

    // BOTH SIDES BLOCKED
    if (leftDistance < obstacleDistance &&
        rightDistance < obstacleDistance &&
        leftDistance != -1 &&
        rightDistance != -1) {

      reverseBuzzersON();

      moveBackward();

      delay(700);

      stopBuzzers();

      turnRight();

      delay(600);

      stopMotors();
    }

    // TURN LEFT
    else if (leftDistance > rightDistance) {

      leftBuzzerON();

      turnLeft();

      delay(500);

      stopBuzzers();

      stopMotors();
    }

    // TURN RIGHT
    else {

      rightBuzzerON();

      turnRight();

      delay(500);

      stopBuzzers();

      stopMotors();
    }
  }

  delay(50);
}

// ================= SOS FUNCTION =================

void handleSOS() {

  // Prevent spam notifications
  if (millis() - oldTime < 3000)
    return;

  if (digitalRead(SOS_BUTTON) == LOW) {

    Serial.println("SOS Notification Sent");

    Blynk.logEvent("notification", Message);

    oldTime = millis();

    while (digitalRead(SOS_BUTTON) == LOW) {
      delay(10);
    }
  }
}

// ================= MAIN BUTTON =================

void handleButton() {

  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {

    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != buttonState) {

      buttonState = reading;

      if (buttonState == LOW) {

        systemEnabled = !systemEnabled;

        if (systemEnabled)
          Serial.println("Walker ON");
        else
          Serial.println("Walker OFF");
      }
    }
  }

  lastButtonReading = reading;
}

// ================= DISTANCE =================

float getDistance(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0)
    return -1;

  float distance = duration * 0.0343 / 2;

  return distance;
}

// ================= MOTOR FUNCTIONS =================

void moveForward() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ================= BUZZERS =================

void leftBuzzerON() {

  digitalWrite(LEFT_BUZZER, HIGH);
  digitalWrite(RIGHT_BUZZER, LOW);
}

void rightBuzzerON() {

  digitalWrite(LEFT_BUZZER, LOW);
  digitalWrite(RIGHT_BUZZER, HIGH);
}

void reverseBuzzersON() {

  digitalWrite(LEFT_BUZZER, HIGH);
  digitalWrite(RIGHT_BUZZER, HIGH);
}

void stopBuzzers() {

  digitalWrite(LEFT_BUZZER, LOW);
  digitalWrite(RIGHT_BUZZER, LOW);
}

// ================= WATER ALARM =================

void waterAlarm() {

  digitalWrite(LEFT_BUZZER, HIGH);
  digitalWrite(RIGHT_BUZZER, HIGH);

  delay(100);

  digitalWrite(LEFT_BUZZER, LOW);
  digitalWrite(RIGHT_BUZZER, LOW);

  delay(100);
}