#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <Encoder.h>
//hello there
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


const float maxVolt = 50.0;
const unsigned int ohmsFormA = 1;
const unsigned int ohmsForuA = 1000;
const byte buttnEnc = 4;
const float idealVref = 1.25;
const float AVref = 1.19;
const float BVref = 1.18;
const float CVref = 1.19;
const float DVref = 1.20;

Adafruit_ADS1015 adcSensor;

Encoder myEnc(2, 3);



U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


unsigned long realPosition = 0;
unsigned long oldRealPosition = 0;
byte modeA = 0;
byte modeB = 0;
byte modeC = 0;
byte modeD = 0;

void setup(void) {
  pinMode(buttnEnc, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.enableUTF8Print();
  u8g2.clearBuffer();

  Wire.begin();
  Serial.begin(9600);
  adcSensor.begin();
  

  u8g2.drawStr(22, 25, "V/I DIGITIZER");
  u8g2.drawStr(13, 40, "hardware by NNNI");
  u8g2.drawStr(18, 52, "software by DDL");
  u8g2.sendBuffer();
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}
float getVal(byte channel = 0, byte mode = 0){
  int16_t val = 0;
  float chanVref = 0.0;
  adcSensor.setGain(GAIN_ONE);
  switch (channel)
  {
  case 0:
    val = adcSensor.readADC_SingleEnded(0);
    chanVref = AVref;
    break;
  case 1:
    val = adcSensor.readADC_SingleEnded(1);
    chanVref = BVref;
    break;
  case 2:
    val = adcSensor.readADC_SingleEnded(2);
    chanVref = CVref;
    break;
  case 3:
    val = adcSensor.readADC_SingleEnded(3);
    chanVref = DVref;
    break;
  default:
    val = adcSensor.readADC_SingleEnded(0);
    chanVref = AVref;
    break;
  }
  
  // modes:
  // 0 - V mode, scale input from -50V to +50V (00.00 V) 
  // 1 - mA mode, scale input from 0mA to 500mA (000 mA)
  // 2 - uA mode, scale input from 0uA to 1000uA (0000 uA)
  // adc reading / resistor value
  // 1 ohm resistor for 1mA to 500mA range
  // 1K for 1uA to 1000uA range
  float res = 0.0;
  switch (mode)
  {
  case 0:
    res = ((adcSensor.computeVolts(val)-chanVref)*(maxVolt/idealVref));
    break;
  case 1:
    res = (adcSensor.computeVolts(val)/ohmsFormA);
    break;
  case 2:
    res = (adcSensor.computeVolts(val)/ohmsForuA);
    break;
  default:
    res = ((adcSensor.computeVolts(val)-chanVref)*(maxVolt/idealVref));
    break;
  }
  return res;
}

byte encoderTick(){
  static long oldPosition  = -999;
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    oldRealPosition = realPosition;
    realPosition = newPosition/4;
  }

  const unsigned int longPressInterval = 500;
  static byte status = 0;
  static byte oldStatus = 0;
  static unsigned long gpTimer = 0; 
  byte ret = 0;

  if(digitalRead(buttnEnc) == LOW && status == 0){
    status = 1;
    gpTimer = millis();
  }else if (!digitalRead(buttnEnc) && status == 1 && millis() - gpTimer >= 20)
  {
    status = 2;
    gpTimer = millis();
  }else if(!digitalRead(buttnEnc) && status == 2 && millis() - gpTimer >= longPressInterval){
    status = 3;
  }else if(digitalRead(buttnEnc) == HIGH){
    status = 0;
  }
  if(status != oldStatus){
    
    if(status == 3){
      Serial.println("pressed");
      ret = 2;
    }else if(status == 0 && oldStatus != 3){
      Serial.println("click");
      ret = 1;
    }else{
      ret = 0;
    }
    oldStatus = status;
  }
return ret;
}

void displayDraw(float value0, float value1, float value2, float value3, byte mode0 = 0, byte mode1 = 0, byte mode2 = 0, byte mode3 = 0){
  static bool locked = true;
  static unsigned long millisTick = 0;
  static byte currSelection = 0;
  bool enter = false;
  if(oldRealPosition > realPosition){
    oldRealPosition -= 1;
    currSelection--;
    millisTick = millis();
  }else if(oldRealPosition < realPosition){  
    oldRealPosition += 1;
    currSelection++;
    millisTick = millis();
  }

  if(currSelection > 3){
    currSelection = 0;
  }

  byte encoderBtnVal = encoderTick();
  if(encoderBtnVal == 2 && locked){
    locked = false;
    millisTick = millis();
  }else if(encoderBtnVal == 1 && !locked && !enter){
    enter = true;
    millisTick = millis();
  }else if(encoderBtnVal == 1 && !locked && enter){
    enter = false;
    millisTick = millis();
  }

  if(millis() - millisTick >= 5000){
    locked = true;
  }
  u8g2.clearBuffer();
  
  if(!locked){
    u8g2.setFont(u8g2_font_open_iconic_thing_2x_t);
    u8g2.setFontPosTop();
    u8g2.drawGlyph(100, 10, 68);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }else{
    u8g2.setFont(u8g2_font_open_iconic_thing_2x_t);
    u8g2.setFontPosTop();
    u8g2.drawGlyph(100, 10, 67);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }
  // modes:
  // 0 - V mode, scale input from -50V to +50V (00.00 V) 
  // 1 - mA mode, scale input from 0mA to 500mA (000 mA)
  // 2 - uA mode, scale input from 0uA to 1000uA (0000 uA)
  u8g2.setCursor(0,10);
  u8g2.print("A: ");
  u8g2.print(value0, 3);
  if(!locked && currSelection == 0 && enter){
    u8g2.setDrawColor(0);
  }else{
    u8g2.setDrawColor(1);
  }
  switch (mode0)
  {
  case 0:
    u8g2.print("V");
    break;
  case 1:
    u8g2.print("mA");
    break;
  case 2:
    u8g2.print("uA");
    break;
  default:
    u8g2.print("V");
    break;
  }
  if(!locked && currSelection == 0){
    u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
    u8g2.setFontPosTop();
    u8g2.drawGlyph(u8g2.tx, u8g2.ty, 77);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }


  u8g2.setCursor(0,25);
  u8g2.print("B: ");
  u8g2.print(value1, 3);
  switch (mode1)
  {
  case 0:    
    u8g2.print("V");
    break;
  case 1: 
    u8g2.print("mA");
    break;
  case 2:
    u8g2.print("uA");
    break;
  default:
    u8g2.print("V");
    break;
  }

  if(!locked && currSelection == 1){
    u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
    u8g2.setFontPosTop();
    u8g2.drawGlyph(u8g2.tx, u8g2.ty, 77);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }


  u8g2.setCursor(0,40);
  u8g2.print("C: ");
  u8g2.print(value2, 3);
  switch (mode2)
  {
  case 0:
    u8g2.print("V");
    break;
  case 1:
    u8g2.print("mA");
    break;
  case 2:
    u8g2.print("uA");
    break;
  default:
    u8g2.print("V");
    break;
  }
  
  if(!locked && currSelection == 2){
    u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
    u8g2.setFontPosTop();
    u8g2.drawGlyph(u8g2.tx, u8g2.ty, 77);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  u8g2.setCursor(0,55);
  u8g2.print("D: ");
  u8g2.print(value3, 3);
  switch (mode3)
  {
  case 0:
    u8g2.print("V");
    break;
  case 1:
    u8g2.print("mA");
    break;
  case 2:
    u8g2.print("uA");
    break;
  default:
    u8g2.print("V");
    break;
  }

  if(!locked && currSelection == 3){
    u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
    u8g2.setFontPosTop();
    u8g2.drawGlyph(u8g2.tx, u8g2.ty, 77);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  u8g2.sendBuffer();
}

void loop(void) {
  float val1 = getVal(0, modeA);
  float val2 = getVal(1, modeB);
  float val3 = getVal(2, modeC);
  float val4 = getVal(3, modeD);

  displayDraw(val1, val2, val3, val4, modeA, modeB, modeC, modeD);
  delay(50);
}



