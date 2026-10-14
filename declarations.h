#pragma once

#include "config.h"
#include "types.h"

/* =========================
   Zmienne globalne
   ========================= */

extern SystemData sys;

extern LogSample logBuffer[MAX_SAMPLES];
extern uint16_t sampleCount;


/* =========================
   drv_RC
   ========================= */

void initRC();
void updateRC();


/* =========================
   drv_Actuators
   ========================= */

void initActuators();
void setMotor(uint16_t pwmUs);
void setRudder(uint16_t pwmUs);
void stopMotor();


/* =========================
   drv_Sensors
   ========================= */

void initSensors();
void updateSensors();


/* =========================
   logic_FSM
   ========================= */

void updateFSM();
void setState(BoatState newState);


/* =========================
   logic_Safety
   ========================= */

void updateSafety();
bool isBatteryLow();
bool isPowerOverload();


/* =========================
   log_Data
   ========================= */

void initLog();
void addSample();
void clearLog();


/* =========================
   hmi_BT
   ========================= */

void initBT();
void updateBT();
void dumpLog();


/* =========================
   hmi_Status
   ========================= */

void initStatus();
void updateStatus();
void setLedColor(uint8_t color);