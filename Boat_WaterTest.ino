#include "declarations.h"

BluetoothSerial SerialBT;
TinyGPSPlus gps;
Adafruit_NeoPixel statusLed(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
INA226_WE ina226( INA226_I2C_ADDR);
Adafruit_BNO055 bno(55, BNO055_I2C_ADDR,&Wire);
CRSFforArduino crsf(&Serial1);

// --- WSPÓLNE DANE SYSTEMU ---
SystemData sys = {};
LogSample logBuffer[MAX_SAMPLES];
uint16_t sampleCount = 0;

// --- ZNACZNIKI CZASU SCHEDULERA ---
static uint32_t lastRcMs        = 0;
static uint32_t lastActuatorsMs = 0;
static uint32_t lastSensorsMs   = 0;
static uint32_t lastLogMs       = 0;
static uint32_t lastStatusMs    = 0;

void setup() {
    Serial.begin(115200);
    initActuators();
    initStatus();
    initSensors();
    initRC();
    initLog();
    initBT();
    initFSM();
    
    const uint32_t now = millis();

    lastRcMs        = now;
    lastActuatorsMs = now;
    lastSensorsMs   = now;
    lastLogMs       = now;
    lastStatusMs    = now;

    Serial.println(
        F("[BOOT] Initialization complete")
    );
}

void loop() {
    const uint32_t now = millis();
    if (now - lastRcMs >= TASK_RC_MS) {
        lastRcMs += TASK_RC_MS;

        updateRC();
        updateSafety();
        updateFSM();
    }


    if (
        now - lastActuatorsMs >= TASK_ACTUATORS_MS
    ) {
        lastActuatorsMs += TASK_ACTUATORS_MS;
        updateActuators();
    }

    if (now - lastSensorsMs >= TASK_SENSOR_MS) {
        lastSensorsMs += TASK_SENSOR_MS;
        updateSensors();
    }

    if (now - lastLogMs >= TASK_LOG_MS) {
        lastLogMs += TASK_LOG_MS;
        addSample();
    }

    if (now - lastStatusMs >= TASK_STATUS_MS) {
        lastStatusMs += TASK_STATUS_MS;
        updateStatus();
    }

    updateBT();
}
