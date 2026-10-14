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

+++++

setup()
 ├─ initRC()
 ├─ initActuators()
 ├─ initSensors()
 ├─ initBT()
 └─ initStatus()

loop()
 ├─ updateRC()
 ├─ updateSensors()
 ├─ updateFSM()
 ├─ updateSafety()
 ├─ updateLog()
 ├─ updateBT()
 └─ updateStatus()