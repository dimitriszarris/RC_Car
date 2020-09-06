#include <SPI.h>  
#include "RF24.h"

RF24 myRadio (7, 8);
byte addresses[][6] = {"0"};


struct joystic_t
{
  int joyX;
  int joyY;
  int horn;
  int lights;
  char crypto[16] = "Q2hyaXN0b3NDYXI=";
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
const int x_joyB = 0;
const int joyX = 1;
const int joyY = 2;
const int y_joyB = 3;
int joyValX, joyValY;

int lights = HIGH;
int horn = HIGH;
const int correctionX = 4;
const int correctionY = 4;

const int joyX_min = 0;
const int joyX_max = 750;
const int joyY_min = 0;
const int joyY_max = 1023;
const int motor_min = 0;
const int motor_max = 255;
const int servo_min = 5;
const int servo_max = 125;

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(x_joyB, INPUT);
  pinMode(y_joyB, INPUT);

  myRadio.begin();
  myRadio.setChannel(100);
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS );
  myRadio.openWritingPipe( addresses[0]);
  delay(1000);
}

void correctValues(int& joyX, int& joyY)
{
  if(joyX >= joyX_min + (joyX_max-joyX_min)/2 && joyX <= joyX_min + (joyX_max-joyX_min)/2 + correctionX)
    joyX = joyX_min + (joyX_max-joyX_min)/2;
  if(joyX < joyX_min)
    joyX = joyX_min;
  if(joyX > joyX_max)
    joyX = joyX_max;
  if(joyY >= joyY_min + (joyY_max-joyY_min)/2 && joyY <= joyY_min + (joyY_max-joyY_min)/2 + correctionY)
    joyY = joyY_min + (joyY_max-joyY_min)/2;
  if(joyY < joyY_min)
    joyY = joyY_min;
  if(joyY > joyY_max)
    joyY = joyY_max;

  Serial.print("Corr X: ");
  Serial.println(joyX);
  Serial.print("Corr Y: ");
  Serial.println(joyY);
}

void loop()
{
  joyValX = analogRead(joyX);
  Serial.print("1 X: ");
  Serial.print(joyValX);
  joyValY = analogRead(joyY);
  Serial.print(", 1 Y: ");
  Serial.println(joyValY);
  correctValues(joyValX, joyValY);
  joyValX = map(joyValX, joyX_min, joyX_max, servo_min, servo_max);
  joyValY = map(joyValY, joyY_min, joyY_max, motor_min, motor_max);
  Serial.print("2 X: ");
  Serial.print(joyValX);
  Serial.print(", 2 Y: ");
  Serial.println(joyValY);
  joy_s.joyX = joyValX;
  joy_s.joyY = joyValY;

/*  int x_joy_button = digitalRead(x_joyB);
  if(x_joy_button == LOW)
  {
    if(lights == LOW)
      lights = HIGH;
    else
      lights = LOW;

    Serial.print("Button lights pressed: ");
    Serial.println(lights);
    joy_s.lights = lights;
  }

  int y_joy_button = digitalRead(y_joyB);
  if(y_joy_button == LOW)
  {
    if(horn == LOW)
      horn = HIGH;
    else
      horn = LOW;

    Serial.print("Button horn pressed: ");
    Serial.println(horn);
    joy_s.horn = horn;
  }
*/
//  Serial.print("Lights: ");
//  Serial.println(lights);

  myRadio.write(&joy_s, sizeof(joy_s));

  delay(20);
}

