#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "GeneralModel.h"

#if SIMULATED_DATA

void initSensor()
{
    // use SIMULATED_DATA, no sensor need to be inited
}

float readTemperature()
{
    return random(20, 30);
}

float readHumidity()
{
    return random(30, 40);
}

#else

static DHT dht(DHT_PIN, DHT_TYPE);
void initSensor()
{
    dht.begin();
}

float readTemperature()
{
    return dht.readTemperature();
}

float readHumidity()
{
    return dht.readHumidity();
}

#endif

bool readMessage(int messageId, char *payload)
{
    float temperature = readTemperature();
    float humidity = readHumidity();
    StaticJsonDocument<MESSAGE_MAX_LEN> root;
    root["deviceId"] = deviceId;
    root["messageId"] = messageId;
    bool temperatureAlert = false;

    // NAN is not the valid json, change it to NULL
    if (std::isnan(temperature))
    {
        root["temperature"] = NULL;
    }
    else
    {
        root["temperature"] = temperature;
    }

    if (std::isnan(humidity))
    {
        root["humidity"] = NULL;
    }
    else
    {
        root["humidity"] = humidity;
    }
    serializeJson(root, payload, MESSAGE_MAX_LEN);
    printf("Payload: %s\r\n\0", payload);
    return root["temperature"] == NULL;
}

char *getSerializedMessage(General *general)
{
    char payload[MESSAGE_MAX_LEN]; // = (char*)malloc(MESSAGE_MAX_LEN);

    StaticJsonDocument<MESSAGE_MAX_LEN> root;
    root["interval"] = general->state.reported_interval;
    root["softwareVersion"] = general->state.version;
    serializeJson(root, payload, (size_t)MESSAGE_MAX_LEN);
    return payload;
}

General *parseTwinMessage(char *message)
{
    printf("parsing twin message\n");
    General *general = (General *)malloc(sizeof(General));

    if (NULL == general)
    {
        (void)printf("ERROR: Failed to allocate memory\r\n");
    }

    printf("start deserializing message..\n\0");
    StaticJsonDocument<MESSAGE_MAX_LEN> root;
    deserializeJson(root, message);
    printf("deserialized\n");
    if (root.isNull())
    {
        Serial.printf("Parse %s failed.\r\n", message);
        return 0;
    }
    printf("checking desired_interval\n");
    // interval
    if (!root["desired"]["interval"].isNull())
    {
        interval = root["desired"]["interval"];
    }
    else if (root.containsKey("interval"))
    {
        interval = root["interval"];
    }
    printf("setting interval to general\n");
    general->settings.desired_interval = interval;
    printf("general->settings.desired_interval assigned with %i\n", interval);
    // version
    if (!root["desired"]["softwareVersion"].isNull())
    {
        printf("Setting general->state.version to %s\n", root["desired"]["softwareVersion"]);
        general->state.version = root["desired"]["softwareVersion"];
    }
    else if (root.containsKey("softwareVersion"))
    {
        general->state.version = root["softwareVersion"];
    }

    printf("everything done.. returning to caller\n");
    return general;
}
