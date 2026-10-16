#include "declarations.h"

/* =========================================================
 * FUNKCJE POMOCNICZE (Przeliczanie mikrosekund na LEDC duty)
 * ========================================================= */

// Okres dla 50 Hz = 20 000 us. Rozdzielczość 16-bit = 65535.
static inline uint32_t usToDuty(uint16_t us) {
    return ((uint32_t)us * 65535UL) / 20000UL;
}

/* =========================================================
 * INICJALIZACJA WYJŚĆ WYKONAWCZYCH (LEDC PWM 50 Hz)
 * ========================================================= */

void initActuators() {
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    // ESP32 Arduino Core v3.x API
    ledcAttach(PIN_ESC, PWM_FREQ_HZ, PWM_BITS);
    ledcAttach(PIN_SERVO, PWM_FREQ_HZ, PWM_BITS);
#else
    // ESP32 Arduino Core v2.x API
    ledcSetup(PWM_CH_ESC, PWM_FREQ_HZ, PWM_BITS);
    ledcAttachPin(PIN_ESC, PWM_CH_ESC);

    ledcSetup(PWM_CH_SERVO, PWM_FREQ_HZ, PWM_BITS);
    ledcAttachPin(PIN_SERVO, PWM_CH_SERVO);
#endif

    // Bezpieczny stan początkowy: silnik natychmiast odcięty, ster w neutrum
    stopMotor();
    setRudder(SERVO_CENTER_US);
}

/* =========================================================
 * STEROWANIE SILNIKIEM (ESC Flycolor)
 * ========================================================= */

void setMotor(uint16_t pwmUs) {
    // Twarde ograniczenie programowe zakresu mikrosekund
    uint16_t clampedUs = constrain(pwmUs, (uint16_t)ESC_MIN_US, (uint16_t)ESC_MAX_US);

    // Filtr strefy martwej: poniżej ESC_DEADZONE_US wymuszamy pełne odcięcie (1000 us)
    if (clampedUs < ESC_DEADZONE_US) {
        clampedUs = ESC_MIN_US;
    }

    uint32_t duty = usToDuty(clampedUs);

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(PIN_ESC, duty);
#else
    ledcWrite(PWM_CH_ESC, duty);
#endif

    // Aktualizacja rzeczywistej wartości w telemetrii systemowej
    sys.escOutputUs = clampedUs;
}

/* =========================================================
 * AWARYJNE / TWARDE ODCIĘCIE NAPĘDU
 * ========================================================= */

void stopMotor() {
    uint32_t duty = usToDuty(ESC_MIN_US);

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(PIN_ESC, duty);
#else
    ledcWrite(PWM_CH_ESC, duty);
#endif

    sys.escOutputUs = ESC_MIN_US;
}

/* =========================================================
 * STEROWANIE SERWEM STERU
 * ========================================================= */

void setRudder(uint16_t pwmUs) {
    uint16_t clampedUs = constrain(pwmUs, (uint16_t)SERVO_MIN_US, (uint16_t)SERVO_MAX_US);
    uint32_t duty = usToDuty(clampedUs);

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(PIN_SERVO, duty);
#else
    ledcWrite(PWM_CH_SERVO, duty);
#endif

    sys.servoOutputUs = clampedUs;
}