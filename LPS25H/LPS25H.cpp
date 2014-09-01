#include <LPS25H.h>
#include <Wire.h>

// Defines ///////////////////////////////////////////////////////////

// The Arduino two-wire interface uses a 7-bit number for the address,
// and sets the last bit correctly based on reads and writes
#define SA0_LOW_ADDRESS  0b1011100
#define SA0_HIGH_ADDRESS 0b1011101

#define TEST_REG_NACK -1
#define LPS25H_WHO_ID   0xBD

// Constructors //////////////////////////////////////////////////////

LPS25H::LPS25H(void)
{
  _device 	= device_25H;
  address 	= SA0_HIGH_ADDRESS;	//default setting is HIGH  
}

// Public Methods ////////////////////////////////////////////////////

// sets or detects device type and slave address; returns bool indicating success
bool LPS25H::init()
{
  if (!detectDevice())
    return false;
    
	translated_regs[-INTERRUPT_CFG] = LPS25H_INTERRUPT_CFG;
    translated_regs[-INT_SOURCE]    = LPS25H_INT_SOURCE;
    translated_regs[-THS_P_L]       = LPS25H_THS_P_L;
    translated_regs[-THS_P_H]       = LPS25H_THS_P_H;
	
	return true;
}

// turns on sensor and enables continuous output
void LPS25H::enableDefault(void)
{
    // 0xB0 = 0b10110000
    // PD = 1 (active mode);  ODR = 011 (12.5 Hz pressure & temperature output data rate)
    writeReg(CTRL_REG1, 0xB0);
}

// writes register
void LPS25H::writeReg(int reg, byte value)
{
  // if dummy register address, look up actual translated address (based on device type)
  if (reg < 0)
  {
    reg = translated_regs[-reg];
  }

  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// reads register
byte LPS25H::readReg(int reg)
{
  byte value;
  
  // if dummy register address, look up actual translated address (based on device type)
  if (reg < 0)
  {
    reg = translated_regs[-reg];
  }

  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false); // restart
  Wire.requestFrom(address, (byte)1);
  value = Wire.read();
  Wire.endTransmission();

  return value;
}

// reads pressure in millibars (mbar)/hectopascals (hPa)
float LPS25H::readPressureMillibars(void)
{
  return (float)readPressureRaw() / 4096;
}

// reads pressure in inches of mercury (inHg)
float LPS25H::readPressureInchesHg(void)
{
  return (float)readPressureRaw() / 138706.5;
}

// reads pressure and returns raw 24-bit sensor output
int32_t LPS25H::readPressureRaw(void)
{
  Wire.beginTransmission(address);
  // assert MSB to enable register address auto-increment
  Wire.write(PRESS_OUT_XL | (1 << 7));
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)3);

  while (Wire.available() < 3);

  uint8_t pxl = Wire.read();
  uint8_t pl = Wire.read();
  uint8_t ph = Wire.read();

  // combine bytes
  return (int32_t)(int8_t)ph << 16 | (uint16_t)pl << 8 | pxl;
}

// reads temperature in degrees C
float LPS25H::readTemperatureC(void)
{
  return 42.5 + (float)readTemperatureRaw() / 480;
}

// reads temperature in degrees F
float LPS25H::readTemperatureF(void)
{
  return 108.5 + (float)readTemperatureRaw() / 480 * 1.8;
}

// reads temperature and returns raw 16-bit sensor output
int16_t LPS25H::readTemperatureRaw(void)
{
  Wire.beginTransmission(address);
  // assert MSB to enable register address auto-increment
  Wire.write(TEMP_OUT_L | (1 << 7));
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)2);

  while (Wire.available() < 2);

  uint8_t tl = Wire.read();
  uint8_t th = Wire.read();

  // combine bytes
  return (int16_t)(th << 8 | tl);
}

// converts pressure in mbar to altitude in meters, using 1976 US
// Standard Atmosphere model (note that this formula only applies to a
// height of 11 km, or about 36000 ft)
//  If altimeter setting (QNH, barometric pressure adjusted to sea
//  level) is given, this function returns an indicated altitude
//  compensated for actual regional pressure; otherwise, it returns
//  the pressure altitude above the standard pressure level of 1013.25
//  mbar or 29.9213 inHg
float LPS25H::pressureToAltitudeMeters(float pressure_mbar, float altimeter_setting_mbar)
{
  return (1 - pow(pressure_mbar / altimeter_setting_mbar, 0.190263)) * 44330.8;
}

// converts pressure in inHg to altitude in feet; see notes above
float LPS25H::pressureToAltitudeFeet(float pressure_inHg, float altimeter_setting_inHg)
{
  return (1 - pow(pressure_inHg / altimeter_setting_inHg, 0.190263)) * 145442;
}

// Private Methods ///////////////////////////////////////////////////

bool LPS25H::detectDevice()
{
  int id = testWhoAmI(address);

  if (id == LPS25H_WHO_ID)return true;
  return false;
}

int LPS25H::testWhoAmI(byte address)
{
  Wire.beginTransmission(address);
  Wire.write(WHO_AM_I);
  Wire.endTransmission();

  Wire.requestFrom(address, (byte)1);
  if (Wire.available())
    return Wire.read();
  else
    return TEST_REG_NACK;
}