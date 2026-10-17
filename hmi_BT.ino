#include "declarations.h"


/* =========================================================
 * OBIEKT ZDEFINIOWANY W Boat_WaterTest.ino
 * ========================================================= */

extern BluetoothSerial SerialBT;


/* =========================================================
 * BUFOR ODBIORCZY
 * ========================================================= */

static char    btRxBuffer[64];
static uint8_t btRxIndex = 0;


/* =========================================================
 * FUNKCJE POMOCNICZE
 * ========================================================= */

static const char* stateName(BoatState state) {
    switch (state) {
        case STATE_BOOT:
            return "BOOT";

        case STATE_DISARMED:
            return "DISARMED";

        case STATE_ARMED:
            return "ARMED";

        case STATE_RECORDING:
            return "RECORDING";

        case STATE_FAILSAFE:
            return "FAILSAFE";

        case STATE_BT_DUMP:
            return "BT_DUMP";

        default:
            return "UNKNOWN";
    }
}


static const char* failsafeName(FailsafeReason reason) {
    switch (reason) {
        case FAILSAFE_NONE:
            return "NONE";

        case FAILSAFE_RC_LOST:
            return "RC_LOST";

        case FAILSAFE_VOLTAGE_LOW:
            return "VOLTAGE_LOW";

        case FAILSAFE_OVERCURRENT:
            return "OVERCURRENT";

        case FAILSAFE_OVERPOWER:
            return "OVERPOWER";

        default:
            return "UNKNOWN";
    }
}


static bool isUnsignedNumber(const char* text) {
    if (text == nullptr || *text == '\0') {
        return false;
    }

    while (*text != '\0') {
        if (!isdigit((unsigned char)*text)) {
            return false;
        }

        text++;
    }

    return true;
}


/* =========================================================
 * STATUS SYSTEMU
 * ========================================================= */

static void printStatus(Print& out) {
    out.println(F("STATUS"));

    out.print(F("STATE="));
    out.println(stateName(sys.state));

    out.print(F("FAILSAFE="));
    out.println(failsafeName(sys.failsafeReason));

    out.print(F("RC="));
    out.println(sys.linkUp ? F("OK") : F("LOST"));

    out.print(F("RSSI_DBM="));
    out.println(sys.rssiDbm);

    out.print(F("LQ_PERCENT="));
    out.println(sys.lqPercent);

    out.print(F("CH1_STEER_US="));
    out.println(sys.rcSteerUs);

    out.print(F("CH3_THRO_US="));
    out.println(sys.rcThroUs);

    out.print(F("CH6_SB_US="));
    out.println(sys.rcSbUs);

    out.print(F("CH7_SC_US="));
    out.println(sys.rcScUs);

    out.print(F("NO_LIMITS="));
    out.println(sys.rcNoLimits ? F("YES") : F("NO"));

    out.print(F("GPS_FIX="));
    out.println(sys.gpsFixValid ? F("YES") : F("NO"));

    out.print(F("GPS_LAT="));
    out.println(sys.lat, 6);

    out.print(F("GPS_LON="));
    out.println(sys.lon, 6);

    out.print(F("GPS_SPEED_KMH="));
    out.println(sys.speedKmh, 2);

    out.print(F("GPS_COG_DEG="));
    out.println(sys.cogDeg, 1);

    out.print(F("GPS_SATELLITES="));
    out.println(sys.satellites);

    out.print(F("GPS_HDOP="));
    out.println(sys.hdop, 2);

    out.print(F("GPS_AGE_MS="));

    if (sys.gpsAgeMs == UINT32_MAX) {
        out.println(F("INVALID"));
    } else {
        out.println(sys.gpsAgeMs);
    }

    out.print(F("MAG_OK="));
    out.println(sys.imuHealthy ? F("YES") : F("NO"));

    out.print(F("HEADING_DEG="));
    out.println(sys.headingDeg, 1);

    out.print(F("VBAT_V="));
    out.println(sys.voltageV, 2);

    out.print(F("CURRENT_A="));
    out.println(sys.currentA, 2);

    out.print(F("POWER_W="));
    out.println(sys.powerW, 1);

    out.print(F("CURRENT_WARNING="));
    out.println(sys.currentWarning ? F("YES") : F("NO"));

    out.print(F("POWER_WARNING="));
    out.println(sys.powerWarning ? F("YES") : F("NO"));

    out.print(F("ESC_OUTPUT_US="));
    out.println(sys.escOutputUs);

    out.print(F("SERVO_OUTPUT_US="));
    out.println(sys.servoOutputUs);

    out.print(F("PWM_LIMIT_US="));
    out.println(sys.maxThrottlePwm);

    out.print(F("PROP="));
    out.println(
        sys.propellerName[0] != '\0'
            ? sys.propellerName
            : "-"
    );

    out.print(F("NOTE="));
    out.println(
        sys.testNote[0] != '\0'
            ? sys.testNote
            : "-"
    );

    out.print(F("SAMPLES="));
    out.print(sampleCount);
    out.print('/');
    out.println(MAX_SAMPLES);
}


/* =========================================================
 * LISTA KOMEND
 * ========================================================= */

static void printHelp(Print& out) {
    out.println(F("COMMANDS:"));
    out.println(F("BTDUMP"));
    out.println(F("STATUS"));
    out.println(F("PWM1450"));
    out.println(F("PROP65"));
    out.println(F("NOTEtext"));
    out.println(F("DUMP"));
    out.println(F("CLEAR"));
    out.println(F("EXIT"));
    out.println(F("HELP"));
}


/* =========================================================
 * INICJALIZACJA BLUETOOTH
 * ========================================================= */

void initBT() {
    sys.btActive    = false;
    sys.btConnected = false;

    btRxIndex = 0;

    sys.maxThrottlePwm = ESC_MAX_US;
    sys.propellerName[0] = '\0';
    sys.testNote[0]      = '\0';
}


/* =========================================================
 * URUCHOMIENIE BLUETOOTH
 * ========================================================= */

void startBT() {
    if (sys.btActive) {
        return;
    }

    if (SerialBT.begin(BT_DEVICE_NAME)) {
        sys.btActive = true;
        btRxIndex    = 0;

        Serial.println("[BT] Bluetooth started");
    } else {
        sys.btActive = false;
        Serial.println("[BT] Bluetooth start failed");
    }
}


/* =========================================================
 * WYŁĄCZENIE BLUETOOTH
 * ========================================================= */

void stopBT() {
    if (!sys.btActive) {
        return;
    }

    SerialBT.end();

    sys.btActive    = false;
    sys.btConnected = false;

    btRxIndex = 0;

    Serial.println("[BT] Bluetooth stopped");
}


/* =========================================================
 * KOMENDA PWM
 *
 * Składnia:
 * PWM1450
 * ========================================================= */

static void handlePwmCommand(const char* valueText) {
    if (!isUnsignedNumber(valueText)) {
        SerialBT.println(F("ERR: INVALID PWM"));
        return;
    }

    const long value = strtol(valueText, nullptr, 10);

    if (value < ESC_DEADZONE_US || value > ESC_MAX_US) {
        SerialBT.print(F("ERR: PWM RANGE "));
        SerialBT.print(ESC_DEADZONE_US);
        SerialBT.print('-');
        SerialBT.println(ESC_MAX_US);
        return;
    }

    sys.maxThrottlePwm = (uint16_t)value;

    SerialBT.print(F("OK: PWM="));
    SerialBT.println(sys.maxThrottlePwm);
}


/* =========================================================
 * KOMENDA PROP
 *
 * Składnia:
 * PROP65
 * ========================================================= */

static void handlePropCommand(const char* valueText) {
    if (valueText == nullptr || *valueText == '\0') {
        SerialBT.println(F("ERR: EMPTY PROP"));
        return;
    }

    if (strlen(valueText) >= sizeof(sys.propellerName)) {
        SerialBT.println(F("ERR: PROP TOO LONG"));
        return;
    }

    strncpy(
        sys.propellerName,
        valueText,
        sizeof(sys.propellerName)
    );

    sys.propellerName[
        sizeof(sys.propellerName) - 1
    ] = '\0';

 
