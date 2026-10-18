#include "declarations.h"


/* =========================================================
 * OBIEKT ZDEFINIOWANY W Boat_WaterTest.ino
 * ========================================================= */

extern Adafruit_NeoPixel statusLed;


/* =========================================================
 * OSTATNI WYŚWIETLONY KOLOR
 * ========================================================= */

static uint8_t lastRed   = 0;
static uint8_t lastGreen = 0;
static uint8_t lastBlue  = 0;


/* =========================================================
 * ZAPIS KOLORU TYLKO PO ZMIANIE
 * ========================================================= */

static void setPixelIfChanged(
    uint8_t red,
    uint8_t green,
    uint8_t blue
) {
    if (
        red   == lastRed &&
        green == lastGreen &&
        blue  == lastBlue
    ) {
        return;
    }

    statusLed.setPixelColor(
        0,
        statusLed.Color(red, green, blue)
    );

    statusLed.show();

    lastRed   = red;
    lastGreen = green;
    lastBlue  = blue;
}


/* =========================================================
 * OGRANICZONE DODAWANIE SKŁADOWEJ KOLORU
 * ========================================================= */

static uint8_t addColor(
    uint8_t value,
    uint8_t addition
) {
    const uint16_t result =
        (uint16_t)value + addition;

    return result > 255
        ? 255
        : (uint8_t)result;
}


/* =========================================================
 * MODYFIKACJA KOLORU PRZY BRAKU FIXA GPS
 *
 * Wzorzec stanu pozostaje bez zmian. Dodawany jest jedynie
 * lekki bursztynowy odcień.
 * ========================================================= */

static void applyGpsWarning(
    uint8_t& red,
    uint8_t& green,
    uint8_t& blue
) {
    if (sys.gpsFixValid) {
        return;
    }

    red   = addColor(red, 35);
    green = addColor(green, 12);

    if (blue > 20) {
        blue -= 20;
    }
}


/* =========================================================
 * INICJALIZACJA NEOPIXELA
 * ========================================================= */

void initStatus() {
    statusLed.begin();

    // Jasność jest regulowana wartościami RGB.
    statusLed.setBrightness(255);

    statusLed.clear();
    statusLed.show();

    lastRed   = 0;
    lastGreen = 0;
    lastBlue  = 0;
}


/* =========================================================
 * GŁÓWNA AKTUALIZACJA STATUSU
 *
 * Wywoływana przez scheduler co TASK_STATUS_MS.
 * ========================================================= */

void updateStatus() {
    const uint32_t now = millis();

    uint8_t red   = 0;
    uint8_t green = 0;
    uint8_t blue  = 0;


    /* =====================================================
     * STATE_FAILSAFE
     *
     * Szybkie czerwone miganie:
     * 100 ms światło / 100 ms ciemność
     * ===================================================== */

    if (sys.state == STATE_FAILSAFE) {
        if ((now / 100) % 2 == 0) {
            red = 255;
        }
    } else if (sys.state == STATE_BOOT) {
        if ((now / 100) % 10 == 0) {
            red = 255; green = 255; blue = 255;
        }
    } else if (sys.state == STATE_DISARMED) {
        float breath = (exp(sin(now / 2000.0 * PI)) - 0.36787944) * 108.0;
        green = constrain(breath, 0, 255);
    } else if (sys.state == STATE_ARMED) {
        green = 255;
    } else if (sys.state == STATE_RECORDING) {
        if ((now / 100) % 2 == 0) {
            red = 128; blue = 255;
        }
    } else if (sys.state == STATE_BT_DUMP) {
        blue = 255;
    }

    applyGpsWarning(red, green, blue);
    setPixelIfChanged(red, green, blue);
}
