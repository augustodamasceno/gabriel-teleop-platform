# Module 3 - Motion Control Firmware in gabriel-teleop-platform

# AVR-C

## **1. Architecture Overview**

The firmware implements a parallel execution model with modular components:

### **Module Structure**
- **interface.h** - System-wide constants and PID parameters
- **system_state.h/c** - SystemState struct definition and initialization
- **controller.h/c** - PID controller encapsulation and update logic
- **communication.h/c** - USART communication for command reception and state transmission
- **pid_controller.h/c** - Generic PID algorithm implementation
- **avr_c.ino** - Main firmware entry point and watchdog configuration

### **Communication Protocol**
The firmware uses a command-based protocol with the following headers:
- `0xFF` - Steering setpoint (2 bytes, little-endian)
- `0xFE` - Acceleration setpoint (2 bytes, little-endian)
- `0xFD` - Direction (1 byte: 0=forward, 1=reverse)

### **Execution Model**
Each loop iteration executes tasks sequentially:
1. Send system state via USART
2. Receive commands (non-blocking with timeout)
3. Update PID controllers
4. 10ms delay + watchdog reset

---

## **2. Datasheet**

The Arduino Uno utilizes the **ATmega328P** 8-bit microcontroller. Detailed technical specifications, register maps, and electrical characteristics can be found in the official datasheet:
> [ATmega328P Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf)

> [AVR Libc Reference](https://avrdudes.github.io/avr-libc/avr-libc-user-manual/index.html) - Standard C library for AVR-GCC.

---

## **3. IDE**

> [Arduino IDE](https://www.arduino.cc/en/software/#ide) - Recommended for graphical development, debugging, and library management.

---

## **4. CLI**

While the Arduino IDE provides a graphical interface, the entire workflow—compilation, deploy, and serial testing—can be performed via the shell using the **Arduino CLI**.

---

### **4.1 Linux / macOS**

#### Commands
```bash
# 1. Install Arduino CLI
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# 2. Setup Core for Arduino Uno (AVR architecture)
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr

# 3. Compile the Firmware and export binaries to a specific 'build' folder
# The FQBN (Fully Qualified Board Name) for Uno is arduino:avr:uno
arduino-cli compile --fqbn arduino:avr:uno --output-dir ./build module3-motion-control-firmware/avr_c

# 4. Deploy (Upload) to the Board
# Replace '/dev/ttyACM0' with your actual connected port (check with 'arduino-cli board list')
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno --input-dir ./build

# 5. Serial Test (Monitor)
# Opens a serial communication session with the board
arduino-cli monitor -p /dev/ttyACM0 --config baudrate=9600
```

### **4.2 Windows**

#### Option A: Winget  
```powershell
winget install ArduinoSA.CLI
```

#### Option B: Scoop  
```powershell  
scoop install arduino-cli
```

#### Option C: Chocolatey  
```powershell  
choco install arduino-cli
```

#### Option D: MSI Installer:  
> [GitHub Releases](https://github.com/arduino/arduino-cli/releases)

#### Commands
```powershell
# 1. Setup Core for Arduino Uno
arduino-cli config init
arduino-cli core update-index
arduino-cli core install arduino:avr

# 2. Compile the Firmware and export binaries to a specific 'build' folder
# Ensure you are in the project root directory
arduino-cli compile --fqbn arduino:avr:uno --output-dir .\build module3-motion-control-firmware\avr_c

# 3. Deploy to the Board
# First, find your port (e.g., COM3) using: arduino-cli board list
# Replace 'COM3' below with your actual port
arduino-cli upload -p COM3 --fqbn arduino:avr:uno --input-dir .\build

# 4. Serial Test (Monitor)
arduino-cli monitor -p COM3 --config baudrate=9600
```

### **4.3 FreeBSD**

#### Commands
```sh
# 1. Install Arduino CLI

sudo pkg install arduino-cli

# 2. Setup Core
arduino-cli config init
arduino-cli core update-index
# NOTE: If this step fails to find FreeBSD tools, you must install 'avr-gcc' and 'avrdude' via pkg
# and configure the CLI to use system tools (advanced setup).
arduino-cli core install arduino:avr

# 3. Compile
arduino-cli compile --fqbn arduino:avr:uno --output-dir ./build module3-motion-control-firmware/avr_c

# 4. Deploy
# CHANGE: Port is /dev/cuaU0 (USB Serial usually starts here)
# PERMISSIONS: Ensure your user is in the 'dialer' group (pw groupmod dialer -m youruser)
arduino-cli upload -p /dev/cuaU0 --fqbn arduino:avr:uno --input-dir ./build

# 5. Serial Test
# CHANGE: Port is /dev/cuaU0
arduino-cli monitor -p /dev/cuaU0 --config baudrate=9600
```

---

## **5. Unit Tests**

The firmware includes automated unit tests written in Python using the `unittest` framework to verify serial communication with the motion control hardware.

### **5.1 Test Structure**

The test suite is located at:
```
module3-motion-control-firmware/tests/avr-c/test_firmware.py
```

### **5.2 Test Functionality**

The unit test validates:
- **Serial Connection**: Establishes communication at 115200 baud
- **Binary Packet Transmission**: Sends properly formatted 5-byte packets (Header + Steer + Accel)
- **Response Verification**: Confirms the firmware echoes the expected format

### **5.3 Expected Response Format**

After sending commands, the firmware responds with current system state:
```
09000 00000 12345 05000
```

Where the fields represent:
- `09000` - Current steering angle (0-18000, represents 0.00-180.00°)
- `00000` - Current acceleration (0-10000, represents 0.00-100.00%)
- `12345` - Steering setpoint
- `05000` - Acceleration setpoint

### **5.4 Running Tests**

#### Prerequisites
> Install the root project uv or pip

#### Run using unittest discovery
```bash
# Run all tests with verbose output
python -m unittest discover -s module3-motion-control-firmware/tests/avr-c -v
```

### **5.5 Configuration**

Update the `PORT_NAME` constant in `test_firmware.py` to match your system:
- **Linux/macOS**: `/dev/ttyUSB0` or `/dev/ttyACM0`
- **Windows**: `COM3`, `COM5`, etc.
- **FreeBSD**: `/dev/cuaU0`