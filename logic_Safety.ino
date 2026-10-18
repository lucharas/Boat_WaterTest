// ============================================================================
// PLIK: logic_Safety.ino (Pełny, naprawiony plik)
// ============================================================================
#include "declarations.h"

static bool     overloadTimerActive = false;
static uint32_t overloadStartMs     = 0;

static bool isVoltageMeasurementValid() {
    return isfinite(sys.voltageV) && sys.voltageV > 5.0f;
}

bool isBatteryLow() {
    return isVoltageMeasurementValid() && sys.voltageV < VBAT_WARN_V;
}

bool isPowerOverload() {
    return sys.currentWarning || sys.powerWarning;
}

static void resetOverloadTimer() {
    overloadTimerActive = false;
    overloadStartMs     = 0;
}

static FailsafeReason checkCriticalOverload(uint32_t now) {
    const bool criticalCurrent = isfinite(sys.currentA) && sys.currentA > CURRENT_FAIL_A;
    const bool criticalPower   = isfinite(sys.powerW) && sys.powerW > POWER_FAIL_W;

    if (!criticalCurrent && !criticalPower) {
        resetOverloadTimer();
        return FAILSAFE_NONE;
    }

    if (!overloadTimerActive) {
        overloadTimerActive = true;
        overloadStartMs     = now;
        return FAILSAFE_NONE;
    }

    if (now - overloadStartMs < OVERLOAD_TIME_MS) {
        return FAILSAFE_NONE;
    }

    if (criticalCurrent) {
        return FAILSAFE_OVERCURRENT;
    }

    return FAILSAFE_OVERPOWER;
}

void updateSafety() {
    const uint32_t now = millis();

    sys.currentWarning = isfinite(sys.currentA) && sys.currentA > CURRENT_WARN_A;
    sys.powerWarning   = isfinite(sys.powerW) && sys.powerW > POWER_WARN_W;

    // Ignorujemy sprawdzanie RC i przeciążeń w BOOT oraz DISARMED
    if (sys.state == STATE_BOOT || sys.state == STATE_DISARMED) {
        sys.failsafeReason = FAILSAFE_NONE;
        resetOverloadTimer();
        return;
    }

    if (!sys.linkUp) {
        sys.failsafeReason = FAILSAFE_RC_LOST;
        resetOverloadTimer();
        return;
    }

    // Niskie napięcie powoduje jedynie ostrzeżenie (funkcja isBatteryLow)
    // Nie wyzwalamy na jego podstawie FailSafe.

    FailsafeReason overloadReason = checkCriticalOverload(now);
    if (overloadReason != FAILSAFE_NONE) {
        sys.failsafeReason = overloadReason;
        return;
    }

    sys.failsafeReason = FAILSAFE_NONE;
}
