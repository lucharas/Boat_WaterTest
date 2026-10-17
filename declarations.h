#pragma once

#include <Arduino.h>
#include "config.h"
#include "types.h"

/* =========================================================
 * ZMIENNE GLOBALNE WSPÓŁDZIELONE (definiowane w Boat_WaterTest.ino)
 * ========================================================= */

extern SystemData sys;
extern LogSample logBuffer[MAX_SAMPLES];
extern uint16_t sampleCount;


/* =========================================================
 * drv_RC (Obsługa protokołu CRSF / ExpressLRS)
 * ========================================================= */

void initRC();
void updateRC();


/* =========================================================
 * drv_Actuators (LEDC PWM dla ESC Flycolor i serwa steru)
 * ========================================================= */

void initActuators();
void updateActuators();
void setMotor(uint16_t pwmUs);
void setRudder(uint16_t pwmUs);
void stopMotor();


/* =========================================================
 * drv_Sensors (I2C: INA226, BNO055 oraz UART: GPS TinyGPS++)
 * ========================================================= */

void initSensors();
void updateSensors();


/* =========================================================
 * logic_FSM (Jawna maszyna stanów łodzi)
 * ========================================================= */

void initFSM();
void updateFSM();
void setState(BoatState newState);


/* =========================================================
 * logic_Safety (Nadzór nad zasilaniem, przeciążeniem i linkiem RC)
 * ========================================================= */

void updateSafety();
bool isBatteryLow();
bool isPowerOverload();


/* =========================================================
 * log_Data (Rejestracja próbek do RAM i eksport CSV)
 * ========================================================= */

void initLog();
void addSample();
void clearLog();
void dumpLog(Stream &out);


/* =========================================================
 * hmi_BT (Bluetooth Serial - diagnostyka i komendy)
 * ========================================================= */

void initBT();
void updateBT();


/* =========================================================
 * hmi_Status (Animacje diody WS2812B NeoPixel)
 * ========================================================= */

void initStatus();
void updateStatus();
void setLedColor(uint8_t r, uint8_t g, uint8_t b);
