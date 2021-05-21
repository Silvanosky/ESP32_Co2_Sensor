#include <Arduino.h>
#include "MHZ19.h"                        
#include <HardwareSerial.h>                               //  Remove if using HardwareSerial or non-uno compatabile device

#define CUSTOM_SETTINGS
#define INCLUDE_SENSOR_MODULE
#define INCLUDE_DATALOGGER_MODULE
#define INCLUDE_NOTIFICATION_MODULE
#include <DabbleESP32.h>
bool isFileOpen = true;
uint8_t closeFileSignalPin = 2;   //this pin is internally pulled up and a push button grounded on one side is connected to pin so that pin detects low logic when push button is pressed.

void initializeFile(){
  Serial.println("Initialize");
  DataLogger.createFile("Air Quality");
  DataLogger.createColumn("C02");
  DataLogger.createColumn("Temp");
}


#define RX_PIN 27 
#define TX_PIN 14 
#define BAUDRATE 9600                                     // Native to the sensor (do not change)

MHZ19 myMHZ19;
HardwareSerial mySerial(1);

unsigned long getDataTimer = 0;

void setup()
{
    Serial.begin(9600);  
   
    mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN);               //  Uno example: Begin Stream with MHZ19 baudrate
    myMHZ19.begin(mySerial);                // *Important, Pass your Stream reference here

    myMHZ19.autoCalibration(false);         // Turn auto calibration OFF

    pinMode(closeFileSignalPin,INPUT_PULLUP);
    Dabble.begin("Silvanosky_C02");    //set bluetooth name of your device
    DataLogger.sendSettings(&initializeFile);
    Dabble.waitForAppConnection();
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
        if( isFileOpen == true)
        {
          DataLogger.send("C02", CO2);
          DataLogger.send("Temp", Temp);
        }
        if (CO2 > 700) {
          Notification.notifyPhone(String("CO2 Level : ") + CO2 + String ("ppm !"));
        }

        getDataTimer = millis();
    }
    if((digitalRead(closeFileSignalPin) == LOW) && isFileOpen == true)
    {
      isFileOpen = false;
      DataLogger.stop();
    }
}
