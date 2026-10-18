#pragma once
#include <Arduino.h>

// --- STANY AUTOMATU I FAILSAFE ---
enum BoatState : uint8_t {
    STATE_BOOT = 0, STATE_DISARMED, STATE_ARMED,
    STATE_RECORDING, STATE_FAILSAFE, STATE_BT_DUMP
};

enum FailsafeReason : uint8_t {
    FAILSAFE_NONE = 0, FAILSAFE_RC_LOST, FAILSAFE_VOLTAGE_LOW,
    FAILSAFE_OVERCURRENT, FAILSAFE_OVERPOWER
};

// --- STRUKTURA PRÓBKI LOGU (Dokładnie 32 B) ---
#pragma pack(push, 1)
struct LogSample {
    uint32_t time;         // Czas od uruchomienia [ms]
    int32_t  glat;         // Szer. geograficzna x 1e6
    int32_t  glon;         // Dług. geograficzna x 1e6
    uint16_t vbat;         // Napięcie [mV]
    int16_t  ibat;         // Prąd [mA]
    uint16_t gsog;         // Prędkość [cm/s]
    uint16_t head;         // Kurs IMU [0,1 stopnia]
    uint16_t gcog;         // Kurs GPS [0,1 stopnia]
    uint16_t thro;         // Sygnał ESC [us]
    uint16_t rudd;         // Sygnał serwa [us]
    uint16_t hdop;         // HDOP x 100
    uint8_t  gsat;         // Liczba satelitów
    uint8_t  gage;         // Wiek danych GPS [100 ms]
    uint16_t rsvd;         // Rezerwa 2 B
};
#pragma pack(pop)

static_assert(sizeof(LogSample) == 32, "LogSample size error");

// --- GŁÓWNA STRUKTURA DANYCH SYSTEMU ---
struct SystemData {
    // Stan systemu i bezpieczeństwo
    BoatState      state;
    FailsafeReason failsafeReason;
    bool currentWarning, powerWarning;

    // Odbiornik RC
    uint16_t rcSteerUs, rcThroUs, rcSbUs, rcScUs;
    bool     rcNoLimits, linkUp;
    int8_t   rssiDbm;
    uint8_t  lqPercent;

    // Zasilanie (INA226)
    float voltageV, currentA, powerW;

    // BNO055
    float headingDeg;
    bool  imuHealthy;
    uint8_t magCalib;       // Jakość kalibracji magnetometru (0-3)

    // GPS
    double   lat, lon;
    float    speedKmh, cogDeg, hdop;
    uint32_t gpsAgeMs;
    uint8_t  satellites;
    bool     gpsFixValid;

    // Rzeczywiste wyjścia
    uint16_t escOutputUs, servoOutputUs;

    // Bluetooth
    bool btActive, btConnected;

    // Ustawienia i metadane testu
    uint16_t maxThrottlePwm;
    char propellerName[8];  // Max 7 znaków + '\0' (np. "55", "55x4.5")
    char testNote[32];      // Max 31 znaków + '\0'
};
