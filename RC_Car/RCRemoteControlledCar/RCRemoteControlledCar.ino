#include <SPI.h>  
#include "RF24.h" 

//////////////////////////////////////////////////////////
#include <Servo.h>

Servo myservo;  // create servo object to control a servo

const int sPin = 9;
//////////////////////////////////////////////////////////
// Motor
const int mEn2 = 3;
const int mIn3 = 2;
const int mIn4 = 4;

//const int lights = 5;

const int ultraFrontTrigger = 19;
const int ultraFrontEcho = 18;
const int OBSTACLE_STOP = 30;

const int MOTOR_LOWER_NON_MOVING_VALUE = 35;
const int SERVO_MINIMUM_VALUE = 5;
const int SERVO_MAXIMUM_VALUE = 175;
const int SERVO_MIDDLE_POSITION = 85;

int last_motor_speed = 125;
int servo_last_position = 85;

long duration;
int distance;

RF24 myRadio (7, 8); 
byte addresses[][6] = {"0"}; 

struct joystic_t
{
  int joyX;
  int joyY;
  int horn;
  int lights;
  char crypto[16];
  void clear()
  {
    joyX = 90;
    joyY = 128;
    lights = 0;
    horn = 0;
  }
};

typedef struct joystic_t JoysticS;
JoysticS joy_s;

bool isFrontObstacle()
{
  digitalWrite(ultraFrontTrigger, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(ultraFrontTrigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultraFrontTrigger, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ultraFrontEcho, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  Serial.print("Distance: ");
  Serial.println(distance);

  if(distance <= OBSTACLE_STOP)
  {
    if(last_motor_speed > 128)
    {
      Serial.print("last_motor_speed 222: ");
      Serial.println(last_motor_speed);
      int joyY = 128 - (last_motor_speed - 128);
      Serial.print("joyY after: ");
      Serial.println(joyY);
      //moveReverse(joyY);
    }
    return true;
  }

  return false;
}

void moveForward(int joyY)
{
//  if(isFrontObstacle())
//    return;

  digitalWrite(mIn3, HIGH);
  digitalWrite(mIn4, LOW);
  int joyVal = map(joyY, 128, 255, 0, 255);
  if(joyVal <= MOTOR_LOWER_NON_MOVING_VALUE) joyVal = 0;
  analogWrite(mEn2, joyVal);
  last_motor_speed = joyY;
  Serial.print("last_motor_speed 111: ");
  Serial.println(last_motor_speed);
}

void moveReverse(int joyY)
{
  digitalWrite(mIn3, LOW);
  digitalWrite(mIn4, HIGH);
  int joyVal = map(joyY, 128, 0, 0, 255);
  if(joyVal <= MOTOR_LOWER_NON_MOVING_VALUE) joyVal = 0;
  analogWrite(mEn2, joyVal);
  last_motor_speed = joyY;
  Serial.print("last_motor_speed 333: ");
  Serial.println(last_motor_speed);
}

void moveMotor(int joyY)
{
  if(joyY >= 255/2)
  {
    moveForward(joyY);
  }
  else if(joyY < 255/2)
  {
    moveReverse(joyY);
  }
}

void setup() 
{
  Serial.begin(115200);
  delay(10);

  myRadio.begin();
  myRadio.setChannel(100);
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.startListening();

  myservo.attach(sPin);

  pinMode(mEn2, OUTPUT);
  pinMode(mIn3, OUTPUT);
  pinMode(mIn4, OUTPUT);
//  pinMode(lights, OUTPUT);

  myservo.write(SERVO_MIDDLE_POSITION);
//  pinMode(ultraFrontTrigger, OUTPUT);
//  pinMode(ultraFrontEcho, INPUT);

  delay(1000);
}


void loop()  
{
  if ( myRadio.available()) 
  {
    while (myRadio.available())
    {
      myRadio.read( &joy_s, sizeof(joy_s) );
      Serial.print("New value Y: ");
      Serial.println(joy_s.joyY);
      Serial.print("New value X:");
      Serial.println(joy_s.joyX);
      if(strncmp(joy_s.crypto, "Q2hyaXN0b3NDYXI=", 16) != 0)
      {
        Serial.print("Wrong crypto: ");
        Serial.println(joy_s.crypto);
        delay(20);
        return;
      }
    }
  }
  else
  {
    Serial.println("Antenna not ready");
    joy_s.clear();
    myRadio.startListening();
    myservo.attach(sPin);
    delay(20);
    return;
  }
  Serial.print("New value Y: ");
  Serial.println(joy_s.joyY);

  if(joy_s.joyX > SERVO_MAXIMUM_VALUE)
    joy_s.joyX = SERVO_MAXIMUM_VALUE;
  if(joy_s.joyX < SERVO_MINIMUM_VALUE)
    joy_s.joyX = SERVO_MINIMUM_VALUE;

  myservo.write(joy_s.joyX);
  servo_last_position = joy_s.joyX;
  Serial.print("New value X:");
  Serial.println(joy_s.joyX);

  moveMotor(joy_s.joyY);

  delay(20);

}

