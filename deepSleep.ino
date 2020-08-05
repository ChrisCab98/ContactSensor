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

char checkChargingState();
char checkShutterState();
char checkLowbattery();

float Vbat = 0.0;

char startAddrSecuritySystemState = 0;
char startAddrShutterState = 1;
char startAddrLowBatteryState = 2;
char startAddrChargingState = 3;

char prevStateSecuritySystem = '0';

#define durationSleep 30 // secondes
#define NB_TRYWIFI 50    // nbr d'essai connexion WiFi, number of try to connect WiFi

void tick()
{
    int state = digitalRead(2); // get the current state of GPIO1 pin
    digitalWrite(2, !state);    // set pin to the opposite state
}

void setup()
{
    char prevStateShutter = '0';
    char prevStateLowBattery = '0';
    char prevStateCharging = '0';

    char CurrentStateShutter = '0';
    char CurrentStateLowBattery = '0';
    char CurrentStateCharging = '0';

    pinMode(2, OUTPUT);
    pinMode(5, INPUT);  // Shutter State
    pinMode(3, INPUT);  // Charging State
    pinMode(4, OUTPUT); // Speeker
    ticker.attach(0.5, tick);
    Serial.begin(115200);
    Serial.println("\n[Serial] Initialized");
    EEPROM.begin(512);
    Serial.println("[EEPROM] Initialized");

    prevStateShutter = (EEPROM.read(startAddrShutterState));
    prevStateLowBattery = (EEPROM.read(startAddrLowBatteryState));
    prevStateCharging = (EEPROM.read(startAddrChargingState));

    Serial.println("[PowerManagement] Reason startup : ");
    Serial.print(ESP.getResetReason());

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
    Serial.println("\n[WiFi] Connected to the WiFi network");
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

    CurrentStateLowBattery = checkLowbattery();
    CurrentStateShutter = checkShutterState();
    CurrentStateCharging = checkChargingState();

    if (CurrentStateLowBattery == prevStateLowBattery)
    {
        Serial.println("[LowBattery] No Change ");
    }
    else
    {
        Serial.println("[LowBattery] Change");
        EEPROM.write(startAddrLowBatteryState, CurrentStateLowBattery);
        EEPROM.commit();

        if (CurrentStateLowBattery == '0')
        {
            MQTT.publish("contactSensor/getStatusLowBattery/chris", "0");
        }
        else
        {
            MQTT.publish("contactSensor/getStatusLowBattery/chris", "1");
        }
    }

    if (CurrentStateShutter == prevStateShutter)
    {
        Serial.println("[ShutterState] No Change");
    }
    else
    {
        Serial.println("[ShutterState] Change");
        EEPROM.write(startAddrShutterState, CurrentStateShutter);
        EEPROM.commit();

        if (CurrentStateShutter == '0')
        {
            MQTT.publish("contactSensor/getContactSensorState/chris", "0");
        }
        else
        {
            MQTT.publish("contactSensor/getContactSensorState/chris", "1");
        }
    }

    // Check retained message

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
    char currentState = '0';
    int i = 0;
    Serial.print("[MQTT Callback] Message arrived in topic: ");
    Serial.println(topic);

    Serial.print("[MQTT Callback] Message : ");
    for (i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
        msg[i] = (char)payload[i];
    }
    Serial.println("");

    msg[i] = '\0';

    prevStateSecuritySystem = (EEPROM.read(startAddrSecuritySystemState));

    if (strcmp(msg, "Disarmed") == 0)
    {
        currentState = '0';
        Serial.println("[MQTT Callback] currentState is Disarmed");
    }

    if (strcmp(msg, "AwayArm") == 0)
    {
        currentState = '1';
        Serial.println("[MQTT Callback]currentState is AwayArm");
    }

    if (prevStateSecuritySystem == currentState)
    {
        Serial.println("[MQTT Callback] No change");
    }

    else
    {
        Serial.println("[MQTT Callback] Change");
        EEPROM.write(startAddrSecuritySystemState, currentState);
        EEPROM.commit();
        if (currentState == '1')
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

char checkLowbattery()
{
    Vbat = fmap(analogRead(A0), 0, 1024, 0.0, 3.3);
    Serial.print("[LowBattery] Voltage : ");
    Serial.println(Vbat);

    if (Vbat < 2.5)
    {
        return '1';
    }
    else
    {
        return '0';
    }
}

char checkShutterState()
{
    if (digitalRead(5) == 0)
    {
        return '0';
    }
    else
    {
        return '1';
    }
}

char checkChargingState()
{
    if (digitalRead(3) == 0)
    {
        return '0';
    }
    else
    {
        return '1';
    }
}