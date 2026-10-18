// ============================================================================
// PLIK: config.h
// ============================================================================
#pragma once

// --- WERSJA PROJEKTU ---
#define FW_NAME                 "Boat_WaterTest"
#define FW_VERSION              "0.3.0"

// --- PINOUT ESP32-WROOM-32D ---
#define PIN_ELRS_RX             34
#define PIN_ELRS_TX             32
#define PIN_ESC                 18
#define PIN_SERVO               19
#define PIN_GPS_RX              16
#define PIN_GPS_TX              17
#define PIN_I2C_SDA             21
#define PIN_I2C_SCL             22
#define PIN_NEOPIXEL            23
#define NUM_PIXELS              1

// --- EXPRESSLRS / CRSF ---
#define ELRS_BAUDRATE           420000
#define CRSF_CH_STEER           1
#define CRSF_CH_THRO            3
#define CRSF_CH_SB              6
#define CRSF_CH_SC              7
#define RC_MIN_US               1000
#define RC_CENTER_US            1500
#define RC_MAX_US               2000
#define RC_SB_LOW_THRESHOLD_US  1300
#define RC_SB_HIGH_THRESHOLD_US 1700
#define RC_SC_HIGH_THRESHOLD_US 1600    // Powyżej: no limits
#define THRO_ARM_THRESHOLD_US   1050
#define ARM_DELAY_MS            2000

// --- ESC I SERWO ---
#define ESC_MIN_US              1000
#define ESC_ARM_US              1000
#define ESC_DEADZONE_US         1180
#define ESC_MAX_US              2000
#define ESC_ARM_TIME_MS         3000
#define ESC_RAMP_STEP_US        20
#define SERVO_MIN_US            1000
#define SERVO_CENTER_US         1500
#define SERVO_MAX_US            2000

// --- PWM LEDC ---
#define PWM_FREQ_HZ             50
#define PWM_BITS                16

// --- INA226 ---
#define INA226_I2C_ADDR         0x40
#define INA226_SHUNT_OHMS       0.005f
#define INA226_MAX_CURRENT_A    25.0f
#define INA226_AVERAGING        INA226_AVERAGE_16
#define INA226_CONVERSION_TIME  INA226_CONV_TIME_1100

// --- BNO055 & GPS ---
#define BNO055_I2C_ADDR         0x28
#define GPS_BAUDRATE            38400

// --- BUFOR LOGOWANIA (3000 rekordów x 32 B = 96 000 B) ---
#define MAX_SAMPLES             3000

// --- SCHEDULER - INTERWAŁY W MILISEKUNDACH ---
#define TASK_RC_MS              20
#define TASK_ACTUATORS_MS       20
#define TASK_SENSOR_MS          100
#define TASK_LOG_MS             200
#define TASK_STATUS_MS          50

// --- OCHRONA NAPIĘCIOWA PAKIETU 6S ---
#define LIPO_CELLS              6
#define CELL_WARN_V             3.30f
#define CELL_CUTOFF_V           3.20f
#define VBAT_WARN_V             (CELL_WARN_V * LIPO_CELLS)
#define VBAT_CUTOFF_V           (CELL_CUTOFF_V * LIPO_CELLS)

// --- OCHRONA PRĄDOWA I MOCOWA ---
#define CURRENT_WARN_A          13.0f
#define CURRENT_FAIL_A          16.0f
#define POWER_WARN_W            280.0f
#define POWER_FAIL_W            330.0f
#define OVERLOAD_TIME_MS        250

// --- BLUETOOTH ---
#define BT_DEVICE_NAME          "Boat_Telemetry"
