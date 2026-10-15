# Specyfikacja projektu: Boat_WaterTest (ESP32 / Arduino IDE)

**Wersja:** 15.10.2026  
**Platforma:** ESP32-WROOM-32D (NodeMCU / DevKit V1), Arduino IDE (ESP32 Core v3.3.7)  
**Cel:** Rejestracja parametrów pracy i efektywności energetycznej napędu łodzi podczas testów wodnych z możliwością badania różnych śrub napędowych przy zadanym limicie PWM gazu.  
**Architektura:** Proceduralna / modularna, jawny automat stanów (FSM), wspólna struktura danych `SystemData`, brak alokacji dynamicznej, statyczny bufor RAM, nieblokujący scheduler oparty na `millis()`.

---

# 1. Komponenty sprzętowe i magistrale

- **Mikrokontroler:** ESP32-WROOM-32D
- **Odbiornik RC:** BETAFPV SuperD ELRS 2.4 GHz (CRSF, Serial1, 420000 baud)
- **Aparatura:** RadioMaster Pocket (EdgeTX / ExpressLRS)

### Mapowanie kanałów RC

| Kanał | Funkcja |
|---------|---------|
| CH1 | Ster (Rudder) |
| CH3 | Gaz (Throttle) |
| CH5 | ARM |
| CH6 (SB) | Włączanie/wyłączanie Bluetooth |
| CH7 (SC) | Natychmiastowy override limitu PWM do 100% |

### Sensory

- **INA226** (I2C, bocznik 5 mΩ)
- **BNO055** (I2C)
- **GPS** (Serial2, 38400 baud, TinyGPS++)

### Elementy wykonawcze

- ESC Flycolor 90A
- Silnik A28M-IPX8 2838 350KV
- Serwo steru
- PWM generowane przez LEDC ESP32

### Interfejs użytkownika

- NeoPixel WS2812B ×1
- Bluetooth Classic SPP (`BluetoothSerial`)

---

# 2. Mapa pinów ESP32

| Nazwa | GPIO | Opis |
|---------|---------|---------|
| PIN_ELRS_RX | 34 | Serial1 RX |
| PIN_ELRS_TX | 32 | Serial1 TX |
| PIN_ESC | 18 | PWM ESC |
| PIN_SERVO | 19 | PWM serwo |
| PIN_GPS_RX | 16 | Serial2 RX |
| PIN_GPS_TX | 17 | Serial2 TX |
| PIN_I2C_SDA | 21 | SDA |
| PIN_I2C_SCL | 22 | SCL |
| PIN_NEOPIXEL | 23 | WS2812B |

---

# 3. Struktura projektu

```text
Boat_WaterTest/
├── Boat_WaterTest.ino
├── config.h
├── types.h
├── declarations.h
├── drv_RC.ino
├── drv_Actuators.ino
├── drv_Sensors.ino
├── logic_FSM.ino
├── logic_Safety.ino
├── log_Data.ino
├── hmi_BT.ino
└── hmi_Status.ino
```

## Boat_WaterTest.ino

- Inicjalizacja sprzętu.
- Definicje instancji bibliotek.
- Alokacja bufora logów.
- Implementacja nieblokującego schedulera.

## config.h

- Definicje pinów.
- Progi alarmowe.
- Parametry RC.
- Parametry schedulera.

## types.h

- Enumeracje stanów.
- Enumeracje failsafe.
- Struktury danych.

## declarations.h

- Prototypy wszystkich funkcji współdzielonych.

## drv_RC.ino

- Obsługa ELRS / CRSF.

## drv_Actuators.ino

- Obsługa ESC i serwa.

## drv_Sensors.ino

- Odczyt GPS.
- Odczyt INA226.
- Odczyt BNO055.

## logic_FSM.ino

- Automat stanów.

## logic_Safety.ino

- Monitor bezpieczeństwa.

## log_Data.ino

- Rejestracja danych.
- Eksport CSV.

## hmi_BT.ino

- Konsola Bluetooth.

## hmi_Status.ino

- Sygnalizacja NeoPixel.

---

# 4. Parametry konfiguracyjne (config.h)

## Kanały CRSF

```cpp
#define CRSF_CH_STEER      1
#define CRSF_CH_THRO       3
#define CRSF_CH_SW_SB      6
#define CRSF_CH_SW_SC      7
```

## Progi sterowania

```cpp
#define THRO_ARM_THRESHOLD_US      1080
#define ESC_DEADZONE_US            1180
#define ESC_MAX_US                 2000

#define RC_SW_LOW_THRESHOLD_US     1300
#define RC_SW_HIGH_THRESHOLD_US    1700
```

## LiPo 6S

```cpp
#define VBAT_WARN_V     19.8f
#define VBAT_FAIL_V     19.2f
```

## Ochrona przeciążeniowa

### Ostrzeżenie

```text
I > 13 A
lub
P > 280 W
```

### Failsafe

```text
I > 16 A
lub
P > 330 W

czas trwania > 250 ms
```

## Bufor próbek

```cpp
#define MAX_SAMPLES 3000
```

Przy strukturze 32 B oznacza to:

```text
3000 × 32 B = 96 000 B
≈ 96 kB RAM
```

## Scheduler

| Zadanie | Częstotliwość | Interwał |
|---------|---------|---------|
| RC | 50 Hz | 20 ms |
| Aktuatory | 50 Hz | 20 ms |
| Sensory | 10 Hz | 100 ms |
| Logowanie | 5 Hz | 200 ms |
| NeoPixel | 20 Hz | 50 ms |

---

# 5. Definicje typów (types.h)

## BoatState

```cpp
enum BoatState : uint8_t
{
    STATE_BOOT,
    STATE_DISARMED,
    STATE_ARMED,
    STATE_RECORDING,
    STATE_FAILSAFE,
    STATE_BT_DUMP
};
```

## FailsafeReason

```cpp
enum FailsafeReason : uint8_t
{
    FAILSAFE_NONE,
    FAILSAFE_RC_LOST,
    FAILSAFE_BATTERY_LOW,
    FAILSAFE_OVERCURRENT,
    FAILSAFE_OVERPOWER
};
```

## LogSample

Struktura rekordu logu:

```cpp
#pragma pack(push,1)

struct LogSample
{
    ...
};

#pragma pack(pop)

static_assert(sizeof(LogSample) == 32);
```

Założenia:

- napięcie → mV
- prąd → mA
- prędkość → cm/s
- kąty → 0.1°

## SystemData

Przechowuje:

- aktualny stan FSM,
- dane GPS,
- kurs IMU,
- napięcie,
- prąd,
- moc,
- kanały RC,
- LQ,
- RSSI,
- limit PWM,
- metadane testu,
- stan Bluetooth.

---

# 6. declarations.h

Wszystkie funkcje wywoływane między zakładkami muszą posiadać prototyp.

Przykłady:

```cpp
void processRC();
void processSensors();
void updateFSM();
void updateSafety();

void startBT();
void stopBT();

void dumpLog(Print& stream);
```

---

# 7. Boat_WaterTest.ino

## Bufor logów

```cpp
LogSample logBuffer[MAX_SAMPLES];

uint16_t sampleCount = 0;
```

## Obiekty globalne

```cpp
BluetoothSerial serialBT;
TinyGPSPlus gps;

Adafruit_NeoPixel statusLed(...);

INA226_WE ina226(...);
Adafruit_BNO055 bno(...);
```

## setup()

- USB Serial 115200
- GPS Serial2 38400
- CRSF Serial1 420000
- I2C 400 kHz
- LEDC PWM
- Bluetooth
- inicjalizacja sensorów
- `maxThrottlePwm = 2000`

## loop()

Wyłącznie scheduler oparty o `millis()`.

---

# 8. drv_RC.ino

Funkcje:

- odczyt CRSF,
- CH1,
- CH3,
- CH6,
- CH7,
- LQ,
- RSSI,
- `isLinkUp()`.

Przy utracie linku:

```text
Throttle → 1000 us
Ster → 1500 us
```

---

# 9. drv_Actuators.ino

PWM:

```cpp
ledcAttach(...)
ledcWrite(...)
```

Parametry:

```text
50 Hz
16 bit
```

## Uzbrajanie ESC

```text
1000 us przez 3000 ms
```

## Deadzone

```cpp
ESC_DEADZONE_US = 1180
```

Jeżeli sygnał niższy:

```text
wymuś 1000 us
```

## Silnik zawsze odcięty w:

- STATE_BOOT
- STATE_DISARMED
- STATE_FAILSAFE
- STATE_BT_DUMP

---

# 10. drv_Sensors.ino

## INA226

Odczyt:

- napięcie
- prąd
- moc

## BNO055

Odczyt:

```text
Yaw 0.0°...359.9°
```

## GPS

Dane:

- latitude
- longitude
- speed m/s
- course over ground
- liczba satelitów

---

# 11. FSM (logic_FSM.ino)

## Przejścia stanów

```text
BOOT
  └─> DISARMED

DISARMED
  └─> ARMED

ARMED
  └─> RECORDING

RECORDING
  └─> ARMED

ANY STATE
  └─> FAILSAFE

DISARMED / ARMED
  └─> BT_DUMP
```

## BOOT → DISARMED

Warunek:

```text
3000 ms
ESC = 1000 us
```

## DISARMED → ARMED

Warunki:

```text
RC Link OK
Brak alarmów
Throttle <= 1080 us
przez minimum 2000 ms
```

## ARMED → RECORDING

```text
Throttle > 1080 us
```

## RECORDING → ARMED

```text
Throttle <= 1080 us
```

lub

```text
sampleCount >= MAX_SAMPLES
```

---

# 12. Limit PWM i override SC

Standardowo:

```text
1080...2000 us
```

mapowane na:

```text
ESC_DEADZONE_US
...
maxThrottlePwm
```

## Przełącznik SC

Jeżeli:

```text
SC > 1700 us
```

to:

```text
limit PWM ignorowany
```

i:

```text
maksimum = 2000 us
```

---

# 13. Zarządzanie Bluetooth

## RECORDING

```text
Bluetooth OFF
```

## DISARMED / ARMED

Stan zgodny z przełącznikiem SB.

## FAILSAFE

```text
Bluetooth ON
```

## BT_DUMP

```text
Bluetooth ON
```

---

# 14. Safety

## Utrata RC

```text
Throttle = 1000 us
Ster = 1500 us
FAILSAFE_RC_LOST
```

## Niskie napięcie

```text
VBAT < 19.2 V
```

↓

```text
FAILSAFE_BATTERY_LOW
```

## Przeciążenie

```text
I > 16 A
```

lub

```text
P > 330 W
```

przez:

```text
>250 ms
```

↓

```text
FAILSAFE_OVERCURRENT
```

lub

```text
FAILSAFE_OVERPOWER
```

---

# 15. log_Data.ino

## Częstotliwość próbkowania

```text
5 Hz
```

tylko w stanie:

```text
STATE_RECORDING
```

## Format CSV

### Metadane

```text
# Firmware
# Propeller
# Note
# Total Samples
```

### Nagłówek danych

```csv
time_ms,lat,lon,vbat_v,current_a,power_w,speed_mps,yaw_deg,cog_deg,pwm_thro,pwm_rudder,sats,flags
```

## Zrzut

```cpp
dumpLog(Print& stream)
```

Obsługiwane strumienie:

```text
Serial
SerialBT
```

---

# 16. hmi_BT.ino

Nazwa urządzenia:

```text
Boat_Telemetry
```

## Komendy

### Ustawienie PWM

```text
PWM 1450
SET PWM 1450
```

### PWM procentowy

```text
PWM80
```

Przykład:

```text
PWM80
```

Zapisuje etykietę:

```text
pwm80
```

### Nazwa śruby

```text
SET PROP M445
```

### Notatka

```text
SET NOTE Test pod wiatr
```

### Status

```text
STATUS
```

### Zrzut CSV

```text
DUMP
```

### Kasowanie danych

```text
CLEAR
```

---

# 17. hmi_Status.ino

Wywoływane:

```text
20 Hz
co 50 ms
```

## Optymalizacja

```cpp
statusLed.show();
```

wyłącznie przy rzeczywistej zmianie RGB.

## Sygnalizacja

| Stan | Kolor |
|--------|--------|
| GPS brak fixa | bursztynowy oddech |
| BOOT | białe błyski |
| DISARMED | zielony oddech |
| ARMED | stały zielony |
| RECORDING | szybkie błyski fioletowo-niebieskie |
| FAILSAFE | szybkie czerwone miganie |
| BT_DUMP | stały niebieski |

---

# 18. Rygory implementacyjne

## Bezwzględny zakaz

```cpp
delay()
```

w:

- loop()
- modułach wykonawczych
- obsłudze sensorów
- obsłudze Bluetooth

Wyjątek stanowi jedynie ewentualna inicjalizacja sprzętu w `setup()`.

## Architektura Arduino IDE

### Tylko w Boat_WaterTest.ino

```cpp
#include <...>
```

oraz:

```cpp
BluetoothSerial
TinyGPSPlus
INA226_WE
Adafruit_BNO055
Adafruit_NeoPixel
```

### Pozostałe moduły

```cpp
#include "declarations.h"
```

i korzystanie z:

```cpp
extern
```

## Prototypy

Wszystkie funkcje współdzielone muszą zostać zadeklarowane w:

```cpp
declarations.h
```

---

# 19. Ograniczenia GPIO ESP32

## Niedozwolone

```text
GPIO 6-11
```

(zajęte przez SPI Flash)

## Niedozwolone dla ESC i serwa

```text
GPIO 0
GPIO 2
GPIO 5
GPIO 12
GPIO 15
```

(piny strappingowe)

## Wejścia tylko INPUT

```text
GPIO 34
GPIO 35
GPIO 36
GPIO 39
```

Dlatego:

```text
GPIO34 = CRSF RX
```

jest poprawnym wykorzystaniem.

---

# 20. Założenia architektoniczne końcowe

- Brak dynamicznej alokacji pamięci.
- Całość działa deterministycznie w oparciu o `millis()`.
- Wszystkie dane wymieniane przez strukturę `SystemData`.
- Rejestracja wyłącznie do RAM.
- Eksport danych przez Bluetooth SPP lub USB.
- Bluetooth automatycznie wyłączany podczas testu na wodzie.
- Przełącznik SC zapewnia natychmiastowe obejście limitu PWM.
- Failsafe zawsze ma najwyższy priorytet względem logowania, Bluetooth i FSM.
