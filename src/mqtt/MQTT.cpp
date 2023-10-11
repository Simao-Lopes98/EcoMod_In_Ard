#include "MQTT.hpp"

namespace MQTT
{
    WiFiClient espClient;
    PubSubClient client(espClient);

    queues::I2C_readings_t rcv_i2c_readings;
    queues::Modbus_readings_t rcv_modbus_readings;
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
                process_data();
                client.publish("sensors/input", packet);
                Serial.println("MQTT: Packet sent");
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
        xQueuePeek(queues::i2c_readings, &rcv_i2c_readings, 100 / portTICK_PERIOD_MS);
        xQueuePeek(queues::modbus_readings, &rcv_modbus_readings, 100 / portTICK_PERIOD_MS);

        //TODO:Add EM packet
        snprintf(packet,512, "{\"ref\":\"sensIN\", \"pH\":\"%s\", \"temperatura\":\"%.2f\", \"EC\":\"%.3s\", \"Turb\":\"%.2f\", \"COD\":\"%.2f\", \"RPM\":\"%d\" }"
        ,rcv_i2c_readings.ph, rcv_modbus_readings.temperature, rcv_i2c_readings.ec,rcv_modbus_readings.turbidity,rcv_modbus_readings.COD,rcv_modbus_readings.pump_RMP);
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
            #if ENV_MQTT_DEBUG
                Serial.println("MQTT: Packet sent");
            #endif
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}
