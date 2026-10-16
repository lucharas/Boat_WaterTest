#include "declarations.h"

// Odwołania do instancji sprzętowych zdefiniowanych w Boat_WaterTest.ino
extern INA226_WE ina226;
extern Adafruit_BNO055 bno;
extern TinyGPSPlus gps;

/* =========================================================
 * INICJALIZACJA CZUJNIKÓW (I2C: INA226, BNO055 | UART: GPS)
 * ========================================================= */

void initSensors() {
    // 1. Magistrala I2C (400 kHz dla szybkiego odczytu)
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);

    // 2. Czujnik zasilania INA226
    if (!ina226.init()) {
        Serial.println("[SENS] Błąd: INA226 nie odpowiada na I2C!");
    } else {
        ina226.setResistorRange(INA226_SHUNT_OHMS, INA226_MAX_CURRENT);
        ina226.setAverage(INA226_AVERAGE_16);
        ina226.setConversionTime(INA226_CONV_TIME_1100);
        Serial.println("[SENS] INA226 zainicjalizowany poprawnie.");
    }

    // 3. Moduł IMU BNO055 (Adres 0x28)
    if (!bno.begin()) {
        Serial.println("[SENS] Błąd: BNO055 nie wykryty na magistrali I2C!");
        sys.imuHealthy = false;
    } else {
        bno.setExtCrystalUse(true);
        sys.imuHealthy = true;
        Serial.println("[SENS] BNO055 zainicjalizowany poprawnie.");
    }

    // 4. Moduł GPS (HardwareSerial 2)
    // GPIO 16 = RX, GPIO 17 = TX
    Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
    Serial.println("[SENS] GPS Serial2 uruchomiony.");

    // Stan początkowy w telemetrii
    sys.vbat          = 0.0f;
    sys.currentA      = 0.0f;
    sys.powerW        = 0.0f;
    sys.yaw           = 0.0f;
    sys.gpsLat        = 0;
    sys.gpsLon        = 0;
    sys.gpsSpeedMs    = 0.0f;
    sys.gpsCourse     = 0.0f;
    sys.gpsSats       = 0;
    sys.gpsFix        = false;
}

/* =========================================================
 * GŁÓWNA AKTUALIZACJA SENSORÓW (Wywoływana w schedulerze: 10 Hz)
 * ========================================================= */

void updateSensors() {
    // -----------------------------------------------------
    // A. Opróżnianie bufora UART GPS (nieblokujące)
    // -----------------------------------------------------
    while (Serial2.available() > 0) {
        gps.encode(Serial2.read());
    }

    // Aktualizacja danych GPS w strukturze systemowej
    if (gps.location.isValid()) {
        sys.gpsLat = gps.location.rawLat().deg * 1000000L + gps.location.rawLat().billionths / 1000L;
        if (gps.location.rawLat().negative) {
            sys.gpsLat = -sys.gpsLat;
        }

        sys.gpsLon = gps.location.rawLng().deg * 1000000L + gps.location.rawLng().billionths / 1000L;
        if (gps.location.rawLng().negative) {
            sys.gpsLon = -sys.gpsLon;
        }

        sys.gpsFix = true;
    } else {
        sys.gpsFix = false;
    }

    if (gps.speed.isValid()) {
        sys.gpsSpeedMs = (float)gps.speed.mps();
    } else {
        sys.gpsSpeedMs = 0.0f;
    }

    if (gps.course.isValid()) {
        sys.gpsCourse = (float)gps.course.deg();
    }

    if (gps.satellites.isValid()) {
        sys.gpsSats = (uint8_t)gps.satellites.value();
    } else {
        sys.gpsSats = 0;
    }

    // -----------------------------------------------------
    // B. Odczyt zasilania INA226
    // -----------------------------------------------------
    ina226.readAndClearFlags();
    sys.vbat     = ina226.getBusVoltage_V();
    sys.currentA = ina226.getCurrent_A();
    sys.powerW   = ina226.getBusPower();

    // -----------------------------------------------------
    // C. Odczyt kursu z BNO055 (Eulera Yaw)
    // -----------------------------------------------------
    if (sys.imuHealthy) {
        sensors_event_t orientationData;
        bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);

        // Oś X w układzie Eulera BNO055 to kąt kursowy Yaw (0.0 do 359.9°)
        float heading = orientationData.orientation.x;
        if (heading < 0.0f) {
            heading += 360.0f;
        } else if (heading >= 360.0f) {
            heading -= 360.0f;
        }
        sys.yaw = heading;
    }
}