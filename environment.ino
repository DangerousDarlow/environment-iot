#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
#include <ArduinoLowPower.h>
#include <PubSubClient.h>
#include <WiFiNINA.h>

#include "secrets.h"

char const ssid[] = SSID;
char const key[] = KEY;
WiFiClient wifiClient;

void handleMessage(char * topic, byte * payload, unsigned int length)
{
}

char const * const location = "outside";
char const * const broker = "192.168.0.29";
char const * const sensorsTopic = "sensors";
char const * const systemsTopic = "systems";
PubSubClient mqttClient(broker, 1883, handleMessage, wifiClient);

Adafruit_AM2320 am2320 = Adafruit_AM2320();

// true if the board has woken up from deepSleep
bool wokenUp = false;

void setup()
{
    analogReadResolution(12);
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

// The sensor temperature response is assumed to be linear across the expected range.
// These values calibrate the specific device.
const float gradient = 1.0447815f;
const float offset = -0.24073213f;

String buildTempHumMessage()
{
    // The first couple of readings are often inaccurate so ignore them
    int count = 0;
    while(count < 5)
    {
        ++count;
        am2320.readTemperature();
        delay(1000);

    }

    float const temperature = am2320.readTemperature() * gradient + offset;
    float const humidity = am2320.readHumidity();

    String message = "environment/temperature+humidity,location=";
    message += location;
    message += " temperature=";
    message += temperature;
    message += ",humidity=";
    message += humidity;
    return message;
}

String buildBatteryMessage()
{
    int const voltage = analogRead(ADC_BATTERY);

    String message = "device/battery,location=";
    message += location;
    message += " voltage=";
    message += voltage;
    return message;
}

void publish(String const & topic, String const & message)
{
    if (mqttClient.publish(topic.c_str(), message.c_str()))
        Serial.println("published: " + message);
    else
        Serial.println("failed to publish: " + message);

}

void deepSleep()
{
    int const sleepMinutes = 5;
    Serial.println("sleep minutes");
    Serial.println(sleepMinutes);

    // Without this delay the mqtt message is never received.
    // I suspect WiFi is shutdown before it’s buffer is flushed.
    delay(5000);

    WiFi.end();
    LowPower.deepSleep(sleepMinutes * 60 * 1000);
    wokenUp = true;
}

void loop() {
    if (wokenUp)
        initialise();

    publish(sensorsTopic, buildTempHumMessage());
    publish(systemsTopic, buildBatteryMessage());

    mqttClient.flush();
    wifiClient.flush();

    deepSleep();
    //delay(2 * 1000);
}