# gabriel-teleop-platform

A Modular Heterogeneous Teleoperation System

## About

Project Gabriel is a modular teleoperation architecture designed to control ground vehicles. The core philosophy is interchangeability: no single technology or hardware component is hard-coded into the architecture.

Every module, from the high-level web framework to the low-level motor driver, is treated as a "black box" defined solely by its inputs and outputs. This enables seamless substitution:

**Software Agnostic**

You can replace the Angular frontend with React or Vue, or rewrite the C++ backend in Rust or Go. As long as the new module respects the defined interfaces, the rest of the system remains unaffected.

**Hardware Independence**

The architecture does not demand specific boards. You can upgrade the Raspberry Pi (Layer 4) to an NVIDIA Jetson for better AI performance, or swap the Arduino (Layer 5) for an STM32 or ESP32. If the replacement implements the correct Serial Protocol and Pinout interface, it will function immediately.

## Architecture

The system is organized into 5 distinct layers, moving from high-level user interaction down to physical voltage modulation:

**Layer 1: User Interface (Angular)**

A responsive web dashboard that splits traffic into two pipes: a Fast Path (WebSocket) for fast telemetry and video, and a Control Path (REST) for authentication and configuration.

**Layer 2: Host Computing (Linux SBC)**

The "Brain" of the vehicle. It orchestrates the Video Server, Backend, and Control Center.

**Layer 3: Direction Logic (Firmware)**

A dedicated Real-Time Firmware module that translates abstract commands (e.g., "Direction, Angle, Acceleration") into precise electrical signals, ensuring safety limits and smooth control.

**Layer 4: Computing Unit (Hardware)**

The physical computing cluster, consisting of a Single Board Computer (Raspberry Pi/Orange Pi) managing the USB peripherals and the high-definition Camera.

**Layer 5: Direction Hardware (Actuation)**

The "Muscle" of the system. A Microcontroller (MCU) drives the H-Bridge Motor Drivers and Steering Servos via PWM and Direction signals, physically moving the chassis.

## Key Features

**Hybrid Networking**

Uses WebSockets for binary streaming (video/telemetry) and REST for state management.

**Zero-Copy Video**

Implementation of POSIX Shared Memory to pass frames from Camera to Web Server without redundant memory allocations.

**Hard Real-Time Safety**

Critical motor logic is offloaded to an MCU, ensuring the vehicle stops immediately if the Linux host freezes or crashes.

**Modular Hardware**

The separation of "Computing Unit" (L4) and "Actuation Hardware" (L5) allows for easy swapping of chassis types (e.g., changing from DC motors to Brushless) without rewriting the host software.

## Overview   

```mermaid
flowchart TD
    %% -- Styling --
    classDef class_ui fill:#0288d1,stroke:#01579b,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_host fill:#43a047,stroke:#1b5e20,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_fw fill:#fb8c00,stroke:#e65100,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_hw fill:#546e7a,stroke:#263238,stroke-width:2px,color:white,font-size:16px,font-weight:bold;

    %% -- LAYER 1: UI (Now inside a box) --
    subgraph L1 ["Layer_1:_User_Interface"]
        UI["Web Interface (Angular)"]:::class_ui
    end

    %% -- LAYER 2: HOST SOFTWARE --
    subgraph L2 ["Layer_2:_Host_Computing"]
        direction LR
        Video["Video Server"]:::class_host
        Backend["Backend Core"]:::class_host
        Control["Control Center"]:::class_host
        DB[("Database")]:::class_host
    end

    %% -- LAYER 3: LOGIC --
    subgraph L3 ["Layer_3:_Direction_Logic"]
        FW["Real Time Direction Firmware"]:::class_fw
    end

    %% -- LAYER 4: COMPUTING UNIT --
    subgraph L4 ["Layer_4:_Computing_Unit"]
        direction LR
        CamHW["Camera"]:::class_hw
        SBC["Single Board Computer<br/>(Raspberry Pi / Orange Pi)"]:::class_hw
    end

    %% -- LAYER 5: HARDWARE --
    subgraph L5 ["Layer_5:_Direction_Hardware"]
        direction LR
        MCU["Microcontroller<br/>(Arduino / Pico)"]:::class_hw
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