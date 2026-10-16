#include <Arduino.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include <TinyGPSPlus.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_NeoPixel.h>
#include <INA226_WE.h>
#include <CRSFforArduino.h>

#include "config.h"
#include "types.h"
#include "declarations.h"

/* =========================================================
 * INSTANCJE OBIEKTÓW SPRZĘTOWYCH I BIBLIOTEK
 * ========================================================= */

BluetoothSerial   SerialBT;
TinyGPSPlus       gps;
Adafruit_NeoPixel statusLed(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
INA226_WE         ina226(INA226_I2C_ADDR);
Adafruit_BNO055   bno(55, BNO055_I2C_ADDR);

/* =========================================================
 * ZMIENNE GLOBALNE WSPÓŁDZIELONE (Zgodnie z declarations.h)
 * ========================================================= */

SystemData sys;

LogSample logBuffer[MAX_SAMPLES];
uint16_t  sampleCount = 0;

/* =========================================================
 * TIMERY SCHEDULERA (millis)
 * ========================================================= */

static uint32_t lastTaskRcMs     = 0;
static uint32_t lastTaskSensorMs = 0;
static uint32_t lastTaskLogMs    = 0;
static uint32_t lastTaskStatusMs = 0;

/* =========================================================
 * SETUP
 * ========================================================= */

void setup() {
    // 1. Port szeregowy USB (konsola serwisowa)
    Serial.begin(115200);
    delay(100); // Krótki czas na stabilizację UART po podłączeniu kabla USB
    Serial.println();
    Serial.println(F("========================================"));
    Serial.print(F("Booting "));
    Serial.print(FW_NAME);
    Serial.print(F(" v"));
    Serial.println(FW_VERSION);
    Serial.println(F("========================================"));

    // 2. Inicjalizacja stanu domyślnego systemu
    memset(&sys, 0, sizeof(SystemData));
    sys.state            = STATE_BOOT;
    sys.previousState    = STATE_BOOT;
    sys.failsafeReason   = FAILSAFE_NONE;
    sys.maxThrottlePwm   = ESC_MAX_US; // Domyślny limit PWM (2000 us)
    strncpy(sys.maxPwmStr, "100%", sizeof(sys.maxPwmStr) - 1);

    // 3. Moduły sprzętowe i peryferia
    initStatus();      // Dioda NeoPixel
    initActuators();   // LEDC PWM dla ESC i serwa (silnik stop, serwo neutrum)
    initSensors();     // Magistrala I2C (INA226, BNO055) + UART2 (GPS)
    initRC();          // ExpressLRS CRSF (HardwareSerial 1)
    initLog();         // Reset bufora próbek RAM
    initBT();          // Flagi modułu Bluetooth

    Serial.println(F("[BOOT] Initialization sequence complete. Holding 1000us for ESC arm..."));
}

/* =========================================================
 * DETERMINISTYCZNY SCHEDULER W LOOP (Całkowity brak delay)
 * ========================================================= */

void loop() {
    uint32_t now = millis();

    // -----------------------------------------------------
    // TASK 1: Odczyt RC, Safety, Maszyna Stanów (50 Hz / 20 ms)
    // -----------------------------------------------------
    if (now - lastTaskRcMs >= TASK_RC_MS) {
        lastTaskRcMs = now;

        updateRC();      // Odbiór i dekodowanie ramek CRSF
        updateSafety();  // Walidacja prądu, mocy, baterii i linku RC
        updateFSM();     // Logika automatu i wysterowanie PWM LEDC
    }

    // -----------------------------------------------------
    // TASK 2: Czujniki INA226, BNO055, GPS (10 Hz / 100 ms)
    // -----------------------------------------------------
    if (now - lastTaskSensorMs >= TASK_SENSOR_MS) {
        lastTaskSensorMs = now;

        updateSensors();
    }

    // -----------------------------------------------------
    // TASK 3: Rejestracja próbek w RAM (5 Hz / 200 ms)
    // -----------------------------------------------------
    if (now - lastTaskLogMs >= TASK_LOG_MS) {
        lastTaskLogMs = now;

        addSample();     // Zapisuje wyłącznie w stanie STATE_RECORDING
    }

    // -----------------------------------------------------
    // TASK 4: Sygnalizacja optyczna NeoPixel (20 Hz / 50 ms)
    // -----------------------------------------------------
    if (now - lastTaskStatusMs >= TASK_STATUS_MS) {
        lastTaskStatusMs = now;

        updateStatus();
    }

    // -----------------------------------------------------
    // Obsługa konsoli Bluetooth (w wolnym czasie pętli)
    // -----------------------------------------------------
    updateBT();
}