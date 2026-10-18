#pragma once

#include <Arduino.h>


/* =========================================================
 * STANY AUTOMATU
 * ========================================================= */

enum BoatState : uint8_t {
    STATE_BOOT = 0,
    STATE_DISARMED,
    STATE_ARMED,
    STATE_RECORDING,
    STATE_FAILSAFE,
    STATE_BT_DUMP
};


/* =========================================================
 * PRZYCZYNY FAILSAFE
 * ========================================================= */

enum FailsafeReason : uint8_t {
    FAILSAFE_NONE = 0,
    FAILSAFE_RC_LOST,
    FAILSAFE_VOLTAGE_LOW,
    FAILSAFE_OVERCURRENT,
    FAILSAFE_OVERPOWER
};


/* =========================================================
 * STRUKTURA PRÓBKI LOGU
 *
 * Rozmiar: dokładnie 32 B
 * Bufor: 3000 x 32 B = 96 000 B
 * ========================================================= */

#pragma pack(push, 1)

struct LogSample {
    uint32_t time;         // Czas od uruchomienia [ms]
    int32_t  glat;         // Szerokość geograficzna x 1e6
    int32_t  glon;         // Długość geograficzna x 1e6
    uint16_t vbat;         // Napięcie [mV]
    int16_t  ibat;         // Prąd [mA]
    uint16_t gsog;         // Prędkość [cm/s]
    uint16_t head;         // Kurs magnetyczny z IMU [0,1 stopnia]
    uint16_t gcog;         // Kurs GPS nad dnem [0,1 stopnia]
    uint16_t thro;         // Sygnał sterujący ESC [us]
    uint16_t rudd;         // Sygnał serwa steru [us]
    uint16_t hdop;         // HDOP x 100
    uint8_t  gsat;         // Liczba satelitów
    uint8_t  gage;         // Wiek danych GPS w jednostkach 100 ms
    uint16_t rsvd;         // Rezerwa 2 B
};

#pragma pack(pop)

static_assert(
    sizeof(LogSample) == 32,
    "LogSample must be exactly 32 bytes"
);


/* =========================================================
 * GŁÓWNA STRUKTURA DANYCH SYSTEMU
 * ========================================================= */

struct SystemData {

    /* -----------------------------------------------------
     * Stan systemu i bezpieczeństwo
     * ----------------------------------------------------- */

    BoatState      state;
    FailsafeReason failsafeReason;

    bool currentWarning;        // Prąd powyżej progu ostrzegawczego
    bool powerWarning;          // Moc powyżej progu ostrzegawczego


    /* -----------------------------------------------------
     * Odbiornik RC / CRSF
     * ----------------------------------------------------- */

    uint16_t rcSteerUs;         // CH1 - ster
    uint16_t rcThroUs;          // CH3 - gaz
    uint16_t rcSbUs;            // CH6 - SB
    uint16_t rcScUs;            // CH7 - SC

    bool     rcNoLimits;        // SC aktywne: pominięcie limitu PWM
    bool     linkUp;            // Poprawny link RC

    int8_t   rssiDbm;           // RSSI [dBm]
    uint8_t  lqPercent;         // Link Quality [%]


    /* -----------------------------------------------------
     * INA226
     * ----------------------------------------------------- */

    float voltageV;             // Napięcie pakietu [V]
    float currentA;             // Prąd [A]
    float powerW;               // Moc chwilowa [W]


    /* -----------------------------------------------------
     * BNO055
     * ----------------------------------------------------- */

    float headingDeg;           // Kurs 0,0...359,9 stopnia
    bool  imuHealthy;           // Poprawny stan sensora


    /* -----------------------------------------------------
     * GPS
     * ----------------------------------------------------- */

    double   lat;               // Szerokość geograficzna [stopnie]
    double   lon;               // Długość geograficzna [stopnie]

    float    speedKmh;          // Prędkość SOG [km/h]
    float    cogDeg;            // Kurs nad dnem [stopnie]
    float    hdop;              // HDOP

    uint32_t gpsAgeMs;          // Wiek danych GPS [ms]
    uint8_t  satellites;        // Liczba satelitów

    bool     gpsFixValid;       // Poprawna pozycja GPS


    /* -----------------------------------------------------
     * Rzeczywiste wyjścia
     * ----------------------------------------------------- */

    uint16_t escOutputUs;       // Sygnał ESC po limitach i rampie
    uint16_t servoOutputUs;     // Sygnał serwa


    /* -----------------------------------------------------
     * Bluetooth
     * ----------------------------------------------------- */

    bool btActive;              // Bluetooth uruchomiony
    bool btConnected;           // Klient Bluetooth połączony


    /* -----------------------------------------------------
     * Ustawienia i metadane testu
     *
     * Przechowywane wyłącznie w RAM.
     * ----------------------------------------------------- */

    uint16_t maxThrottlePwm;    // Maksymalny sygnał ESC [us]

    char propellerName[16];     // Maksymalnie 15 znaków + '\0'
    char testNote[32];          // Maksymalnie 31 znaków + '\0'
};
