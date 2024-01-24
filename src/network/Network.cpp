#include "Network.hpp"

namespace Network
{

    esp_err_t write_cred_nvs(const char *newSsid, const char *newPassword)
    {
        
        const char *key_word_ssid = "creds_newSsid";
        const char *key_word_pass = "creds_newPass";
        nvs_handle_t my_handle;

        //Save SSID
        esp_err_t err = nvs_open(key_word_ssid, NVS_READWRITE, &my_handle);
        if (err != ESP_OK) {
            Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
            return err;
        }
        nvs_set_str(my_handle,key_word_ssid,newSsid);
        err = nvs_commit(my_handle);
        if (err != ESP_OK)
        {
            Serial.printf("Error saving new SSID");
            return err;
        }
        nvs_close(my_handle);
        
        //Save Password
        esp_err_t err = nvs_open(key_word_pass, NVS_READWRITE, &my_handle);
        if (err != ESP_OK) {
            Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
            return err;
        }
        nvs_set_str(my_handle,key_word_pass,newPassword);
        err = nvs_commit(my_handle);
        if (err != ESP_OK)
        {
            Serial.printf("Error saving new Password");
            return err;
        }
        nvs_close(my_handle);
        
        return err;
    }

    char *read_SSID_nvs()
    {

    }

    char *read_Pass_nvs()
    {

    }

    void setup_wifi()
    {
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }

        WiFi.mode(WIFI_AP_STA);
        Serial.println("Starting AP");
        Serial.println(ENV_AP_SSID);

        WiFi.softAP(ENV_AP_SSID, ENV_AP_PASSWORD);

        Serial.println("");
        Serial.println("Connected to WiFi");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());

        #if !ENV_SOLO_AP_MODE
            Serial.println();
            Serial.print("Connecting to STA");
            Serial.println(ENV_SSID);

            WiFi.begin(ENV_SSID, ENV_PASSWORD);

            while (WiFi.status() != WL_CONNECTED)
            {
                Serial.println("Network: Trying to connect");
                vTaskDelay(750/portTICK_PERIOD_MS);
            }
            Serial.println("");
            Serial.println("WiFi connected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
        #endif
    }

    void change_STA_cred(const char *newSsid, const char *newPassword)
    {
        WiFi.disconnect(true,false);
        while (WiFi.status() == WL_CONNECTED)
        {
            delay(1000);
            Serial.println("Disconnecting from current Wi-Fi network...");
        }

        WiFi.begin(newSsid, newPassword);

        Serial.print("Connecting to new Wi-Fi network ");
        Serial.println(newSsid);

        while (WiFi.status() != WL_CONNECTED)//! It can get stuck..
        {
            delay(1000);
            Serial.println("Connecting to new Wi-Fi network...");
        }

        Serial.println("Connected to new Wi-Fi network");
        Serial.print("New STA IP address: ");
        Serial.println(WiFi.localIP());
    }

    void taskNetwork(void *pvParameters)
    {
        setup_wifi();

        Serial.println("Network: Booted");
        while (true)
        {
            if (uxQueueMessagesWaiting(queues::sta_cred))
            {
                queues::STA_cred_t inc_cred;
                xQueueReceive(queues::sta_cred, &inc_cred, 10 / portTICK_PERIOD_MS);
                change_STA_cred(inc_cred.ssid,inc_cred.password);
            }
            vTaskDelay(2500 / portTICK_PERIOD_MS);
        }
    }
}