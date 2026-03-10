#include <ESP32Servo.h>
#include <AccelStepper.h>
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
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
Servo servo;
Adafruit_ST7735 display(-1, DISPLAY_DC, DISPLAY_RST);
AccelStepper stepperY(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper stepperX(AccelStepper::DRIVER, STEP2, DIR2);
bool ender() {
  return analogRead(BUTTONS_PIN) > 100 && analogRead(BUTTONS_PIN) < 4000;
}
bool bttn() {
  return analogRead(BUTTONS_PIN) < 100;
}
int mms2stpY(float mms) {
  return 3000 / 37.4 * mms;
}
float stp2mmsY(int stp) {
  return 3000 / 37.4 / stp;
}
int mms2stpX(float mms) {
  return mms / PI / 40 * 360 / 1.8 * 16;
}
float stp2mmsX(int stp) {
  return 1 / stp / PI / 40 * 360 / 1.8 * 16;
}
class Point {
public:
  float x, y;

  Point(float x = 0.0f, float y = 0.0f)
    : x(x), y(y) {}

  Point operator+(const Point& other) const {
    return Point(x + other.x, y + other.y);
  }

  Point operator-(const Point& other) const {
    return Point(x - other.x, y - other.y);
  }

  Point operator*(float scalar) const {
    return Point(x * scalar, y * scalar);
  }

  bool operator==(const Point& other) const {
    return x == other.x && y == other.y;
  }
  int xSteps() {
    return mms2stpX(x);
  }
  int ySteps() {
    return mms2stpY(y);
  }
};
volatile float offset = 0;
void goToPoint(float x,float y) {
  stepperX.moveTo(mms2stpX(x));
  stepperY.moveTo(mms2stpY(y));
  while (stepperX.run() || stepperY.run())
    ;
}
void drawArc(float start_x,float start_y,float center_x,float center_y, float dist_rad)
{
  goToPoint(start_x, start_y);
  for(int i = 0;i<100;i++)
  {
    goToPoint(center_x + sqrtf(pow(center_x-start_x,2) + pow(center_y-start_y,2)) * cos(PI+atan((start_y-center_y)/(start_x-center_x)) + dist_rad/100*i),
    center_y + sqrtf(pow(center_x-start_x,2) + pow(center_y-start_y,2)) * sin(PI+atan((start_y-center_y)/(start_x-center_x)) + dist_rad/100*i) );
  }
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
void setup() {
  Serial.begin(115200);
  display.initR(INITR_BLACKTAB);
  display.setRotation(3);
  display.invertDisplay(true);
  display.fillScreen(ST77XX_BLACK);
  xTaskCreatePinnedToCore(AnimationTask, "FlagTask", 4096, NULL, 1, NULL, 0);
  stepperY.setMaxSpeed(10000);
  stepperY.setAcceleration(100000);
  pinMode(BUTTONS_PIN, INPUT);
  pinMode(LED, OUTPUT);
  while (!ender()) {
    stepperY.setSpeed(-10000);
    stepperY.run();
  }
  stepperY.stop();
  delay(1000);
  stepperY.setCurrentPosition(0);
  stepperY.runToNewPosition(mms2stpY(40));
  while (analogRead(line_sensor_pins[7]) < 4000) {
    stepperX.setSpeed(3000);
    stepperX.run();
  }
  stepperX.stop();
  stepperX.setCurrentPosition(0);
  stepperX.runToNewPosition(mms2stpX(100));
  // goToPoint(100,100);
  // drawArc(100, 100, 120, 100, 1);
}


void loop() {
}
