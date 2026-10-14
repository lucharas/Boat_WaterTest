#pragma once

#include <Arduino.h>

/* =========================================================
 * Stany systemu
 * ========================================================= */

enum BoatState
{
    DISARMED,
    ARMED,
    RECORDING,
    FAILSAFE,
    BT_DUMP
};


/* =========================================================
 * Dane RC
 * ========================================================= */

struct RcData
{
    uint16_t throttleUs;
    uint16_t rudderUs;

    bool valid;
    uint32_t lastUpdateMs;
};


/* =========================================================
 * Dane GPS
 * ========================================================= */

struct GpsData
{
    double latitude;
    double longitude;

    float speedKmh;
    float courseDeg;

    uint8_t satellites;
    float hdop;

    bool valid;
};


/* =========================================================
 * Dane z INA226
 * ========================================================= */

struct PowerData
{
    float voltageV;
    float currentA;
    float powerW;

    bool valid;
};


/* =========================================================
 * Dane IMU
 * ========================================================= */

struct ImuData
{
    float headingDeg;

    float rollDeg;
    float pitchDeg;

    bool valid;
};


/* =========================================================
 * Jedna próbka logu
 * ========================================================= */

struct LogSample
{
    uint32_t timeMs;

    uint16_t throttleUs;
    uint16_t rudderUs;

    double latitude;
    double longitude;

    float speedKmh;

    float voltageV;
    float currentA;
    float powerW;

    float headingDeg;

    uint8_t satellites;
    float hdop;
};


/* =========================================================
 * Dane całego systemu
 * ========================================================= */

struct SystemData
{
    BoatState state;

    RcData rc;
    GpsData gps;
    PowerData power;
    ImuData imu;

    bool armed;
    bool recording;
    bool failsafe;
};