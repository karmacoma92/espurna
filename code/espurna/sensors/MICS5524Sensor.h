// -----------------------------------------------------------------------------
// Analog Sensor (maps to an analogRead)
// Copyright (C) 2017-2018 by Xose Pérez <xose dot perez at gmail dot com>
// -----------------------------------------------------------------------------

#if SENSOR_SUPPORT && MICS5524_SUPPORT

#pragma once

// Set ADC to TOUT pin
// #undef ADC_MODE_VALUE
// #define ADC_MODE_VALUE ADC_TOUT

#include "Arduino.h"
#include "BaseSensor.h"
#include "../debug.h"

extern "C" {
    #include "../libs/fs_math.h"
}

class MICS5524Sensor : public BaseSensor {

public:

    // ---------------------------------------------------------------------
    // Public
    // ---------------------------------------------------------------------

    MICS5524Sensor(): BaseSensor() {
        _count = 2;
        _sensor_id = SENSOR_MICS5524_ID;
    }

    void calibrate() {
        setR0(_getResistance());
    }

    // ---------------------------------------------------------------------

    void setAnalogGPIO(unsigned char gpio) {
        _redGPIO = gpio;
    }

    unsigned char getAnalogGPIO() {
        return _redGPIO;
    }

    void setRL(unsigned long Rl) {
        if (Rl > 0) _Rl = Rl;
    }

    unsigned long getRL() {
        return _Rl;
    }

    void setR0(unsigned long R0) {
        if (R0 > 0) _R0 = R0;
    }

    unsigned long getR0() {
        return _R0;
    }

    // ---------------------------------------------------------------------
    // Sensor API
    // ---------------------------------------------------------------------

    // Initialization method, must be idempotent
    void begin() {
        pinMode(_redGPIO, INPUT);
        _ready = true;
    }

    // Pre-read hook (usually to populate registers with up-to-date data)
    void pre() {
        _Rs = _getResistance();
    }

    // Descriptive name of the sensor
    String description() {
        return String("MICS-5524 @ TOUT");
    }

    // Descriptive name of the slot # index
    String slot(unsigned char index) {
        return description();
    };

    // Address of the sensor (it could be the GPIO or I2C address)
    String address(unsigned char index) {
        return String("0");
    }

    // Type for slot # index
    unsigned char type(unsigned char index) {
        if (0 == index) return MAGNITUDE_RESISTANCE;
        if (1 == index) return MAGNITUDE_CO;
        return MAGNITUDE_NONE;
    }

    // Current value for slot # index
    double value(unsigned char index) {
        if (0 == index) return _Rs;
        if (1 == index) return _getPPM();
        return 0;
    }

private:

    unsigned long _getReading() {
        return analogRead(_redGPIO);
    }

    double _getResistance() {

        // get voltage (1 == reference) from analog pin
        double voltage = (float) _getReading() / 1024.0;

        // schematic: 3v3 - Rs - P - Rl - GND
        // V(P) = 3v3 * Rl / (Rs + Rl)
        // Rs = 3v3 * Rl / V(P) - Rl = Rl * ( 3v3 / V(P) - 1)
        // 3V3 voltage is cancelled
        double resistance = (voltage > 0) ? _Rl * ( 1 / voltage - 1 ) : 0;
        resistance = (float) _getReading();

        return resistance;
    }

    double _getPPM() {

        // According to the datasheet (https://airqualityegg.wikispaces.com/file/view/mics-5524-CO.pdf)

        return 764.2976 * fs_pow(2.71828, -7.6389 * ((float) _Rs / _R0));

    }

    unsigned long _R0 = MICS5524_R0;            // R0, calibration value at 25º on air
    unsigned long _Rl = MICS5524_RL;            // RL, load resistance
    unsigned long _Rs = 0;                      // cached resistance
    unsigned char _redGPIO = MICS5524_RED_PIN;

};

#endif // SENSOR_SUPPORT && MICS5524_SUPPORT
// }
//
// void setSamples(unsigned int samples) {
//     if (_samples > 0) _samples = samples;
// }
//
// void setDelay(unsigned long micros) {
//     _micros = micros;
// }
//
// void setFactor(double factor) {
//     DEBUG_MSG(("[] Factor set to: %s \n"), String(factor,6).c_str());
//     _factor = factor;
// }
//
// void setOffset(double offset) {
//     DEBUG_MSG(("[MICS5524] Offset set to: %s \n"), String(offset,6).c_str());
//     _offset = offset;
// }
//
// void setPercentage(double percentage) {
//     DEBUG_MSG(("[MICS5524] Percentage set to: %s \n"), String(percentage,6).c_str());
//     _percentage = percentage;
// }
// // ---------------------------------------------------------------------
//
// unsigned int getSamples() {
//     return _samples;
// }
//
// unsigned long getDelay() {
//     return _micros;
// }
//
// double getFactor() {
//     return _factor;
// }
//
// double getOffset() {
//     return _offset;
// }
//
// double getPercentage() {
//     return _percentage;
// }
//
// // ---------------------------------------------------------------------
// // Sensor API
// // ---------------------------------------------------------------------
//
// // Initialization method, must be idempotent
// void begin() {
//     pinMode(0, INPUT);
//     _ready = true;
// }
//
// // Descriptive name of the sensor
// String description() {
//     return String("MICS-5524 @ TOUT");
// }
//
// // Descriptive name of the slot # index
// String slot(unsigned char index) {
//     return description();
// };
//
// // Address of the sensor (it could be the GPIO or I2C address)
// String address(unsigned char index) {
//     return String("0");
// }
//
// // Type for slot # index
// unsigned char type(unsigned char index) {
//     if (index == 0) return MAGNITUDE_ANALOG;
//     return MAGNITUDE_NONE;
// }
//
// // Current value for slot # index
// // Changed return type as moving to scaled value
// double value(unsigned char index) {
//     if (index == 0) return _read();
//     return 0;
// }
//
// protected:
//
// //CICM: this should be for raw values
// // renaming protected function "_read" to "_rawRead"
// unsigned int _rawRead() {
//     if (1 == _samples) return analogRead(0);
//     unsigned long sum = 0;
//     for (unsigned int i=0; i<_samples; i++) {
//         if (i>0) delayMicroseconds(_micros);
//         sum += analogRead(0);
//     }
//     return sum / _samples;
// }
//
// //CICM: and proper read should be scalable and thus needs sign
// //and decimal part
// double _read() {
//   //Raw measure could also be a class variable with getter so that can
//   //be reported through MQTT, ...
//   unsigned int rawValue;
//   double scaledValue;
//   //const double _percentage = 100;
//   // Debugging doubles to string
//   DEBUG_MSG(("[MICS5524] Started standard read, factor: %s , offset: %s, percentage: %s, decimals: %d \n"), String(_factor).c_str(), String(_offset).c_str(), String(_percentage).c_str(), ANALOG_DECIMALS);
//   rawValue = _rawRead();
//   DEBUG_MSG(("[MICS5524] Raw read received: %d \n"), rawValue);
//   //scaledValue = _offset - _factor*rawValue;
//   //scaledValue = _percentage - (_factor*rawValue  + _offset);
//   scaledValue = rawValue;
//   DEBUG_MSG(("[MICS5524] Scaled value result: %s \n"), String(scaledValue).c_str());
//   return scaledValue;
// }
//
//
// unsigned int _samples = 1;
// unsigned long _micros = 0;
// //CICM: for scaling and offset, also with getters and setters
// double _factor = 1.0;
// double _offset = 0.0;
// double _percentage = 0.0;
//
//     unsigned long _R0 = MICS5524_R0;            // R0, calibration value at 25º on air
//     unsigned long _Rl = MICS5524_RL;            // RL, load resistance
//     unsigned long _Rs = 0;                      // cached resistance
//     unsigned char _redGPIO = MICS5524_RED_PIN;
// };

//#endif // SENSOR_SUPPORT && MICS5524_SUPPORT
