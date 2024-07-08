#include <Adafruit_NeoPixel.h>
#include "TRSensors.h"
#include <Wire.h>

#define PWMA   6           //Left Motor Speed pin (ENA)
#define AIN2   A0          //Motor-L forward (IN2).
#define AIN1   A1          //Motor-L backward (IN1)
#define PWMB   5           //Right Motor Speed pin (ENB)
#define BIN1   A2          //Motor-R forward (IN3)
#define BIN2   A3          //Motor-R backward (IN4)
#define PIN 7
#define NUM_SENSORS 5
#define Addr  0x20

TRSensors trs =   TRSensors();
unsigned int sensorValues[NUM_SENSORS];
unsigned int position;
uint16_t i, j;
byte value;
unsigned long lasttime = 0;
Adafruit_NeoPixel RGB = Adafruit_NeoPixel(4, PIN, NEO_GRB + NEO_KHZ800);

void PCF8574Write(byte data);
byte PCF8574Read();
uint32_t Wheel(byte WheelPos);

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println("TRSensor example");
  Wire.begin();
  pinMode(PWMA,OUTPUT);                     
  pinMode(AIN2,OUTPUT);      
  pinMode(AIN1,OUTPUT);
  pinMode(PWMB,OUTPUT);       
  pinMode(AIN1,OUTPUT);     
  pinMode(AIN2,OUTPUT);  
  SetSpeeds(0,0);
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  
  while(value != 0xEF)  //wait button pressed
  {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;
  }
  
  RGB.begin();
  RGB.setPixelColor(0,0x00FF00 );
  RGB.setPixelColor(1,0x00FF00 );
  RGB.setPixelColor(2,0x00FF00 );
  RGB.setPixelColor(3,0x00FF00);
  RGB.show(); 
  delay(500);
//  analogWrite(PWMA,60);
//  analogWrite(PWMB,60);
  for (int i = 0; i < 100; i++)  // make the calibration take about 10 seconds
  {
    if(i<25 || i >= 75)
    { 
      SetSpeeds(80,-80);
    }
    else
    {
        SetSpeeds(-80,80);
    }
    trs.calibrate();       // reads all sensors 100 times
  }
  SetSpeeds(0,0); 
  RGB.setPixelColor(0,0x0000FF );
  RGB.setPixelColor(1,0x0000FF );
  RGB.setPixelColor(2,0x0000FF );
  RGB.setPixelColor(3,0x0000FF);
  RGB.show(); // Initialize all pixels to 'off'
  
  value = 0;
  while(value != 0xEF)  //wait button pressed
  {
    PCF8574Write(0x1F | PCF8574Read());
    value = PCF8574Read() | 0xE0;
    position = trs.readLine(sensorValues)/200;
 
  }

}

// This function, causes the 3pi to follow a segment of the maze until
// it detects an intersection, a dead end, or the finish.
void follow_segment()
{
  int last_proportional = 0;
  long integral=0;

  while(1)
  {
    // Get the position of the line.
    unsigned int position = trs.readLine(sensorValues);

    int proportional = ((int)position) - 2000;
    int derivative = proportional - last_proportional;
    integral += proportional;
    last_proportional = proportional;

    int power_difference = proportional/20 + derivative*10 + integral/10000 ;                          //PID

    const int maximum = 80; // the maximum speed
    if (power_difference > maximum)
      power_difference = maximum;
    if (power_difference < -maximum)
      power_difference = - maximum;

    if (power_difference < 0)
    {
      analogWrite(PWMA,maximum + power_difference);
      analogWrite(PWMB,maximum);
    }
    else
    {
      analogWrite(PWMA,maximum);
      analogWrite(PWMB,maximum - power_difference);
    }

   if(millis() - lasttime >100)                                                          // when encounter a dead end or a junction it come out of the function
   {
    if (sensorValues[1] < 150 && sensorValues[2] < 150 && sensorValues[3] < 150)
    {
      return;
    }
    else if (sensorValues[0] > 600 || sensorValues[4] > 600)
    {
      return;
    }
   }
  }
}

void turn(unsigned char dir)
{
 // if(millis() - lasttime >500)
  {
    switch(dir)
    {
    case 'L':
      // Turn left.
      SetSpeeds(-80, 80);
      delay(180);                                            //delay
      break;
    case 'R':
      // Turn right.
      SetSpeeds(80, -80);
      delay(180);                                            //delay
      break;
    case 'B':
      // Turn around.
      SetSpeeds(70, -70);
      delay(420);                                            //delay
      break;
    case 'S':
      // Don't do anything!
      break;
    }
  }
  SetSpeeds(0, 0);
  // Serial.write(dir);
  // Serial.println();
  lasttime = millis();   
}

unsigned char select_turn(unsigned char found_left, unsigned char found_straight, unsigned char found_right)         // LSRB priority function
{
  if (found_left)
    return 'L';
  else if (found_straight)
    return 'S';
  else if (found_right)
    return 'R';
  else
    return 'B';
}

char path[300] = "";
unsigned char path_length = 0;      // the length of the path

void simplify_path()
{
  if (path_length < 3 || path[path_length-2] != 'B')       // only simplify the path if the second-to-last turn was a 'B'
    return;

  int total_angle = 0;
  for (int i = 1; i <= 3; i++)
  {
    switch (path[path_length - i])
    {
    case 'R':
      total_angle += 90;
      break;
    case 'L':
      total_angle += 270;
      break;
    case 'B':
      total_angle += 180;
      break;
    }
  }

  // Get the angle as a number between 0 and 360 degrees.
  total_angle = total_angle % 360;

  // Replace all of those turns with a single one.
  switch (total_angle)
  {
  case 0:
    path[path_length - 3] = 'S';
    break;
  case 90:
    path[path_length - 3] = 'R';
    break;
  case 180:
    path[path_length - 3] = 'B';
    break;
  case 270:
    path[path_length - 3] = 'L';
    break;
  }

  // The path is now two steps shorter.
  path_length -= 2;
}

void loop() {
  while (1)
  {
    follow_segment();
    // Slowing down when an intersection is detected
    SetSpeeds(30, 30);
    delay(40);                                                                                              //delay

    unsigned char found_left = 0;
    unsigned char found_straight = 0;
    unsigned char found_right = 0;

    // Now read the sensors and check the intersection type.
    trs.readLine(sensorValues);

    // Check for left and right exits.
    if (sensorValues[0] > 600)
      found_left = 1;
    if (sensorValues[4] > 600)
      found_right = 1;

    // Drive straight a bit more - this is enough to line up our wheels with the intersection.
    SetSpeeds(30, 30);
    delay(100);                                                                                // change the delay here to go forward for inspection

    // Check for a straight exit.
    trs.readLine(sensorValues);
    if (sensorValues[1] > 600 || sensorValues[2] > 600 || sensorValues[3] > 600)
      found_straight = 1;

    // Check for the ending spot.
    // If all three middle sensors are on dark black, we have
    
    if (sensorValues[1] > 600 && sensorValues[2] > 600 && sensorValues[3] > 600)                                        
    { SetSpeeds(50, 50);
      delay(100);
      if(sensorValues[0] > 600 && sensorValues[4] > 600){
      SetSpeeds(0, 0);  // solved the maze.
      break;
      }
    }

    // Intersection identification is complete.
    unsigned char dir = select_turn(found_left, found_straight, found_right);

    // Make the turn indicated by the path.
    turn(dir);

    // Store the intersection in the path variable.
    path[path_length] = dir;
    path_length++;

    // Simplify the learned path.
    simplify_path();
  }

  // Solved the maze!

  // Now enter an infinite loop - we can re-run the maze as many times as we want to.
  while (1)
  {
    SetSpeeds(0, 0);
    Serial.println("End !!!");

    delay(500);

    value = 0;
    while(value != 0xEF)  //wait button pressed
    {
      PCF8574Write(0x1F | PCF8574Read());
      value = PCF8574Read() | 0xE0;
    }
    delay(1000);

    // Re-run the maze.  It's not necessary to identify the intersections, so this loop is really simple.
    for (int i = 0; i < path_length; i++)
    {
      follow_segment();
      SetSpeeds(30, 30);                                          // Drive straight while slowing down, as before.
      delay(190);

      // Make a turn according to the instruction stored in path[i].
      turn(path[i]);
    }

    // Follow the last segment up to the finish.
    follow_segment();

    // Now we should be at the finish!  Restart the loop.
  }
}

void SetSpeeds(int Aspeed,int Bspeed)
{
  if(Aspeed < 0)
  {
    digitalWrite(AIN1,HIGH);
    digitalWrite(AIN2,LOW);
    analogWrite(PWMA,-Aspeed);      
  }
  else
  {
    digitalWrite(AIN1,LOW); 
    digitalWrite(AIN2,HIGH);
    analogWrite(PWMA,Aspeed);  
  }
  
  if(Bspeed < 0)
  {
    digitalWrite(BIN1,HIGH);
    digitalWrite(BIN2,LOW);
    analogWrite(PWMB,-Bspeed);      
  }
  else
  {
    digitalWrite(BIN1,LOW); 
    digitalWrite(BIN2,HIGH);
    analogWrite(PWMB,Bspeed);  
  }
}

void PCF8574Write(byte data)
{
  Wire.beginTransmission(Addr);
  Wire.write(data);
  Wire.endTransmission(); 
}

byte PCF8574Read()
{
  int data = -1;
  Wire.requestFrom(Addr, 1);
  if(Wire.available()) {
    data = Wire.read();
  }
  return data;
}
