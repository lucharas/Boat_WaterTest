// ============================================================================
// PLIK: drv_RC.ino
// ============================================================================
#include "declarations.h"

static bool rcFrameValid = false;

// --- CALLBACK KANAŁÓW RC ---
static void onReceiveRcChannels(serialReceiverLayer::rcChannels_t *channels) {
    rcFrameValid = channels->valid && !channels->failsafe;
    if (!rcFrameValid) return;

    sys.rcSteerUs = constrain(crsf.rcToUs(crsf.getChannel(CRSF_CH_STEER)), (int)RC_MIN_US, (int)RC_MAX_US);
    sys.rcThroUs  = constrain(crsf.rcToUs(crsf.getChannel(CRSF_CH_THRO)), (int)RC_MIN_US, (int)RC_MAX_US);
    sys.rcSbUs    = constrain(crsf.rcToUs(crsf.getChannel(CRSF_CH_SB)), (int)RC_MIN_US, (int)RC_MAX_US);
    sys.rcScUs    = constrain(crsf.rcToUs(crsf.getChannel(CRSF_CH_SC)), (int)RC_MIN_US, (int)RC_MAX_US);

    sys.rcNoLimits = (sys.rcScUs >= RC_SC_HIGH_THRESHOLD_US);
}

// --- CALLBACK STATYSTYK ŁĄCZA ---
static void onLinkStatisticsUpdate(serialReceiverLayer::link_statistics_t linkStatistics) {
    sys.rssiDbm   = linkStatistics.rssi;
    sys.lqPercent = constrain(linkStatistics.lqi, 0, 100);
}

// --- WARTOŚCI BEZPIECZNE ---
static void setRcSafeValues() {
    sys.rcSteerUs  = RC_CENTER_US;
    sys.rcThroUs   = sys.rcSbUs = sys.rcScUs = RC_MIN_US;
    sys.rcNoLimits = false;
}

// --- INICJALIZACJA CRSF / EXPRESSLRS ---
void initRC() {
    setRcSafeValues();
    sys.linkUp = rcFrameValid = false;
    sys.rssiDbm = -128;
    sys.lqPercent = 0;

    Serial1.begin(ELRS_BAUDRATE, SERIAL_8N1, PIN_ELRS_RX, PIN_ELRS_TX);

    if (!crsf.begin()) {
        Serial.println(F("[RC] CRSF initialization failed"));
        return;
    }

    crsf.setRcChannelsCallback(onReceiveRcChannels);
    crsf.setLinkStatisticsCallback(onLinkStatisticsUpdate);
    Serial.println(F("[RC] CRSF initialized"));
}

// --- AKTUALIZACJA ODBIORNIKA (co TASK_RC_MS) ---
void updateRC() {
    crsf.update();
    sys.linkUp = rcFrameValid;

    if (!sys.linkUp) {
        setRcSafeValues();
        sys.lqPercent = 0;
    }
}