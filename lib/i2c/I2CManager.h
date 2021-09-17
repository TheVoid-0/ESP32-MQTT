#pragma once
#include <Arduino.h>
#include <Wire.h>

class I2CManager
{
private:
    bool is_done_scanning;
    bool is_same_address;
    std::vector<uint8_t> addresses;

public:
    void find_I2C_Address();
};

void I2CManager::find_I2C_Address()
{
    if (!Wire.available())
    {
        Wire.begin();
    }

    if (is_done_scanning)
        return;
    byte error, address;
    int nDevices;

    is_same_address = true;

    Serial.println("Scanning...");
    int foundThisRound = 0;

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println(" !");

            if (foundThisRound % 2 == 0)
            {
                if (addresses[0] != address)
                {
                    Serial.println("address 0 diferente");
                    addresses[0] = address;
                    is_same_address = false;
                }
            }
            else
            {
                if (addresses[1] != address)
                {
                    Serial.println("address 1 diferente");
                    addresses[1] = address;
                    is_same_address = false;
                }
            }
            foundThisRound++;
            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
    {
        Serial.println("done\n found: ");
        Serial.println(foundThisRound);
        is_done_scanning = true;
    }
    delay(1000); // wait 1 seconds for next scan
}