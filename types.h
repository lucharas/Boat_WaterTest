#pragma once

#include <Arduino.h>
#include "config.h"

/* =========================================================
 * 1. MASZYNA STANÓW (FSM)
 * ========================================================= */

enum BoatState : uint8_t {
    STATE_BOOT = 0,         // Inicjalizacja sprzętu, kalibracja, sekwencja 3s dla ESC
    STATE_DISARMED,         // Silnik STOP (1000 us), oczekiwanie na spełnienie warunków uzbrojenia
    STATE_ARMED,            // Gotowość do startu, silnik STOP (gaz na minimum)
    STATE_RECORDING,        // Aktywny rejs - zbieranie próbek do RAM przy gazie > 1080 us
    STATE_FAILSAFE,         // Błąd krytyczny (bateria, przeciążenie, brak RC) - silnik STOP
    STATE_BT_DUMP           // Zrzut danych CSV przez BT - silnik bezwzględnie zablokowany
};

enum FailsafeReason : uint8_t {
    FAILSAFE_NONE = 0,
    FAILSAFE_RC_LOST,       // Utrata linku CRSF / brak pakietów
    FAILSAFE_BATTERY_LOW,   // Napięcie pakietu spadło poniżej progu odcięcia (19.2 V)
    FAILSAFE_OVERCURRENT,   // Prąd przekroczył 16 A przez > 250 ms
    FAILSAFE_OVERPOWER      // Moc przekroczyła 330 W przez > 250 ms
};


/* =========================================================
 * 2. REKORD DANYCH POMIAROWYCH (LogSample - dokładnie 32B)
 * ========================================================= */

#pragma pack(push, 1)
struct LogSample {
    uint32_t timestampMs;   // 4B: Czas od uruchomienia (millis)
    int32_t  lat;           // 4B: Szerokość geograficzna (scaled 1e-6)
    int32_t  lon;           // 4B: Długość geograficzna (scaled 1e-6)
    uint16_t voltageMv;     // 2B: Napięcie szyny zasilania w mV
    int16_t  currentMa;     // 2B: Prąd w mA
    uint16_t powerMw;       // 2B: Moc w 0.1 W
    uint16_t speedCms;      // 2B: Prędkość SOG z GPS w cm/s (1 m/s = 100 cm/s)
    uint16_t headingDeg10;  // 2B: Kurs IMU (Yaw z BNO055) w 0.1 deg (0-3599)
    uint16_t cogDeg10;      // 2B: Kurs GPS (COG) w 0.1 deg (0-3599)
    uint16_t pwmThrottle;   // 2B: Sygnał PWM wysłany na ESC (1000-2000 us)
    uint16_t pwmRudder;     // 2B: Sygnał PWM wysłany na serwo (1000-2000 us)
    uint8_t  satellites;    // 1B: Liczba satelitów GPS
    uint8_t  stateFlags;    // 1B: Maska bitowa (stan FSM, status fixa GPS, alarmy)
};
#pragma pack(pop)

static_assert(sizeof(LogSample) == 32, "LogSample size mismatch! Must be exactly 32 bytes.");


/* =========================================================
 * 3. ZBIORCZA STRUKTURA DANYCH SYSTEMOWYCH (SystemData)
 * ========================================================= */

struct SystemData {
    // Stan automatu
    BoatState      state;
    BoatState      previousState;
    FailsafeReason failsafeReason;

    // Aparatura i link RC (CRSF / ELRS)
    uint16_t rcThrottleUs;      // Bieżący odczyt gazu z aparatury (CH3)
    uint16_t rcRudderUs;        // Bieżący odczyt steru z aparatury (CH1)
    uint16_t rcSwitchSbUs;      // Przełącznik SB (CH6) - sterowanie BT
    uint16_t rcSwitchScUs;      // Przełącznik SC (CH7) - zapasowy
    bool     rcLinkUp;          // Flaga aktywnego linku radiowego
    uint8_t  rcLinkQuality;     // Link Quality w %
    int8_t   rcRssi;            // RSSI w dBm

    // Nastawy aktuatorów i limit testowy
    uint16_t escOutputUs;       // Wartość PWM wysyłana na ESC
    uint16_t servoOutputUs;     // Wartość PWM wysyłana na serwo
    uint16_t maxThrottlePwm;    // Maksymalny limit PWM dla drążka (1180 - 2000 us)
    char     maxPwmStr[8];      // Tekstowa etykieta zadanego limitu (np. "pwm80")

    // Zasilanie (INA226)
    float    vbat;              // Napięcie pakietu w V
    float    currentA;          // Prąd w A
    float    powerW;            // Moc w W
    bool     batteryLow;        // Flaga ostrzeżenia o niskim napięciu (< 19.8 V)
    bool     powerOverload;     // Flaga ostrzeżenia o przeciążeniu (> 13 A / > 280 W)

    // Orientacja przestrzenna (BNO055)
    float    yaw;               // Kurs kompasowy (0.0 - 359.9°)
    bool     imuHealthy;        // Flaga poprawnej komunikacji z BNO055

    // Pozycjonowanie (GPS)
    int32_t  gpsLat;            // Raw latitude (1e-6)
    int32_t  gpsLon;            // Raw longitude (1e-6)
    float    gpsSpeedMs;        // Prędkość SOG w m/s
    float    gpsCourse;         // Kurs nad dnem COG w stopniach
    uint8_t  gpsSats;           // Liczba śledzonych satelitów
    bool     gpsFix;            // Flaga ważności pozycji GPS

    // Bluetooth
    bool     btActive;          // Czy moduł BT jest włączony sprzętowo
    bool     btConnected;       // Czy klient jest połączony

    // Metadane testu
    char     propellerName[24]; // Nazwa testowanej śruby
    char     testNote[32];      // Notatka do bieżącego testu
};