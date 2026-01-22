# gabriel-teleop-platform
# A Modular Heterogeneous Teleoperation Platform  

## About

Project Gabriel is a modular teleoperation architecture designed to control ground vehicles. The core philosophy is interchangeability: no single technology or hardware component is hard-coded into the architecture.

Every module, from the high-level web framework to the low-level motor driver, is treated as a "black box" defined solely by its inputs and outputs. This enables seamless substitution:

**Software Agnostic**

You can replace the Angular frontend with React or Vue, or rewrite the C++ backend in Rust or Go. As long as the new module respects the defined interfaces, the rest of the system remains unaffected.

**Hardware Independence**

The architecture does not demand specific boards. You can upgrade the Raspberry Pi (Module 4) to an NVIDIA Jetson for better AI performance, or swap the Arduino (Module 5) for an STM32 or ESP32. If the replacement implements the correct Serial Protocol and Pinout interface, it will function immediately.

## Architecture

The system is organized into 5 distinct modules:

**Module 1: User Interface (Web)**  
* A responsive, browser-based interface designed to provide an immersive, game-like teleoperation experience.
* Uses WebSocket for real-time video/telemetry and REST (Representational State Transfer) for configuration.  

**Module 2: High-Level Control Software (HLC)**  
* The "Brain" - runs on Module 4's Single Board Computer (SBC).  
* Components: Video Server, Backend, Control Center, Database.  
* Orchestrates user commands and video streaming.  

**[Module 3: Motion Control Firmware](docs/module3-motion-control-firmware.md)**  
* Real-time embedded software running on Module 5's Microcontroller (MCU).  
* Translates abstract commands into precise motor control signals.  

**Module 4: Host Computer (Hardware)**  
* Physical computing platform (SBC): Raspberry Pi/Orange Pi + Camera.  
* Executes Module 2 software and interfaces with Module 5 via USB.  

**Module 5: Motion Control Hardware**  
* The "Muscle" - MCU, motor drivers, servos, and motors.  
* Executes Module 3 firmware for hard real-time control.  

## Overview   

```mermaid
flowchart TD
    classDef class_ui fill:#0288d1,stroke:#01579b,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_host fill:#43a047,stroke:#1b5e20,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_fw fill:#fb8c00,stroke:#e65100,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_hw fill:#546e7a,stroke:#263238,stroke-width:2px,color:white,font-size:16px,font-weight:bold;
    classDef class_act fill:#c62828,stroke:#b71c1c,stroke-width:2px,color:white,font-size:16px,font-weight:bold;

    classDef spacer fill:none,stroke:none,color:none,width:0px,height:20px;

    subgraph M1 ["Module 1: User Interface"]
        direction TB
        SpaceM1[ ]:::spacer
        UI["Frontend"]:::class_ui
        SpaceM1 ~~~ UI
    end

    subgraph M2 ["Module 2: High-Level Control (HLC)"]
        direction TB
        SpaceM2[ ]:::spacer
        Video["Video Server"]:::class_host
        Backend["Backend"]:::class_host
        Control["Control Center"]:::class_host
        DB[("Database")]:::class_host
        
        SpaceM2 ~~~ Video
        Video --> Backend
        Backend <--> Control & DB
    end

    subgraph M4 ["Module 4: Host Computer"]
        direction TB
        SpaceM4[ ]:::spacer
        CamHW["Camera"]:::class_hw
        SBC["Single Board Computer<br/>(Raspberry Pi / Orange Pi)"]:::class_hw
        
        SpaceM4 ~~~ CamHW
        CamHW -- "USB" --> SBC
    end

    SpaceM1 ~~~ SpaceM2 ~~~ SpaceM4

    subgraph M3 ["Module 3: Motion Control Firmware"]
        direction TB
        SpaceM3[ ]:::spacer
        FW["Real-Time Firmware"]:::class_fw
        SpaceM3 ~~~ FW
    end

    subgraph M5 ["Module 5: Motion Control Hardware"]
        direction TB
        SpaceM5[ ]:::spacer
        MCU["Microcontroller<br/>(Arduino / Pico)"]:::class_act
        Driver["Motor Driver<br/>(H-Bridge)"]:::class_act
        Servo["Servomotor<br/>(Steering)"]:::class_act
        DC["DC Motor<br/>(Traction)"]:::class_act
        
        SpaceM5 ~~~ MCU
        MCU --> Driver & Servo
        Driver --> DC
    end

    UI <==>|"WebSocket / REST"| Backend
    CamHW -.->|Source| Video
    Control <==>|"Serial Protocol"| FW
    FW ==> MCU
    SBC <==>|"USB Cable"| MCU

    linkStyle default font-size:14px;
```

## Tech Stack (Base Implementation)

- **Module 1:** Angular
- **Module 2:** C++ (backend), C++ (video-server), C++ (control center) and PostgreSQL (database)  
- **Module 3:** AVR C (firmware)
- **Module 4:** Raspberry Pi 5 (SBC) and any USB camera
- **Module 5:** Arduino Uno


## Directory Structure

The repository follows a modular structure where each module has its own directory:

```
module<number>-<module-name>/
    <submodule>/
        <tech-choice-1>/
        <tech-choice-2>/
        ...
```

Each module directory contains subdirectories for submodules, which in turn contain different technology implementations. This allows the same module to be implemented in multiple programming languages or frameworks, reinforcing the platform's technology-agnostic philosophy.

**Examples:**
- `module1-user-interface/frontend/angular/` - Angular implementation
- `module1-user-interface/frontend/react/` - Alternative React implementation
- `module2-high-level-control/video-server/cpp/` - C++ video server implementation
- `module3-motion-control-firmware/avr-c/` - AVR C firmware
