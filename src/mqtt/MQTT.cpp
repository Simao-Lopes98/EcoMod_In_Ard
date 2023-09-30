#include "MQTT.hpp"

namespace MQTT
{
    WiFiClient espClient;
    PubSubClient client(espClient);

    char ph_reading[10];
    char ec_reading[10];
    char packet[512];

    void callback(char *topic, byte *payload, unsigned int length)
    {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");
        for (int i = 0; i < length; i++)
        {
            Serial.print((char)payload[i]);
        }
        Serial.println();
    }

    void reconnect()
    {
        while (!client.connected())
        {
            Serial.print("Attempting MQTT connection...");
            // Create a random client ID
            String clientId = "ESP32-Simao";
            clientId += String(random(0xffff), HEX);
            // Attempt to connect
            if (client.connect(clientId.c_str()))
            {
                Serial.println("Ola");
                client.publish("inTopic", "Ola");
                client.publish("EMTopic", "Ola");
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.print(client.state());
                Serial.println(" try again in 5 seconds");
                vTaskDelay(2000 / portTICK_PERIOD_MS);
            }
        }
    }

    void process_data()
    {
        xQueuePeek(queues::ph_reading, &ph_reading, 100 / portTICK_PERIOD_MS);
        xQueuePeek(queues::ec_reading, &ec_reading, 100 / portTICK_PERIOD_MS);
        //TODO: Add the other queues

        snprintf(packet,512, "{\"ref\":\"sensIN\", \"pH\":\"%s\", \"temperatura\":\"%.2f\", \"EC\":\"%.3s\"}", ph_reading, 10.2, ec_reading);
    }

    void taskMQTT(void *pvParameters) // Task de envio de parametros para o broker
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        client.setServer(ENV_MQTT_BROKER, 1883);
        client.setCallback(callback);
        Serial.println("MQTT: Booted");
        while (true)
        {
            if (!client.connected())
            {
                reconnect();
            }
            client.loop();

            process_data();
            client.publish("sensors/input", packet);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}
