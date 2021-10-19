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

// uncomment if you want the values to be printed to Serial as well
#define printToSerial

// adjust these constants for calibration and different pins
Encoder myEnc(2, 3);
const float maxVolt = 50.0;
const float ohmsFormA = 1.0;
const float AmAconst = 1.0;
const float BmAconst = 1.0;
const float CmAconst = 1.0;
const float DmAconst = 1.0;
const float AuAconst = 1.0;
const float BuAconst = 1.0;
const float CuAconst = 1.0;
const float DuAconst = 1.0;
const float ohmsForuA = 1000.0;
const byte buttnEnc = 4;
const float idealVrefA = 1.25;
const float idealVrefB = 1.25;
const float idealVrefC = 1.25;
const float idealVrefD = 1.25;
const float AVref = 1.19;
const float BVref = 1.18;
const float CVref = 1.19;
const float DVref = 1.20;


Adafruit_ADS1015 adcSensor;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

//U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

unsigned long realPosition = 0;
unsigned long oldRealPosition = 0;
byte modeA = 0;
byte modeB = 0;
byte modeC = 0;
byte modeD = 0;

void setup(void)
{
  pinMode(buttnEnc, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH); // indicates the beginning of the startup process

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.enableUTF8Print();
  u8g2.clearBuffer();

  Wire.begin();

#ifdef printToSerial
  Serial.begin(9600);
#endif

  adcSensor.begin();

  u8g2.drawStr(22, 25, "V/I DIGITIZER");
  u8g2.drawStr(13, 40, "hardware by NNNI");
  u8g2.drawStr(18, 52, "software by DDL");
  u8g2.sendBuffer();
  u8g2.setFontPosTop();
  delay(1000);

  digitalWrite(LED_BUILTIN, LOW); // indicates the end of the startup process
}

//gets the value from the ADC depending on the mode

// modes:
// 0 - V mode, scale input from -50V to +50V (00.00 V)
// 1 - mA mode, scale input from 0mA to 500mA (000 mA)
// 2 - uA mode, scale input from 0uA to 1000uA (0000 uA)
// adc reading / resistor value
// 1 ohm resistor for 1mA to 500mA range
// 1K for 1uA to 1000uA range

float getVal(byte channel = 0, byte mode = 0)
{
  int16_t val = 0;
  float chanVref = 0.0;
  float chanIdealVref = 0;
  float chanelmAconst = 1.0;
  float chaneluAconst = 1.0;
  adcSensor.setGain(GAIN_ONE);
  switch (channel)
  {
  case 0:
    val = adcSensor.readADC_SingleEnded(0);
    chanVref = AVref;
    chanIdealVref = idealVrefA;
    chanelmAconst = AmAconst;
    chaneluAconst = AuAconst;
    break;
  case 1:
    val = adcSensor.readADC_SingleEnded(1);
    chanVref = BVref;
    chanIdealVref = idealVrefB;
    chanelmAconst = BmAconst;
    chaneluAconst = BuAconst;
    break;
  case 2:
    val = adcSensor.readADC_SingleEnded(2);
    chanVref = CVref;
    chanIdealVref = idealVrefC;
    chanelmAconst = CmAconst;
    chaneluAconst = CuAconst;
    break;
  case 3:
    val = adcSensor.readADC_SingleEnded(3);
    chanVref = DVref;
    chanIdealVref = idealVrefD;
    chanelmAconst = DmAconst;
    chaneluAconst = DuAconst;
    break;
  default:
    val = adcSensor.readADC_SingleEnded(0);
    chanVref = AVref;
    chanIdealVref = idealVrefA;
    chanelmAconst = AmAconst;
    chaneluAconst = AuAconst;
    break;
  }

  float res = 0.0;
  switch (mode)
  {
  case 0:
    res = ((adcSensor.computeVolts(val) - chanVref) * (maxVolt / chanIdealVref));
    break;
  case 1:
    res = (adcSensor.computeVolts(val) / ohmsFormA) * chanelmAconst;
    res = res * 1000;
    break;
  case 2:
    res = (adcSensor.computeVolts(val) / ohmsForuA) * chaneluAconst;
    break;
  default:
    res = ((adcSensor.computeVolts(val) - chanVref) * (maxVolt / chanIdealVref));
    break;
  }
  return res;
}

// handles encoder and buttons presses
byte encoderTick()
{
  static long oldPosition = -999;
  long newPosition = myEnc.read();
  if (newPosition != oldPosition)
  {
    oldPosition = newPosition;
    oldRealPosition = realPosition;
    realPosition = newPosition / 4;
  }

  const unsigned int longPressInterval = 500;
  static byte status = 0;
  static byte oldStatus = 0;
  static unsigned long gpTimer = 0;
  byte ret = 0;

  if (digitalRead(buttnEnc) == LOW && status == 0)
  {
    status = 1;
    gpTimer = millis();
  }
  else if (!digitalRead(buttnEnc) && status == 1 && millis() - gpTimer >= 20)
  {
    status = 2;
    gpTimer = millis();
  }
  else if (!digitalRead(buttnEnc) && status == 2 && millis() - gpTimer >= longPressInterval)
  {
    status = 3;
  }
  else if (digitalRead(buttnEnc) == HIGH)
  {
    status = 0;
  }
  if (status != oldStatus)
  {

    if (status == 3)
    {
      ret = 2;
    }
    else if (status == 0 && oldStatus != 3)
    {
      ret = 1;
    }
    else
    {
      ret = 0;
    }
    oldStatus = status;
  }
  return ret;
}

void displayDraw(float value0, float value1, float value2, float value3, byte mode0 = 0, byte mode1 = 0, byte mode2 = 0, byte mode3 = 0)
{
  static bool locked = true;
  static unsigned long millisTick = 0;
  static int currSelection = 0;
  static int lastSelection = 0;
  static int currentEntered = -1;

  if (oldRealPosition > realPosition)
  {
    oldRealPosition -= 1;
    currSelection--;
    millisTick = millis();
  }
  else if (oldRealPosition < realPosition)
  {
    oldRealPosition += 1;
    currSelection++;
    millisTick = millis();
  }

  if (currSelection > 3 && currentEntered == -1)
  {
    currSelection = 0;
  }
  else if (currSelection < 0 && currentEntered == -1)
  {
    currSelection = 3;
  }
  else if (currSelection > 2 && currentEntered > -1)
  {
    currSelection = 0;
  }
  else if (currSelection < 0 && currentEntered > -1)
  {
    currSelection = 2;
  }

  byte encoderBtnVal = encoderTick();
  if (encoderBtnVal == 2 && locked)
  {
    locked = false;
    millisTick = millis();
  }
  else if (encoderBtnVal == 1 && !locked && currentEntered == -1)
  {
    currentEntered = currSelection;
    lastSelection = currSelection;
    currSelection = 0;
    millisTick = millis();
  }
  else if (encoderBtnVal == 1 && !locked && currentEntered >= 0)
  {
    currentEntered = -1;
    currSelection = lastSelection;
    millisTick = millis();
  }

  if (currentEntered >= 0)
  {
    switch (currentEntered)
    {
    case 0:
      modeA = currSelection;
      break;
    case 1:
      modeB = currSelection;
      break;
    case 2:
      modeC = currSelection;
      break;
    case 3:
      modeD = currSelection;
      break;
    }
  }

  if (millis() - millisTick >= 5000)
  {
    locked = true;
    currentEntered = -1;
    currSelection = 0;
  }

  u8g2.clearBuffer();

  // modes:
  // 0 - V mode, scale input from -50V to +50V (00.00 V)
  // 1 - mA mode, scale input from 0mA to 500mA (000 mA)
  // 2 - uA mode, scale input from 0uA to 1000uA (0000 uA)
  u8g2.setCursor(0, 5);
  u8g2.print("A: ");
  if(mode0 == 1)
    u8g2.print(value0, 0);
  else
    u8g2.print(value0, 3);

  byte oldX = u8g2.tx;

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
  if (currentEntered == 0)
  {
    u8g2.drawLine(oldX, u8g2.ty + 10, u8g2.tx, u8g2.ty + 10);
  }

  if ((!locked && currSelection == 0 && currentEntered == -1) || currentEntered == 0)
  {
    u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
    u8g2.drawGlyph(u8g2.tx, u8g2.ty, 77);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  u8g2.setCursor(0, 20);
  u8g2.print("B: ");
  if(mode1 == 1)
    u8g2.print(value1, 0);
  else
    u8g2.print(value1, 3);

  oldX = u8g2.tx;

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
  if (currentEntered == 1)
  {
    u8g2.drawLine(oldX, u8g2.ty + 10, u8g2.tx, u8g2.ty + 10);
  }

  if ((!locked && currSelection == 1 && currentEntered == -1) || currentEntered == 1)
  {
    u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
    u8g2.drawGlyph(u8g2.tx, u8g2.ty, 77);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  u8g2.setCursor(0, 35);
  u8g2.print("C: ");
  if(mode2 == 1)
    u8g2.print(value2, 0);
  else
    u8g2.print(value2, 3);

  oldX = u8g2.tx;

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
  if (currentEntered == 2)
  {
    u8g2.drawLine(oldX, u8g2.ty + 10, u8g2.tx, u8g2.ty + 10);
  }

  if ((!locked && currSelection == 2 && currentEntered == -1) || currentEntered == 2)
  {
    u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
    u8g2.drawGlyph(u8g2.tx, u8g2.ty, 77);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  u8g2.setCursor(0, 50);
  u8g2.print("D: ");
  if(mode3 == 1)
    u8g2.print(value3, 0);
  else
    u8g2.print(value3, 3);
  oldX = u8g2.tx;

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
  if (currentEntered == 3)
  {
    u8g2.drawLine(oldX, u8g2.ty + 10, u8g2.tx, u8g2.ty + 10);
  }

  if ((!locked && currSelection == 3 && currentEntered == -1) || currentEntered == 3)
  {
    u8g2.setFont(u8g2_font_open_iconic_arrow_1x_t);
    u8g2.drawGlyph(u8g2.tx, u8g2.ty, 77);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  if (!locked)
  {
    u8g2.setFont(u8g2_font_open_iconic_thing_2x_t);
    u8g2.drawGlyph(100, 10, 68);
    u8g2.setFont(u8g2_font_ncenB08_tr);
  }

  u8g2.sendBuffer();
}

void printSerial(float value0, float value1, float value2, float value3, byte mode0 = 0, byte mode1 = 0, byte mode2 = 0, byte mode3 = 0)
{
  Serial.print("Vals: ");
  Serial.print(value0, 3);
  switch (mode0)
  {
  case 0:
    Serial.print("V");
    break;
  case 1:
    Serial.print("mA");
    break;
  case 2:
    Serial.print("uA");
    break;
  default:
    break;
  }
  Serial.print("; ");
  Serial.print(value1, 3);
  switch (mode1)
  {
  case 0:
    Serial.print("V");
    break;
  case 1:
    Serial.print("mA");
    break;
  case 2:
    Serial.print("uA");
    break;
  default:
    break;
  }
  Serial.print("; ");
  Serial.print(value2, 3);
  switch (mode2)
  {
  case 0:
    Serial.print("V");
    break;
  case 1:
    Serial.print("mA");
    break;
  case 2:
    Serial.print("uA");
    break;
  default:
    break;
  }
  Serial.print("; ");
  Serial.print(value3, 3);
  switch (mode3)
  {
  case 0:
    Serial.print("V");
    break;
  case 1:
    Serial.print("mA");
    break;
  case 2:
    Serial.print("uA");
    break;
  default:
    break;
  }
  Serial.println(";");
}

void loop(void)
{
  float val1 = getVal(0, modeA);
  float val2 = getVal(1, modeB);
  float val3 = getVal(2, modeC);
  float val4 = getVal(3, modeD);

  displayDraw(val1, val2, val3, val4, modeA, modeB, modeC, modeD);

#ifdef printToSerial
  printSerial(val1, val2, val3, val4, modeA, modeB, modeC, modeD);
#endif

  delay(50);
}
