#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL3Qk2IK1F2"
#define BLYNK_TEMPLATE_NAME "Predictive Maintenance System for a Motor"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "max6675.h"

// Wi-Fi credentials
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// Stepper motor pins
#define stepPin D5
#define dirPin D6

// Vibration sensor pin
#define vibPin D1

// MAX6675 thermocouple pins
int thermoSO = D2;
int thermoCS = D3;
int thermoSCK = D4;

// Create thermocouple object
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);

// Timing variables
unsigned long lastStepTime = 0;
unsigned long lastTempRead = 0;

// Stepper control variables
int stepState = LOW;
bool motorRunning = true;

// Temperature variables
float temp = 0;
float tempLimit = 50;  // Safety temperature limit (°C)

void setup() {
  Serial.begin(115200);

  // Set pin modes
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(vibPin, INPUT);

  // Connect to Wi-Fi + Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Set motor direction
  digitalWrite(dirPin, HIGH);
}

void loop() {
  Blynk.run();

  unsigned long now = micros();

  // STEP 1: Non-blocking stepper motor control
  if (motorRunning && (now - lastStepTime >= 700)) {
    lastStepTime = now;

    stepState = !stepState;
    digitalWrite(stepPin, stepState);
  }

  // STEP 2: Read temperature every 250 ms
  if (millis() - lastTempRead > 250) {
    lastTempRead = millis();

    temp = thermocouple.readCelsius();

    Blynk.virtualWrite(V0, temp);
  }

  // STEP 3: Read vibration sensor
  int vibration = digitalRead(vibPin);
  Blynk.virtualWrite(V1, vibration);

  // STEP 4: Safety logic
  // Stop motor if temperature exceeds the limit
  // or vibration is detected.
  if (temp > tempLimit || vibration == HIGH) {
    motorRunning = false;
    Blynk.virtualWrite(V2, 0);
  } else {
    motorRunning = true;
    Blynk.virtualWrite(V2, 255);
  }
}
