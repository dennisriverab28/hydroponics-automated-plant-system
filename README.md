# Automated Hydroponics System

**Course:** ECE 4380 — Embedded Systems, Texas Tech University  
**Team:** Alex (SlashAlexx) · Dennis Rivera  
**Hardware:** STM32L476 + ESP32 · Dual-MCU architecture

---

## Overview

A dual-microcontroller automated plant watering and grow light management system. The STM32L476 runs a custom FreeRTOS firmware that handles all sensor reading, pump control, and LED management. It communicates over UART with an ESP32, which hosts a WiFi web dashboard accessible from any browser on the network. The dashboard provides real-time sensor graphs, a watering log, and manual controls for the pump and grow lights.

---

## System Architecture

```
STM32L476 (Firmware / Control)
    │
    ├── ADC ──── Capacitive Soil Moisture Sensor
    ├── I2C ──── INA219 Power Monitor (grow light current/voltage)
    ├── GPIO ─── Relay → Water Pump
    ├── PWM ──── WS2812B RGB LED Strip (grow lights)
    └── UART ──► ESP32 (JSON messages)
                    │
                    └── WiFi → Browser Dashboard
                              ├── Live moisture graph
                              ├── LED power graph
                              ├── Moisture gauge
                              ├── Watering log
                              ├── Manual pump button
                              └── LED brightness control
```

---

## Hardware

| Component | Detail |
|-----------|--------|
| STM32 MCU | STM32L476RGTx (ARM Cortex-M4) |
| WiFi MCU | ESP32 |
| Moisture Sensor | Capacitive soil moisture sensor (ADC) |
| Power Monitor | INA219 (I2C) — monitors grow light current/voltage |
| Pump | Water pump controlled via relay |
| Grow Lights | WS2812B addressable RGB LED strip |
| Communication | STM32 → ESP32 via UART (JSON packets) |

---

## Features

### STM32 Firmware
- **FreeRTOS** — multitask RTOS managing sensor reads, state machine, and UART communication concurrently
- **State machine** with 6 states: Init, Auto, Manual, MoistureUpdate, PowerCheck, Tick
- **Auto watering** — reads soil moisture, runs pump for 2 seconds when soil is dry
- **Overcurrent protection** — INA219 monitors grow light current; flags fault and flashes error LED if >1.5A
- **WS2812B RGB LED control** — addressable color control of grow light strip via custom PWM driver (`ws28xx.c`)
- **UART JSON messaging** — sends live sensor data to ESP32 as JSON (`{"MoistureLevel": 84.0}`)
- **UART command reception** — receives `AUTO`, `MANUAL`, `LED_FULL`, `LED_DIM0`, `PUMP_ON`, `PUMP_OFF` commands from ESP32

### ESP32 Dashboard (Web App)
- **WiFi-hosted** — browser-based interface accessible at `http://<ESP32-IP>/`
- **WebSocket** — real-time bidirectional communication with auto-reconnect
- **Moisture graph** — 50-point rolling line chart of soil moisture over time
- **LED power graph** — 50-point rolling line chart of grow light power (mW)
- **Moisture gauge** — color-gradient canvas bar (red → yellow → green → blue) with live marker
- **LED brightness control** — toggle button: Full brightness (`LED_FULL`) / Dimmed (`LED_DIM0`)
- **Manual water button** — sends `PUMP_ON`, auto-disables for 2 seconds, then sends `PUMP_OFF`
- **Watering log** — timestamped history of all watering events (auto + manual), newest-first
- **Live clock** — 12-hour real-time clock on dashboard
- **SPIFFS file system** — dashboard HTML/CSS/JS served from ESP32 flash filesystem

---

## Project Structure

```
Hydroponics_Automated_Plant_System/
├── src/
│   ├── STM32/                          # STM32L476 firmware (STM32CubeIDE)
│   │   ├── Core/
│   │   │   ├── Inc/
│   │   │   │   ├── grow_led.h          # Grow light LED control
│   │   │   │   ├── ina219.h            # INA219 power monitor driver
│   │   │   │   ├── moisture_model.h    # Moisture averaging model
│   │   │   │   ├── moisture_sensor.h   # ADC moisture reading
│   │   │   │   ├── relay_pump.h        # Water pump relay
│   │   │   │   ├── rtos.h              # FreeRTOS init
│   │   │   │   ├── rtos_tasks.h        # Task definitions
│   │   │   │   ├── system_state.h      # State machine
│   │   │   │   ├── uart_comms.h        # STM32↔ESP32 UART
│   │   │   │   ├── uart_queue.h        # UART message queue
│   │   │   │   ├── ws28xx.h            # WS2812B RGB LED driver
│   │   │   │   └── ...
│   │   │   └── Src/
│   │   │       ├── main.c              # Entry point
│   │   │       ├── grow_led.c          # LED brightness + color control
│   │   │       ├── ina219.c            # I2C power monitor driver
│   │   │       ├── moisture_model.c    # Moisture level averaging
│   │   │       ├── moisture_sensor.c   # ADC soil moisture read
│   │   │       ├── relay_pump.c        # Pump relay driver
│   │   │       ├── rtos.c              # FreeRTOS task init
│   │   │       ├── rtos_tasks.c        # All RTOS task implementations
│   │   │       ├── system_state.c      # 6-state state machine
│   │   │       ├── uart_comms.c        # JSON UART send/receive
│   │   │       ├── uart_queue.c        # UART DMA/interrupt queue
│   │   │       ├── ws28xx.c            # WS2812B bit-bang PWM driver
│   │   │       └── ...
│   │   ├── Drivers/                    # STM32L4 HAL + CMSIS
│   │   ├── hardware_verification.ioc   # STM32CubeMX config
│   │   └── STM32L476RGTX_FLASH.ld
│   │
│   └── ESP32/PlantWatering/            # ESP32 Arduino project
│       ├── PlantWatering.ino           # Main sketch
│       ├── network.cpp / network.hpp   # WiFi + WebSocket server
│       ├── FileSystem.cpp / .hpp       # SPIFFS file system
│       └── data/                       # Dashboard files (flashed to SPIFFS)
│           ├── index.html              # Dashboard page
│           ├── script.js               # Graphs, controls, WebSocket client
│           ├── style.css               # Dashboard styling
│           ├── auth.json               # Auth config
│           └── watering-data.json      # Persistent watering history
│
└── README.md
```

---

## State Machine

| State | Description |
|-------|-------------|
| `State_Init` | Set default LED color, read initial moisture, switch to AUTO |
| `State_Auto` | Auto water if dry (pump 2s), skip if wet or in fail state |
| `State_Manual` | Auto watering disabled — pump and LEDs controlled from dashboard |
| `State_UpdateMoisture` | Read raw and averaged ADC; update global moisture value |
| `State_UpdatePower` | Read INA219 voltage/current/power; flag overcurrent >1.5A |
| `State_StateMachineTick` | Periodic tick — dispatches to Auto or Manual state |

---

## Build & Run

### STM32 (STM32CubeIDE)
1. Open STM32CubeIDE → `File > Open Projects from File System` → select `src/STM32/`
2. Build: `Project > Build All` (`Ctrl+B`)
3. Flash with ST-Link: `Run > Debug`
4. Serial Monitor at **115200 baud** for debug output

### ESP32 (Arduino IDE)
1. Open `src/ESP32/PlantWatering/PlantWatering.ino`
2. Install libraries via Library Manager:
   - `AsyncTCP`
   - `ESPAsyncWebServer`
   - `ArduinoJson`
3. Upload SPIFFS data files using **ESP32 Sketch Data Upload** tool
4. Upload firmware → connect ESP32 via USB → click Upload
5. Check Serial Monitor for the ESP32's local IP address

### Access Dashboard
```
http://<ESP32-IP>/
```
Open in any browser on the same network.

---

## UART Protocol

STM32 sends JSON to ESP32:
```json
{"MoistureLevel": 84.0}
```

ESP32 sends commands to STM32:
| Command | Action |
|---------|--------|
| `AUTO` | Switch to automatic watering mode |
| `MANUAL` | Switch to manual mode |
| `LED_FULL` | Set grow lights to full brightness |
| `LED_DIM0` | Dim grow lights |
| `PUMP_ON` | Turn pump on |
| `PUMP_OFF` | Turn pump off |
