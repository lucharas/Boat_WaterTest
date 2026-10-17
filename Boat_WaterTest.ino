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

static uint32_t lastTaskRcMs        = 0;
static uint32_t lastTaskActuatorsMs = 0;
static uint32_t lastTaskSensorMs    = 0;
static uint32_t lastTaskLogMs       = 0;
static uint32_t lastTaskStatusMs    = 0;

/* =========================================================
 * SETUP
 * ========================================================= */

void setup() {
    // 1. Port szeregowy USB (konsola serwisowa)
    Serial.begin(115200);
    delay(100); // Czas na ustabilizowanie UART po wpięciu USB
    Serial.println();
    Serial.println(F("========================================"));
    Serial.print(F("Booting "));
    Serial.print(FW_NAME);
    Serial.print(F(" v"));
    Serial.println(FW_VERSION);
    Serial.println(F("========================================"));

    // 2. Inicjalizacja stanu domyślnego systemu
    memset(&sys, 0, sizeof(SystemData));
    sys.state          = STATE_BOOT;
    sys.failsafeReason = FAILSAFE_NONE;
    sys.maxThrottlePwm = ESC_MAX_US; // Domyślny limit PWM (2000 us)
    strncpy(sys.propellerName, "DEFAULT", sizeof(sys.propellerName) - 1);
    strncpy(sys.testNote, "WATER_TEST", sizeof(sys.testNote) - 1);

    // 3. Moduły sprzętowe i peryferia
    initStatus();      // Dioda NeoPixel
    initActuators();   // LEDC PWM dla ESC i serwa (silnik stop 1000 us, serwo neutrum 1500 us)
    initSensors();     // Magistrala I2C (INA226, BNO055) + UART2 (GPS)
    initRC();          // ExpressLRS CRSF (HardwareSerial 1)
    initLog();         // Zerowanie wskaźnika bufora próbek RAM
    initBT();          // Inicjalizacja stosu Bluetooth Classic SPP

    Serial.println(F("[BOOT] System initialization complete. Entering BOOT hold state."));
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

        updateRC();      // Odbiór i dekodowanie ramek CRSF (gaz, ster, przełączniki SB i SC)
        updateSafety();  // Walidacja prądu, mocy, baterii i linku RC
        updateFSM();     // Logika automatu i ustalenie sygnałów sterujących
    }

    // -----------------------------------------------------
    // TASK 2: Aktuatory - LEDC PWM i rampa rozbiegu (50 Hz / 20 ms)
    // -----------------------------------------------------
    if (now - lastTaskActuatorsMs >= TASK_ACTUATORS_MS) {
        lastTaskActuatorsMs = now;

        updateActuators(); // Przeliczenie rampy ESC i fizyczne wysterowanie LEDC
    }

    // -----------------------------------------------------
    // TASK 3: Czujniki INA226, BNO055, GPS (10 Hz / 100 ms)
    // -----------------------------------------------------
    if (now - lastTaskSensorMs >= TASK_SENSOR_MS) {
        lastTaskSensorMs = now;

        updateSensors();
    }

    // -----------------------------------------------------
    // TASK 4: Rejestracja próbek w RAM (5 Hz / 200 ms)
    // -----------------------------------------------------
    if (now - lastTaskLogMs >= TASK_LOG_MS) {
        lastTaskLogMs = now;

        addSample();     // Zapisuje próbkę wyłącznie w stanie STATE_RECORDING
    }

    // -----------------------------------------------------
    // TASK 5: Sygnalizacja optyczna NeoPixel (20 Hz / 50 ms)
    // -----------------------------------------------------
    if (now - lastTaskStatusMs >= TASK_STATUS_MS) {
        lastTaskStatusMs = now;

        updateStatus();
    }

    // -----------------------------------------------------
    // Obsługa konsoli Bluetooth (w każdej iteracji pętli)
    // -----------------------------------------------------
    updateBT();
}
