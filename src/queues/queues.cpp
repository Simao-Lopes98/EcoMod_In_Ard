#include "queues.hpp"

namespace queues
{

    QueueHandle_t data; // Mailbox
    QueueHandle_t sta_cred;
    QueueHandle_t i2c_readings; // Mailbox
    QueueHandle_t modbus_readings;//Mailbox
    QueueHandle_t pump_rpm;

    void setup()
    {
        data = xQueueCreate(1, sizeof(uint8_t));

        sta_cred = xQueueCreate(2, sizeof(STA_cred_t));
        i2c_readings = xQueueCreate(1, sizeof(I2C_readings_t));
        modbus_readings = xQueueCreate(1, sizeof(Modbus_readings_t));
        modbus_readings = xQueueCreate(2, sizeof(uint16_t));
    }
}
