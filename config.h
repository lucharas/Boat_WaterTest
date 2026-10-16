#pragma once

/* =========================================================
 * WERSJA PROJEKTU
 * ========================================================= */

#define FW_NAME             "Boat_WaterTest"
#define FW_VERSION          "0.1.0"


/* =========================================================
 * PINOUT ESP32 WROOM-32D
 * ========================================================= */

// Odbiornik RC (ExpressLRS CRSF - HardwareSerial 1)
#define PIN_ELRS_RX         34      // Wejście ESP (podłącz do TX odbiornika SuperD)
#define PIN_ELRS_TX         32      // Wyjście ESP (podłącz do RX odbiornika SuperD pod telemetrię)

// Napęd i sterowanie (wyjścia PWM)
#define PIN_ESC             18      // Bezpieczny pin wyjściowy dla regulatora
#define PIN_SERVO           19      // Bezpieczny pin wyjściowy dla serwa steru

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
 * PROTOKÓŁ RC (ExpressLRS / CRSF - RadioMaster Pocket)
 * ========================================================= */

#define ELRS_BAUDRATE       420000  // Domyślny baudrate protokołu CRSF

// Kanały CRSF (indeksowane standardowo 1-16)
#define CRSF_CH_STEER       1       // Kanał 1: Ster
#define CRSF_CH_THRO        3       // Kanał 3: Gaz (Throttle)
// Uwaga: CH5 (AUX1) w ELRS zarezerwowany jest pod stan ARM
#define CRSF_CH_SW_SB       6       // Kanał 6 (AUX2): Przełącznik SB (3-pozycyjny) -> Włącznik/Wyłącznik BT
#define CRSF_CH_SW_SC       7       // Kanał 7 (AUX3): Przełącznik SC (3-pozycyjny) -> Zapasowy / funkcyjny

// Alias semantyczny dla kodu logiki
#define CRSF_CH_BT_SWITCH   CRSF_CH_SW_SB

// Wartości kanałów po dekodowaniu do standardu mikrosekund
#define RC_MIN_US           1000
#define RC_CENTER_US        1500
#define RC_MAX_US           2000

#define THRO_ARM_THRESHOLD_US 1080  // Powyżej tego progu startuje rejestracja (STATE_RECORDING)

// Progi detekcji stanów przełączników 3-pozycyjnych
#define RC_SW_LOW_THRESHOLD_US  1300 // Poniżej 1300 us = Pozycja LOW (np. BT OFF)
#define RC_SW_HIGH_THRESHOLD_US 1700 // Powyżej 1700 us = Pozycja HIGH (np. BT ON)


/* =========================================================
 * ESC & SERVO
 * ========================================================= */

#define ESC_MIN_US          1000
#define ESC_MAX_US          2000
#define ESC_ARM_US          1000
#define ESC_DEADZONE_US     1180    // Poniżej silnik może nie kręcić stabilnie
#define ESC_ARM_TIME_MS     3000    // Czas trzymania 1000 us na start dla Flycolor

#define SERVO_MIN_US        1000
#define SERVO_CENTER_US     1500
#define SERVO_MAX_US        2000

// Konfiguracja sprzętowa PWM LEDC (50 Hz / 16-bit)
#define PWM_FREQ_HZ         50
#define PWM_BITS            16

#define PWM_CH_ESC          0
#define PWM_CH_SERVO        1


/* =========================================================
 * CZUJNIKI (INA226, BNO055, GPS)
 * ========================================================= */

// INA226
#define INA226_I2C_ADDR     0x40
#define INA226_SHUNT_OHMS   0.005f  // Bocznik R005 (5 mOhm)
#define INA226_MAX_CURRENT  25.0f   // Maksymalny oczekiwany prąd (A)

// BNO055
#define BNO055_I2C_ADDR     0x28

// GPS (Dla odświeżania 5–10 Hz zalecane 38400 lub 115200 baud)
#define GPS_BAUDRATE        38400


/* =========================================================
 * BUFOR I LOGOWANIE
 * ========================================================= */

#define LOG_RATE_HZ         5

// 3000 próbek * 32 bajty = ~96 KB w pamięci RAM (ok. 10 minut testu)
#define MAX_SAMPLES         3000


/* =========================================================
 * HARMONOGRAM SCHEDULERA (millis)
 * ========================================================= */

#define TASK_RC_MS          20      // 50 Hz – odczyt CRSF, FSM, aktualizacja LEDC
#define TASK_SENSOR_MS      100     // 10 Hz – INA226, BNO055, parsowanie bufora GPS
#define TASK_LOG_MS         200     // 5 Hz  – zapis próbki do logBuffer w RAM
#define TASK_STATUS_MS      50      // 20 Hz – płynna animacja/pulsowanie NeoPixel


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

#define OVERLOAD_TIME_MS    250     // Czas przekroczenia progu wyzwalający Failsafe


/* =========================================================
 * FSM & BLUETOOTH
 * ========================================================= */

#define ARM_DELAY_MS        2000    // Czas gazu na minimum wymagany do przejścia w ARMED
#define BT_DEVICE_NAME      "Boat_Telemetry"