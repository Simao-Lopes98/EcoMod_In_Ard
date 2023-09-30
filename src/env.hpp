#pragma once



/*
*   MQTT
*/
#define ENV_MQTT_BROKER "192.168.1.101" // endereço a ligar mqtt.eclipseprojects.io

/*
*   Network
*/
#define ENV_SSID "Vodafone-E2EE07"
#define ENV_PASSWORD "AMMDXFACHEH4744J"

/*
*   I2C
*/
#define ENV_PH_SENS_ADDR 0x63
#define ENV_EC_SENS_ADDR 0x64

/*
*
*/
#define ENV_TASK_NETWORK        1
#define ENV_TASK_MQTT           1
#define ENV_TASK_HTTPSERVER     1
#define ENV_TASK_OTA            1
#define ENV_TASK_I2C            1