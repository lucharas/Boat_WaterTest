// ============================================================================
// PLIK: logic_FSM.ino
// ============================================================================
#include "declarations.h"

static uint32_t bootStartMs = 0;

enum SbPosition : uint8_t { SB_LOW = 0, SB_MIDDLE, SB_HIGH };

static uint8_t getSbPosition() {
    if (sys.rcSbUs < RC_SB_LOW_THRESHOLD_US) return SB_LOW;
    if (sys.rcSbUs > RC_SB_HIGH_THRESHOLD_US) return SB_HIGH;
    return SB_MIDDLE;
}

static uint16_t calculateMotorOutput() {
    if (sys.rcThroUs <= THRO_ARM_THRESHOLD_US) return ESC_MIN_US;

    uint16_t maxUs = sys.rcNoLimits ? ESC_MAX_US : constrain(sys.maxThrottlePwm, (uint16_t)ESC_DEADZONE_US, (uint16_t)ESC_MAX_US);
    uint32_t inRange = RC_MAX_US - THRO_ARM_THRESHOLD_US;
    uint32_t outRange = maxUs - ESC_DEADZONE_US;
    uint32_t inVal = constrain(sys.rcThroUs, (uint16_t)THRO_ARM_THRESHOLD_US, (uint16_t)RC_MAX_US) - THRO_ARM_THRESHOLD_US;

    return ESC_DEADZONE_US + (inVal * outRange) / inRange;
}

static void controlActuators() {
    if (!sys.linkUp) { stopMotor(); setRudder(SERVO_CENTER_US); return; }
    setMotor(calculateMotorOutput());
    setRudder(sys.rcSteerUs);
}

void setState(BoatState newState) {
    if (sys.state == newState) return;
    sys.state = newState;

    switch (newState) {
        case STATE_BOOT:
            bootStartMs = millis();
            stopMotor(); setRudder(SERVO_CENTER_US); stopBT();
            break;
        case STATE_DISARMED:
            stopMotor(); setRudder(SERVO_CENTER_US); startBT();
            break;
        case STATE_ARMED:
            stopMotor(); setRudder(sys.linkUp ? sys.rcSteerUs : SERVO_CENTER_US); stopBT();
            break;
        case STATE_RECORDING:
            stopBT();
            break;
        case STATE_FAILSAFE:
            stopMotor(); setRudder(SERVO_CENTER_US); stopBT();
            break;
        case STATE_BT_DUMP:
            stopMotor(); setRudder(SERVO_CENTER_US); startBT();
            break;
    }
}

void initFSM() {
    bootStartMs = millis();
    sys.state = STATE_BOOT;
    sys.failsafeReason = FAILSAFE_NONE;
    stopMotor(); setRudder(SERVO_CENTER_US); stopBT();
}

void updateFSM() {
    const uint32_t now = millis();
    const uint8_t sb = getSbPosition();

    // 1. Priorytet: Failsafe (zabezpieczenia i odcięcie ignorowane dla boot/disarmed zarządza logic_Safety)
    if (sys.failsafeReason != FAILSAFE_NONE && sys.state != STATE_FAILSAFE) {
        setState(STATE_FAILSAFE);
    }

    // 2. Obsługa maszyny stanów
    switch (sys.state) {
        case STATE_BOOT:
            if (now - bootStartMs >= ESC_ARM_TIME_MS) {
                setState(STATE_DISARMED);
            }
            break;

        case STATE_FAILSAFE:
            // Wyjście z failsafe: brak błędu, link OK, przełącznik SB na dole (-100%)
            if (sys.failsafeReason == FAILSAFE_NONE && sys.linkUp && sb == SB_LOW) {
                setState(STATE_DISARMED);
            }
            break;

        case STATE_DISARMED:
            // Przejście do uzbrojenia: link OK, przełącznik SB na środek (0%)
            if (sys.linkUp && sb == SB_MIDDLE) {
                setState(STATE_ARMED);
            }
            break;

        case STATE_ARMED:
            if (sb == SB_HIGH) {
                setState(STATE_RECORDING);
            } else if (sb == SB_LOW) {
                setState(STATE_DISARMED);
            } else {
                controlActuators();
            }
            break;

        case STATE_RECORDING:
            if (sb == SB_LOW) {
                setState(STATE_DISARMED);
            } else if (sb == SB_MIDDLE) {
                setState(STATE_ARMED);
            } else {
                controlActuators();
            }
            break;

        case STATE_BT_DUMP:
            // Wyjście z trybu zrzutu odbywa się asynchronicznie poprzez komendę EXIT z modułu BT
            break;
    }
}