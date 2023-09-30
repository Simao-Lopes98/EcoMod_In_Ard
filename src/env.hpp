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
*   ModBus
*/
#define ENV_PUMP_ID 2// ID da Bomba (Numero no mostrador da própria bomba)
#define ENV_TURB_ID 40 //ID do sensor de Tubr na entrada
#define ENV_COD_ID 1// ID da sensor de carga organica
#define ENV_EM_ID 3//ID da estação meterológica

/*
*
*/
#define ENV_TASK_NETWORK        1
#define ENV_TASK_MQTT           1
#define ENV_TASK_HTTPSERVER     0
#define ENV_TASK_OTA            0
#define ENV_TASK_I2C            0
#define ENV_TASK_MODBUS         1
