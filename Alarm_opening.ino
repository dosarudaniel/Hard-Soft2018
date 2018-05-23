int buzzer = 9;//the pin of the active buzzer
int inPin = 7;     // pushbutton connected to digital pin 7
int val = 0;       // variable to store the read value

void setup()
{
  Serial.begin(9600);
  pinMode(buzzer,OUTPUT);//initialize the buzzer pin as an output
  pinMode(inPin, INPUT);        // sets the digital pin 7 as input
}

void loop()
{
  unsigned char i; //define a variable
//  delay(1000);
  val = digitalRead(inPin);     // read the input pin
  
  if(val == HIGH){
    Serial.println(val);
  }
  else
    {  
      Serial.println(val);
      //output an frequency
      for(i=0;i<80;i++)
      {
        digitalWrite(buzzer,HIGH);
        delay(1);//wait for 1ms
        digitalWrite(buzzer,LOW);
        delay(1);//wait for 1ms
      }
      //output another frequency
      for(i=0;i<100;i++)
      {
        digitalWrite(buzzer,HIGH);
        delay(2);//wait for 2ms
        digitalWrite(buzzer,LOW);
        delay(2);//wait for 2ms
      }
  }
} 
