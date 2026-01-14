# gabriel-teleop-platform

A Modular Heterogeneous Teleoperation System

## About

Project Gabriel is a modular teleoperation architecture designed to control ground vehicles. The core philosophy is interchangeability: no single technology or hardware component is hard-coded into the architecture.

Every module, from the high-level web framework to the low-level motor driver, is treated as a "black box" defined solely by its inputs and outputs. This enables seamless substitution:

**Software Agnostic**

You can replace the Angular frontend with React or Vue, or rewrite the C++ backend in Rust or Go. As long as the new module respects the defined interfaces, the rest of the system remains unaffected.

**Hardware Independence**

The architecture does not demand specific boards. You can upgrade the Raspberry Pi (Layer 3) to an NVIDIA Jetson for better AI performance, or swap the Arduino (Layer 4) for an STM32 or ESP32. If the replacement implements the correct Serial Protocol and Pinout interface, it will function immediately.

## Overview   

```mermaid
flowchart TD
    %% -- Styling --
    classDef class_ui fill:#0288d1,stroke:#01579b,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_video fill:#7b1fa2,stroke:#4a148c,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_backend fill:#43a047,stroke:#1b5e20,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_control fill:#c62828,stroke:#b71c1c,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_db fill:#00897b,stroke:#004d40,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_fw fill:#fb8c00,stroke:#e65100,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_hw fill:#546e7a,stroke:#263238,stroke-width:2px,color:white,font-size:16px,font-weight:bold;

    %% -- LAYER 1: CENTRAL SOFTWARE --
    subgraph L1 ["Layer_1:_Central_Software"]
        direction LR
        UI["Web User Interface"]:::class_ui
        Video["Video Server"]:::class_video
        Backend["Backend"]:::class_backend
        Control["Control Center"]:::class_control
        DB[("Database")]:::class_db
    end

    %% -- LAYER 2: DIRECTION SOFTWARE --
    subgraph L2 ["Layer_2:_Direction_Software"]
        FW["Real Time Direction Firmware"]:::class_fw
    end

    %% -- LAYER 3: CENTRAL HARDWARE --
    subgraph L3 ["Layer_3:_Central_Hardware"]
        direction LR
        CamHW["Camera"]:::class_hw
        SBC["Single Board Computer<br/>(e.g., Raspberry Pi / Orange Pi)"]:::class_hw
    end

    %% -- LAYER 4: DIRECTION HARDWARE --
    subgraph L4 ["Layer_4:_Direction_Hardware"]
        direction LR
        MCU["Microcontroller<br/>(e.g., Arduino / Pico)"]:::class_hw
        Driver["Motor Driver<br/>(H-Bridge)"]:::class_hw
        Servo["Servomotor<br/>(Steering)"]:::class_hw
        DC["DC Motor<br/>(Traction)"]:::class_hw
    end

    %% -- CONNECTIONS --
    
    %% UI -> Host
    UI <==>|"WebSocket / REST"| Backend

    %% Host Internal
    Video == "Shared Mem" ==> Backend
    Backend <-->|"Shared Mem"| Control
    Backend <--> DB

    %% Host -> FW
    Control <==>|"Serial Protocol"| FW

    %% FW -> Hardware
    FW ==> MCU

    %% Hardware Physical Chain
    CamHW ==>|"USB"| SBC
    SBC <==>|"USB Cable"| MCU
    
    %% DETAILED MOTOR CONTROL SIGNALS
    MCU ==>|"PWM (Speed) +<br/>DIR (Reverse)"| Driver
    MCU ==>|"PWM"| Servo
    
    Driver ==>|"Power"| DC

    %% Camera Source Link
    CamHW -.->|Source| Video

    %% -- LINK STYLING --
    linkStyle default font-size:14px;
```

## Architecture

The system is organized into 4 distinct layers, moving from high-level user interaction down to physical voltage modulation:

### Layer 1: Central Software

**Modules:**
- Web User Interface
- Video Server
- Backend  
- Control Center
- Database

**Purpose:** The "Brain" of the vehicle. It orchestrates the web dashboard (WebSocket/REST), video streaming, backend logic, and control coordination running on a Linux SBC.

### Layer 2: Direction Software

**Module:**
- Real Time Direction Firmware

**Purpose:** A dedicated Real-Time Firmware module that translates abstract commands (e.g., "Direction, Angle, Acceleration") into precise electrical signals, ensuring safety limits and smooth control.

### Layer 3: Central Hardware

**Modules:**
- Camera
- Single Board Computer (e.g., Raspberry Pi / Orange Pi)

**Purpose:** The physical computing cluster managing USB modules and camera input.

### Layer 4: Direction Hardware

**Modules:**
- Microcontroller (e.g., Arduino / Pico)
- Motor Driver (H-Bridge)
- Servomotor (Steering)
- DC Motor (Traction)

**Purpose:** The "Muscle" of the system. The MCU drives the H-Bridge Motor Drivers and Steering Servos via PWM and Direction signals, physically moving the chassis.

## Key Features

**Hybrid Networking**

Uses WebSockets for binary streaming (video/telemetry) and REST for state management.

**Zero-Copy Video**

Implementation of POSIX Shared Memory to pass frames from Camera to Web Server without redundant memory allocations.

**Hard Real-Time Safety**

Critical motor logic is offloaded to an MCU, ensuring the vehicle stops immediately if the Linux host freezes or crashes.

**Modular Hardware**

The separation of "Central Hardware" (L3) and "Direction Hardware" (L4) allows for easy swapping of chassis types (e.g., changing from DC motors to Brushless) without rewriting the host software.