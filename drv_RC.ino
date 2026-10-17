#include "declarations.h"


/* =========================================================
 * STAN WEWNĘTRZNY MODUŁU
 * ========================================================= */

static bool rcFrameValid = false;


/* =========================================================
 * CALLBACK KANAŁÓW RC
 * ========================================================= */

static void onReceiveRcChannels(
    serialReceiverLayer::rcChannels_t *channels
) {
    rcFrameValid = channels->valid && !channels->failsafe;

    if (!rcFrameValid) {
        return;
    }

    sys.rcSteerUs = constrain(
        crsf.rcToUs(crsf.getChannel(CRSF_CH_STEER)),
        (int)RC_MIN_US,
        (int)RC_MAX_US
    );

    sys.rcThroUs = constrain(
        crsf.rcToUs(crsf.getChannel(CRSF_CH_THRO)),
        (int)RC_MIN_US,
        (int)RC_MAX_US
    );

    sys.rcSbUs = constrain(
        crsf.rcToUs(crsf.getChannel(CRSF_CH_SB)),
        (int)RC_MIN_US,
        (int)RC_MAX_US
    );

    sys.rcScUs = constrain(
        crsf.rcToUs(crsf.getChannel(CRSF_CH_SC)),
        (int)RC_MIN_US,
        (int)RC_MAX_US
    );

    sys.rcNoLimits =
        sys.rcScUs >= RC_SC_HIGH_THRESHOLD_US;
}


/* =========================================================
 * CALLBACK STATYSTYK ŁĄCZA
 * ========================================================= */

static void onLinkStatisticsUpdate(
    serialReceiverLayer::link_statistics_t linkStatistics
) {
    sys.rssiDbm  = linkStatistics.rssi;
    sys.lqPercent = constrain(
        linkStatistics.lqi,
        0,
        100
    );
}


/* =========================================================
 * WARTOŚCI BEZPIECZNE
 * ========================================================= */

static void setRcSafeValues() {
    sys.rcSteerUs  = RC_CENTER_US;
    sys.rcThroUs   = RC_MIN_US;
    sys.rcSbUs     = RC_MIN_US;
    sys.rcScUs     = RC_MIN_US;
    sys.rcNoLimits = false;
}


/* =========================================================
 * INICJALIZACJA CRSF / EXPRESSLRS
 * ========================================================= */

void initRC() {
    setRcSafeValues();

    sys.linkUp   = false;
    sys.rssiDbm  = -128;
    sys.lqPercent = 0;

    rcFrameValid = false;

    Serial1.begin(
        ELRS_BAUDRATE,
        SERIAL_8N1,
        PIN_ELRS_RX,
        PIN_ELRS_TX
    );

    if (!crsf.begin()) {
        Serial.println("[RC] CRSF initialization failed");
        return;
    }

    crsf.setRcChannelsCallback(onReceiveRcChannels);
    crsf.setLinkStatisticsCallback(onLinkStatisticsUpdate);

    Serial.println("[RC] CRSF initialized");
}


/* =========================================================
 * AKTUALIZACJA ODBIORNIKA
 *
 * Wywoływana przez scheduler co TASK_RC_MS.
 * ========================================================= */

void updateRC() {
    crsf.update();

    sys.linkUp = crsf.isLinkUp() && rcFrameValid;

    if (!sys.linkUp) {
        setRcSafeValues();
        sys.lqPercent = 0;
    }
}
