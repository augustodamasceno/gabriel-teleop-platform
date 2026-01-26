# Motion Control Firmware Tests

Unit tests for the AVR-C Motion Control Firmware serial communication protocol.

## Test Overview

The test suite validates the firmware's command processing and state reporting capabilities. Each test sends a command packet and verifies that the system state response contains the correct values.

## Hardware Setup Requirements

**IMPORTANT:** Tests must be performed with the Arduino board connected alone to the computer.

- Connect only the Arduino board via USB cable
- DO NOT connect any external components to the Arduino pins
- DO NOT connect motors, servos, or any actuators
- DO NOT connect power supplies to the pins
- Ensure no shields or modules are attached

This isolated setup ensures that:
- Tests validate communication protocol only
- No external hardware can interfere with test results
- No risk of damaging external components during testing
- Consistent and repeatable test conditions

## Communication Protocol

### Command Packet Structure

Commands are sent as 4-byte binary packets:

