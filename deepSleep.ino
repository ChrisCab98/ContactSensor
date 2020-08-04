#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <PubSubClient.h>

WiFiClient mainESP;
PubSubClient MQTT(mainESP);
Ticker ticker;

const char *ssid = "TP-Link_1526"; // WiFi SSID
const char *password = "00102698"; // WiFi password
const char *mqtt_server = "192.168.1.25";

float fmap(float x, float in_min, float in_max, float out_min, float out_max);
void securitySystemAwayArm();
void securitySystemDisarmed();

float Vbat = 0.0;

int startAddr = 0;
char prevState[2] = "0";

#define durationSleep 30 // secondes
#define NB_TRYWIFI 50    // nbr d'essai connexion WiFi, number of try to connect WiFi

void tick()
{
    int state = digitalRead(2); // get the current state of GPIO1 pin
    digitalWrite(2, !state);    // set pin to the opposite state
}

void setup()
{
    pinMode(2, OUTPUT);
    ticker.attach(0.5, tick);
    Serial.begin(115200);
    Serial.println("\n[Serial] Initialized");
    EEPROM.begin(512);
    Serial.println("[EEPROM] Initialized");
    Serial.println("[EEPROM] Size : " + String(EEPROM.length()));

    Serial.println("[PowerManagement] Reason startup :");
    Serial.println(ESP.getResetReason());

    WiFi.begin(ssid, password);

    int _try = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("..");
        delay(500);
        _try++;
        if (_try >= NB_TRYWIFI)
        {
            Serial.println("[WiFi] Impossible to connect WiFi network, go to deep sleep");
            ESP.deepSleep(durationSleep * 1000000);
        }
    }
    Serial.println("[WiFi] Connected to the WiFi network");
    Serial.print("[WiFi] IP address: ");
    Serial.println(WiFi.localIP());

    MQTT.setServer(mqtt_server, 1883);
    MQTT.setCallback(callback);

    while (!MQTT.connected())
    {
        Serial.println("[MQTT] Connecting to MQTT...");

        if (MQTT.connect("ESP32Client"))
        {

            Serial.println("[MQTT] connected");
        }
        else
        {

            Serial.print("failed with state ");
            Serial.print(MQTT.state());
            delay(2000);
        }
    }

    Serial.println("[MQTT] Sub to setTargetState");
    MQTT.subscribe("setTargetState/", 1);

    Vbat = fmap(analogRead(A0), 0, 1024, 0.0, 3.3);
    Serial.println(Vbat);

    // Check battery voltage
    if (Vbat < 2.5)
    {
        MQTT.publish("contactSensor/getStatusLowBattery/chris", "1");
        Serial.println("[BatteryState] Send notification about Low Battery");
    }
    else
    {
        MQTT.publish("contactSensor/getStatusLowBattery/chris", "0");
        Serial.println("[BatteryState] Send notification about Battery");
    }

    // Check current state of contactSensor

    if (digitalRead(5) == 0)
    {
        Serial.println("[State] Volet Fermés");
        MQTT.publish("contactSensor/getContactSensorState/chris", "0");
    }
    else
    {
        Serial.println("[State] Volet Ouverts");
        MQTT.publish("contactSensor/getContactSensorState/chris", "1");
    }

    // Check retain message

    for (int i = 0; i < 10; i++)
    {
        MQTT.loop(); //Ensure we've sent & received everything
        delay(100);
    }

    Serial.println("[PowerManagement] Going to Deep Seep");
    ESP.deepSleep(durationSleep * 1000000);
}

void loop()
{
}

void callback(char *topic, byte *payload, unsigned int length)
{
    char msg[10];
    char currentState[2] = "0";
    int i = 0;
    Serial.print("[MQTT Callback] Message arrived in topic: ");
    Serial.println(topic);

    Serial.print("[MQTT Callback] Message:");
    for (i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
        msg[i] = (char)payload[i];
    }
    Serial.println("");

    msg[i] = '\0';

    prevState[0] = (EEPROM.read(startAddr));
    Serial.print("[EEPROM] Previous State : ");
    Serial.println(prevState);

    if (strcmp(msg, "Disarmed") == 0)
    {
        currentState[0] = '0';
        Serial.println("[MQTT Callback] currentState is Disarmed");
    }

    if (strcmp(msg, "AwayArm") == 0)
    {
        currentState[0] = '1';
        Serial.println("[MQTT Callback]currentState is AwayArm");
    }

    Serial.println(currentState);

    if (strcmp(prevState, currentState) == 0)
    {
        Serial.println("[MQTT Callback] No change");
    }

    else
    {
        Serial.println("[MQTT Callback] Change");
        EEPROM.write(startAddr, currentState[0]);
        EEPROM.commit();
        if (strcmp(currentState, "1") == 0)
        {
            MQTT.publish("getCurrentState/", "AwayArm");
            securitySystemAwayArm();
        }
        else
        {
            MQTT.publish("getCurrentState/", "Disarmed");
            securitySystemDisarmed();
        }
    }
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void securitySystemAwayArm()
{
    analogWriteFreq(2048);
    analogWrite(4, 512);
    delay(100);
    analogWrite(4, 0);
    delay(100);
    analogWriteFreq(2048);
    analogWrite(4, 512);
    delay(100);
    analogWrite(4, 0);
}

void securitySystemDisarmed()
{
    analogWriteFreq(2048);
    analogWrite(4, 512);
    delay(200);
    analogWrite(4, 0);
    delay(200);
    analogWriteFreq(2048);
    analogWrite(4, 512);
    delay(200);
    analogWrite(4, 0);
}