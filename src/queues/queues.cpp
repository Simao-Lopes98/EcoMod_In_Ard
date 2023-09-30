#include "queues.hpp"

namespace queues
{

    QueueHandle_t data; // Mailbox
    QueueHandle_t sta_cred;
    QueueHandle_t ph_reading; // Mailbox
    QueueHandle_t ec_reading; // Mailbox

    void setup()
    {
        data = xQueueCreate(1, sizeof(uint8_t)); // Example queue
        sta_cred = xQueueCreate(2, sizeof(STA_cred_t));
        ph_reading = xQueueCreate(1, 10 * sizeof(char));
        ec_reading = xQueueCreate(1, 10 * sizeof(char));
    }
}
