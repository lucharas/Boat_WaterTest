#include "declarations.h"

/* =========================================================
 * INICJALIZACJA BUFORA LOGOWANIA
 * ========================================================= */

void initLog() {
    clearLog();
}

/* =========================================================
 * CZYSZCZENIE BUFORA (Reset licznika próbek)
 * ========================================================= */

void clearLog() {
    sampleCount = 0;
}

/* =========================================================
 * ZAPIS PRÓBKI DO STATYCZNEJ TABLICY W RAM (Częstotliwość: 5 Hz)
 * ========================================================= */

void addSample() {
    // Rejestrujemy wyłącznie w trakcie aktywnego rejsu w stanie RECORDING
    if (sys.state != STATE_RECORDING) {
        return;
    }

    // Zabezpieczenie przed przepełnieniem statycznego bufora RAM (3000 próbek)
    if (sampleCount >= MAX_SAMPLES) {
        return;
    }

    LogSample& sample = logBuffer[sampleCount];

    // Czas systemowy w ms
    sample.timestampMs = millis();

    // Pozycja geograficzna (przeskalowana do int32_t 1e-6)
    sample.lat = sys.gpsLat;
    sample.lon = sys.gpsLon;

    // Parametry zasilania (przeliczenie na jednostki całkowite bez float)
    sample.voltageMv = (uint16_t)constrain((int32_t)(sys.vbat * 1000.0f), 0, 65535);
    sample.currentMa = (int16_t)constrain((int32_t)(sys.currentA * 1000.0f), -32768, 32767);
    sample.powerMw   = (uint16_t)constrain((int32_t)(sys.powerW * 10.0f), 0, 65535);

    // Hydrodynamika i nawigacja
    // Prędkość: m/s -> cm/s (1 m/s = 100 cm/s; rozdzielczość 0.01 m/s)
    sample.speedCms     = (uint16_t)constrain((int32_t)(sys.gpsSpeedMs * 100.0f), 0, 65535);
    sample.headingDeg10 = (uint16_t)constrain((int32_t)(sys.yaw * 10.0f), 0, 3599);
    sample.cogDeg10     = (uint16_t)constrain((int32_t)(sys.gpsCourse * 10.0f), 0, 3599);

    // Nastawy sygnałów PWM (1000 - 2000 us)
    sample.pwmThrottle = sys.escOutputUs;
    sample.pwmRudder   = sys.servoOutputUs;

    // Satelity i flagi stanu
    sample.satellites = sys.gpsSats;
    
    // Maska bitowa: bit 0..2 = FSM state, bit 3 = GPS fix, bit 4 = Low Bat, bit 5 = Overload
    uint8_t flags = ((uint8_t)sys.state & 0x07);
    if (sys.gpsFix)        flags |= (1 << 3);
    if (sys.batteryLow)    flags |= (1 << 4);
    if (sys.powerOverload) flags |= (1 << 5);
    sample.stateFlags = flags;

    sampleCount++;
}

/* =========================================================
 * ZRZUT DANYCH W FORMACIE CSV (Bluetooth / Serial)
 * ========================================================= */

void dumpLog(Print& stream) {
    // 1. Metadane testu
    stream.println(F("# === BOAT WATER TEST LOG DUMP ==="));
    stream.print(F("# Firmware: ")); stream.println(FW_NAME " " FW_VERSION);
    stream.print(F("# Propeller: ")); stream.println(sys.propellerName[0] ? sys.propellerName : "DEFAULT");
    stream.print(F("# Note: ")); stream.println(sys.testNote[0] ? sys.testNote : "NONE");
    stream.print(F("# Total Samples: ")); stream.println(sampleCount);

    // 2. Nagłówki kolumn
    stream.println(F("time_ms,lat,lon,vbat_v,current_a,power_w,speed_mps,yaw_deg,cog_deg,pwm_thro,pwm_rudder,sats,flags"));

    // 3. Wypisanie rekordów z pamięci RAM
    for (uint16_t i = 0; i < sampleCount; i++) {
        const LogSample& s = logBuffer[i];

        stream.print(s.timestampMs);
        stream.print(',');

        // Współrzędne GPS z precyzją 6 miejsc po przecinku
        stream.print(s.lat / 1000000.0, 6);
        stream.print(',');
        stream.print(s.lon / 1000000.0, 6);
        stream.print(',');

        // Zasilanie
        stream.print(s.voltageMv / 1000.0f, 2);
        stream.print(',');
        stream.print(s.currentMa / 1000.0f, 2);
        stream.print(',');
        stream.print(s.powerMw / 10.0f, 1);
        stream.print(',');

        // Prędkość w m/s
        stream.print(s.speedCms / 100.0f, 2);
        stream.print(',');

        // Kąty kursowe w stopniach
        stream.print(s.headingDeg10 / 10.0f, 1);
        stream.print(',');
        stream.print(s.cogDeg10 / 10.0f, 1);
        stream.print(',');

        // PWM
        stream.print(s.pwmThrottle);
        stream.print(',');
        stream.print(s.pwmRudder);
        stream.print(',');

        // Satelity i flagi
        stream.print(s.satellites);
        stream.print(',');
        stream.println(s.stateFlags);
    }

    stream.println(F("# === END OF DUMP ==="));
}