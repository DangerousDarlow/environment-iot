#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
#include <PubSubClient.h>
#include <WiFiNINA.h>

#include "secrets.h"

char const ssid[] = SSID;
char const key[] = KEY;
int status = WL_IDLE_STATUS;
WiFiClient wifiClient;

void handleMessage(char * topic, byte * payload, unsigned int length)
{
}

char const * const location = "living_room";
char const * const broker = "192.168.0.29";
char const * const topic = "sensors";
PubSubClient mqttClient(broker, 1883, handleMessage, wifiClient);

Adafruit_AM2320 am2320 = Adafruit_AM2320();

void setup() {
    Serial.begin(115200);
    delay(1000);

    // start the temperature and humidity sensor
    am2320.begin();
    
    Serial.println("wifi connect");
    WiFi.begin(ssid, key);

    int waits = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        ++waits;
        if (waits == 5)
        {
            // wifi hasn't worked so start it again
            Serial.println("wifi connect");
            WiFi.begin(ssid, key);
            waits = 0;
        }

        Serial.println("wifi waiting");
        delay(2000);
    }

    Serial.println("wifi connected");

    if (mqttClient.connect(location))
    {
        Serial.println("broker connected");
    }
    else
    {
        Serial.println("broker connection failed");
    }
}

String buildMessage()
{
    float const temperature = am2320.readTemperature();
    float const humidity = am2320.readHumidity();
    String message = "environment/temperature+humidity,location=living\\ room";
    message += " temperature=";
    message += temperature;
    message += ",humidity=";
    message += humidity;
    return message;
}

void loop() {
    String const message = buildMessage();
    if (mqttClient.publish(topic, message.c_str()))
    {
        Serial.println("published " + message);
    }
    else
    {
        Serial.println("failed to publish");
    }

    delay(2000);
}