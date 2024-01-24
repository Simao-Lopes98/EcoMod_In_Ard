#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "../env.hpp"
#include "../queues/queues.hpp"
#include "esp_system.h"
#include <Log.hpp>

namespace Network
{
    void setup_wifi();
    void change_STA_cred(const char *newSsid, const char *newPassword);
    void taskNetwork(void *pvParameters);
}
