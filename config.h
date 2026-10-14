#pragma once

/* =========================================================
 * WERSJA PROJEKTU
 * ========================================================= */

#define FW_NAME        "Boat_WaterTest"
#define FW_VERSION     "0.1.0"


/* =========================================================
 * PINOUT ESP32
 * ========================================================= */

// Odbiornik RC
#define PIN_RC_THRO     34
#define PIN_RC_RUDD     35

// Napęd
#define PIN_ESC         25
#define PIN_SERVO       26

// GPS (UART2)
#define PIN_GPS_RX      16
#define PIN_GPS_TX      17

// I2C
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22

// RGB LED
#define PIN_LED_R       27
#define PIN_LED_G       14
#define PIN_LED_B       12


/* =========================================================
 * RC INPUT
 * ========================================================= */

#define RC_MIN_US       900
#define RC_CENTER_US    1500
#define RC_MAX_US       2100

#define RC_TIMEOUT_MS   200


/* =========================================================
 * ESC
 * ========================================================= */

#define ESC_MIN_US      1000
#define ESC_MAX_US      2000

#define ESC_ARM_US      1000
#define ESC_ARM_TIME_MS 3000


/* =========================================================
 * SERVO
 * ========================================================= */

#define SERVO_MIN_US    1000
#define SERVO_CENTER_US 1500
#define SERVO_MAX_US    2000


/* =========================================================
 * PWM (LEDC)
 * ========================================================= */

#define PWM_FREQ_HZ     50
#define PWM_BITS        16

#define PWM_CH_ESC      0
#define PWM_CH_SERVO    1


/* =========================================================
 * GPS
 * ========================================================= */

#define GPS_BAUDRATE    9600   //jeśli 5hz to większe baudrate?


/* =========================================================
 * LOGOWANIE
 * ========================================================= */

#define LOG_RATE_HZ     5

// około 10 min przy 5 Hz
#define MAX_SAMPLES     3000


/* =========================================================
 * SCHEDULER
 * ========================================================= */

// 50 Hz
#define TASK_RC_MS          20

// 10 Hz
#define TASK_SENSOR_MS     100

// 5 Hz
#define TASK_LOG_MS        200

// 2 Hz
#define TASK_STATUS_MS     500


/* =========================================================
 * ZASILANIE
 * ========================================================= */

#define LIPO_CELLS          6

#define CELL_WARN_V      3.30f
#define CELL_CUTOFF_V    3.20f

#define VBAT_WARN_V      (CELL_WARN_V * LIPO_CELLS)
#define VBAT_CUTOFF_V    (CELL_CUTOFF_V * LIPO_CELLS)


/* =========================================================
 * PRĄD / MOC
 * ========================================================= */

#define CURRENT_WARN_A      13.0f
#define CURRENT_FAIL_A      16.0f

#define POWER_WARN_W       280.0f
#define POWER_FAIL_W       330.0f

#define OVERLOAD_TIME_MS    250


/* =========================================================
 * FSM
 * ========================================================= */

#define ARM_DELAY_MS       2000


/* =========================================================
 * BLUETOOTH
 * ========================================================= */

#define BT_DEVICE_NAME "BoatWaterTest"


/* =========================================================
 * KOLORY LED
 * ========================================================= */

#define LED_OFF      0
#define LED_RED      1
#define LED_GREEN    2
#define LED_BLUE     3
#define LED_YELLOW   4
#define LED_CYAN     5
#define LED_MAGENTA  6
#define LED_WHITE    7