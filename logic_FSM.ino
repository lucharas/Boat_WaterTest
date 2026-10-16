#include "declarations.h"

static uint32_t armTimerStartMs = 0;
static bool     armTimerActive   = false;

/* =========================================================
 * PRZEŁĄCZANIE STANÓW FSM
 * ========================================================= */

void setState(BoatState newState) {
    if (sys.state == newState) {
        return;
    }

    sys.previousState = sys.state;
    sys.state = newState;

    switch (newState) {
        case STATE_BOOT:
        case STATE_DISARMED:
            stopMotor();
            armTimerActive = false;
            break;

        case STATE_ARMED:
            stopMotor();
            break;

        case STATE_RECORDING:
            // Wymuszone odcięcie radia BT na czas pływania (ochrona pasma 2.4 GHz)
            stopBT();
            break;

        case STATE_FAILSAFE:
            stopMotor();
            setRudder(SERVO_CENTER_US);
            // Aktywacja BT w celach ratunkowych/diagnostycznych
            startBT();
            break;

        case STATE_BT_DUMP:
            stopMotor();
            startBT();
            break;
    }
}

/* =========================================================
 * GŁÓWNA AKTUALIZACJA FSM (Wywoływana w schedulerze: 50 Hz)
 * ========================================================= */

void updateFSM() {
    uint32_t now = millis();

    // 1. Zarządzanie radiem BT w stanach spoczynku za pomocą przełącznika SB
    if (sys.state == STATE_DISARMED || sys.state == STATE_ARMED) {
        if (sys.rcLinkUp && sys.rcSwitchSbUs > RC_SW_HIGH_THRESHOLD_US) {
            if (!sys.btActive) {
                startBT();
            }
        } else if (sys.rcLinkUp && sys.rcSwitchSbUs < RC_SW_LOW_THRESHOLD_US) {
            if (sys.btActive) {
                stopBT();
            }
        }
    }

    // 2. Obsługa bieżącego stanu automatu
    switch (sys.state) {
        case STATE_BOOT:
            stopMotor();
            setRudder(SERVO_CENTER_US);
            if (now >= ESC_ARM_TIME_MS) {
                setState(STATE_DISARMED);
            }
            break;

        case STATE_DISARMED:
            stopMotor();
            setRudder(sys.rcLinkUp ? sys.rcRudderUs : SERVO_CENTER_US);

            // Uzbrojenie: aktywny link, brak awarii baterii, drążek na minimum przez 2000 ms
            if (sys.rcLinkUp && !sys.batteryLow && sys.rcThrottleUs <= THRO_ARM_THRESHOLD_US) {
                if (!armTimerActive) {
                    armTimerStartMs = now;
                    armTimerActive = true;
                } else if (now - armTimerStartMs >= ARM_DELAY_MS) {
                    armTimerActive = false;
                    setState(STATE_ARMED);
                }
            } else {
                armTimerActive = false;
            }
            break;

        case STATE_ARMED:
            stopMotor();
            setRudder(sys.rcRudderUs);

            // Ruch drążka gazu powyżej progu startuje rejs
            if (sys.rcThrottleUs > THRO_ARM_THRESHOLD_US) {
                setState(STATE_RECORDING);
            }
            break;

        case STATE_RECORDING:
            // Mapowanie pełnego skoku drążka gazu na zadany z BT limit [ESC_DEADZONE_US .. sys.maxThrottlePwm]
            {
                uint16_t mappedThrottle = map(sys.rcThrottleUs,
                                              THRO_ARM_THRESHOLD_US, RC_MAX_US,
                                              ESC_DEADZONE_US, sys.maxThrottlePwm);

                setMotor(mappedThrottle);
                setRudder(sys.rcRudderUs);
            }

            // Zatrzymanie rejsu po odpuszczeniu gazu lub zapełnieniu bufora RAM
            if (sys.rcThrottleUs <= THRO_ARM_THRESHOLD_US || sampleCount >= MAX_SAMPLES) {
                setState(STATE_ARMED);
            }
            break;

        case STATE_FAILSAFE:
            stopMotor();
            setRudder(SERVO_CENTER_US);
            break;

        case STATE_BT_DUMP:
            stopMotor();
            setRudder(SERVO_CENTER_US);
            break;
    }
}