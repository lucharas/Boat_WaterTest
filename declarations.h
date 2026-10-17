#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include <TinyGPSPlus.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_NeoPixel.h>
#include <INA226_WE.h>
#include <CRSFforArduino.hpp>

#include "config.h"
#include "types.h"


/* =========================================================
 * WSPÓLNE DANE SYSTEMOWE
 *
 * Definicje znajdują się w Boat_WaterTest.ino.
 * ========================================================= */

extern SystemData sys;

extern LogSample logBuffer[MAX_SAMPLES];
extern uint16_t  sampleCount;


/* =========================================================
 * OBIEKTY SPRZĘTOWE I BIBLIOTECZNE
 *
 * Definicje znajdują się w Boat_WaterTest.ino.
 * ========================================================= */

extern BluetoothSerial   SerialBT;
extern TinyGPSPlus       gps;
extern Adafruit_NeoPixel statusLed;
extern INA226_WE         ina226;
extern Adafruit_BNO055   bno;
extern CRSFforArduino    crsf;


/* =========================================================
 * drv_RC.ino
 *
 * ExpressLRS / CRSF
 * ========================================================= */

void initRC();
void updateRC();


/* =========================================================
 * drv_Actuators.ino
 *
 * ESC i serwo
 * ========================================================= */

void initActuators();
void updateActuators();

void setMotor(uint16_t pwmUs);
void stopMotor();

void setRudder(uint16_t pwmUs);


/* =========================================================
 * drv_Sensors.ino
 *
 * INA226, BNO055 i GPS
 * ========================================================= */

void initSensors();
void updateSensors();


/* =========================================================
 * logic_FSM.ino
 *
 * Maszyna stanów
 * ========================================================= */

void 
