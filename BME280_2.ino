#include "SparkFunBME280.h"
//Library allows either I2C or SPI, so include both.
#include "Wire.h"
#include "SPI.h"
 
BME280 capteur;
 
void setup() {
 
   Serial.begin(9600);
  while (!Serial) {
    // Attente de l'ouverture du port série pour Arduino LEONARDO
  }
  
  //configuration du capteur
  capteur.settings.commInterface = I2C_MODE; 
  capteur.settings.I2CAddress = 0x76;
  capteur.settings.runMode = 3; 
  capteur.settings.tStandby = 0;
  capteur.settings.filter = 0;
  capteur.settings.tempOverSample = 1 ;
  capteur.settings.pressOverSample = 1;
  capteur.settings.humidOverSample = 1;
 
  Serial.println("Starting BME280... ");
  delay(10);  // attente de la mise en route du capteur. 2 ms minimum
  // chargement de la configuration du capteur
  capteur.begin();
}
 
void loop() {
  Serial.print("Température: ");
  Serial.print(capteur.readTempC(), 2);
  Serial.print(" °C");
  Serial.print("\t Pression: ");
  Serial.print(capteur.readFloatPressure(), 2);
  Serial.print(" Pa");
  Serial.print("\t humidité relative : ");
  Serial.print(capteur.readFloatHumidity(), 2);
  Serial.println(" %");
  delay(1000);
}
