#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
// Mailbox = Task Overwrite ->[]-> Task Peek

// When sending pointers to allocated memory free them on the receiving task, or if its a mailbox queue free it before overwriting

namespace queues
{
    void setup();

    struct STA_cred_t
    {
        char ssid [35];
        char password [35];
    };
    

    extern QueueHandle_t data; // Mailbox
    extern QueueHandle_t sta_cred;
    extern QueueHandle_t ph_reading;// Mailbox
    extern QueueHandle_t ec_reading;// Mailbox
}
