#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "GeneralModel.h"
#include <ESP8266WiFi.h>

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
    char payload[MESSAGE_MAX_LEN];
    StaticJsonDocument<MESSAGE_MAX_LEN> root;
    root["interval"] = general->state.reported_interval;
    root["fwVersion"] = general->state.version;
    root["update_state"] = general->state.update_state;
    root["update_url"] = general->settings.update_url;
    root["device"]["heap_free"] = ESP.getFreeHeap();
    root["device"]["sketch_free"] = ESP.getFreeSketchSpace();
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

    StaticJsonDocument<MESSAGE_MAX_LEN> root;
    deserializeJson(root, message);
    printf("deserialized\n");

    if (root.isNull())
    {
        Serial.printf("Parse %s failed.\n", message);
        return 0;
    }

    printf("checking desired_interval\n");
    // interval
    if (!root["desired"]["interval"].isNull())
    {
        general->settings.desired_interval = root["desired"]["interval"];
    }
    else if (root.containsKey("interval"))
    {
        general->settings.desired_interval = root["interval"];
    }

    printf("desired interval:%i\n", general->settings.desired_interval);

    // version
    if (!root["desired"]["fwVersion"].isNull())
    {
        strcpy(general->state.version, root["desired"]["fwVersion"]);
    }
    else if (root.containsKey("fwVersion"))
    {
        strcpy(general->state.version, root["fwVersion"]);
    }

    printf("version :%s\r\n", general->state.version);

    // update url
    if (!root["desired"]["update_url"].isNull())
    {
        if (sizeof general->settings.update_url > sizeof((const char *)root["desired"]["update_url"]))
        {
            strcpy(general->settings.update_url, (const char *)root["desired"]["update_url"]);
        }
    }
    else if (root.containsKey("update_url"))
    {
        if (sizeof general->settings.update_url > sizeof((const char *)root["update_url"]))
        {
            strcpy(general->settings.update_url, (const char *)root["update_url"]);
        }
    }
    printf("update url:%s sizeof %d strlen %d\r\n", general->settings.update_url, sizeof general->settings.update_url, strlen(general->settings.update_url));
    return general;
}