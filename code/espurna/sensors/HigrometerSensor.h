// -----------------------------------------------------------------------------
// Analog Sensor (maps to an analogRead)
// Copyright (C) 2017-2018 by Xose Pérez <xose dot perez at gmail dot com>
// -----------------------------------------------------------------------------

#if SENSOR_SUPPORT && ANALOGH_SUPPORT

#pragma once

// Set ADC to TOUT pin
// #undef ADC_MODE_VALUE
// #define ADC_MODE_VALUE ADC_TOUT

#include <Arduino.h>
#include "../utils.h"
#include "BaseSensor.h"
#include "BaseAnalogSensor.h"
#include "../debug.h"

class HigrometerSensor : public BaseAnalogSensor {

    public:

        // ---------------------------------------------------------------------
        // Public
        // ---------------------------------------------------------------------

        HigrometerSensor() {
            _count = 1;
            _sensor_id = SENSOR_ANALOG_ID;
        }

        void setSamples(unsigned int samples) {
            if (_samples > 0) _samples = samples;
        }

        void setDelay(unsigned long micros) {
            _micros = micros;
        }

        void setFactor(double factor) {
            #if SENSOR_DEBUG
              DEBUG_MSG(("[H_SENSOR] Factor set to: %s \n"), String(factor,6).c_str());
            #endif
            _factor = factor;
        }

        void setOffset(double offset) {
            #if SENSOR_DEBUG
              DEBUG_MSG(("[H_SENSOR] Offset set to: %s \n"), String(offset,6).c_str());
            #endif
            _offset = offset;
        }

        void setPercentage(double percentage) {
            #if SENSOR_DEBUG
              DEBUG_MSG(("[H_SENSOR] Percentage set to: %s \n"), String(percentage,6).c_str());
            #endif
            _percentage = percentage;
        }
        // ---------------------------------------------------------------------

        unsigned int getSamples() {
            return _samples;
        }

        unsigned long getDelay() {
            return _micros;
        }

        double getFactor() {
            return _factor;
        }

        double getOffset() {
            return _offset;
        }

        double getPercentage() {
            return _percentage;
        }

        // ---------------------------------------------------------------------
        // Sensor API
        // ---------------------------------------------------------------------

        // Initialization method, must be idempotent
        void begin() {
            pinMode(0, INPUT);
            _ready = true;
        }

        // Descriptive name of the sensor
        String description() {
            return String("ANALOGH @ TOUT");
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
            if (index == 0) return MAGNITUDE_ANALOG;
            return MAGNITUDE_NONE;
        }

        // Current value for slot # index
        // Changed return type as moving to scaled value
        double value(unsigned char index) {
            if (index == 0) return _read();
            return 0;
        }

    protected:

        //CICM: this should be for raw values
        // renaming protected function "_read" to "_rawRead"
        unsigned int _rawRead() {
            if (1 == _samples) return analogRead(0);
            unsigned long sum = 0;
            for (unsigned int i=0; i<_samples; i++) {
                if (i>0) delayMicroseconds(_micros);
                sum += analogRead(0);
            }
            return sum / _samples;
        }

        //CICM: and proper read should be scalable and thus needs sign
        //and decimal part
        double _read() {
          //Raw measure could also be a class variable with getter so that can
          //be reported through MQTT, ...
          unsigned int rawValue;
          double scaledValue;
          //const double _percentage = 100;
          // Debugging doubles to string
          rawValue = _rawRead();
          //scaledValue = _offset - _factor*rawValue;
          scaledValue = _percentage - (_factor*rawValue  + _offset);
          #if SENSOR_DEBUG
            DEBUG_MSG(("[H_SENSOR] Started standard read, factor: %s , offset: %s, percentage: %s, decimals: %d \n"), String(_factor).c_str(), String(_offset).c_str(), String(_percentage).c_str(), ANALOG_DECIMALS);
            DEBUG_MSG(("[H_SENSOR] Raw read received: %d \n"), rawValue);
            DEBUG_MSG(("[H_SENSOR] Scaled value result: %s \n"), String(scaledValue).c_str());
          #endif
          return scaledValue;
        }


        unsigned int _samples = 1;
        unsigned long _micros = 0;
        //CICM: for scaling and offset, also with getters and setters
        double _factor = 1.0;
        double _offset = 0.0;
        double _percentage = 0.0;

};

#endif // SENSOR_SUPPORT && ANALOGH_SUPPORT
