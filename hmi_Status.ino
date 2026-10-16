#include "declarations.h"

// Odwołanie do instancji diody zdefiniowanej w Boat_WaterTest.ino
extern Adafruit_NeoPixel statusLed;

// Poprzednio wysłane składowe koloru (ochrona przed zbędnym show())
static uint8_t lastR = 0;
static uint8_t lastG = 0;
static uint8_t lastB = 0;

/* =========================================================
 * FUNKCJA POMOCNICZA: WARUNKOWY ZAPIS KOLORU
 * ========================================================= */

static void setPixelIfChanged(uint8_t r, uint8_t g, uint8_t b) {
    if (r != lastR || g != lastG || b != lastB) {
        statusLed.setPixelColor(0, statusLed.Color(r, g, b));
        statusLed.show();
        lastR = r;
        lastG = g;
        lastB = b;
    }
}

/* =========================================================
 * INICJALIZACJA DIODY STATUSOWEJ
 * ========================================================= */

void initStatus() {
    statusLed.begin();
    statusLed.setBrightness(255); // Pełna skala, jasność regulujemy programowo
    setPixelIfChanged(0, 0, 0);   // Start z wygaszoną diodą
}

/* =========================================================
 * GŁÓWNA AKTUALIZACJA SYGNALIZACJI (Scheduler: 20 Hz / 50 ms)
 * ========================================================= */

void updateStatus() {
    uint32_t now = millis();
    uint8_t targetR = 0;
    uint8_t targetG = 0;
    uint8_t targetB = 0;

    // 1. Stany alarmowe i serwisowe mają bezwzględny priorytet
    if (sys.state == STATE_FAILSAFE) {
        // Agresywne miganie czerwone (okres 200 ms: 100 ms ON / 100 ms OFF)
        if ((now % 200) < 100) {
            targetR = 255;
            targetG = 0;
            targetB = 0;
        } else {
            targetR = 0;
            targetG = 0;
            targetB = 0;
        }
        setPixelIfChanged(targetR, targetG, targetB);
        return;
    }

    if (sys.state == STATE_BT_DUMP) {
        // Stałe światło niebieskie (transfer danych / tryb serwisowy)
        setPixelIfChanged(0, 0, 180);
        return;
    }

    // 2. Ostrzeżenie: brak fixa GPS w stanach spoczynkowych (BOOT, DISARMED, ARMED)
    // Jeśli nie mamy fixa GPS, pulsujemy kolorem bursztynowym/żółtym
    if (!sys.gpsFix && (sys.state == STATE_BOOT || sys.state == STATE_DISARMED || sys.state == STATE_ARMED)) {
        // Płynny trójkątny oddech (okres 1500 ms)
        uint16_t phase = now % 1500;
        uint8_t breath;
        if (phase < 750) {
            breath = (phase * 160) / 750;
        } else {
            breath = ((1500 - phase) * 160) / 750;
        }
        breath = constrain(breath, 10, 160);

        // Bursztyn: R = breath, G = breath * 0.45, B = 0
        targetR = breath;
        targetG = (breath * 45) / 100;
        targetB = 0;

        setPixelIfChanged(targetR, targetG, targetB);
        return;
    }

    // 3. Normalna sygnalizacja stanów pracy automatu (z ważnym fixem GPS lub w trakcie rejsu)
    switch (sys.state) {
        case STATE_BOOT:
            // Krótkie białe mignięcie co 500 ms
            if ((now % 500) < 50) {
                targetR = 80; targetG = 80; targetB = 80;
            } else {
                targetR = 0;  targetG = 0;  targetB = 0;
            }
            break;

        case STATE_DISARMED: {
            // Spokojny zielony oddech (okres 2000 ms)
            uint16_t phase = now % 2000;
            uint8_t breath;
            if (phase < 1000) {
                breath = (phase * 120) / 1000;
            } else {
                breath = ((2000 - phase) * 120) / 1000;
            }
            targetR = 0;
            targetG = constrain(breath, 5, 120);
            targetB = 0;
            break;
        }

        case STATE_ARMED:
            // Stałe, wyraźne światło zielone (gotowość do napędu)
            targetR = 0;
            targetG = 180;
            targetB = 0;
            break;

        case STATE_RECORDING:
            // Szybkie błyski fioletowo-niebieskie (okres 250 ms: 125 ms fiolet / 125 ms niebieski)
            if ((now % 250) < 125) {
                targetR = 140; targetG = 0;   targetB = 220; // Fiolet
            } else {
                targetR = 0;   targetG = 40;  targetB = 220; // Błękit
            }
            break;

        default:
            targetR = 0; targetG = 0; targetB = 0;
            break;
    }

    setPixelIfChanged(targetR, targetG, targetB);
}