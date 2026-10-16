#include "declarations.h"

extern BluetoothSerial SerialBT;

static char btRxBuffer[64];
static uint8_t btRxIndex = 0;

/* =========================================================
 * ZARZĄDZANIE RADIEM BLUETOOTH
 * ========================================================= */

void initBT() {
    sys.btActive    = false;
    sys.btConnected = false;
    btRxIndex       = 0;
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
        sys.btActive    = false;
        sys.btConnected = false;
        btRxIndex       = 0;
    }
}

/* =========================================================
 * PARSOWANIE KOMEND TEKSTOWYCH ASCII
 * ========================================================= */

static void parseBtCommand(char* cmd) {
    // Usunięcie ewentualnych białych znaków z początku
    while (*cmd == ' ' || *cmd == '\t') cmd++;

    // 1. Zmiana limitu PWM w formacie "PWM80" lub "pwm80" (wartość w procentach 0-100%)
    if (strncasecmp(cmd, "PWM", 3) == 0 && isdigit(cmd[3])) {
        int percent = atoi(&cmd[3]);
        percent = constrain(percent, 0, 100);

        // Przeliczenie 0-100% na zakres roboczy [ESC_DEADZONE_US .. ESC_MAX_US]
        sys.maxThrottlePwm = ESC_DEADZONE_US + ((ESC_MAX_US - ESC_DEADZONE_US) * percent) / 100;
        snprintf(sys.maxPwmStr, sizeof(sys.maxPwmStr), "pwm%d", percent);

        SerialBT.print(F("OK: MAX PWM set to "));
        SerialBT.print(sys.maxThrottlePwm);
        SerialBT.print(F(" us ("));
        SerialBT.print(sys.maxPwmStr);
        SerialBT.println(F(")"));
        return;
    }

    // 2. Zmiana limitu PWM w formacie "PWM <us>" (np. "PWM 1450" lub "SET PWM 1450")
    if (strncasecmp(cmd, "PWM ", 4) == 0 || strncasecmp(cmd, "SET PWM ", 8) == 0) {
        char* valStr = strchr(cmd, ' ');
        if (strncasecmp(cmd, "SET PWM ", 8) == 0) {
            valStr = strchr(valStr + 1, ' ');
        }

        if (valStr) {
            int val = atoi(valStr + 1);
            if (val >= ESC_DEADZONE_US && val <= ESC_MAX_US) {
                sys.maxThrottlePwm = (uint16_t)val;
                snprintf(sys.maxPwmStr, sizeof(sys.maxPwmStr), "%d", val);
                SerialBT.print(F("OK: MAX PWM set to "));
                SerialBT.print(sys.maxThrottlePwm);
                SerialBT.println(F(" us"));
            } else {
                SerialBT.println(F("ERR: PWM out of range (1180 - 2000 us)"));
            }
        }
        return;
    }

    // 3. SET PROP <nazwa_śruby>
    if (strncasecmp(cmd, "SET PROP ", 9) == 0) {
        strncpy(sys.propellerName, cmd + 9, sizeof(sys.propellerName) - 1);
        sys.propellerName[sizeof(sys.propellerName) - 1] = '\0';
        SerialBT.print(F("OK: Propeller set to "));
        SerialBT.println(sys.propellerName);
        return;
    }

    // 4. SET NOTE <notatka>
    if (strncasecmp(cmd, "SET NOTE ", 9) == 0) {
        strncpy(sys.testNote, cmd + 9, sizeof(sys.testNote) - 1);
        sys.testNote[sizeof(sys.testNote) - 1] = '\0';
        SerialBT.print(F("OK: Note set to "));
        SerialBT.println(sys.testNote);
        return;
    }

    // 5. STATUS
    if (strcasecmp(cmd, "STATUS") == 0) {
        SerialBT.println(F("--- SYSTEM STATUS ---"));
        SerialBT.print(F("State: "));       SerialBT.println((uint8_t)sys.state);
        SerialBT.print(F("Vbat: "));        SerialBT.print(sys.vbat, 2); SerialBT.println(F(" V"));
        SerialBT.print(F("Current: "));     SerialBT.print(sys.currentA, 2); SerialBT.println(F(" A"));
        SerialBT.print(F("Power: "));       SerialBT.print(sys.powerW, 1); SerialBT.println(F(" W"));
        SerialBT.print(F("GPS Fix: "));     SerialBT.println(sys.gpsFix ? "YES" : "NO");
        SerialBT.print(F("Sats: "));        SerialBT.println(sys.gpsSats);
        SerialBT.print(F("Speed: "));       SerialBT.print(sys.gpsSpeedMs, 2); SerialBT.println(F(" m/s"));
        SerialBT.print(F("Max Thro PWM: "));SerialBT.print(sys.maxThrottlePwm);
        SerialBT.print(F(" us ("));         SerialBT.print(sys.maxPwmStr); SerialBT.println(F(")"));
        SerialBT.print(F("Samples in RAM: ")); SerialBT.print(sampleCount);
        SerialBT.print(F(" / "));           SerialBT.println(MAX_SAMPLES);
        return;
    }

    // 6. DUMP
    if (strcasecmp(cmd, "DUMP") == 0) {
        setState(STATE_BT_DUMP);
        dumpLog(SerialBT);
        setState(STATE_DISARMED);
        return;
    }

    // 7. CLEAR
    if (strcasecmp(cmd, "CLEAR") == 0) {
        clearLog();
        SerialBT.println(F("OK: Log buffer cleared."));
        return;
    }

    SerialBT.println(F("ERR: Unknown command"));
}

/* =========================================================
 * GŁÓWNA PĘTLA OBSŁUGI BLUETOOTH
 * ========================================================= */

void updateBT() {
    if (!sys.btActive) {
        return;
    }

    sys.btConnected = SerialBT.hasClient();

    while (SerialBT.available() > 0) {
        char c = (char)SerialBT.read();

        if (c == '\r' || c == '\n') {
            if (btRxIndex > 0) {
                btRxBuffer[btRxIndex] = '\0';
                parseBtCommand(btRxBuffer);
                btRxIndex = 0;
            }
        } else {
            if (btRxIndex < sizeof(btRxBuffer) - 1) {
                btRxBuffer[btRxIndex++] = c;
            }
        }
    }
}