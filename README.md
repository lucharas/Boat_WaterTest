**Boat_WaterTest - specyfikacja projektu**
ver.17.10.2026

**Cel**

Rejestracja parametrów pracy i efektywności energetycznej napędu łodzi podczas testów różnych śrub i limitów PWM. Dane są zapisywane do RAM i zrzucane przez USB lub Bluetooth.

**Platforma i architektura**

- ESP32-WROOM-32D, Arduino IDE, ESP32 Core 3.3.7
- Architektura proceduralna i modularna
- Jawna maszyna stanów
- Wspólna struktura SystemData
- Brak dynamicznej alokacji pamięci
- Statyczny bufor logów w RAM
- Scheduler nieblokujący oparty na millis()
- Bez delay() poza konieczną inicjalizacją w setup()

**Sprzęt**

- Odbiornik BETAFPV SuperD ELRS 2.4 GHz, CRSF, Serial1, 420000 baud
- Aparatura RadioMaster Pocket, EdgeTX, ExpressLRS
- INA226, I2C, bocznik 5 mΩ
- BNO055, I2C
- GPS, Serial2, 38400 baud, TinyGPS++
- ESC Flycolor 90 A
- Silnik A28M-IPX8 2838 350 KV
- Serwo steru
- NeoPixel WS2812B, 1 szt.
- Bluetooth Classic SPP, BluetoothSerial

**Kanały RC**

- CH1 - ster
- CH3 - gaz
- CH6, SB - sterowanie stanami: -100% DISARMED, 0% ARMED, 100% RECORDING
- CH7, SC - natychmiastowe obejście limitu PWM do 100%

**Piny ESP32**

- GPIO34 - CRSF RX
- GPIO32 - CRSF TX
- GPIO18 - ESC
- GPIO19 - serwo
- GPIO16 - GPS RX
- GPIO17 - GPS TX
- GPIO21 - I2C SDA
- GPIO22 - I2C SCL
- GPIO23 - NeoPixel

- GPIO6-11 są zajęte przez pamięć Flash
- GPIO0, 2, 5, 12 i 15 nie mogą sterować ESC ani serwem
- GPIO34 jest tylko wejściem, więc nadaje się do CRSF RX

**Struktura projektu**

- Boat_WaterTest.ino - setup(), obiekty globalne, bufor logów i scheduler
- config.h - piny, progi, parametry RC i czasy zadań
- types.h - stany, przyczyny failsafe i struktury danych
- declarations.h - prototypy funkcji używanych między zakładkami
- drv_RC.ino - CRSF, kanały, LQ, RSSI i stan linku
- drv_Actuators.ino - ESC i serwo
- drv_Sensors.ino - GPS, INA226 i BNO055
- logic_FSM.ino - maszyna stanów
- logic_Safety.ino - zabezpieczenia
- log_Data.ino - zapis próbek i eksport CSV
- hmi_BT.ino - konsola Bluetooth i komendy
- hmi_Status.ino - NeoPixel

**Stany**

- STATE_BOOT - inicjalizacja i uzbrajanie ESC
- STATE_DISARMED - silnik odcięty, oczekiwanie na SB = 0%
- STATE_ARMED - sterowanie aktywne, bez zapisu próbek
- STATE_RECORDING - sterowanie aktywne i zapis danych
- STATE_FAILSAFE - awaryjne odcięcie silnika
- STATE_BT_DUMP - tryb serwisowy; priorytetem jest obsługa Bluetooth

**Przejścia stanów**

- Zasilanie lub reset prowadzi do STATE_BOOT
- STATE_BOOT zawsze przechodzi automatycznie do STATE_DISARMED po ESC_ARM_TIME
- STATE_DISARMED przechodzi do STATE_BT_DUMP po komendzie BTDUMP odebranej przez Bluetooth
- STATE_BT_DUMP wraca do STATE_DISARMED po komendzie EXIT
- STATE_DISARMED przechodzi do STATE_ARMED, gdy link RC jest poprawny i SB = 0%
- STATE_ARMED przechodzi do STATE_RECORDING, gdy SB = 100%
- STATE_ARMED przechodzi do STATE_DISARMED, gdy SB = -100%
- STATE_RECORDING przechodzi do STATE_ARMED, gdy SB = 0% albo bufor RAM jest pełny
- STATE_RECORDING przechodzi do STATE_DISARMED, gdy SB = -100%
- Utrata RC albo krytyczne napięcie, prąd lub moc powodują przejście do STATE_FAILSAFE
- STATE_FAILSAFE wraca do STATE_DISARMED dopiero po ustaniu błędu, odzyskaniu linku i ustawieniu SB = -100%

Failsafe ma pierwszeństwo przed maszyną stanów, logowaniem i Bluetooth.

**Sterowanie ESC i serwem**

- PWM: LEDC, 50 Hz, 16 bit
- ESC podczas startu otrzymuje 1000 us przez ESC_ARM_TIME, domyślnie 3000 ms
- ESC_DEADZONE_US = 1180
- ESC_MAX_US = 2000
- Poniżej deadzone wyjście ESC jest wymuszane na 1000 us
- Wzrost sygnału ESC jest ograniczony rampą rozbiegu; jej wartość zostanie dobrana podczas testów
- Zmniejszenie gazu może być natychmiastowe; DISARMED i FAILSAFE zawsze odcinają silnik bez rampy
- W STATE_BOOT, STATE_DISARMED, STATE_FAILSAFE i STATE_BT_DUMP silnik jest zawsze odcięty
- Po utracie RC gaz = 1000 us, ster = 1500 us

**Limit PWM**

- Maksymalne PWM jest ustawiane komendą Bluetooth i przechowywane w SystemData
- Zakres gazu jest mapowany do zakresu ESC_DEADZONE_US...maxThrottlePwm
- Gdy SC jest w pozycji wysokiej, limit jest pomijany, a maksimum wynosi 2000 us
- Po resecie domyślny maxThrottlePwm wynosi 2000 us

**Sensory i dane bieżące**

- GPS: pozycja, prędkość, COG, liczba satelitów, HDOP, wiek fixa i ważność fixa
- BNO055: kurs 0,0...359,9 stopnia oraz stan sensora
- INA226: napięcie, prąd i moc
- RC: kanały, stan linku, LQ i RSSI
- SystemData przechowuje również stan FSM, limit PWM, stan Bluetooth i metadane testu

**Bezpieczeństwo**

- Ostrzeżenie: prąd > 13 A lub moc > 280 W
- Failsafe: napięcie < 19,2 V
- Failsafe: prąd > 16 A lub moc > 330 W przez ponad 250 ms
- Przyczyna failsafe jest zapisywana w FailsafeReason

**Logowanie**

- Logowanie działa tylko w STATE_RECORDING
- Częstotliwość zapisu: 5 Hz
- MAX_SAMPLES = 3000
- Rekord ma docelowo 32 B, więc bufor zajmuje około 96 kB RAM
- Wartości całkowite: napięcie w mV, prąd w mA, prędkość w cm/s, kąty w 0,1 stopnia
- Zapełnienie bufora kończy nagrywanie i przełącza system do STATE_ARMED

- Metadane CSV: wersja firmware, śruba, ustawiony limit PWM, notatka i liczba próbek
- Dane CSV: czas, pozycja, napięcie, prąd, moc, prędkość, yaw, COG, PWM gazu, PWM steru, satelity, HDOP i wiek danych GPS
- Zrzut jest możliwy przez USB Serial lub Bluetooth

**Struktura próbki**

```cpp
struct LogSample {
    uint32_t timestampMs;
    int32_t  lat;
    int32_t  lon;
    uint16_t voltageMv;
    int16_t  currentMa;
    uint16_t speedCms;
    uint16_t headingDeg10;
    uint16_t cogDeg10;
    uint16_t pwmThrottle;
    uint16_t pwmRudder;
    uint8_t  satellites;
    uint16_t hdop100;
    uint8_t  gpsAge100ms;
    uint16_t reserved; 
};
```

- lat i lon są skalowane przez 1e-6
- headingDeg10 i cogDeg10 przechowują kąt w 0,1 stopnia
- hdop100 przechowuje HDOP pomnożony przez 100, np. 0,85 jako 85
- gpsAge100ms przechowuje wiek danych GPS w jednostkach 100 ms; wartość jest ograniczana do 255, czyli 25,5 s
- Liczba satelitów, HDOP i wiek danych pozwalają ocenić jakość każdej próbki GPS
- Rezerwa wyrównująca do pełnych 32 bajtów (2 B)
- Struktura zajmuje 32 B bez pola flag
- Rozmiar należy potwierdzić przez static_assert(sizeof(LogSample) == 32)

**Bluetooth i komendy**

- Nazwa urządzenia: Boat_Telemetry
- Bluetooth służy do konfiguracji, diagnostyki i zrzutu danych
- W STATE_DISARMED Bluetooth nasłuchuje komendy BTDUMP
- W STATE_BT_DUMP sterowanie silnikiem i serwem oraz zapis danych są wyłączone
- Bluetooth jest wyłączony podczas STATE_RECORDING

- BTDUMP - przechodzi z STATE_DISARMED do STATE_BT_DUMP
- STATUS - zwraca stan FSM, RC, GPS, MAG, V_BAT, prąd, moc, zajętość bufora, limit PWM i dane śruby
- PWM1450 - ustawia maksymalne PWM ESC
- PROP65 - zapisuje oznaczenie lub rozmiar śruby w metadanych logu
- NOTEipsumipsum - zapisuje krótką notatkę w metadanych
- DUMP - wysyła metadane i dane CSV
- CLEAR - zeruje licznik próbek i logicznie czyści bufor
- EXIT - kończy STATE_BT_DUMP i przechodzi do STATE_DISARMED
- HELP - wyświetla krótką listę komend

Komendy mają jedną stałą składnię, bez spacji i aliasów. Wielkość liter nie ma znaczenia.

**Ustawienia testu**

- Limit PWM, oznaczenie śruby i notatka są przechowywane tylko w RAM
- Wejście do STATE_BT_DUMP, wykonanie DUMP i komenda CLEAR nie zmieniają tych ustawień
- CLEAR zeruje wyłącznie licznik próbek i logicznie czyści bufor logu
- Po resecie ustawienia wracają do wartości domyślnych

**Scheduler**

- RC - 50 Hz, 20 ms
- Aktuatory - 50 Hz, 20 ms
- Sensory - 10 Hz, 100 ms
- Logowanie - 5 Hz, 200 ms
- NeoPixel - 20 Hz, 50 ms
- Bluetooth w STATE_DISARMED i STATE_BT_DUMP - obsługiwany w każdej iteracji loop(), bez czekania na wolny interwał

**Sygnalizacja NeoPixel**

- STATE_BOOT - białe błyski
- STATE_DISARMED - zielony oddech
- STATE_ARMED - stały zielony
- STATE_RECORDING - szybkie błyski fioletowo-niebieskie
- STATE_FAILSAFE - szybkie czerwone miganie
- STATE_BT_DUMP - stały niebieski
- Brak fixa GPS może modyfikować sygnalizację stanu, ale nie powinien całkowicie zastępować informacji o stanie FSM
- statusLed.show() jest wywoływane tylko po zmianie koloru

**Rygory implementacyjne**

- loop() zawiera wyłącznie wywołania schedulera i obsługę zadań
- Brak delay() w loop(), sterowaniu, sensorach i Bluetooth
- Wszystkie funkcje używane między zakładkami mają prototyp w declarations.h
- Obiekty bibliotek i bufory są definiowane jeden raz w Boat_WaterTest.ino, a w pozostałych plikach udostępniane przez extern
- Rejestracja odbywa się wyłącznie do RAM
- Nie używać przerwań, dopóki pomiary nie wykażą, że scheduler nie spełnia wymagań czasowych


