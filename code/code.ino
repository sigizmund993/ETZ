#include <ESP43Servo.h>
#include <AccleStepper.h>
#include <wire.h>
const uint8_t line_sensor_pins[8] = {36,39,34,35,32,33,25,26};
#define LED 15                                                                                
#define SERVO 0
#define PIN1 19
#define PIN2 5
#define DIR1 4
#define STEP1 2
#define DIR2 0
#define STEP2 0
#define BTTN 22
#define DISPLAY_SCL 18
#define DISPLAY_SDA 23
#define DISPLAY_DC 21
Servo servo;

void setup() {
  Serial2.begin(9600,SERIAL_8N1);
  for(int i = 0,i<8,i++)
    pinMode(line_sensor_pins[i],INPUT);
  pinMode(LED,OUTPUT);
  pinMode(PIN1,INPUT);
  pinMode(PIN2,INPUT);
  pinMode(BTTN,INPUT_PULLUP);
  servo.attach(SERVO);


}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED,1);
  delay(1000);
  digitalWrite(LED,0);
  delay(1000);
}
