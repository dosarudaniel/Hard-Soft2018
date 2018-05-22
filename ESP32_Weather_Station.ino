    /////////////////////////////////////////////////////////////////
   //         ESP32 Weather Station Project     v1.00             //
  //       Get the latest version of the code here:              //
 //         http://educ8s.tv/esp32-weather-station              //
/////////////////////////////////////////////////////////////////


#define ARDUINOJSON_ENABLE_PROGMEM 0

#include "Adafruit_BME280.h" //https://github.com/Takatsuki0204/BME280-I2C-ESP32
#include <ArduinoJson.h>    //https://github.com/bblanchon/ArduinoJson
#include <WiFi.h>

const char* ssid     = "House MD";
const char* password = "aspire2inspire";
String CityID = "253394"; //Sparta, Greece
String APIKEY = "yourAPIkey";
#define ALTITUDE 216.0 // Altitude in Sparta, Greece

#define I2C_SDA 27
#define I2C_SCL 26
#define LED_PIN 2
#define BME280_ADDRESS 0x76  //If the sensor does not work, try the 0x77 address as well

float temperature = 0;
float humidity = 0;
float pressure = 0;
int weatherID = 0;

Adafruit_BME280 bme(I2C_SDA, I2C_SCL);

WiFiClient client;
char* servername ="54.149.135.147";  // remote server we will connect to
String result;

String weatherDescription ="";
String weatherLocation = "";
float Temperature;

void setup() 
{
  Serial.begin(9600);

  initSensor();

  connectToWifi();
}

void loop() {
 getTemperature();
 
 getHumidity();
 
 getPressure();
 getWeatherData();
 delay(2000);
}

void connectToWifi()
{
  WiFi.enableSTA(true);
  
  delay(2000);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
}

void initSensor()
{
  bool status = bme.begin(BME280_ADDRESS);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

float getTemperature()
{
  temperature = bme.readTemperature();
}

float getHumidity()
{
  humidity = bme.readHumidity();
}

float getPressure()
{
  pressure = bme.readPressure();
  pressure = bme.seaLevelForAltitude(ALTITUDE,pressure);
  pressure = pressure/100.0F;
}

void getWeatherData() //client function to send/receive GET request data.
{
  String result ="";
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(servername, httpPort)) {
        return;
    }
      // We now create a URI for the request
    //String url = "/data/2.5/forecast?id="+CityID+"&units=metric&cnt=1&APPID="+APIKEY;
    //  String url = "/api/t/1/"+String(temperature); // send temperature
    //  String url = "/api/p/1/"+String(pressure); // send pressure
    //  String url = "/api/u/1/"+String(humidity); // send humidity
    String url = "/api/a/1/"+String(temperature)+ "/" + String(pressure) + "/" + String(humidity);
       // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + servername + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server
    while(client.available()) {
        result = client.readStringUntil('\r');
    }

result.replace('[', ' ');
result.replace(']', ' ');

char jsonArray [result.length()+1];
result.toCharArray(jsonArray,sizeof(jsonArray));
jsonArray[result.length() + 1] = '\0';

StaticJsonBuffer<1024> json_buf;
JsonObject &root = json_buf.parseObject(jsonArray);
if (!root.success())
{
  Serial.println("parseObject() failed");
}

String location = root["city"]["name"];
String temperature = root["list"]["main"]["temp"];
String weather = root["list"]["weather"]["main"];
String description = root["list"]["weather"]["description"];
String idString = root["list"]["weather"]["id"];
String timeS = root["list"]["dt_txt"];

weatherID = idString.toInt();
Serial.println (url);
}
