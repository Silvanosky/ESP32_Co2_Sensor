#include <Arduino.h>
#include "MHZ19.h"                        
#include <HardwareSerial.h>                               //  Remove if using HardwareSerial or non-uno compatabile device

#define CUSTOM_SETTINGS
#define INCLUDE_SENSOR_MODULE
#define INCLUDE_DATALOGGER_MODULE
#define INCLUDE_NOTIFICATION_MODULE
#include <DabbleESP32.h>

void initializeFile(){
  Serial.println("Initialize");
  DataLogger.createFile("Air Quality");
  DataLogger.createColumn("C02");
  DataLogger.createColumn("Temp");
}


#define RX_PIN 23
#define TX_PIN 22 
#define BAUDRATE 9600                                     // Native to the sensor (do not change)

MHZ19 myMHZ19;
HardwareSerial mySerial(1);

unsigned long getDataTimer = 0;
unsigned long timeout = 0;
unsigned long state = 0;

bool connected = true;

void setup()
{
    Serial.begin(9600);  
   
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);               //  Uno example: Begin Stream with MHZ19 baudrate
    myMHZ19.begin(mySerial);                // *Important, Pass your Stream reference here

    myMHZ19.autoCalibration(false);         // Turn auto calibration OFF

    Dabble.begin("Silvanosky_C02");    //set bluetooth name of your device
    DataLogger.sendSettings(&initializeFile);
    Notification.clear();
    Notification.setTitle("C02 Alert");
}

void loop()
{
    Dabble.processInput();             //this function is used to refresh data obtained from smartphone.Hence calling this function is mandatory in order to get data properly from your mobile.
    if (millis() - getDataTimer >= 2000)
    {
        int CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)
        
        Serial.print("CO2 (ppm): ");                      
        Serial.println(CO2);                                

        int8_t Temp;
        Temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
        Serial.print("Temperature (C): ");                  
        Serial.println(Temp);

        DataLogger.send("C02", CO2);
        DataLogger.send("Temp", Temp);
        
        if (CO2 < 700) {
          state = 0;
        } else if (CO2 > 700 && state < 1) {
          Notification.notifyPhone(String("CO2 Level : ") + CO2 + String ("ppm !"));
          state = 1;
        } else if (CO2 > 850 && state < 2) {
          Notification.notifyPhone(String("CO2 Level : ") + CO2 + String ("ppm !"));
          state = 2;
        } else if (CO2 > 1000 && state < 3) {
          Notification.notifyPhone(String("CO2 Level : ") + CO2 + String ("ppm !"));
          state = 3;
        } else if (CO2 > 1500 && state < 4) {
          Notification.notifyPhone(String("CO2 Level : ") + CO2 + String ("ppm !"));
          state = 4;
        }

        getDataTimer = millis();
    }
}
