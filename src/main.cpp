#include <Arduino.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic



#define BUTTON_PIN 27
#define RGB_LED_PIN 38

// setting PWM properties
const int freq = 10000;
const int ledChannel = 0;
const int resolution = 8;

const int TachoPin = 47;

int InterruptCounter, rpm;

void countup() {
    InterruptCounter++;
}

void display_rpm() {
    Serial.print("Counts: ");
    Serial.print(InterruptCounter, 1);
    Serial.print(" RPM: ");
    Serial.println(rpm);
}

void measure() {
    InterruptCounter = 0;
    attachInterrupt(digitalPinToInterrupt(TachoPin), countup, RISING);
    delay(1000);
    detachInterrupt(digitalPinToInterrupt(TachoPin));
    rpm = (InterruptCounter / 2) * 60;
    display_rpm();
}


void WifiManagerTimeout()
{
    Serial.println("Going to reboot from WifiManagerTimeout");
    ESP.restart();
}

void setup() {
    Serial.begin(115200);
    neopixelWrite(RGB_LED_PIN, 255, 0, 0);

    WiFiManager wm;
    wm.setConfigPortalTimeout(180);
    wm.setConfigPortalTimeoutCallback(WifiManagerTimeout);
//    wm.resetSettings();
    bool res;
    res = wm.autoConnect("PWM_CONTROLLER");
    if (!res) {
        Serial.println("Failed to connect");
        ESP.restart();
    } else {
        Serial.println("connected...yeey :)");
    }
    neopixelWrite(RGB_LED_PIN, 0, 255, 0);

    // configure LED PWM functionalities
//    ledcSetup(ledChannel, freq, resolution);
    // attach the channel to the GPIO to be controlled
//    ledcAttachPin(BUTTON_PIN, ledChannel);
//    ledcWrite(ledChannel, 255);
}

void loop() {
    Serial.println("Tick");
    measure();
}