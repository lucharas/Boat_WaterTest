#pragma once

/* =========================================================
 * WERSJA PROJEKTU
 * ========================================================= */

#define FW_NAME             "Boat_WaterTest"
#define FW_VERSION          "0.2.0"


/* =========================================================
 * PINOUT ESP32 WROOM-32D
 * ========================================================= */

// Odbiornik RC (ExpressLRS CRSF - HardwareSerial 1)
#define PIN_ELRS_RX         34      // Wejście ESP (Serial1 RX, input-only)
#define PIN_ELRS_TX         32      // Wyjście ESP (Serial1 TX, telemetria CRSF)

// Napęd i sterowanie (wyjścia PWM)
#define PIN_ESC             18      // Wyjście PWM dla regulatora Flycolor 90A
#define PIN_SERVO           19      // Wyjście PWM dla serwa steru

// GPS (HardwareSerial 2)
#define PIN_GPS_RX          16      // RX2 (podłącz do TX modułu GPS)
#define PIN_GPS_TX          17      // TX2 (podłącz do RX modułu GPS)

// Magistrala I2C (INA226 + BNO055)
#define PIN_I2C_SDA         21
#define PIN_I2C_SCL         22

// Dioda adresowalna NeoPixel (WS2812B)
#define PIN_NEOPIXEL        23
#define NUM_PIXELS          1


/* =========================================================
 * PROTOKÓŁ RC (ExpressLRS / CRSF)
 * ========================================================= */

#define ELRS_BAUDRATE       420000  // Domyślny baudrate CRSF

// Kanały CRSF (indeksowane 1-16)
#define CRSF_CH_STEER       1       // Kanał 1: Ster (CH1)
#define CRSF_CH_THRO        3       // Kanał 3: Gaz (CH3)
#define CRSF_CH_SB          6       // Kanał 6: Przełącznik SB (-100% DISARMED, 0% ARMED, 100% RECORDING)
#define CRSF_CH_SC          7       // Kanał 7: Przełącznik SC (Override limitu PWM do 100%)

// Wartości sygnałów RC po dekodowaniu do mikrosekund
#define RC_MIN_US           1000
#define RC_CENTER_US        1500
#define RC_MAX_US           2000

// Progi detekcji przełącznika SB (3-pozycyjny)
#define RC_SW_LOW_THRESHOLD_US   1300   // Poniżej: SB = -100% (DISARMED)
#define RC_SW_HIGH_THRESHOLD_US  1700   // Powyżej: SB = +100% (RECORDING), pomiędzy: ARMED

// Próg detekcji przełącznika SC (Override limitu PWM)
#define RC_SW_OVERRIDE_TH_US     1600   // Powyżej: no limits


/* =========================================================
 * ESC & SERVO
 * ========================================================= */

#define ESC_MIN_US          1000
#define ESC_DEADZONE_US     1180    // Poniżej tej wartości wyjście wymuszane na 1000 us
#define ESC_MAX_US          2000
#define ESC_ARM_US          1000
#define ESC_ARM_TIME_MS     3000    // Czas trzymania 1000 us podczas startu dla Flycolor
#define ESC_RAMP_STEP_US    20      // Maksymalny przyrost sygnału na cykl 20 ms (rampa rozbiegu)

#define SERVO_MIN_US        1000
#define SERVO_CENTER_US     1500
#define SERVO_MAX_US        2000

// Sprzętowy PWM LEDC (ESP32 Core v3.x - konfiguracja bezpośrednio na pinach)
#define PWM_FREQ_HZ         50      // 50 Hz dla ESC i serwa
#define PWM_BITS            16      // Rozdzielczość 16-bit


/* =========================================================
 * CZUJNIKI (INA226, BNO055, GPS)
 * ========================================================= */

// INA226
#define INA226_I2C_ADDR     0x40
#define INA226_SHUNT_OHMS   0.005f  // Bocznik R005 (5 mOhm)
#define INA226_MAX_CURRENT  25.0f   // Maksymalny oczekiwany prąd (A)
#define INA226_AVERAGING       INA226_AVERAGE_16
#define INA226_CONVERSION_TIME INA226_CONV_TIME_1100

// BNO055
#define BNO055_I2C_ADDR     0x28

// GPS
#define GPS_BAUDRATE        38400


/* =========================================================
 * BUFOR I LOGOWANIE
 * ========================================================= */

// 3000 próbek * 32 bajty = 96 000 B w pamięci RAM (10 minut testu przy 5 Hz)
#define MAX_SAMPLES         3000


/* =========================================================
 * HARMONOGRAM SCHEDULERA (millis)
 * ========================================================= */

#define TASK_RC_MS          20      // 50 Hz – odczyt CRSF
#define TASK_ACTUATORS_MS   20      // 50 Hz – aktualizacja LEDC i rampa
#define TASK_SENSOR_MS      100     // 10 Hz – INA226, BNO055, bufor GPS
#define TASK_LOG_MS         200     // 5 Hz  – zapis próbki do RAM
#define TASK_STATUS_MS      50      // 20 Hz – animacja NeoPixel


/* =========================================================
 * ZASILANIE I OCHRONA BATERII (LiPo 6S)
 * ========================================================= */

#define LIPO_CELLS          6

#define CELL_WARN_V         3.30f
#define CELL_CUTOFF_V       3.20f

#define VBAT_WARN_V         (CELL_WARN_V * LIPO_CELLS)    // 19.8 V
#define VBAT_CUTOFF_V       (CELL_CUTOFF_V * LIPO_CELLS)  // 19.2 V


/* =========================================================
 * OGRANICZENIA NAPĘDU I WARUNKI FAILSAFE
 * ========================================================= */

#define CURRENT_WARN_A      13.0f
#define CURRENT_FAIL_A      16.0f

#define POWER_WARN_W        280.0f
#define POWER_FAIL_W        330.0f

#define OVERLOAD_TIME_MS    250     // Czas przeciążenia wyzwalający Failsafe


/* =========================================================
 * BLUETOOTH
 * ========================================================= */

#define BT_DEVICE_NAME      "Boat_Telemetry"
