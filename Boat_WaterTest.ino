#include "declarations.h"


/* =========================================================
 * OBIEKTY SPRZĘTOWE I BIBLIOTECZNE
 *
 * Definiowane dokładnie jeden raz.
 * Pozostałe moduły korzystają z deklaracji extern
 * znajdujących się w declarations.h.
 * ========================================================= */

BluetoothSerial SerialBT;

TinyGPSPlus gps;

Adafruit_NeoPixel statusLed(
    NUM_PIXELS,
    PIN_NEOPIXEL,
    NEO_GRB + NEO_KHZ800
);

INA226_WE ina226(
    INA226_I2C_ADDR
);

Adafruit_BNO055 bno(
    55,
    BNO055_I2C_ADDR,
    &Wire
);

CRSFforArduino crsf(
    &Serial1
);


/* =========================================================
 * WSPÓLNE DANE SYSTEMU
 * ========================================================= */

SystemData sys = {};

LogSample logBuffer[MAX_SAMPLES];

uint16_t sampleCount = 0;


/* =========================================================
 * ZNACZNIKI CZASU SCHEDULERA
 * ========================================================= */

static uint32_t lastRcMs        = 0;
static uint32_t lastActuatorsMs = 0;
static uint32_t lastSensorsMs   = 0;
static uint32_t lastLogMs       = 0;
static uint32_t lastStatusMs    = 0;


/* =========================================================
 * SETUP
 * ========================================================= */

void setup() {
    Serial.begin(115200);

    /*
     * Najpierw inicjalizowane są wyjścia, aby ESC od początku
     * otrzymywał bezpieczny sygnał.
     */

    initActuators();
    initStatus();

    /*
     * Następnie urządzenia wejściowe i bufory.
     */

    initSensors();
    initRC();
    initLog();
    initBT();

    /*
     * FSM jest uruchamiany jako ostatni, ponieważ korzysta
     * z aktuatorów oraz funkcji zarządzania Bluetooth.
     */

    initFSM();

    /*
     * Wszystkie zadania schedulera zaczynają liczyć czas
     * od zakończenia inicjalizacji.
     */

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


/* =========================================================
 * LOOP
 *
 * Zawiera wyłącznie scheduler i obsługę zadań.
 * ========================================================= */

void loop() {
    const uint32_t now = millis();


    /* =====================================================
     * ODBIORNIK RC, BEZPIECZEŃSTWO I FSM
     *
     * Interwał: TASK_RC_MS
     * ===================================================== */

    if (now - lastRcMs >= TASK_RC_MS) {
        lastRcMs += TASK_RC_MS;

        updateRC();
        updateSafety();
        updateFSM();
    }


    /* =====================================================
     * AKTUATORY
     *
     * Interwał: TASK_ACTUATORS_MS
     * ===================================================== */

    if (
        now - lastActuatorsMs >=
        TASK_ACTUATORS_MS
    ) {
        lastActuatorsMs += TASK_ACTUATORS_MS;

        updateActuators();
    }


    /* =====================================================
     * SENSORY
     *
     * Interwał: TASK_SENSOR_MS
     * ===================================================== */

    if (now - lastSensorsMs >= TASK_SENSOR_MS) {
        lastSensorsMs += TASK_SENSOR_MS;

        updateSensors();
    }


    /* =====================================================
     * LOGOWANIE
     *
     * Interwał: TASK_LOG_MS
     * addSample() sam sprawdza STATE_RECORDING.
     * ===================================================== */

    if (now - lastLogMs >= TASK_LOG_MS) {
        lastLogMs += TASK_LOG_MS;

        addSample();
    }


    /* =====================================================
     * NEOPIXEL
     *
     * Interwał: TASK_STATUS_MS
     * ===================================================== */

    if (now - lastStatusMs >= TASK_STATUS_MS) {
        lastStatusMs += TASK_STATUS_MS;

        updateStatus();
    }


    /* =====================================================
     * BLUETOOTH
     *
     * Obsługiwany w każdej iteracji pętli. Funkcja szybko
     * kończy działanie, jeżeli Bluetooth jest wyłączony.
     * ===================================================== */

    updateBT();
}
