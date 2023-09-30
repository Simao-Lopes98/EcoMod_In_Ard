#include "ModBus.hpp"

namespace ModBus
{
    ModbusRTU mb; // Class

    bool cb(Modbus::ResultCode event, uint16_t transactionId, void *data)
    { // Callback to monitor errors
        if (event != Modbus::EX_SUCCESS)
        { // Caso ocurrer um ERRO
            Serial.print("Request result: 0x");
            Serial.print(event, HEX);
            Serial.println();
        }
        return true;
    }

    float floatCOD(long int byte0, long int byte1, long int byte2, long int byte3)
    { // Recebe os registo e converte para float (sensor COD)
        long int realbyte0, realbyte1, realbyte2, realbyte3;
        char S;
        long int E, M;
        float D;
        realbyte0 = byte3;
        realbyte1 = byte2;
        realbyte2 = byte1;
        realbyte3 = byte0;
        if ((realbyte0 & 0x80) == 0)
        {
            S = 0; // Positivo
        }
        else
        {
            S = 1; // Negativo
        }
        E = ((realbyte0 << 1) | (realbyte1 & 0x80) >> 7) - 127;
        M = ((realbyte1 & 0x7f) << 16) | (realbyte2 << 8) | realbyte3;
        D = pow(-1, S) * (1.0 + M / pow(2, 23)) * pow(2, E);
        return D;
    }

    float floatTurb(uint16_t registo1, uint16_t registo2)
    { // Recebe 2 registos em inteiro e converte em float pelo protocolo IEEE754
        float valorFloat;
        int j = 0;
        uint32_t rg = 0;
        int expAux = 0;
        int e = 0;
        float mantissa = 0;
        int auxMantissa = 0;
        rg = (registo1 << 14) | registo2;
        expAux = (rg >> 21) & 255; // Apanha o exponencial
        e = expAux - 127;
        auxMantissa = rg << 11; // Apanha a mantissa
        auxMantissa = auxMantissa >> 11;
        for (int i = 23; i > 1; i--) // Recolhe cada casa decimal e muitiplca. Ex: 2^(-23)..
        {
            mantissa = mantissa + (((auxMantissa >> j) & 1) * powl(2, -i));
            j++;
        }
        valorFloat = (1 + mantissa) * pow(2, e); // Formula final
        return valorFloat;
    }

    float read_temperature()
    {
        float temp = 1.23;
        uint16_t temperature_reg[2];

        if (!mb.slave())
        {
            mb.writeHreg(ENV_TURB_ID, REG_TURB_START_MES, 3, cb); // ID, registo e evento a escrever. 3 = Medir temperatura e Tubr
            while (mb.slave())
            {
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        if (!mb.slave())
        {
            mb.readHreg(ENV_TURB_ID, REG_TURB_TEMP, temperature_reg, 2, cb);
            while (mb.slave())
            {
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        temp = floatTurb(temperature_reg[0], temperature_reg[1]); // Faz a conversao de INT para Float
        return temp;
    }

    float read_turb()
    {
        uint16_t turb_reg[2];
        float turb = 1.23;
        if (!mb.slave())
        {
            mb.writeHreg(ENV_TURB_ID, REG_TURB_START_MES, 3, cb); // ID, registo e evento a escrever. 3 = Medir temperatura e Tubr
            while (mb.slave())
            {
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        if (!mb.slave())
        {
            mb.readHreg(ENV_TURB_ID, REG_TURB_TURB, turb_reg, 2, cb);
            while (mb.slave())
            {
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        turb = floatTurb(turb_reg[0], turb_reg[1]); // Faz a conversao de INT para Float
        return turb;
    }

    float read_cod()
    {
        float cod = 1.23;
        uint16_t cod_reg[2];
        if (!mb.slave())
        {                                                         // Check if no transaction in progress
            mb.readHreg(ENV_COD_ID, REG_COD_COD, cod_reg, 2, cb); // ID, nº do Registo, var onde guardr, evento caso falhe
            while (mb.slave())
            { // Check if transaction is active
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        cod = floatCOD(0x00, 0x00, ((cod_reg[1] & 0xFF00) >> 8), cod_reg[1] & 0x00FF); // Operação logica MSB e LSB
        return cod;
    }

    void read_EM(float *parametrosEstacaoMetero)
    {
        uint16_t regEM[1];

        for (int i = 0; i < 11; i++)
        { // Varrer todos os registos da estação
            if (!mb.slave())
            {                                            // Check if no transaction in progress
                mb.readHreg(ENV_EM_ID, i, regEM, 1, cb); // ID, nº do Registo, var onde guardr, evento caso falhe
                while (mb.slave())
                { // Check if transaction is active
                    mb.task();
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                }
            }
            parametrosEstacaoMetero[i] = regEM[0] / 10;
            if (i == 9) // Pluviosidade tem de ser dividido por 100
                parametrosEstacaoMetero[i] = regEM[0] / 100;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    int read_pump()
    {
        uint16_t reg_pump[1];
        reg_pump[0] = 123;
        // Recebe as informações da bomba
        if (!mb.slave())
        {                                                            // Para receber informação sobre a bomba
            mb.readHreg(ENV_PUMP_ID, REG_PUMP_RPM, reg_pump, 1, cb); // ID da bomba, nº do Registo, var onde guardr, evento caso falhe
            while (mb.slave())
            { // Check if transaction is active
                mb.task();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }
        return (int)reg_pump[0];
    }

    void taskModbus(void *pvParameters)
    {

        float temperatura_reading = 0.0;
        float turb_reading = 0.0;
        float COD_reading = 0.0;
        float EM_reading[12];
        int pump_RPM = 0;

        Serial1.begin(9600, SERIAL_8N2, 16, 17); // BaudRate, 1 Start Bit 8 Data e 2 StopBit, Rx,Tx
        mb.begin(&Serial1);
        mb.master();

        Serial.println("MobBus: Booted");
        while (true)
        {
            // Readings
            temperatura_reading = read_temperature();
            turb_reading = read_turb();
            COD_reading = read_cod();
            read_EM(EM_reading);
            pump_RPM = read_pump();

            // TODO: Add a ENV for debug
            Serial.println("Temperature: " + String(temperatura_reading) + " ,Turb: " + String(turb_reading) + " ,COD: " + String(COD_reading) + " ,RPM: " + String(pump_RPM));

            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }
}