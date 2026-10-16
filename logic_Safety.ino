#include "declarations.h"

// Licznik czasu trwania stanu przeciążenia krytycznego (I > 16A lub P > 330W)
static uint32_t overloadStartTimeMs = 0;
static bool     isOverloadActive    = false;

/* =========================================================
 * GŁÓWNA AKTUALIZACJA BEZPIECZEŃSTWA (Wywoływana w schedulerze)
 * ========================================================= */

void updateSafety() {
    uint32_t now = millis();

    // W stanach BOOT, FAILSAFE i BT_DUMP nie wyzwalamy ponownego Failsafe
    if (sys.state == STATE_BOOT || sys.state == STATE_FAILSAFE || sys.state == STATE_BT_DUMP) {
        isOverloadActive = false;
        return;
    }

    // -----------------------------------------------------
    // 1. Sprawdzenie łącza radiowego RC (CRSF / ELRS)
    // -----------------------------------------------------
    if (!sys.rcLinkUp) {
        sys.failsafeReason = FAILSAFE_RC_LOST;
        setState(STATE_FAILSAFE);
        return;
    }

    // -----------------------------------------------------
    // 2. Nadzór napięcia baterii (LiPo 6S)
    // -----------------------------------------------------
    // Sprawdzamy tylko, gdy bateria jest fizycznie podłączona (Vbat > 5.0 V)
    if (sys.vbat > 5.0f) {
        // Poziom ostrzegawczy (< 19.8 V)
        sys.batteryLow = (sys.vbat < VBAT_WARN_V);

        // Poziom krytyczny / odcięcie (< 19.2 V)
        if (sys.vbat < VBAT_CUTOFF_V) {
            sys.failsafeReason = FAILSAFE_BATTERY_LOW;
            setState(STATE_FAILSAFE);
            return;
        }
    } else {
        sys.batteryLow = false;
    }

    // -----------------------------------------------------
    // 3. Nadzór przeciążenia prądu i mocy (INA226)
    // -----------------------------------------------------
    // Poziom ostrzegawczy (> 13 A lub > 280 W)
    sys.powerOverload = (sys.currentA > CURRENT_WARN_A || sys.powerW > POWER_WARN_W);

    // Poziom krytyczny (> 16 A lub > 330 W)
    bool criticalCurrent = (sys.currentA > CURRENT_FAIL_A);
    bool criticalPower   = (sys.powerW > POWER_FAIL_W);

    if (criticalCurrent || criticalPower) {
        if (!isOverloadActive) {
            // Początek zdarzenia przeciążeniowego
            overloadStartTimeMs = now;
            isOverloadActive    = true;
        } else if (now - overloadStartTimeMs >= OVERLOAD_TIME_MS) {
            // Przekroczenie limitu trwające nieprzerwanie dłużej niż 250 ms
            if (criticalCurrent) {
                sys.failsafeReason = FAILSAFE_OVERCURRENT;
            } else {
                sys.failsafeReason = FAILSAFE_OVERPOWER;
            }
            setState(STATE_FAILSAFE);
            return;
        }
    } else {
        // Reset licznika czasu, jeśli parametry wróciły do normy przed upływem 250 ms
        isOverloadActive = false;
    }
}

/* =========================================================
 * FUNKCJE POMOCNICZE
 * ========================================================= */

bool isBatteryLow() {
    return sys.batteryLow;
}

bool isPowerOverload() {
    return sys.powerOverload;
}