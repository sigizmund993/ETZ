#include <ESP32Servo.h>//SET BOARD TO DOIT ESP32 DEVKIT V1
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#define OFFSET_Y -10
#define OFFSET_X 28
const uint8_t line_sensor_pins[8] = { 34, 14, 27, 26, 25, 39, 36, 35 };
#define LED 15
#define SERVO 12
#define PIN1 19
#define PIN2 5
#define DIR1 4
#define STEP1 2
#define DIR2 32 
#define STEP2 33
#define BUTTONS_PIN 13
#define DISPLAY_RST 21
#define DISPLAY_DC 22
#define LIN_SPEED 100
Servo servo;
Adafruit_ST7735 display(-1, DISPLAY_DC, DISPLAY_RST);
AccelStepper stepperY(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper stepperX(AccelStepper::DRIVER, STEP2, DIR2);
MultiStepper steppers;
bool ender() {
  return analogRead(BUTTONS_PIN) > 100 && analogRead(BUTTONS_PIN) < 4000;
}
bool bttn() {
  return analogRead(BUTTONS_PIN) < 100;
}
int mms2stpY(float mms) {
  return 3000 / 37.3 * mms;
}
// float stp2mmsY(int stp) {
//   return 3000 / 37.4 / stp;
// }
int mms2stpX(float mms) {
  return mms/(PI*40)*200*16;
}
// float stp2mmsX(int stp) {
//   return 1 / stp / PI / 40 * 360 / 1.8 * 16;
// }

volatile float offset = 0;
void goToPoint(float x,float y) {
  long positions[2] = {mms2stpX(x),mms2stpY(y)};
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
}
void drawArc(float start_x,float start_y,float center_x,float center_y, float dist_rad)
{
  raise();
  goToPoint(start_x, start_y);
  lower();
  for(int i = 0;i<100;i++)
  {
    goToPoint(center_x + sqrtf(pow(center_x-start_x,2) + pow(center_y-start_y,2)) * cos(PI+atan((start_y-center_y)/(start_x-center_x)) + dist_rad/100*i),
    center_y + sqrtf(pow(center_x-start_x,2) + pow(center_y-start_y,2)) * sin(PI+atan((start_y-center_y)/(start_x-center_x)) + dist_rad/100*i) );
  }
  raise();
}
void AnimationTask(void* pvParameters) {
  int x = 0, y = 20, w = 160, h = 88;
  int stripeH = h / 3;

  for (;;) {
    for (int i = 0; i < w; i++) {
      int wave = sin(i * 0.15 + offset) * 4;
      display.drawFastVLine(x + i, y + wave, stripeH, ST77XX_WHITE);
      display.drawFastVLine(x + i, y + wave + stripeH, stripeH, ST77XX_RED);
      display.drawFastVLine(x + i, y + wave + stripeH * 2, stripeH, ST77XX_BLUE);
      display.drawPixel(x + i, y + wave - 1, ST77XX_BLACK);
      display.drawPixel(x + i, y + wave + h, ST77XX_BLACK);
    }
    offset += 0.3;
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void calibrateY()
{
  while (!ender()) {
    stepperY.setSpeed(-10000);
    stepperY.run();
  }
  stepperY.stop();
  delay(1000);
  stepperY.setCurrentPosition(mms2stpY(OFFSET_Y));
}
void calibrateX()
{
  scan(0,40);
  while (analogRead(line_sensor_pins[7]) < 4000) {
    stepperX.setSpeed(3000);
    stepperX.run();
  }
  stepperX.stop();
  stepperX.setCurrentPosition(mms2stpX(OFFSET_X));
}
void raise()
{
  servo.write(0);
  delay(100);
}
void lower()
{
  servo.write(60);
  delay(100);

}
bool scan(float x, float y)
{
  goToPoint(x+OFFSET_X+7,y+OFFSET_Y-5);
  delay(1000);
  return analogRead(line_sensor_pins[7])>4000;
}
bool scans[12];
void setup() {
  Serial.begin(115200);
  display.initR(INITR_BLACKTAB);
  display.setRotation(3);
  display.invertDisplay(true);
  display.fillScreen(ST77XX_BLACK);
  xTaskCreatePinnedToCore(AnimationTask, "FlagTask", 4096, NULL, 1, NULL, 0);
  stepperY.setMaxSpeed(10000);
  stepperY.setAcceleration(10000);
  stepperX.setMaxSpeed(3000);
  stepperX.setAcceleration(3000);
  steppers.addStepper(stepperX);
  steppers.addStepper(stepperY);
  pinMode(BUTTONS_PIN, INPUT);
  pinMode(LED, OUTPUT);
  servo.attach(SERVO);
  raise();
  calibrateY();
  calibrateX();
  gotoPoint(0,150);
  lower();
  goToPoint(0,0);
  goToPoint(150,0);
  raise();
  goToPoint(0,0);
  // scans[0] = scan(45,27);
  // scans[1] = scan(55,27);

  // scans[2] = scan(75,27);
  // scans[3] = scan(85,27);

  // scans[4] = scan(105,27);
  // scans[5] = scan(115,27);

  // goToPoint(0,0);
}


void loop() {
  // goToPoint(0,0);
  // goToPoint(0,0);
  // delay(1000);
  // lower();
  // goToPoint(100,0);
  // delay(1000);
  // goToPoint(0,100);
  // delay(1000);
  // goToPoint(100,100);
  // delay(1000);
  // raise();

}
