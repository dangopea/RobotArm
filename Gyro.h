#include <Wire.h>

const float accScale= 9.81f/ 16384;
const float rotScale= 1.f / 131;

class Gyro {

  public:
  uint8_t address; //Stores I2C address of the gyroscope sensor.
  float positionA[3]; //Array to store the accelerometer readings.
  float rotationV[3]; //Array to store the gyroscope readings.
  int correctionR[3]; //Array to store the gyro calibration offsets.
  float temperature; //Stores the sensor temperature.
  int oldTime; //Variable to track the time of the last sensor update.
  int gscale, ascale; //Stores gyro scale setting & accelerometer scale setting
  
  Gyro(int ascale = 0, int gscale = 0, uint8_t address = 0x68) {
    this->address = address;   
    //wake up call
    this->ascale = ascale;
    this->gscale = gscale;
    correctionR[0] = correctionR[1] = correctionR[2] = 0;
    oldTime = 0;  
  }

  void begin() {
    Wire.begin();
    wakeUp();    
    setScale();
  }

  void wakeUp() {
    Wire.beginTransmission(address);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
  }

  void setScale() {
    Wire.beginTransmission(address);
    Wire.write(0x1B);
    Wire.write(gscale << 3);
    Wire.endTransmission(true);

    Wire.beginTransmission(address);
    Wire.write(0x1C);
    Wire.write(ascale << 3);
    Wire.endTransmission(true);
  }

  inline signed short readShort() const {
    return (Wire.read() << 8) | Wire.read();
  }

  void calculateCorrection(int samples = 100) {
    long dr[3] = {0, 0, 0};

    for(int i = 0; i < samples; i++) {
      Wire.beginTransmission(address);
      Wire.write(0x3B);
      Wire.endTransmission(false);
      Wire.requestFrom((uint8_t)address, (uint8_t)14, (uint8_t)true);

      positionA[0] = readShort() * accScale * (1 << ascale);
      positionA[1] = readShort() * accScale * (1 << ascale);
      positionA[2] = readShort() * accScale * (1 << ascale);

      temperature = readShort() / 340.f + 36.53f;

      dr[0] += readShort();
      dr[1] += readShort();
      dr[2] += readShort();
    
      delay(10);
    }

    correctionR[0] = dr[0] / samples;
    correctionR[1] = dr[0] / samples;
    correctionR[2] = dr[0] / samples;
  }

  int poll() {
    Wire.beginTransmission(address);
    Wire.write(0x3B);
    Wire.endTransmission(false);

    Wire.requestFrom((uint8_t)address, (uint8_t)14, (uint8_t)true);
    positionA[0] = readShort() * accScale * (1 << ascale);
    positionA[1] = readShort() * accScale * (1 << ascale);
    positionA[2] = readShort() * accScale * (1 << ascale);

    temperature = readShort() / 340.f + 36.53f;

    rotationV[0] = (readShort() - correctionR[0]) * rotScale * (1 << gscale);
    rotationV[1] = (readShort() - correctionR[1]) * rotScale * (1 << gscale);
    rotationV[2] = (readShort() - correctionR[2]) * rotScale * (1 << gscale);

    return Wire.available();
  }
};



