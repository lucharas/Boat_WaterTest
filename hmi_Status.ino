// ============================================================================
// PLIK: hmi_Status.ino
// ============================================================================
#include "declarations.h"

extern Adafruit_NeoPixel statusLed;

static uint8_t lastRed = 0, lastGreen = 0, lastBlue = 0;

static void setPixelIfChanged(uint8_t r, uint8_t g, uint8_t b) {
    if (r == lastRed && g == lastGreen && b == lastBlue) return;
    statusLed.setPixelColor(0, statusLed.Color(r, g, b));
    statusLed.show();
    lastRed = r; lastGreen = g; lastBlue = b;
}

static inline uint8_t addColor(uint8_t val, uint8_t add) {
    return ((uint16_t)val + add > 255) ? 255 : (val + add);
}

static void applyGpsWarning(uint8_t& r, uint8_t& g, uint8_t& b) {
    if (sys.gpsFixValid) return;
    r = addColor(r, 35);
    g = addColor(g, 12);
    if (b > 20) b -= 20;
}

void initStatus() {
    statusLed.begin();
    statusLed.setBrightness(255);
    statusLed.clear();
    statusLed.show();
    lastRed = lastGreen = lastBlue = 0;
}

void updateStatus() {
    const uint32_t now = millis();
    uint8_t red = 0, green = 0, blue = 0;

    switch (sys.state) {
        case STATE_FAILSAFE:
            if ((now / 100) % 2 == 0) red = 255;
            break;

        case STATE_BOOT:
            if ((now / 100) % 10 == 0) red = green = blue = 255;
            break;

        case STATE_DISARMED: {
            float breath = (exp(sin(now / 2000.0 * PI)) - 0.36787944f) * 108.0f;
            green = (uint8_t)constrain(breath, 0.0f, 255.0f);
            break;
        }

        case STATE_ARMED:
            green = 255;
            break;

        case STATE_RECORDING:
            if ((now / 100) % 2 == 0) { red = 128; blue = 255; }
            break;

        case STATE_BT_DUMP:
            blue = 255;
            break;
    }

    applyGpsWarning(red, green, blue);
    setPixelIfChanged(red, green, blue);
}