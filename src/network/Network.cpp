#include "Network.hpp"

namespace Network
{
    const char *TAG = "Network";

    void setup_wifi()
    {
        WiFi.mode(WIFI_AP_STA);
        service_log(TAG, "Starting AP");
        service_log(TAG, ENV_AP_SSID);

        WiFi.softAP(ENV_AP_SSID, ENV_AP_PASSWORD);

        service_log(TAG, "Connected to WiFi");
        service_log(TAG, "AP IP address: %s", WiFi.softAPIP().toString());

        #if !ENV_SOLO_AP_MODE
            service_log(TAG, "Connecting to STA %s", ENV_SSID);

            WiFi.begin(ENV_SSID, ENV_PASSWORD);

            while (WiFi.status() != WL_CONNECTED)
            {
                service_log(TAG, "Trying to connect");
                vTaskDelay(750 / portTICK_PERIOD_MS);
            }
            service_log(TAG, "WiFi connected");
            service_log(TAG, "IP address: ");
            service_log(TAG, "%s", WiFi.localIP().toString());
        #endif
    }

    void change_STA_cred(const char *newSsid, const char *newPassword)
    {
        WiFi.disconnect(true, false);
        while (WiFi.status() == WL_CONNECTED)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            service_log(TAG, "Disconnecting from current Wi-Fi network...");
        }
        WiFi.begin(newSsid, newPassword);
        service_log(TAG,"Connecting to new Wi-Fi network %s", newSsid);

        while (WiFi.status() != WL_CONNECTED) //! It can get stuck..
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            service_log(TAG, "Connecting to new Wi-Fi network...");
        }

        service_log(TAG, "Connected to new Wi-Fi network");
        service_log(TAG, "New STA IP address: %s", WiFi.localIP().toString());
    }

    void taskNetwork(void *pvParameters)
    {
        setup_wifi();

        service_log(TAG, "Booted");
        while (true)
        {
            if (uxQueueMessagesWaiting(queues::sta_cred))
            {
                queues::STA_cred_t inc_cred;
                xQueueReceive(queues::sta_cred, &inc_cred, 10 / portTICK_PERIOD_MS);
                change_STA_cred(inc_cred.ssid, inc_cred.password);
            }
            vTaskDelay(2500 / portTICK_PERIOD_MS);
        }
    }
}