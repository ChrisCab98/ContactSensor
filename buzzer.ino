// // char *ssid = "DVB";                                     //Wi-Fi AP Name
// // char *password = "tesla tranquille la pauvre";          //Wi-Fi Password
// char *ssid = "TP-Link_1526";                            //Wi-Fi AP Name
// char *password = "00102698";                            //Wi-Fi Password
// char *mqtt_server = "192.168.1.25";                     //MQTT Server IP
// char *mqtt_name = "Christopher Bedroom Contact Sensor"; //MQTT device name
// char *mqtt_topic = "setTargetState";                    //MQTT topic for communication
// char *mqtt_ending = "/data";                            //MQTT subsection for communication

// #include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
// #include <WiFiUdp.h>
// #include <PubSubClient.h>
// WiFiClient mainESP;
// PubSubClient MQTT(mainESP);

// char *mqtt_subtopic = "/data"; //MQTT topic for communication
// char *mqtt_maintopic = mqtt_topic;
// int stateSecuritySystem = -1;

// void setup()
// {

//     Serial.begin(115200);

//     startWiFi();
//     startMQTT();

//     pinMode(4, OUTPUT);
//     pinMode(5, INPUT);
    
//     attachInterrupt(digitalPinToInterrupt(5), alarm, CHANGE);
// }

// void loop()
// {
//     MQTT.loop();
//     // Reconnect Wifi
//     if (WiFi.status() != WL_CONNECTED)
//     {
//         startWiFi();
//     }

//     if (!MQTT.connected())
//     {
//         startMQTT();
//     }
// }

// void callback(char *topic, byte *payload, unsigned int length)
// {
//     char msg[4];
//     int i = 0;
//     Serial.print("Message arrived [");
//     Serial.print(topic);
//     Serial.print("] ");

//     if (strcmp(topic, "setTargetState/") == 0)
//     {

//         Serial.println("New State for Alarm");

//         for (i = 0; i < length; i++)
//         {
//             //Serial.print((char)payload[i]);
//             msg[i] = (char)payload[i];
//         }

//         msg[i] = '\0';
//         Serial.println();
//         Serial.println(msg);

//         if (strcmp(msg, "AwayArm") == 0)
//         {
//             Serial.println("attachInterrupt to pin 5 with FALLING / RISING detection");
//             analogWriteFreq(2048);
//             analogWrite(4, 512);
//             delay(100);
//             analogWrite(4, 0);
//             delay(100);
//             analogWriteFreq(2048);
//             analogWrite(4, 512);
//             delay(100);
//             analogWrite(4, 0);
//             MQTT.publish("getCurrentState/", "AwayArm");
//             stateSecuritySystem = 1;
//         }

//         if (strcmp(msg, "Disarmed") == 0)
//         {
//             Serial.println("Alarm OFF");
//             // detachInterrupt(digitalPinToInterrupt(5));
//             analogWriteFreq(2048);
//             analogWrite(4, 512);
//             delay(200);
//             analogWrite(4, 0);
//             delay(200);
//             analogWriteFreq(2048);
//             analogWrite(4, 512);
//             delay(200);
//             analogWrite(4, 0);
//             stateSecuritySystem = 0;
//             MQTT.publish("getCurrentState/", "Disarmed");
//         }
//     }
// }

// void startWiFi()
// {
//     WiFi.begin(ssid, password);
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         delay(500);
//         Serial.println("Connecting to WiFi..");
//     }

//     Serial.println("Connected to the WiFi network");
//     Serial.print("IP address: ");
//     Serial.println(WiFi.localIP());
// }

// void startMQTT()
// {
//     MQTT.setServer(mqtt_server, 1883);
//     MQTT.setCallback(callback);

//     while (!MQTT.connected())
//     {
//         Serial.println("Connecting to MQTT...");

//         if (MQTT.connect("ESP32Client"))
//         {

//             Serial.println("connected");
//         }
//         else
//         {

//             Serial.print("failed with state ");
//             Serial.print(MQTT.state());
//             delay(2000);
//         }
//     }

//     MQTT.subscribe("setTargetState/", 1);
//     Serial.println("Sub to setTargetState 1");
// }

// ICACHE_RAM_ATTR void alarm()
// {
//     if (digitalRead(5) == 0)
//     {
//         Serial.println("Volets Fermés");
//         if (stateSecuritySystem == 0)
//         {
//             analogWriteFreq(2048);
//             analogWrite(4, 0);
//         }

//         MQTT.publish("contactSensor/getContactSensorState/chris", "0");
//     }

//     else
//     {
//         Serial.println("Volets Ouvert !");
//         if (stateSecuritySystem == 1)
//         {
//             analogWriteFreq(2048);
//             analogWrite(4, 512);
//         }

//         MQTT.publish("contactSensor/getContactSensorState/chris", "1");
//     }
// }
