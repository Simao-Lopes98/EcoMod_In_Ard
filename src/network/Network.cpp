#include "Network.hpp"

namespace Network
{
    const char *TAG = "Network";

    esp_err_t write_cred_nvs(const char *newSsid, const char *newPassword)
    {
        
        const char *key_word_ssid = "creds_newSsid";
        const char *key_word_pass = "creds_newPass";
        nvs_handle_t my_handle;

        //Save SSID
        esp_err_t err = nvs_open(key_word_ssid, NVS_READWRITE, &my_handle);
        if (err != ESP_OK) {
            service_log(TAG,"Error (%s) opening NVS handle!\n", esp_err_to_name(err));
            return err;
        }
        nvs_set_str(my_handle,key_word_ssid,newSsid);
        err = nvs_commit(my_handle);
        if (err != ESP_OK)
        {
            service_log(TAG,"Error saving new SSID");
            return err;
        }
        nvs_close(my_handle);
        
        //Save Password
        esp_err_t err = nvs_open(key_word_pass, NVS_READWRITE, &my_handle);
        if (err != ESP_OK) {
            service_log(TAG,"Error (%s) opening NVS handle!\n", esp_err_to_name(err));
            return err;
        }
        nvs_set_str(my_handle,key_word_pass,newPassword);
        err = nvs_commit(my_handle);
        if (err != ESP_OK)
        {
            service_log(TAG,"Error saving new Password");
            return err;
        }
        nvs_close(my_handle);
        
        return err;
    }

    esp_err_t read_string_nvs(const char * key_word, char *string)
    {
        nvs_handle_t my_handle;
        //Save SSID
        esp_err_t err = nvs_open(key_word, NVS_READWRITE, &my_handle);
         if (err != ESP_OK) {
            service_log(TAG,"Error (%s) opening NVS handle!\n", esp_err_to_name(err));
            strcpy(string,"\0"); //Returns empty string
            return err;
        }
        size_t size = 0;
        nvs_get_str(my_handle,key_word,string, &size);
        service_log(TAG,"String read = %s", string);
        return ESP_OK;
    }

    void setup_wifi()
    {
        char SSID[32];
        char Password[32];

        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
        }

        WiFi.mode(WIFI_AP_STA);
        service_log(TAG,"Starting AP");
        service_log(TAG,ENV_AP_SSID);

        WiFi.softAP(ENV_AP_SSID, ENV_AP_PASSWORD);

        service_log(TAG,"Connected to WiFi");
        Serial.print("AP IP address: ");
        service_log(TAG,"%s",WiFi.softAPIP().toString());

        #if !ENV_SOLO_AP_MODE
            read_string_nvs("creds_newSsid",SSID);
            read_string_nvs("creds_newSsid",Password);
            if(strlen(SSID) == 0 || strlen(Password) == 0)
            {
                service_log(TAG,"Connecting to STA");
                service_log(TAG,ENV_SSID);

                WiFi.begin(ENV_SSID, ENV_PASSWORD);

                while (WiFi.status() != WL_CONNECTED)
                {
                    service_log(TAG,"Network: Trying to connect");
                    vTaskDelay(750/portTICK_PERIOD_MS);
                }
                service_log(TAG,"WiFi connected");
                service_log(TAG,"IP address: ");
                service_log(TAG,"%s",WiFi.localIP().toString());
            }else{
                service_log(TAG,"Connecting to STA");
                service_log(TAG,SSID);

                WiFi.begin(SSID, Password);

                while (WiFi.status() != WL_CONNECTED)
                {
                    service_log(TAG,"Network: Trying to connect");
                    vTaskDelay(750/portTICK_PERIOD_MS);
                }
                service_log(TAG,"WiFi connected");
                service_log(TAG,"IP address: ");
                service_log(TAG,"%s",WiFi.localIP().toString()); 
            }
            
        #endif
    }

    void change_STA_cred(const char *newSsid, const char *newPassword)
    {
        WiFi.disconnect(true,false);
        while (WiFi.status() == WL_CONNECTED)
        {
            vTaskDelay(1000/portTICK_PERIOD_MS);
            service_log(TAG,"Disconnecting from current Wi-Fi network...");
        }
        write_cred_nvs(newSsid,newPassword);
        WiFi.begin(newSsid, newPassword);

        Serial.print("Connecting to new Wi-Fi network ");
        service_log(TAG,newSsid);

        while (WiFi.status() != WL_CONNECTED)//! It can get stuck..
        {
            vTaskDelay(1000/portTICK_PERIOD_MS);
            service_log(TAG,"Connecting to new Wi-Fi network...");
        }

        service_log(TAG,"Connected to new Wi-Fi network");
        Serial.print("New STA IP address: ");
        service_log(TAG,"%s",WiFi.localIP().toString());
    }

    void taskNetwork(void *pvParameters)
    {
        setup_wifi();

        service_log(TAG,"Network: Booted");
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