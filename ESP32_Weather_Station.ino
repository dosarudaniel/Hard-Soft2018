#define ARDUINOJSON_ENABLE_PROGMEM 0

#include "Adafruit_BME280.h" //https://github.com/Takatsuki0204/BME280-I2C-ESP32
#include <ArduinoJson.h>    //https://github.com/bblanchon/ArduinoJson
#include <WiFi.h>
#include <math.h>

#define MAX_12BITS 4095

#define I2C_SDA 27
#define I2C_SCL 26
#define LED_PIN 2
#define BME280_ADDRESS 0x76  //If the sensor does not work, try the 0x77 address as well
#define SOIL_HUMIDITY_MODULE_PORT A0
#define trigPin 23
#define echoPin 22
#define trigPin1 9
#define echoPin1 10
#define trigPin2 5
#define echoPin2 6

#define ALTITUDE 325.0 // Altitude in Suceava, Romania
#define MOUNT_DISTANCE 14.50 // in cm, max 400cm, min 5cm for snowAcc

const char* ssid     = "House MD";
const char* password = "aspire2inspire";

float temperature = 0;
float humidity = 0;
float pressure = 0;
float snowAcc = 0;
int soil_hum = 0;
int weatherID = 0;

long duration1, duration2;

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
 getSnowAcc();
 getSoilHumidity();
 getWind();
 getWeatherData(); // send data to server using GET
 delay(2000);      // change this to change "sample rate"
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
  initSnow();
  initWind();
}

void initSnow()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void initWind() {
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
}

float getSnowAcc()
{
  long duration;
  digitalWrite(trigPin, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  Serial.println(duration);
  float soundSpeed;
  //float temperature = 25; // in grade celsius
  soundSpeed = 20.05*sqrt(temperature + 273.15); // metri pe sec
  
  float distance = (duration*soundSpeed/2)/10000; // in cm * 
  snowAcc = MOUNT_DISTANCE - distance;
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

int getSoilHumidity() {
    soil_hum = analogRead(SOIL_HUMIDITY_MODULE_PORT);
    soil_hum = MAX_12BITS - soil_hum;
    soil_hum = soil_hum * 100 / MAX_12BITS;
}

void getWind() {
  // N-S
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  // W-E
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  // show results
  Serial.print(duration1);
  Serial.print(" ");
  Serial.print(duration2);
  Serial.println();
  
  //delay(500);
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
    //  String url = "/api/s/1/"+String(snowAcc); // send snow accumulation
    // TODO:add soil humidity String url = "/api/?/1/"+String(soil_hum); // soil humidity
    // TODO: add wind(when its done)
    String url = "/api/a/1/"+String(temperature)+ "/" + String(pressure) + "/" + String(humidity)+ "/" + String(snowAcc);
    
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
