#define ARDUINOJSON_ENABLE_PROGMEM 0

#include "Adafruit_BME280.h" //https://github.com/Takatsuki0204/BME280-I2C-ESP32
#include <ArduinoJson.h>    //https://github.com/bblanchon/ArduinoJson
#include <WiFi.h>
#include <math.h>
//#include <BluetoothSerial.h>

#define MAX_12BITS 4095

#define I2C_SDA 27
#define I2C_SCL 26
#define LED_PIN 2
#define BME280_ADDRESS 0x76  //If the sensor does not work, try the 0x77 address as well
#define SOIL_HUMIDITY_MODULE_PORT A0
#define trigPin 2     // snow
#define echoPin 5     // snow

#define GAS_MODULE_PORT A3

#define ALTITUDE 325.0 // Altitude in Suceava, Romania
#define MOUNT_DISTANCE 14.50 // in cm, max 400cm, min 5cm for snowAcc

// For wind: 1:N->S ; 2: W->E, please connect HCSR04 to the following pins
#define trigPin1 14 // 
#define echoPin1 12
#define trigPin2 32
#define echoPin2 33
#define NR_TESTS 10
#define SENSITIVITY 10

// All of the following pins are analogic pin from arduino UNO
long duration1, duration2;
long dur1[NR_TESTS];
long dur2[NR_TESTS];
float avg1 = 664.0; // Experiment in lab
float avg2 = 643.0; // Experiment in lab
// STILL air values
float avg1_ref;
float avg2_ref;

int windDirection; // degrees
double windSpeed = 0.0;

// Wifi SSID + password
const char* ssid     = "House MD";
const char* password = "aspire2inspire";

char* servername ="54.149.135.147";  // remote server we will connect to
String result;

// sensorsValues
float temperature = 23;
float humidity = 40;
float pressure = 1010;
float snowAcc = 0;
int soil_hum = 0;
float gas_value = 0;
int weatherID = 0;

long duration, aux;

Adafruit_BME280 bme(I2C_SDA, I2C_SCL);

//BluetoothSerial SerialBT;

String weatherDescription ="";
String weatherLocation = "";
float Temperature;

void setup()
{
  Serial.begin(9600);
  //SerialBT.begin("B_ESP32");
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
 getGas();
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
  bool status;
  while(1) {
    status = bme.begin(BME280_ADDRESS);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
  } else break;
  delay(2000);
  }
  initSnow();
  initWind();
  pinMode(GAS_MODULE_PORT, INPUT); //Set gas sensor as input
}

void initSnow() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void initWind() {
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  
  for (int i = 0; i < NR_TESTS; i++) {
      digitalWrite(trigPin1, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin1, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin1, LOW);
      duration1 = pulseIn(echoPin1, HIGH);
      dur1[i] = duration1;
    
      digitalWrite(trigPin2, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin2, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin2, LOW);
      duration2 = pulseIn(echoPin2, HIGH);
      dur2[i] = duration2;
  }
  // calculate average
  float sum1 = 0;
  float sum2 = 0;
  
  for (int i = 0; i < NR_TESTS; i++) {
     sum1 += dur1[i];
     sum2 += dur2[i];
  }
  avg1_ref = sum1/NR_TESTS;
  avg2_ref = sum2/NR_TESTS;
}

void getSnowAcc() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  //pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  aux = duration* 0.017;

  delay(250);
  snowAcc = MOUNT_DISTANCE - aux;
}

float getTemperature() {
  temperature = bme.readTemperature();
}

float getHumidity() {
  humidity = bme.readHumidity();
}

float getPressure() {
  pressure = bme.readPressure();
  pressure = pressure/100.0F;
}

int getSoilHumidity() {
  soil_hum = analogRead(SOIL_HUMIDITY_MODULE_PORT);
  soil_hum = MAX_12BITS - soil_hum;
  soil_hum = soil_hum * 100 / MAX_12BITS;
}

float getGas() {
  gas_value = analogRead(GAS_MODULE_PORT);
}

void getWind() {
  windSpeed = 0.0;
  // take measurements
  for (int i = 0; i < NR_TESTS; i++) {
      digitalWrite(trigPin1, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin1, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin1, LOW);
      duration1 = pulseIn(echoPin1, HIGH);
      dur1[i] = duration1;
    
      digitalWrite(trigPin2, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin2, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin2, LOW);
      duration2 = pulseIn(echoPin2, HIGH);
      dur2[i] = duration2;
  }
  // calculate average
  float sum1 = 0;
  float sum2 = 0;
  
  for (int i = 0; i < NR_TESTS; i++) {
     sum1 += dur1[i];
     sum2 += dur2[i];
  }

  avg1 = sum1/NR_TESTS;
  avg2 = sum2/NR_TESTS;

  double dif1 = -(avg1 - avg1_ref); // if dif1 > 0 the wind is blowing from N
  double dif2 = avg2 - avg2_ref;    // if dif2 > 0 the wind is blowing from E
  /*    Where are the ultrasonic transducers?
   *          N (TX)
              ^
              |
              |
              | 
  V(TX) ---------------------> E (RX)
              |
              |
              |
              |
              S (RX)
  */
  
  double rad = atan2 (dif1, dif2); // The returned value is in the range [-pi, +pi]
  // pi/2 means North a.k.a windDirection 0
  // pi   means West  a.k.a windDirection 270
  
  if (rad <= M_PI/2 && rad > 0) { // Cadran 1
    rad = M_PI/2 - rad;
  } else if (rad <= 0 && rad > -M_PI/2 ){  // Cadran 4
    rad = -rad + M_PI/2;
  } else if (rad <= -M_PI/2 && rad >= -M_PI){ // Cadran 3
    rad = -rad + M_PI/2;
  } else { // Cadran 2
    rad = M_PI - rad + (3/2)*M_PI;
  }

  float degrees = 180*rad/M_PI;
  
  dif1 = abs(dif1 - SENSITIVITY);
  dif2 = abs(dif2 - SENSITIVITY);
  windSpeed = sqrt(dif1*dif1 + dif2*dif2);
  // final values!!
  windSpeed = windSpeed/100; // Calibration..
  windDirection = (int) degrees;
  
  Serial.println("--------> windSpeed =  " + String(windSpeed) + " direction " +String(windDirection));
  
//  Serial.print(avg1);
//  Serial.print(" ");
//  Serial.print(avg2);
//  Serial.print("<====");
//  Serial.println();
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
  // Format /api/a/:device/:t/:ha/:hs/:p/:pd/:ws/:wd/:s/:pollution/:rain/:battery/:charging
  String url = "/api/a/";
  url += String(1) + "/";             // device
  url += String(temperature)+ "/";
  url += String(humidity) + "/";
  url += String(soil_hum) + "/"; 
  url += String(pressure) + "/";
  url += String(0) + "/";
  url += String(windSpeed) + "/";
  url += String(windDirection) + "/";
  url += String(snowAcc) + "/";
  url += String(0) + "/";
  url += String(0) + "/";
  url += String(96) + "/";
  url += String(100);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + servername + "\r\n" +
               "Connection: close\r\n\r\n");
  //SerialBT.println(url);
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
  if (!root.success()) {
    Serial.println("parseObject() failed");
  }
  
  String location = root["city"]["name"];
  String temperature = root["list"]["main"]["temp"];
  String weather = root["list"]["weather"]["main"];
  String description = root["list"]["weather"]["description"];
  String idString = root["list"]["weather"]["id"];
  String timeS = root["list"]["dt_txt"];
  
  weatherID = idString.toInt();
  Serial.println(url);
}
