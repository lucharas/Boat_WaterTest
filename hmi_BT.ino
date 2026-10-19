// ============================================================================
// PLIK: hmi_BT.ino
// ============================================================================
#include "declarations.h"

extern BluetoothSerial SerialBT;

static char    btRxBuffer[64];
static uint8_t btRxIndex = 0;

static const char* stateName(BoatState s) {
    switch (s) {
        case STATE_BOOT:      return "BOOT";
        case STATE_DISARMED:  return "DISARMED";
        case STATE_ARMED:     return "ARMED";
        case STATE_RECORDING: return "RECORDING";
        case STATE_FAILSAFE:  return "FAILSAFE";
        case STATE_BT_DUMP:   return "BT_DUMP";
        default:              return "UNKNOWN";
    }
}

static const char* failsafeName(FailsafeReason r) {
    switch (r) {
        case FAILSAFE_NONE:        return "NONE";
        case FAILSAFE_RC_LOST:     return "RC_LOST";
        case FAILSAFE_VOLTAGE_LOW: return "VOLTAGE_LOW";
        case FAILSAFE_OVERCURRENT: return "OVERCURRENT";
        case FAILSAFE_OVERPOWER:   return "OVERPOWER";
        default:                   return "UNKNOWN";
    }
}

static bool isUnsignedNumber(const char* t) {
    if (!t || !*t) return false;
    while (*t) { if (!isdigit((unsigned char)*t++)) return false; }
    return true;
}

static void printStatus(Print& out) {
    out.println(F("STATUS"));
    out.printf("STATE=%s\n", stateName(sys.state));
    out.printf("FAILSAFE=%s\n", failsafeName(sys.failsafeReason));
    out.printf("RC=%s\n", sys.linkUp ? "OK" : "LOST");
    out.printf("RSSI_DBM=%d\nLQ_PERCENT=%u\n", sys.rssiDbm, sys.lqPercent);
    out.printf("CH1_STEER_US=%u\nCH3_THRO_US=%u\nCH6_SB_US=%u\nCH7_SC_US=%u\n", sys.rcSteerUs, sys.rcThroUs, sys.rcSbUs, sys.rcScUs);
    out.printf("NO_LIMITS=%s\n", sys.rcNoLimits ? "YES" : "NO");
    out.printf("GPS_FIX=%s\nGPS_LAT=%.6f\nGPS_LON=%.6f\n", sys.gpsFixValid ? "YES" : "NO", sys.lat, sys.lon);
    out.printf("GPS_SPEED_KMH=%.2f\nGPS_COG_DEG=%.1f\nGPS_SATELLITES=%u\nGPS_HDOP=%.2f\n", sys.speedKmh, sys.cogDeg, sys.satellites, sys.hdop);
    if (sys.gpsAgeMs == UINT32_MAX) out.println(F("GPS_AGE_MS=INVALID"));
    else out.printf("GPS_AGE_MS=%u\n", sys.gpsAgeMs);
    out.printf("MAG_OK=%s\nHEADING_DEG=%.1f\n", sys.imuHealthy ? "YES" : "NO", sys.headingDeg);
    out.printf("VBAT_V=%.2f\nCURRENT_A=%.2f\nPOWER_W=%.1f\n", sys.voltageV, sys.currentA, sys.powerW);
    out.printf("CURRENT_WARNING=%s\nPOWER_WARNING=%s\n", sys.currentWarning ? "YES" : "NO", sys.powerWarning ? "YES" : "NO");
    out.printf("ESC_OUTPUT_US=%u\nSERVO_OUTPUT_US=%u\nPWM_LIMIT_US=%u\n", sys.escOutputUs, sys.servoOutputUs, sys.maxThrottlePwm);
    out.printf("PROP=%s\n", sys.propellerName[0] ? sys.propellerName : "-");
    out.printf("NOTE=%s\n", sys.testNote[0] ? sys.testNote : "-");
    out.printf("SAMPLES=%u/%u\n", sampleCount, MAX_SAMPLES);
}

static void printHelp(Print& out) {
    out.println(F("COMMANDS:\nBTDUMP\nSTATUS\nPWM1450\nPROP65\nNOTEtext\nDUMP\nCLEAR\nEXIT\nHELP"));
}

void initBT() {
    sys.btActive = sys.btConnected = false;
    btRxIndex = 0;
    sys.maxThrottlePwm = ESC_MAX_US;
    sys.propellerName[0] = sys.testNote[0] = '\0';
}

void startBT() {
    if (!sys.btActive) {
        SerialBT.begin(BT_DEVICE_NAME);
        sys.btActive = true;
    }
}

void stopBT() {
    if (sys.btActive) {
        SerialBT.end();
        sys.btActive = false;
        sys.btConnected = false;
    }
}

static void processBtCommand(const char* cmd) {
    String c = String(cmd);
    c.trim();
    if (c.length() == 0) return;
    
    String cUpper = c;
    cUpper.toUpperCase();

    if (cUpper == "BTDUMP") {
        if (sys.state == STATE_DISARMED) {
            setState(STATE_BT_DUMP);
            SerialBT.println(F("OK BTDUMP"));
        } else {
            SerialBT.println(F("ERR NOT DISARMED"));
        }
    }
    else if (cUpper == "EXIT") {
        if (sys.state == STATE_BT_DUMP) {
            setState(STATE_DISARMED);
            SerialBT.println(F("OK EXIT"));
        } else {
            SerialBT.println(F("ERR NOT BT_DUMP"));
        }
    }
    else if (cUpper == "STATUS") {
        printStatus(SerialBT);
    }
    else if (cUpper == "DUMP") {
        dumpLog(SerialBT);
    }
    else if (cUpper == "CLEAR") {
        clearLog();
        SerialBT.println(F("OK CLEAR"));
    }
    else if (cUpper == "HELP") {
        printHelp(SerialBT);
    }
    else if (cUpper.startsWith("PWM")) {
        String valStr = cUpper.substring(3);
        if (isUnsignedNumber(valStr.c_str())) {
            uint16_t val = (uint16_t)valStr.toInt();
            if (val >= ESC_DEADZONE_US && val <= ESC_MAX_US) {
                sys.maxThrottlePwm = val;
                SerialBT.printf("OK PWM=%u\n", val);
            } else {
                SerialBT.println(F("ERR OUT OF RANGE"));
            }
        } else {
            SerialBT.println(F("ERR INVALID PWM"));
        }
    }
    else if (cUpper.startsWith("PROP")) {
        // Wycinamy za pomocą oryginalnego ciągu (c), żeby zachować wielkość liter
        String valStr = c.substring(4);
        strncpy(sys.propellerName, valStr.c_str(), sizeof(sys.propellerName) - 1);
        sys.propellerName[sizeof(sys.propellerName) - 1] = '\0';
        SerialBT.printf("OK PROP=%s\n", sys.propellerName);
    }
    else if (cUpper.startsWith("NOTE")) {
        // Zachowanie oryginalnej wielkości liter notatki
        String valStr = c.substring(4);
        strncpy(sys.testNote, valStr.c_str(), sizeof(sys.testNote) - 1);
        sys.testNote[sizeof(sys.testNote) - 1] = '\0';
        SerialBT.printf("OK NOTE=%s\n", sys.testNote);
    }
    else {
        SerialBT.println(F("ERR UNKNOWN CMD"));
    }
}

void updateBT() {
    if (!sys.btActive) return;
    
    sys.btConnected = SerialBT.hasClient();
    
    while (SerialBT.available() > 0) {
        char c = SerialBT.read();
        // Oczekujemy końca linii (CR lub LF)
        if (c == '\n' || c == '\r') {
            if (btRxIndex > 0) {
                btRxBuffer[btRxIndex] = '\0';
                processBtCommand(btRxBuffer);
                btRxIndex = 0;
            }
        } else {
            if (btRxIndex < sizeof(btRxBuffer) - 1) {
                btRxBuffer[btRxIndex++] = c;
            }
        }
    }
}