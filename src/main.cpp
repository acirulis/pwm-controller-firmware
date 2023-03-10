#include <Arduino.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <PubSubClient.h>


#define PWM_PIN 48
#define RGB_LED_PIN 38
#define TACHO_PIN 47 // listen to IRQ



// PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

const char *mqtt_server = "207.154.238.45";
const char *mqtt_user = "venti";
const char *mqtt_password = "venti12pwm";



volatile int InterruptCounter;
int rpm;

WiFiClient espClient;
PubSubClient client(espClient);

int to_int(byte *payload, unsigned int length) {
    char buffer[20];

    memcpy(buffer, payload, length);
    buffer[length] = '\0';

    char *end = nullptr;
    long value = strtol(buffer, &end, 10);

// Check for conversion errors
    if (end == buffer || errno == ERANGE) {
        Serial.println("to_int conversion error.");
        return 0;
    } else {
        return value;
    }
}


void reconnect() {
    client.setServer(mqtt_server, 1883);
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect("pwm-controller-1", mqtt_user, mqtt_password)) {
            Serial.println("connected");
            client.subscribe("venti/12pwm");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" rebooting in 5 secs.");
            delay(5000);
            ESP.restart();
        }
    }
}

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
    attachInterrupt(digitalPinToInterrupt(TACHO_PIN), countup, RISING);
    delay(1000);
    detachInterrupt(digitalPinToInterrupt(TACHO_PIN));
    rpm = (InterruptCounter / 2) * 60;
    display_rpm();
}


void WifiManagerTimeout() {
    Serial.println("Going to reboot from WifiManagerTimeout");
    ESP.restart();
}

void mqtt_callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    int pwm_value = to_int(payload, length);
    Serial.print("Setting new PWM  value: ");
    Serial.println(pwm_value);
    ledcWrite(ledChannel, pwm_value);
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

    client.setCallback(mqtt_callback);

    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(PWM_PIN, ledChannel);
    ledcWrite(ledChannel, 0);
}

void loop() {
//    Serial.println("Tick");
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    //measure();
}