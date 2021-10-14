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

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


const float vref = 3.3;
unsigned long realPosition = 0;


void setup(void) {
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
    break;
  
  default:
    break;
  }
  return res;
}

void encoderTick(){
  static long oldPosition  = -999;
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
    realPosition = newPosition/4;
  }
}

void displayDraw(float value0, float value1, float value2, float value3, byte mode = 0){
  u8g2.clearBuffer();
  char cstr[4];
  String buff = "";
  switch (mode)
  {
  case 0:
    memset(&cstr[0], 0, sizeof(cstr));
    buff = "";
    // itoa(value0, cstr, 10);
    buff.concat(value0);
    buff.toCharArray(cstr, 4);
    u8g2.drawStr(0,10, cstr);
    u8g2.drawStr(30,10, "V0");


    Serial.print(cstr);
    Serial.print(", ");
    Serial.print(value0);
    Serial.print("; ");


    memset(&cstr[0], 0, sizeof(cstr));
    buff = "";
    // itoa(value1, cstr, 10);
    buff.concat(value1);
    buff.toCharArray(cstr, 4);
    u8g2.drawStr(0, 25, cstr);
    u8g2.drawStr(30, 25, "V1");


    Serial.print(cstr);
    Serial.print(", ");
    Serial.print(value1);
    Serial.print("; ");

    memset(&cstr[0], 0, sizeof(cstr));
    buff = "";
    // itoa(value2, cstr, 10);
    buff.concat(value2);
    buff.toCharArray(cstr, 4);
    u8g2.drawStr(0,40, cstr);
    u8g2.drawStr(30,40, "V2");


    Serial.print(cstr);
    Serial.print(", ");
    Serial.print(value2);
    Serial.print("; ");


    memset(&cstr[0], 0, sizeof(cstr));
    buff = "";
    // itoa(value3, cstr, 10);
    buff.concat(value3);
    buff.toCharArray(cstr, 4);
    u8g2.drawStr(0,55, cstr);
    u8g2.drawStr(30,5, "V3");


    Serial.print(cstr);
    Serial.print(", ");
    Serial.print(value3);
    Serial.print("; ");
    break;
    
  default:
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

  Serial.print("Vals: ");
  displayDraw(val1, val2, val3, val4);
  Serial.println();
  // displayDraw(getVal(0), getVal(1), getVal(2), getVal(3), 0);
  delay(50);
}

