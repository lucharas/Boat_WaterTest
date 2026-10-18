// ============================================================================
// PLIK: log_data.ino
// ============================================================================
#include "declarations.h"
#include <math.h>

static uint16_t floatToUint16(float v, float m) { 
    return (isfinite(v) && v > 0.0f) ? (v * m >= 65535.0f ? 65535 : (uint16_t)lroundf(v * m)) : 0; 
}
static int16_t floatToInt16(float v, float m) { 
    return isfinite(v) ? (v * m <= -32768.0f ? -32768 : (v * m >= 32767.0f ? 32767 : (int16_t)lroundf(v * m))) : 0; 
}
static uint16_t angleToDeg10(float a) { 
    if (!isfinite(a)) return 0; 
    while (a < 0.0f) a += 360.0f; 
    while (a >= 360.0f) a -= 360.0f; 
    return (uint16_t)lroundf(a * 10.0f); 
}
static int32_t coordinateToInt(double c) { 
    return isfinite(c) ? (c * 1000000.0 <= -2147483648.0 ? INT32_MIN : (c * 1000000.0 >= 2147483647.0 ? INT32_MAX : (int32_t)llround(c * 1000000.0))) : 0; 
}
static uint8_t gpsAgeTo100ms(uint32_t age) { 
    return (age == UINT32_MAX) ? 255 : (age / 100UL > 255UL ? 255 : (uint8_t)(age / 100UL)); 
}

void initLog() { sampleCount = 0; }
void clearLog() { sampleCount = 0; }

void addSample() {
    if (sys.state != STATE_RECORDING || sampleCount >= MAX_SAMPLES) {
        if (sampleCount >= MAX_SAMPLES) setState(STATE_ARMED);
        return;
    }

    LogSample& s = logBuffer[sampleCount];
    s.time = millis();
    s.glat = sys.gpsFixValid ? coordinateToInt(sys.lat) : 0;
    s.glon = sys.gpsFixValid ? coordinateToInt(sys.lon) : 0;
    s.gsog = floatToUint16(sys.speedKmh, 100.0f / 3.6f);
    s.gcog = angleToDeg10(sys.cogDeg);
    s.gsat = sys.satellites;
    s.hdop = floatToUint16(sys.hdop, 100.0f);
    s.gage = gpsAgeTo100ms(sys.gpsAgeMs);
    s.vbat = floatToUint16(sys.voltageV, 1000.0f);
    s.ibat = floatToInt16(sys.currentA, 1000.0f);
    s.head = angleToDeg10(sys.headingDeg);
    s.thro = sys.escOutputUs;
    s.rudd = sys.servoOutputUs;
    s.rsvd = 0;

    if (++sampleCount >= MAX_SAMPLES) {
        setState(STATE_ARMED);
    }
}

static void dumpMetadata(Print& out) {
    out.printf("# firmware=%s,%s\n", FW_NAME, FW_VERSION);
    out.printf("# propeller=%s\n", sys.propellerName[0] != '\0' ? sys.propellerName : "-");
    out.printf("# pwm_limit_us=%u\n", sys.maxThrottlePwm);
    out.printf("# note=%s\n", sys.testNote[0] != '\0' ? sys.testNote : "-");
    out.printf("# samples=%u\n", sampleCount);
    out.printf("# sample_size_bytes=%u\n", sizeof(LogSample));
    out.printf("# buffer_capacity=%u\n", MAX_SAMPLES);
}

void dumpLog(Print& out) {
    out.println(F("# BOAT_WATER_TEST"));
    dumpMetadata(out);
    out.println(F("time,glat,glon,vbat,ibat,gsog,head,gcog,thro,rudd,hdop,gsat,gage,rsvd"));
    
    for (uint16_t i = 0; i < sampleCount; i++) {
        const LogSample& s = logBuffer[i];
        out.printf("%u,%d,%d,%u,%d,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
            s.time, s.glat, s.glon, s.vbat, s.ibat, s.gsog, s.head,
            s.gcog, s.thro, s.rudd, s.hdop, s.gsat, s.gage, s.rsvd
        );
    }
    out.println(F("# END"));
}
