#pragma once

#include "config.h"
#include "types.h"

/* =========================================================
 * ZMIENNE GLOBALNE (współdzielone między modułami)
 * ========================================================= */

extern SystemData sys;
extern LogSample logBuffer[MAX_SAMPLES];
extern uint16_t sampleCount;


/* =========================================================
 * drv_RC
 * ========================================================= */

void initRC();
void updateRC();


/* =========================================================
 * drv_Actuators
 * ========================================================= */

void initActuators();
void setMotor(uint16_t pwmUs);
void setRudder(uint16_t pwmUs);
void stopMotor();


/* =========================================================
 * drv_Sensors
 * ========================================================= */

void initSensors();
void updateSensors();


/* =========================================================
 * logic_FSM
 * ========================================================= */

void updateFSM();
void setState(BoatState newState);


/* =========================================================
 * logic_Safety
 * ========================================================= */

void updateSafety();
bool isBatteryLow();
bool isPowerOverload();


/* =========================================================
 * log_Data
 * ========================================================= */

void initLog();
void addSample();
void clearLog();
void dumpLog(Print& stream);  // Przyjmuje SerialBT lub Serial


/* =========================================================
 * hmi_BT (Zarządzanie radiem i konsolą telemetryczną)
 * ========================================================= */

void initBT();
void updateBT();
void startBT();              // Aktywacja radia BluetoothSerial
void stopBT();               // Wyłączenie radia BT (ochrona pasma 2.4 GHz)
void dumpLog();              // Zrzut bufora RAM w formacie CSV


/* =========================================================
 * hmi_Status (Sygnalizacja NeoPixel)
 * ========================================================= */

void initStatus();
void updateStatus();