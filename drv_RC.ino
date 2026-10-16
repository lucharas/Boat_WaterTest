#include "declarations.h"

// Obiekt odbiornika CRSF (zainicjalizowany w oparciu o sprzętowy Serial1)
// Zgodnie z wytycznymi instancja może być również przeniesiona do Boat_WaterTest.ino
static CRSFforArduino crsf;

/* =========================================================
 * INICJALIZACJA ODBIORNIKA ELRS (CRSF)
 * ========================================================= */

void initRC() {
    // Inicjalizacja magistrali Serial1 dla protokołu CRSF (420 000 baud)
    // GPIO 34 = RX (Input-only), GPIO 32 = TX
    Serial1.begin(ELRS_BAUDRATE, SERIAL_8N1, PIN_ELRS_RX, PIN_ELRS_TX);

    crsf.begin(Serial1);

    // Stan początkowy w strukturze systemowej
    sys.rcLinkUp       = false;
    sys.rcLinkQuality  = 0;
    sys.rcRssi         = -128;
    sys.rcThrottleUs   = RC_MIN_US;
    sys.rcRudderUs     = RC_CENTER_US;
    sys.rcSwitchSbUs   = RC_MIN_US;
    sys.rcSwitchScUs   = RC_MIN_US;
}

/* =========================================================
 * AKTUALIZACJA DANYCH RC (Częstotliwość wywołania: 50 Hz)
 * ========================================================= */

void updateRC() {
    // Odpytanie bufora portu szeregowego i dekodowanie ramek
    crsf.update();

    // Sprawdzenie stanu połączenia z nadajnikiem RadioMaster Pocket
    sys.rcLinkUp = crsf.isLinkUp();

    if (sys.rcLinkUp) {
        // Pobranie telemetrii łącza radiowego
        sys.rcLinkQuality = crsf.getLinkQuality();
        sys.rcRssi        = crsf.getReceiveRssi();

        // Odczyt drążków i przełączników z aparatury (zakres 1000 - 2000 us)
        // crsf.getChannel(x) zwraca wartość PWM w mikrosekundach
        sys.rcThrottleUs = constrain(crsf.getChannel(CRSF_CH_THRO), (int)RC_MIN_US, (int)RC_MAX_US);
        sys.rcRudderUs   = constrain(crsf.getChannel(CRSF_CH_STEER), (int)RC_MIN_US, (int)RC_MAX_US);
        
        // Przełączniki funkcyjne (RadioMaster Pocket: SB = CH6, SC = CH7)
        sys.rcSwitchSbUs = constrain(crsf.getChannel(CRSF_CH_SW_SB), (int)RC_MIN_US, (int)RC_MAX_US);
        sys.rcSwitchScUs = constrain(crsf.getChannel(CRSF_CH_SW_SC), (int)RC_MIN_US, (int)RC_MAX_US);
    } else {
        // FAILSAFE NA POZIOMIE WEJŚCIA: Brak ramek RC
        sys.rcLinkQuality = 0;
        sys.rcRssi        = -128;

        // Natychmiastowe zerowanie zadajników w buforze wejściowym
        sys.rcThrottleUs = RC_MIN_US;      // 1000 us (odcięcie gazu)
        sys.rcRudderUs   = RC_CENTER_US;    // 1500 us (ster na wprost)
        // Stany przełączników w razie utraty sygnału pozostawiamy na poziomie bezpiecznym (LOW)
        sys.rcSwitchSbUs = RC_MIN_US;
        sys.rcSwitchScUs = RC_MIN_US;
    }
}