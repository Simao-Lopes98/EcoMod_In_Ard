#include "MQTT.hpp"

namespace MQTT
{
    WiFiClient espClient;
    PubSubClient client(espClient);

    queues::I2C_readings_t rcv_i2c_readings;
    queues::Modbus_readings_t rcv_modbus_readings;
    char packet[512];
    uint16_t msg_sec_period = ENV_SEND_PERIOD_SEC;

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
                client.publish(ENV_IN_SENSORS_TOPIC, packet);
                #if ENV_MQTT_DEBUG
                    Serial.printf("MQTT: Packet sent: %s",packet);
                #endif
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
        if(uxQueueMessagesWaiting(queues::i2c_readings))
        {
            xQueuePeek(queues::i2c_readings, &rcv_i2c_readings, 100 / portTICK_PERIOD_MS);
        }
        if(uxQueueMessagesWaiting(queues::modbus_readings))
        {
            xQueuePeek(queues::modbus_readings, &rcv_modbus_readings, 100 / portTICK_PERIOD_MS);
        }
        //Submerged sensors
        snprintf(packet,512, "{\"ref\":\"sensIN\", \"pH\":\"%s\", \"temperatura\":\"%.2f\", \"EC\":\"%.3s\", \"Turb\":\"%.2f\", \"COD\":\"%.2f\" }"
        ,rcv_i2c_readings.ph, rcv_modbus_readings.temperature, rcv_i2c_readings.ec,rcv_modbus_readings.turbidity,rcv_modbus_readings.COD);
    }
    
    void initialize_values()
    {
        strcpy(rcv_i2c_readings.ec,"0");
        strcpy(rcv_i2c_readings.ph,"0.0");
        rcv_modbus_readings.COD=0.0;
        rcv_modbus_readings.turbidity=0.0;
        rcv_modbus_readings.temperature=0.0;
    }

    void taskMQTT(void *pvParameters) // Task de envio de parametros para o broker
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        client.setServer(ENV_MQTT_BROKER, 1883);
        client.setCallback(callback);
        initialize_values();
        Serial.println("MQTT: Booted");
        while (true)
        {
            if (!client.connected())
            {
                reconnect();
            }
            client.loop();

            process_data();
            client.publish(ENV_IN_SENSORS_TOPIC, packet);
            #if ENV_MQTT_DEBUG
                Serial.printf("MQTT: Packet sent: %s",packet);
            #endif

            if(uxQueueMessagesWaiting(queues::msg_period))
            {
                xQueueReceive(queues::msg_period,&msg_sec_period,10/portTICK_PERIOD_MS);
                #if ENV_MQTT_DEBUG
                    Serial.printf("MQTT: Message sending waiting time updated to: %"PRIu16"\n",msg_sec_period);
                #endif
            }

            vTaskDelay(msg_sec_period * 1000 / portTICK_PERIOD_MS);
        }
    }
}
