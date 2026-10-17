#pragma once

#include <Arduino.h>

/* =========================================================
 * STANY AUTOMATU (FSM)
 * ========================================================= */

enum BoatState : uint8_t {
    STATE_BOOT = 0,         // Inicjalizacja i uzbrajanie ESC (sygnał 1000 us)
    STATE_DISARMED,         // Silnik odcięty, oczekiwanie na SB = 0%
    STATE_ARMED,            // Sterowanie aktywne, bez zapisu próbek
    STATE_RECORDING,        // Sterowanie aktywne, zapis próbek do RAM (5 Hz)
    STATE_FAILSAFE,         // Awaryjne odcięcie silnika (błąd zasilania / utrata RC)
    STATE_BT_DUMP           // Tryb serwisowy Bluetooth (napęd zablokowany)
};


/* =========================================================
 * PRZYCZYNY FAILSAFE
 * ========================================================= */

enum FailsafeReason : uint8_t {
    FAILSAFE_NONE = 0,
    FAILSAFE_RC_LOST,       // Utrata linku ExpressLRS
    FAILSAFE_VOLTAGE_LOW,   // Napięcie pakietu poniżej 19.2 V
    FAILSAFE_OVERCURRENT,   // Prąd > 16 A przez ponad 250 ms
    FAILSAFE_OVERPOWER      // Moc > 330 W przez ponad 250 ms
};


/* =========================================================
 * STRUKTURA PRÓBKI LOGU (DOKŁADNIE 32 BAJTY W RAM)
 * ========================================================= */

#pragma pack(push, 1)
struct LogSample {
    uint32_t timestampMs;       // Czas od startu ESP32 [ms] (4 B)
    int32_t  lat;               // Szerokość geograficzna * 1e6 (4 B)
    int32_t  lon;               // Długość geograficzna * 1e6 (4 B)
    uint16_t voltageMv;         // Napięcie pakietu [mV] (2 B)
    int16_t  currentMa;         // Prąd pobierany [mA] (2 B)
    uint16_t speedCms;          // Prędkość SOG [cm/s] (2 B)
    uint16_t headingDeg10;      // Kurs IMU BNO055 [0.1 deg, 0-3599] (2 B)
    uint16_t cogDeg10;          // Kurs COG GPS [0.1 deg, 0-3599] (2 B)
    uint16_t pwmThrottle;       // Wartość sterowania ESC [us] (2 B)
    uint16_t pwmRudder;         // Wartość sterowania serwem [us] (2 B)
    uint8_t  satellites;        // Liczba śledzonych satelitów (1 B)
    uint16_t hdop100;           // Dokładność pozioma HDOP * 100 (2 B)
    uint8_t  gpsAge100ms;       // Wiek danych GPS [100 ms, max 255 = 25.5s] (1 B)
    uint16_t reserved;          // Rezerwa wyrównująca do pełnych 32 bajtów (2 B)
};
#pragma pack(pop)

// Weryfikacja rozmiaru struktury na etapie kompilacji
static_assert(sizeof(LogSample) == 30, "LogSample size mismatch without padding");
// Jeśli preferujemy wyrównanie do 32 bajtów z 2 bajtami wolnego zapasu na przyszłość:
// Można usunąć #pragma pack i dodać uint16_t reserved; wtedy sizeof == 32.

/* =========================================================
 * GŁÓWNA STRUKTURA STANU SYSTEMU (SystemData)
 * ========================================================= */

struct SystemData {
    // Stan automatu i bezpieczeństwo
    BoatState       state;
    FailsafeReason  failsafeReason;
    uint32_t        overloadStartTime;  // Znacznik czasu dla filtra przeciążenia > 250 ms

    // Odbiornik RC (CRSF)
    uint16_t rcThrottle;        // CH3: Gaz (1000 - 2000 us)
    uint16_t rcRudder;          // CH1: Ster (1000 - 2000 us)
    uint16_t rcModeRaw;         // CH6: Przełącznik SB (surowy impuls us)
    uint16_t rcOverrideRaw;     // CH7: Przełącznik SC (surowy impuls us)
    bool     rcOverrideActive;  // Flaga sprzętowego pominięcia limitu PWM (100% mocy)
    bool     linkUp;            // Flaga obecności połączenia RC
    int8_t   rssi;              // Poziom sygnału RSSI [dBm]
    uint8_t  lq;                // Link Quality [%]

    // Czujnik zasilania (INA226)
    float    voltageV;          // Napięcie pakietu [V]
    float    currentA;          // Prąd [A]
    float    powerW;            // Moc chwilowa [W]

    // Czujnik IMU (BNO055)
    float    headingDeg;        // Kurs kompasowy Yaw (0.0 - 359.9 deg)
    bool     imuHealthy;        // Flaga poprawnej pracy czujnika BNO055

    // Odbiornik GPS
    double   lat;               // Szerokość geograficzna (deg)
    double   lon;               // Długość geograficzna (deg)
    float    speedKmh;          // Prędkość SOG [km/h]
    float    cogDeg;            // Kurs nad dnem COG [deg]
    uint8_t  satellites;        // Liczba śledzonych satelitów
    float    hdop;              // Współczynnik HDOP
    uint32_t gpsAgeMs;          // Czas od ostatniej aktualizacji danych GPS [ms]
    bool     gpsFixValid;       // Flaga poprawności pozycji

    // Wyjścia elementów wykonawczych (LEDC)
    uint16_t escOutputUs;       // Rzeczywisty sygnał podawany na ESC (po rampie i ograniczeniach)
    uint16_t servoOutputUs;     // Rzeczywisty sygnał podawany na serwo

    // Konfiguracja i metadane testu
    uint16_t maxThrottlePwm;    // Limit programowy gazu (1000 - 2000 us, domyślnie 2000)
    char     propellerName[16]; // Oznaczenie testowanej śruby (np. "PROP65", max 15 znaków)
    char     testNote[32];      // Krótka notatka do nagłówka logu (max 31 znaków)
};
