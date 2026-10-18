y#include "declarations.h"


/* =========================================================
 * OBIEKTY ZDEFINIOWANE W Boat_WaterTest.ino
 * ========================================================= */

extern INA226_WE       ina226;
extern Adafruit_BNO055 bno;
extern TinyGPSPlus     gps;


/* =========================================================
 * STAN WEWNĘTRZNY MODUŁU
 * ========================================================= */

static bool ina226Healthy = false;


/* =========================================================
 * INICJALIZACJA WARTOŚCI SYSTEMOWYCH
 * ========================================================= */

static void resetSensorData() {
    sys.voltageV = 0.0f;
    sys.currentA = 0.0f;
    sys.powerW   = 0.0f;

    sys.headingDeg = 0.0f;
    sys.imuHealthy = false;

    sys.lat         = 0.0;
    sys.lon         = 0.0;
    sys.speedKmh    = 0.0f;
    sys.cogDeg      = 0.0f;
    sys.hdop        = 0.0f;
    sys.gpsAgeMs    = UINT32_MAX;
    sys.satellites  = 0;
    sys.gpsFixValid = false;
}


/* =========================================================
 * INICJALIZACJA CZUJNIKÓW
 * ========================================================= */

void initSensors() {
    resetSensorData();

    /* -----------------------------------------------------
     * Magistrala I2C
     * ----------------------------------------------------- */

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);


    /* -----------------------------------------------------
     * INA226
     * ----------------------------------------------------- */

    ina226Healthy = ina226.init();

    if (ina226Healthy) {
        ina226.setResistorRange(
            INA226_SHUNT_OHMS,
            INA226_MAX_CURRENT_A
        );

        ina226.setAverage(INA226_AVERAGING);
        ina226.setConversionTime(INA226_CONVERSION_TIME);
        ina226.setMeasureMode(INA226_CONTINUOUS);

        Serial.println("[SENS] INA226 initialized");
    } else {
        Serial.println("[SENS] INA226 not detected");
    }


    /* -----------------------------------------------------
     * BNO055
     * ----------------------------------------------------- */

    sys.imuHealthy = bno.begin();

    if (sys.imuHealthy) {
        bno.setExtCrystalUse(true);
        Serial.println("[SENS] BNO055 initialized");
    } else {
        Serial.println("[SENS] BNO055 not detected");
    }


    /* -----------------------------------------------------
     * GPS
     * ----------------------------------------------------- */

    Serial2.begin(
        GPS_BAUDRATE,
        SERIAL_8N1,
        PIN_GPS_RX,
        PIN_GPS_TX
    );
    delay(100);
    Serial2.print(F("$PUBX,40,GLL,0,0,0,0,0,0*5C\r\n"));
    Serial2.print(F("$PUBX,40,GSA,0,0,0,0,0,0*4E\r\n"));
    Serial2.print(F("$PUBX,40,GSV,0,0,0,0,0,0*59\r\n"));
    Serial2.print(F("$PUBX,40,VTG,0,0,0,0,0,0*5E\r\n"));

    Serial.println("[SENS] GPS UART initialized & configured");
}


/* =========================================================
 * ODCZYT GPS
 * ========================================================= */

static void updateGps() {

    /* -----------------------------------------------------
     * Opróżnienie bufora UART
     * ----------------------------------------------------- */

    while (Serial2.available() > 0) {
        gps.encode((char)Serial2.read());
    }


    /* -----------------------------------------------------
     * Pozycja i wiek danych
     * ----------------------------------------------------- */

    sys.gpsFixValid = gps.location.isValid();

    if (sys.gpsFixValid) {
        sys.lat      = gps.location.lat();
        sys.lon      = gps.location.lng();
        sys.gpsAgeMs = gps.location.age();
    } else {
        sys.lat      = 0.0;
        sys.lon      = 0.0;
        sys.gpsAgeMs = UINT32_MAX;
    }


    /* -----------------------------------------------------
     * Prędkość
     * ----------------------------------------------------- */

    if (gps.speed.isValid()) {
        sys.speedKmh = (float)gps.speed.kmph();
    } else {
        sys.speedKmh = 0.0f;
    }


    /* -----------------------------------------------------
     * Kurs nad dnem
     * ----------------------------------------------------- */

    if (gps.course.isValid()) {
        sys.cogDeg = (float)gps.course.deg();
    } else {
        sys.cogDeg = 0.0f;
    }


    /* -----------------------------------------------------
     * HDOP
     * ----------------------------------------------------- */

    if (gps.hdop.isValid()) {
        sys.hdop = (float)gps.hdop.hdop();
    } else {
        sys.hdop = 0.0f;
    }


    /* -----------------------------------------------------
     * Liczba satelitów
     * ----------------------------------------------------- */

    if (gps.satellites.isValid()) {
        const uint32_t satelliteCount = gps.satellites.value();

        sys.satellites = (uint8_t)constrain(
            satelliteCount,
            0UL,
            255UL
        );
    } else {
        sys.satellites = 0;
    }
}


/* =========================================================
 * ODCZYT INA226
 * ========================================================= */

static void updateIna226() {
    if (!ina226Healthy) {
        sys.voltageV = 0.0f;
        sys.currentA = 0.0f;
        sys.powerW   = 0.0f;
        return;
    }

    ina226.readAndClearFlags();

    sys.voltageV = ina226.getBusVoltage_V();
    sys.currentA = ina226.getCurrent_A();

    // getBusPower() zwraca mW
    sys.powerW = ina226.getBusPower() / 1000.0f;
}


/* =========================================================
 * ODCZYT BNO055
 * ========================================================= */

static void updateBno055() {
    if (!sys.imuHealthy) {
        sys.headingDeg = 0.0f;
        return;
    }

    sensors_event_t orientationData;

    const bool readSuccessful = bno.getEvent(
        &orientationData,
        Adafruit_BNO055::VECTOR_EULER
    );

    if (!readSuccessful) {
        sys.imuHealthy = false;
        sys.headingDeg = 0.0f;
        return;
    }

    float heading = orientationData.orientation.x;

    if (!isfinite(heading)) {
        sys.imuHealthy = false;
        sys.headingDeg = 0.0f;
        return;
    }

    while (heading < 0.0f) {
        heading += 360.0f;
    }

    while (heading >= 360.0f) {
        heading -= 360.0f;
    }

    sys.headingDeg = heading;
}


/* =========================================================
 * GŁÓWNA AKTUALIZACJA SENSORÓW
 *
 * Wywoływana przez scheduler co TASK_SENSOR_MS.
 * ========================================================= */

void updateSensors() {
    updateGps();
    updateIna226();
    updateBno055();
}
