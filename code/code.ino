#include <ESP32Servo.h>  //SET BOARD TO DOIT ESP32 DEVKIT V1
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#define OFFSET_Y 40
#define OFFSET_X 26
const uint8_t line_sensor_pins[8] = { 26, 14, 27, 34, 35, 36, 39, 25 };
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
#define Y_MULTIPLIER 1
#define X_MULTIPLIER 1
Servo servo;
Adafruit_ST7735 display(-1, DISPLAY_DC, DISPLAY_RST);
AccelStepper stepperY(AccelStepper::DRIVER, STEP1, DIR1);
AccelStepper stepperX(AccelStepper::DRIVER, STEP2, DIR2);
MultiStepper steppers;
int menu_len = 10;
const char* menu[] = {"task1","task2","task3"};
int cnt = 0;
int cur_menu_pos = 0;
void menu_func()
{ 
  if(bttn())
    cnt++;
  else
    cnt = 0;
  if(cnt > 100 && !bttn())
  {
    // if(cur_menu_pos>=menu_len-1)
    //   cur_menu_pos = 0;
    // else
    cur_menu_pos++;
    display.setTextColor(ST77XX_WHITE);
    display.setCursor(10,30);
    display.write(menu[cur_menu_pos]);

  }
  Serial.println(bttn());
}
bool ender() {
  return analogRead(BUTTONS_PIN) > 100 && analogRead(BUTTONS_PIN) < 4000;
}
bool bttn() {
  return analogRead(BUTTONS_PIN) < 100;
}
int mms2stpY(float mms) {
  return 3000 / 37.3 * mms * 2*Y_MULTIPLIER;
}
// float stp2mmsY(int stp) {
//   return 3000 / 37.4 / stp;
// }
int mms2stpX(float mms) {
  return mms / (PI * 40) * 200 * 16*X_MULTIPLIER;
}
// float stp2mmsX(int stp) {
//   return 1 / stp / PI / 40 * 360 / 1.8 * 16;
// }

volatile float offset = 0;
void goToPoint(float x, float y) {
  long positions[2] = { mms2stpX(x), mms2stpY(y) };
  steppers.moveTo(positions);
  steppers.runSpeedToPosition();
}
void drawArc(float start_x, float start_y, float center_x, float center_y, float dist_rad) {
  raise();
  goToPoint(start_x, start_y);
  lower();
  for (int i = 0; i < 101; i++) {
    goToPoint(center_x + sqrtf(pow(center_x - start_x, 2) + pow(center_y - start_y, 2)) * cos(PI + atan((start_y - center_y) / (start_x - center_x)) + dist_rad / 100 * i),
              center_y + sqrtf(pow(center_x - start_x, 2) + pow(center_y - start_y, 2)) * sin(PI + atan((start_y - center_y) / (start_x - center_x)) + dist_rad / 100 * i));
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
    display.setCursor(10, 10);
    display.setTextColor(ST77XX_BLACK);
    display.setTextSize(5);
    display.write("ZOV");
    
    offset += 0.3;
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
void print_line_sensors() {
  for (int i = 0; i < 8; i++) {
    Serial.print(analogRead(line_sensor_pins[i]));
    Serial.print(" ");
  }
  Serial.println();
}
void calibrate() {
  while (!ender()) {
    stepperY.setSpeed(-40000*Y_MULTIPLIER);
    stepperY.run();
  }
  stepperY.stop();
  delay(1000);
  stepperY.move(mms2stpY(70));
  while (stepperY.run())
    ;
  while (analogRead(line_sensor_pins[4]) < 4090) {
    stepperX.setSpeed(3000*X_MULTIPLIER);
    stepperX.run();
  }
  stepperX.stop();
  delay(1000);
  stepperX.move(mms2stpX(15));
  while (stepperX.run())
    ;
  delay(1000);
  int cnt = 0;
  bool flag = true;
  while (flag) {
    flag = analogRead(line_sensor_pins[0]) < 4000;
    stepperY.setSpeed(-40000*Y_MULTIPLIER);
    stepperY.run();
  }
  stepperY.stop();
  stepperY.setCurrentPosition(mms2stpY(OFFSET_Y));
  stepperX.setCurrentPosition(mms2stpX(15 + OFFSET_X));
}
void raise() {
  servo.write(0);
  delay(100);
}
void lower() {
  servo.write(60);
  delay(100);
}
bool scan(float x, float y) {
  goToPoint(x +OFFSET_X+7, y + OFFSET_Y-55);
  delay(500);
  return analogRead(line_sensor_pins[7]) > 4000;
}
bool scans[12];
void setup() {
  Serial.begin(115200);
  display.initR(INITR_BLACKTAB);
  display.setRotation(3);
  display.invertDisplay(true);
  display.fillScreen(ST77XX_BLACK);
  xTaskCreatePinnedToCore(AnimationTask, "FlagTask", 4096, NULL, 1, NULL, 0);
  stepperY.setMaxSpeed(30000*Y_MULTIPLIER);
  stepperY.setAcceleration(200000*Y_MULTIPLIER);
  stepperX.setMaxSpeed(3000*X_MULTIPLIER);
  stepperX.setAcceleration(500000*X_MULTIPLIER);
  steppers.addStepper(stepperX);
  steppers.addStepper(stepperY);
  pinMode(BUTTONS_PIN, INPUT);
  pinMode(LED, OUTPUT);
  servo.attach(SERVO);
  raise();
  calibrate();
  for(int i = 0;i<100;i+=10)
  {
    for(int j = 0;j<100;j+=10)
    {
      goToPoint(i,j);
      lower();
      raise();
    }
  }
}


void loop() {
  // menu_func();
  // Serial.println(bttn());
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
