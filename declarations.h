// ============================================================================
// PLIK: declarations.h
// ============================================================================
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

// --- WSPÓLNE DANE SYSTEMOWE ---
extern SystemData sys;
extern LogSample logBuffer[MAX_SAMPLES];
extern uint16_t  sampleCount;

// --- OBIEKTY SPRZĘTOWE I BIBLIOTECZNE ---
extern BluetoothSerial   SerialBT;
extern TinyGPSPlus       gps;
extern Adafruit_NeoPixel statusLed;
extern INA226_WE         ina226;
extern Adafruit_BNO055   bno;
extern CRSFforArduino    crsf;

// --- drv_RC.ino (ExpressLRS / CRSF) ---
void initRC();
void updateRC();

// --- drv_Actuators.ino (ESC i serwo) ---
void initActuators();
void updateActuators();
void setMotor(uint16_t pwmUs);
void stopMotor();
void setRudder(uint16_t pwmUs);

// --- drv_Sensors.ino (INA226, BNO055 i GPS) ---
void initSensors();
void updateSensors();

// --- logic_FSM.ino (Maszyna stanów) ---
void setState(BoatState newState);
void initFSM();
void updateFSM();

// --- logic_Safety.ino (Zabezpieczenia) ---
bool isBatteryLow();
bool isPowerOverload();
void updateSafety();

// --- log_data.ino (Logowanie i eksport CSV) ---
void initLog();
void clearLog();
void addSample();
void dumpLog(Print& out);

// --- hmi_BT.ino (Konsola Bluetooth) ---
void initBT();
void startBT();
void stopBT();
void updateBT();

// --- hmi_Status.ino (Sygnalizacja NeoPixel) ---
void initStatus();
void updateStatus();