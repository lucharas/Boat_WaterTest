#include "declarations.h"

#include <math.h>


/* =========================================================
 * FUNKCJE POMOCNICZE
 * ========================================================= */

static uint16_t floatToUint16(
    float value,
    float multiplier
) {
    if (!isfinite(value) || value <= 0.0f) {
        return 0;
    }

    const float scaled = value * multiplier;

    if (scaled >= 65535.0f) {
        return 65535;
    }

    return (uint16_t)lroundf(scaled);
}


static int16_t floatToInt16(
    float value,
    float multiplier
) {
    if (!isfinite(value)) {
        return 0;
    }

    const float scaled = value * multiplier;

    if (scaled <= -32768.0f) {
        return -32768;
    }

    if (scaled >= 32767.0f) {
        return 32767;
    }

    return (int16_t)lroundf(scaled);
}


static uint16_t angleToDeg10(float angleDeg) {
    if (!isfinite(angleDeg)) {
        return 0;
    }

    while (angleDeg < 0.0f) {
        angleDeg += 360.0f;
    }

    while (angleDeg >= 360.0f) {
        angleDeg -= 360.0f;
    }

    return (uint16_t)lroundf(angleDeg * 10.0f);
}


static int32_t coordinateToInt(double coordinate) {
    if (!isfinite(coordinate)) {
        return 0;
    }

    const double scaled = coordinate * 1000000.0;

    if (scaled <= -2147483648.0) {
        return INT32_MIN;
    }

    if (scaled >= 2147483647.0) {
        return INT32_MAX;
    }

    return (int32_t)llround(scaled);
}


static uint8_t gpsAgeTo100ms(uint32_t ageMs) {
    if (ageMs == UINT32_MAX) {
        return 255;
    }

    const uint32_t age100ms = ageMs / 100UL;

    return age100ms > 255UL
        ? 255
        : (uint8_t)age100ms;
}


/* =========================================================
 * INICJALIZACJA LOGOWANIA
 * ========================================================= */

void initLog() {
    sampleCount = 0;
}


/* =========================================================
 * LOGICZNE CZYSZCZENIE BUFORA
 *
 * Dane w RAM nie są nadpisywane. Zostaje wyzerowany jedynie
 * licznik próbek.
 * ========================================================= */

void clearLog() {
    sampleCount = 0;
}


/* =========================================================
 * ZAPIS PRÓBKI
 *
 * Wywoływany przez scheduler co TASK_LOG_MS.
 * Zapis następuje tylko w STATE_RECORDING.
 * ========================================================= */

void addSample() {
    if (sys.state != STATE_RECORDING) {
        return;
    }


    /* -----------------------------------------------------
     * Pełny bufor kończy nagrywanie
     * ----------------------------------------------------- */

    if (sampleCount >= MAX_SAMPLES) {
        setState(STATE_ARMED);
        return;
    }


    /* -----------------------------------------------------
     * Pobranie miejsca na kolejną próbkę
     * ----------------------------------------------------- */

    LogSample& sample = logBuffer[sampleCount];


    /* -----------------------------------------------------
     * Czas
     * ----------------------------------------------------- */

    sample.timestampMs = millis();


    /* -----------------------------------------------------
     * GPS
     *
     * Przy braku poprawnego fixa współrzędne są zerowane.
     * Jakość danych nadal można rozpoznać po HDOP, liczbie
     * satelitów i wieku pozycji.
     * ----------------------------------------------------- */

    if (sys.gpsFixValid) {
        sample.lat = coordinateToInt(sys.lat);
        sample.lon = coordinateToInt(sys.lon);
    } else {
        sample.lat = 0;
        sample.lon = 0;
    }

    sample.speedCms = floatToUint16(
        sys.speedKmh,
        100.0f / 3.6f
    );

    sample.cogDeg10 = angleToDeg10(sys.cogDeg);

    sample.satellites = sys.satellites;

    sample.hdop100 = floatToUint16(
        sys.hdop,
        100.0f
    );

    sample.gpsAge100ms = gpsAgeTo100ms(
        sys.gpsAgeMs
    );


    /* -----------------------------------------------------
     * INA226
     * ----------------------------------------------------- */

    sample.voltageMv = floatToUint16(
        sys.voltageV,
        1000.0f
    );

    sample.currentMa = floatToInt16(
        sys.currentA,
        1000.0f
    );


    /* -----------------------------------------------------
     * BNO055
     * ----------------------------------------------------- */

    sample.headingDeg10 = angleToDeg10(
        sys.headingDeg
    );


    /* -----------------------------------------------------
     * Rzeczywiste sygnały wyjściowe
     * ----------------------------------------------------- */

    sample.pwmThrottle = sys.escOutputUs;
    sample.pwmRudder   = sys.servoOutputUs;


    /* -----------------------------------------------------
     * Pole zarezerwowane
     * ----------------------------------------------------- */

    sample.reserved = 0;


    /* -----------------------------------------------------
     * Zatwierdzenie próbki
     * ----------------------------------------------------- */

    sampleCount++;


    /* -----------------------------------------------------
     * Ostatnia próbka zapełniła bufor
     * ----------------------------------------------------- */

    if (sampleCount >= MAX_SAMPLES) {
        setState(STATE_ARMED);
    }
}


/* =========================================================
 * ZRZUT METADANYCH
 * ========================================================= */

static void dumpMetadata(Print& out) {
    out.print(F("# firmware="));
    out.print(FW_NAME);
    out.print(',');
    out.println(FW_VERSION);

    out.print(F("# propeller="));

    if (sys.propellerName[0] != '\0') {
        out.println(sys.propellerName);
    } else {
        out.println('-');
    }

    out.print(F("# pwm_limit_us="));
    out.println(sys.maxThrottlePwm);

    out.print(F("# note="));

    if (sys.testNote[0] != '\0') {
        out.println(sys.testNote);
    } else {
        out.println('-');
    }

    out.print(F("# samples="));
    out.println(sampleCount);

    out.print(F("# sample_size_bytes="));
    out.println(sizeof(LogSample));

    out.print(F("# buffer_capacity="));
    out.println(MAX_SAMPLES);
}


/* =========================================================
 * ZRZUT POJEDYNCZEJ PRÓBKI CSV
 * ========================================================= */

static void dumpSample(Print& out, const LogSample& sample) {
    out.print(sample.time); out.print(',');
    out.print(sample.glat); out.print(',');
    out.print(sample.glon); out.print(',');
    out.print(sample.vbat); out.print(',');
    out.print(sample.ibat); out.print(',');
    out.print(sample.gsog); out.print(',');
    out.print(sample.head); out.print(',');
    out.print(sample.gcog); out.print(',');
    out.print(sample.thro); out.print(',');
    out.print(sample.rudd); out.print(',');
    out.print(sample.hdop); out.print(',');
    out.print(sample.gsat); out.print(',');
    out.println(sample.gage);
}

/* =========================================================
 * ZRZUT CAŁEGO LOGU
 *
 * Obsługuje zarówno Serial, jak i BluetoothSerial.
 * ========================================================= */

void dumpLog(Print& out) {
    out.println(F("# BOAT_WATER_TEST"));

    dumpMetadata(out);

    out.println(F("time,glat,glon,vbat,ibat,gsog,head,gcog,thro,rudd,hdop,gsat,gage"));

    for (uint16_t index = 0;
         index < sampleCount;
         index++) {

        dumpSample(
            out,
            logBuffer[index]
        );
    }

    out.println(F("# END"));
}
