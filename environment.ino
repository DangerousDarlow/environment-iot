#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
#include <ArduinoLowPower.h>
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

// true if the board has woken up from deepSleep
bool wokenUp = false;

void setup()
{
    initialise();
}

void initialise()
{
    Serial.begin(115200);

    // make sure you have a long delay so you stand a chance
    // of reprogramming the device before it deep sleeps
    delay(5000);

    // this is to aid diagnostics should I ever have multiple devices and forget which one is which
    Serial.println(location);

    // start the temperature and humidity sensor
    am2320.begin();
    
    Serial.println("wifi connect");
    WiFi.begin(ssid, key);

    int waits = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
        ++waits;
        if (waits == 5)
        {18010
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
    float temperature = 0;
    do
    {
       temperature = am2320.readTemperature();
    } while (isnan(temperature));
        
    float const humidity = am2320.readHumidity();

    String message = "environment/temperature+humidity,location=living\\ room";
    message += " temperature=";
    message += temperature;
    message += ",humidity=";
    message += humidity;
    return message;
}

void loop() {
    if (wokenUp)
        initialise();

    String const message = buildMessage();
    if (mqttClient.publish(topic, message.c_str()))
        Serial.println("published " + message);
    else
        Serial.println("failed to publish");

    mqttClient.flush();
    wifiClient.flush();

    int const sleepMinutes = 2;
    Serial.println("sleep minutes");
    Serial.println(sleepMinutes);
    LowPower.deepSleep(sleepMinutes * 60 * 1000);
    wokenUp = true;

    // delay(2 * 1000);
}