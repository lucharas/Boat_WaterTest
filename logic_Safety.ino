#include "declarations.h"


/* =========================================================
 * STAN WEWNĘTRZNY MODUŁU
 * ========================================================= */

static bool     overloadTimerActive = false;
static uint32_t overloadStartMs     = 0;


/* =========================================================
 * SPRAWDZENIE POPRAWNOŚCI POMIARU NAPIĘCIA
 *
 * Chroni przed wywołaniem failsafe przed pierwszym poprawnym
 * odczytem INA226, gdy voltageV ma wartość 0.
 * ========================================================= */

static bool isVoltageMeasurementValid() {
    return isfinite(sys.voltageV) &&
           sys.voltageV > 5.0f;
}


/* =========================================================
 * OSTRZEŻENIE NAPIĘCIOWE
 * ========================================================= */

bool isBatteryLow() {
    return isVoltageMeasurementValid() &&
           sys.voltageV < VBAT_WARN_V;
}


/* =========================================================
 * OSTRZEŻENIE PRĄDOWE LUB MOCOWE
 * ========================================================= */

bool isPowerOverload() {
    return sys.currentWarning ||
           sys.powerWarning;
}


/* =========================================================
 * RESET LICZNIKA PRZECIĄŻENIA
 * ========================================================= */

static void resetOverloadTimer() {
    overloadTimerActive = false;
    overloadStartMs     = 0;
}


/* =========================================================
 * SPRAWDZENIE PRZECIĄŻENIA KRYTYCZNEGO
 *
 * Prąd lub moc muszą przekraczać próg nieprzerwanie przez
 * OVERLOAD_TIME_MS.
 * ========================================================= */

static FailsafeReason checkCriticalOverload(
    uint32_t now
) {
    const bool criticalCurrent =
        isfinite(sys.currentA) &&
        sys.currentA > CURRENT_FAIL_A;

    const bool criticalPower =
        isfinite(sys.powerW) &&
        sys.powerW > POWER_FAIL_W;


    /* -----------------------------------------------------
     * Brak przeciążenia krytycznego
     * ----------------------------------------------------- */

    if (!criticalCurrent && !criticalPower) {
        resetOverloadTimer();
        return FAILSAFE_NONE;
    }


    /* -----------------------------------------------------
     * Początek przeciążenia
     * ----------------------------------------------------- */

    if (!overloadTimerActive) {
        overloadTimerActive = true;
        overloadStartMs     = now;

        return FAILSAFE_NONE;
    }


    /* -----------------------------------------------------
     * Przeciążenie trwa jeszcze zbyt krótko
     * ----------------------------------------------------- */

    if (now - overloadStartMs < OVERLOAD_TIME_MS) {
        return FAILSAFE_NONE;
    }


    /* -----------------------------------------------------
     * Przekroczenie czasu krytycznego
     *
     * Prąd ma pierwszeństwo, jeżeli oba limity zostały
     * przekroczone jednocześnie.
     * ----------------------------------------------------- */

    if (criticalCurrent) {
        return FAILSAFE_OVERCURRENT;
    }

    return FAILSAFE_OVERPOWER;
}


/* =========================================================
 * GŁÓWNA KONTROLA BEZPIECZEŃSTWA
 *
 * Wywoływana po updateRC() i przed updateFSM().
 * ========================================================= */

void updateSafety() {
    const uint32_t now = millis();


    /* =====================================================
     * FLAGI OSTRZEGAWCZE
     *
     * Nie powodują jeszcze failsafe.
     * ===================================================== */

    sys.currentWarning =
        isfinite(sys.currentA) &&
        sys.currentA > CURRENT_WARN_A;

    sys.powerWarning =
        isfinite(sys.powerW) &&
        sys.powerW > POWER_WARN_W;


    /* =====================================================
     * STATE_BOOT
     *
     * Podczas startu nie oceniamy utraty RC ani parametrów
     * zasilania. Sensory i odbiornik mogą nie mieć jeszcze
     * poprawnych danych.
     * ===================================================== */

    if (sys.state == STATE_BOOT) {
        sys.failsafeReason = FAILSAFE_NONE;
        resetOverloadTimer();
        return;
    }


    /* =====================================================
     * UTRATA ŁĄCZA RC
     *
     * Ma najwyższy priorytet.
     * ===================================================== */

    if (!sys.linkUp) {
        sys.failsafeReason = FAILSAFE_RC_LOST;
        resetOverloadTimer();
