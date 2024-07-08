#include <IRremote.h>

#define IR_RECEIVE_PIN 4


#define IR_BUTTON_2 24
#define IR_BUTTON_8 82
#define IR_BUTTON_4 8
#define IR_BUTTON_6 90

#define IR_BUTTON_1 12
#define IR_BUTTON_3 94
#define IR_BUTTON_5 28
#define IR_BUTTON_PLAY_PAUSE 64

#define SpeedDown 7            //Key:VOL-
#define SpeedUp 21              //Key:VOL+
#define ResetSpeed 9           //Key:EQ
#define Repeat 255               //press and hold the key

#define PWMA   6           //Left Motor 100 pin (ENA)
#define AIN2   A0          //Motor-L forward (IN2).
#define AIN1   A1          //Motor-L backward (IN1)
#define PWMB   5           //Right Motor 100 pin (ENB)
#define BIN1   A2          //Motor-R forward (IN3)
#define BIN2   A3          //Motor-R backward (IN4)
#define IR  4              //he infrare remote receiver pin 

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN);
  pinMode(IR,INPUT);
  pinMode(PWMA,OUTPUT);                     
  pinMode(AIN2,OUTPUT);      
  pinMode(AIN1,OUTPUT);
  pinMode(PWMB,OUTPUT);       
  pinMode(AIN1,OUTPUT);     
  pinMode(AIN2,OUTPUT);  
  Serial.println("IR control example");
}

void loop() {
  if (IrReceiver.decode()) {
    IrReceiver.resume();
    int command = IrReceiver.decodedIRData.command;
    switch (command) {
      case IR_BUTTON_2: {
        Serial.println("Pressed on button 1");
        forward();
        break;
      }
      case IR_BUTTON_4: {
        Serial.println("Pressed on button 2");
        left();
        break;
      }
      case IR_BUTTON_6: {
        Serial.println("Pressed on button 3");
        right();
        break;
      }
      case IR_BUTTON_5: {
        Serial.println("Pressed on button 3");
        stop();
        break;
      }
      case IR_BUTTON_8: {
        Serial.println("Pressed on button 3");
        backward();
        break;
      }
      case IR_BUTTON_PLAY_PAUSE: {
        Serial.println("Pressed on button play/pause");
        stop();
        break;
      }
      default: {
        Serial.println("Button not recognized");
        stop();
      }
    }
  }
}

void forward()
{
  analogWrite(PWMA,97);
  analogWrite(PWMB,100);
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,LOW);  
  digitalWrite(BIN2,HIGH); 
}

void backward()
{
  analogWrite(PWMA,100);
  analogWrite(PWMB,98);
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,HIGH); 
  digitalWrite(BIN2,LOW);  
}

void right()
{
  analogWrite(PWMA,50);
  analogWrite(PWMB,50);
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,HIGH);
  digitalWrite(BIN1,HIGH); 
  digitalWrite(BIN2,LOW);  
}

void left()
{
  analogWrite(PWMA,50);
  analogWrite(PWMB,50);
  digitalWrite(AIN1,HIGH);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW); 
  digitalWrite(BIN2,HIGH);  
}

void stop()
{
  analogWrite(PWMA,0);
  analogWrite(PWMB,0);
  digitalWrite(AIN1,LOW);
  digitalWrite(AIN2,LOW);
  digitalWrite(BIN1,LOW); 
  digitalWrite(BIN2,LOW);  
}
