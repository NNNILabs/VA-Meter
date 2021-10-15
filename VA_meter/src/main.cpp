#include <Arduino.h>
#include <U8g2lib.h>
#include <Adafruit_ADS1X15.h>
#include <Wire.h>
#include <Encoder.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

Adafruit_ADS1015 adcSensor;

Encoder myEnc(2, 3);
const byte buttnEnc = 4;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


unsigned long realPosition = 0;
const float maxVolt = 50.0;

void setup(void) {
  pinMode(buttnEnc, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);

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
  uint16_t val = 0;
  adcSensor.setGain(GAIN_ONE);
  switch (channel)
  {
  case 0:
    val = adcSensor.readADC_SingleEnded(0);
    break;
  case 1:
    val = adcSensor.readADC_SingleEnded(1);
    break;
  case 2:
    val = adcSensor.readADC_SingleEnded(2);
    break;
  case 3:
    val = adcSensor.readADC_SingleEnded(3);
    break;
  default:
    val = adcSensor.readADC_SingleEnded(0);
    break;
  }
  
  // modes:
  // 0 - V mode, scale input from -50V to +50V (00.00 V) 
  // 1 - mA mode, scale input from 0mA to 500mA (000 mA)
  // 2 - uA mode, scale input from 0uA to 1000uA (0000 uA)
  float res = 0.0;
  switch (mode)
  {
  case 0:
    res = adcSensor.computeVolts(val);
    res = (maxVolt/1.25)*(res-1.25);
    break;
  
  default:
    break;
  }
  return res;
}

byte encoderTick(){
  static long oldPosition  = -999;
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
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
  if(encoderTick() == 2){
    locked = false;
    millisTick = millis();
  }
  if(millis() - millisTick >= 5000){
    locked = true;
  }
  u8g2.clearBuffer();
  
  if(!locked){
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.setFontPosTop();
    u8g2.drawUTF8(100, 10, "🔓");
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }else{
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    u8g2.setFontPosTop();
    u8g2.drawUTF8(100, 10, "🔒");
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }
  // modes:
  // 0 - V mode, scale input from -50V to +50V (00.00 V) 
  // 1 - mA mode, scale input from 0mA to 500mA (000 mA)
  // 2 - uA mode, scale input from 0uA to 1000uA (0000 uA)
  switch (mode0)
  {
  case 0:
    u8g2.setCursor(0,10);
    u8g2.print("A: ");
    u8g2.print(value0, 3);
    u8g2.print("V");
    break;
  case 1:
    u8g2.setCursor(0,10);
    u8g2.print("A: ");
    u8g2.print(value0, 3);
    u8g2.print("mA");
    break;
  case 2:
    u8g2.setCursor(0,10);
    u8g2.print("A: ");
    u8g2.print(value0, 3);
    u8g2.print("uA");
    break;
  default:
    u8g2.setCursor(0,10);
    u8g2.print("A: ");
    u8g2.print(value0, 3);
    u8g2.print("V");
    break;
  }

  switch (mode1)
  {
  case 0:    
    u8g2.setCursor(0,25);
    u8g2.print("B: ");
    u8g2.print(value1, 3);
    u8g2.print("V");
    break;
  case 1: 
    u8g2.setCursor(0,25);
    u8g2.print("B: ");
    u8g2.print(value1, 3);
    u8g2.print("mA");
    break;
  case 2:
    u8g2.setCursor(0,25);
    u8g2.print("B: ");
    u8g2.print(value1, 3);
    u8g2.print("uA");
  default:
    u8g2.setCursor(0,25);
    u8g2.print("B: "); 
    u8g2.print(value1, 3);
    u8g2.print("V");
    break;
  }
  switch (mode2)
  {
  case 0:
    u8g2.setCursor(0,40);
    u8g2.print("C: ");
    u8g2.print(value2, 3);
    u8g2.print("V");
    break;
  case 1:
    u8g2.setCursor(0,40);
    u8g2.print("C: ");
    u8g2.print(value2, 3);
    u8g2.print("mA");
    break;
  case 2:
    u8g2.setCursor(0,40);
    u8g2.print("C: ");
    u8g2.print(value2, 3);
    u8g2.print("uA");
  default:
    u8g2.setCursor(0,40);
    u8g2.print("C: ");
    u8g2.print(value2, 3);
    u8g2.print("V");
    break;
  }
  
  switch (mode3)
  {
  case 0:
    u8g2.setCursor(0,55);
    u8g2.print("D: ");
    u8g2.print(value3, 3);
    u8g2.print("V");
    break;
  case 1:
    u8g2.setCursor(0,55);
    u8g2.print("D: ");
    u8g2.print(value3, 3);
    u8g2.print("mA");
    break;
  case 2:
    u8g2.setCursor(0,55);
    u8g2.print("D: ");
    u8g2.print(value3, 3);
    u8g2.print("uA");
    break;
  default:
    u8g2.setCursor(0,55);
    u8g2.print("D: ");
    u8g2.print(value3, 3);
    u8g2.print("V");
    break;
  }


  u8g2.sendBuffer();
}

void loop(void) {
  encoderTick();
  float val1 = getVal(0);
  float val2 = getVal(1);
  float val3 = getVal(2);
  float val4 = getVal(3);

  displayDraw(val1, val2, val3, val4);
  delay(50);
}

