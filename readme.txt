Boat_WaterTest.ino
│
├── config.h
├── types.h
├── declarations.h
│
├── drv_RC.ino
├── drv_Actuators.ino
├── drv_Sensors.ino
│
├── logic_FSM.ino
├── logic_Safety.ino
│
├── log_Data.ino
│
├── hmi_BT.ino
└── hmi_Status.ino

setup()
 ├─ Serial.begin(115200)
 ├─ initActuators()
 ├─ initStatus()
 ├─ initSensors()
 ├─ initRC()
 ├─ initLog()
 ├─ initBT()
 └─ initFSM()

loop()
 ├─ [co 20 ms]  updateRC() -> updateSafety() -> updateFSM()
 ├─ [co 20 ms]  updateActuators()
 ├─ [co 100 ms] updateSensors()
 ├─ [co 200 ms] addSample()
 ├─ [co 50 ms]  updateStatus()
 └─ [co obieg]  updateBT()