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
const byte encoderBtn = 4;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);


const float Vzeroadc = 1.25;
const float Vmax = 50.0;
unsigned long realPosition = 0;


void setup(void) {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  u8g2.clearBuffer();
  u8g2.drawStr(15,30, "V/I DIGITIZER");
  u8g2.drawStr(30, 45, "hardware by NNNI");
  u8g2.drawStr(30, 55, "software by DDL");
  u8g2.sendBuffer();
  delay(1000);
  pinMode(encoderBtn, INPUT);
  Wire.begin();
  Serial.begin(9600);
  if (adcSensor.begin() == true)
  {
    Serial.println("Device found. I2C connections are good.");
    u8g2.clearBuffer();
    u8g2.drawStr(0,30, "ADC connected!");
    u8g2.sendBuffer();
  }
  else
  {
    Serial.println("Device not found. Check wiring.");
    u8g2.clearBuffer();
    u8g2.drawStr(0,30, "Can't connect to ADC!");
    u8g2.sendBuffer();
    //while (1); // stall out forever
    
  }
  digitalWrite(13, LOW);
    u8g2.clearBuffer();
    u8g2.drawStr(0,30, "Init complete");
    u8g2.sendBuffer();
}
float getVal(byte channel = 0, byte mode = 0){
  uint16_t val = 0;
  adcSensor.setGain(GAIN_FOUR);
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
    res = (Vmax/Vzeroadc)*(res-Vzeroadc);
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

void displayDraw(double value0, double value1, double value2, double value3, byte mode = 0){
  u8g2.clearBuffer();
  char cstr[5];
  switch (mode)
  {
  case 0:
    memset(&cstr[0], 0, sizeof(cstr));
    sprintf(cstr, "%.2f", value0);
    u8g2.drawStr(0,10, cstr);
    u8g2.drawStr(30,10, "V0");

    memset(&cstr[0], 0, sizeof(cstr));
    sprintf(cstr, "%.2f", value1);
    u8g2.drawStr(0,25, cstr);
    u8g2.drawStr(30,25, "V1");

    memset(&cstr[0], 0, sizeof(cstr));
    sprintf(cstr, "%.2f", value2);
    u8g2.drawStr(0,40, cstr);
    u8g2.drawStr(30,40, "V2");

    memset(&cstr[0], 0, sizeof(cstr));
    sprintf(cstr, "%.2f", value3);
    u8g2.drawStr(0,55, cstr);
    u8g2.drawStr(30,55, "V3");
    break;
  
  default:
    break;
  }

  u8g2.sendBuffer();
}

void buttonTick(byte button){
  // TODO
}

void loop(void) {
  encoderTick();
  float val1 = getVal(0);
  float val2 = getVal(1);
  float val3 = getVal(2);
  float val4 = getVal(3);

  displayDraw(val1, val2, val3, val4);

  Serial.print("Vals: ");
  Serial.print(val1);
  Serial.print("; ");
  Serial.print(val2);
  Serial.print("; ");
  Serial.print(val3);
  Serial.print("; ");
  Serial.print(val4);
  Serial.println();
  // displayDraw(getVal(0), getVal(1), getVal(2), getVal(3), 0);
  delay(50);
}

