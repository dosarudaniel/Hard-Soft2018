#include <math.h>
// directia 1 == N-S
// directia 2 == E-V

#define NR_TESTS 10
#define SENZITIVITATE 10
// All of the following pins are analogic pin from arduino UNO
const int trigPin1 = 9;
const int echoPin1 = 10;

const int trigPin2 = 5;
const int echoPin2 = 6;

long duration1, duration2;
long dur1[NR_TESTS];
long dur2[NR_TESTS];
float avg1 = 687.0; // Experiment in lab
float avg2 = 639.0; // Experiment in lab
  
void setup() {
  Serial.begin(9600);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
}

double max1 = -1;
double min1 = 100000;

double max2 = -1;
double min2 = 100000;

#define STILL -1
#define N      0
#define NE     45
#define E      90
#define SE     135
#define S      180
#define SV     225
#define V      270
#define NV     315

int direction;

void loop() {
  double speed = 0.0;
  direction = STILL;
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

  double old_avg1 = avg1;
  double old_avg2 = avg2;
  avg1 = sum1/NR_TESTS;
  avg2 = sum2/NR_TESTS;

  double dif1 = avg1 - old_avg1;
  double dif2 = avg2 - old_avg2;
  
  char c1;
  char c2;

  if (dif1 > SENZITIVITATE) {
      Serial.println("Bate vântul de la SUD");
      c1 = 'S';
  } else if (-dif1 > SENZITIVITATE){
      Serial.println("Bate vântul de la NORD");
      c1 = 'N';
  } else { 
      Serial.println("NU Bate vântul de la N-S");
      c1 = ' ';
  }

  if (dif2 > SENZITIVITATE) {
      Serial.println("Bate vântul de la Est");
      c2 = 'E';
  } else if (-dif2 > SENZITIVITATE){
      Serial.println("Bate vântul de la Vest");
      c2 = 'V';
  } else { 
      Serial.println("NU Bate vântul de la V-E");
      c2 = ' ';
  }

  String dirString = String(c1)+String(c2);
  if (dirString == "N ") {
    direction = N;
  } 
  if (dirString == "NE") {
    direction = NE;
  }
  if (dirString == " E") {
    direction = E;
  }
  if (dirString == "SE") {
    direction = SE;
  }
  if (dirString == "S ") {
    direction = S;
  }
  if (dirString == "SV") {
    direction = SV;
  }
  if (dirString == " V") {
    direction = V;
  }
  if (dirString == "NV") {
    direction = NV;
  }

  dif1 = abs(dif1 - SENZITIVITATE);
  dif2 = abs(dif2 - SENZITIVITATE);
  speed = sqrt(dif1*dif1 + dif2*dif2);
  speed = speed/10; // Calibration
  if(isnan(speed)) {
    Serial.println("E NAN " + String(dif1) + " " +String(dif2));
  }
  if (direction != STILL) {
    Serial.println(String(direction) + " " + String(speed));
  } else {
    Serial.println("0 0");
  }

  Serial.print(avg1);
  Serial.print(" ");
  Serial.print(avg2);
  Serial.println();
  Serial.println("====");

  delay(1000);
}
