#include "declarations.h"

extern INA226_WE       ina226;
extern Adafruit_BNO055 bno;
extern TinyGPSPlus     gps;

static bool ina226Healthy = false;

static void resetSensorData() {
    sys.voltageV = sys.currentA = sys.powerW = sys.headingDeg = sys.speedKmh = sys.cogDeg = sys.hdop = 0.0f;
    sys.lat = sys.lon = 0.0;
    sys.gpsAgeMs = UINT32_MAX;
    sys.satellites = sys.magCalib = 0;
    sys.imuHealthy = sys.gpsFixValid = false;
}

void initSensors() {
    resetSensorData();

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);

    ina226Healthy = ina226.init();
    if (ina226Healthy) {
        ina226.setResistorRange(INA226_SHUNT_OHMS, INA226_MAX_CURRENT_A);
        ina226.setAverage(INA226_AVERAGING);
        ina226.setConversionTime(INA226_CONVERSION_TIME);
        ina226.setMeasureMode(INA226_CONTINUOUS);
        Serial.println(F("[SENS] INA226 ok"));
    } else {
        Serial.println(F("[SENS] INA226 err"));
    }

    sys.imuHealthy = bno.begin();
    if (sys.imuHealthy) {
        bno.setExtCrystalUse(true);
        Serial.println(F("[SENS] BNO055 ok"));
    } else {
        Serial.println(F("[SENS] BNO055 err"));
    }

    Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    delay(100);
    Serial2.print(F("$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n"));
    Serial2.print(F("$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n"));
    Serial2.print(F("$PUBX,40,GSV,0,0,0,0,0,0*59\r\n"));
    Serial2.print(F("$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n"));
    Serial.println(F("[SENS] GPS ok"));
}

static void updateGps() {
    while (Serial2.available() > 0) gps.encode((char)Serial2.read());

    sys.gpsFixValid = gps.location.isValid();
    sys.lat         = sys.gpsFixValid ? gps.location.lat() : 0.0;
    sys.lon         = sys.gpsFixValid ? gps.location.lng() : 0.0;
    sys.gpsAgeMs    = sys.gpsFixValid ? gps.location.age() : UINT32_MAX;

    sys.speedKmh   = gps.speed.isValid() ? (float)gps.speed.kmph() : 0.0f;
    sys.cogDeg     = gps.course.isValid() ? (float)gps.course.deg() : 0.0f;
    sys.hdop       = gps.hdop.isValid() ? (float)gps.hdop.hdop() : 0.0f;
    sys.satellites = gps.satellites.isValid() ? (uint8_t)constrain(gps.satellites.value(), 0UL, 255UL) : 0;
}

static void updateIna226() {
    if (!ina226Healthy) {
        sys.voltageV = sys.currentA = sys.powerW = 0.0f;
        return;
    }
    ina226.readAndClearFlags();
    sys.voltageV = ina226.getBusVoltage_V();
    sys.currentA = ina226.getCurrent_A();
    sys.powerW   = ina226.getBusPower() / 1000.0f;
}

static void updateBno055() {
    if (!sys.imuHealthy) { sys.headingDeg = 0.0f; return; }

    uint8_t sysC, gyro, accel, mag = 0;
    bno.getCalibration(&sysC, &gyro, &accel, &mag);
    sys.magCalib = mag;

    sensors_event_t event;
    if (!bno.getEvent(&event, Adafruit_BNO055::VECTOR_EULER) || !isfinite(event.orientation.x)) {
        sys.imuHealthy = false;
        sys.headingDeg = 0.0f;
        return;
    }

    float h = event.orientation.x;
    while (h < 0.0f)   h += 360.0f;
    while (h >= 360.0f) h -= 360.0f;
    sys.headingDeg = h;
}

void updateSensors() {
    updateGps();
    updateIna226();
    updateBno055();
}
