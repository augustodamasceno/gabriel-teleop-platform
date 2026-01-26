#!/usr/bin/env python3
"""
gabriel-teleop-platform - Unit Tests for Motion Control Firmware

Copyright (c) 2026, Augusto Damasceno.
All rights reserved.

SPDX-License-Identifier: BSD-2-Clause
"""


import serial
import struct
import time
import unittest


class TestMotionControlFirmware(unittest.TestCase):
    """Test suite for Motion Control Firmware serial communication."""

    PORT_NAME = 'COM5'  # Change to '/dev/ttyUSB0' on Linux
    BAUD_RATE = 115200
    TIMEOUT = 2

    # Command headers from interface.h (use 181+ to avoid collision with data and state header)
    CMD_HEADER_STEERING = 0xB5      # 181
    CMD_HEADER_ACCELERATION = 0xB6  # 182
    CMD_HEADER_DIRECTION = 0xB7     # 183
    CMD_HEADER_BRAKES = 0xB8        # 184
    
    # System state header (0xC3 0x3C 0xA5 - unique sync pattern)
    SYSTEM_STATE_HEADER_1 = 0xC3
    SYSTEM_STATE_HEADER_2 = 0x3C
    SYSTEM_STATE_HEADER_3 = 0xA5

    def setUp(self):
        """Set up serial connection before each test."""
        self.ser = serial.Serial(
            self.PORT_NAME,
            self.BAUD_RATE,
            timeout=self.TIMEOUT
        )
        time.sleep(2)  # Wait for Arduino to reset
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

    def tearDown(self):
        """Close serial connection after each test."""
        if self.ser.is_open:
            self.ser.close()

    def send_steering_command(self, value):
        """
        Send steering command to firmware.

        Args:
            value: Steering setpoint (0-18000)

        Returns:
            None
        """
        if value < 0 or value > 18000:
            raise ValueError("Steering value must be between 0 and 18000")
        
        packet = struct.pack('<BH', self.CMD_HEADER_STEERING, value)
        self.ser.write(packet)
        time.sleep(0.05)  # Give firmware time to process

    def send_acceleration_command(self, value):
        """
        Send acceleration command to firmware.

        Args:
            value: Acceleration setpoint (0-10000)

        Returns:
            None
        """
        if value < 0 or value > 10000:
            raise ValueError("Acceleration value must be between 0 and 10000")
        
        packet = struct.pack('<BH', self.CMD_HEADER_ACCELERATION, value)
        self.ser.write(packet)
        time.sleep(0.05)

    def send_direction_command(self, direction):
        """
        Send direction command to firmware.

        Args:
            direction: 0=forward, 1=reverse

        Returns:
            None
        """
        if direction not in [0, 1]:
            raise ValueError("Direction must be 0 or 1")
        
        packet = struct.pack('<BB', self.CMD_HEADER_DIRECTION, direction)
        self.ser.write(packet)
        time.sleep(0.05)

    def send_brakes_command(self, brakes):
        """
        Send brakes command to firmware.

        Args:
            brakes: 0=disengaged, 1=engaged

        Returns:
            None
        """
        if brakes not in [0, 1]:
            raise ValueError("Brakes must be 0 or 1")
        
        packet = struct.pack('<BB', self.CMD_HEADER_BRAKES, brakes)
        self.ser.write(packet)
        time.sleep(0.05)

    def read_system_state(self):
        """
        Read system state packet from firmware.

        System state format (17 bytes total):
        - header (3 bytes) - 0xC3 0x3C 0xA5 (unique sync pattern)
        - steering_angle (2 bytes)
        - steering_angle_setpoint (2 bytes)
        - steering_manipulate_variable (2 bytes)
        - acceleration (2 bytes)
        - acceleration_setpoint (2 bytes)
        - acceleration_manipulate_variable (2 bytes)
        - direction (1 byte)
        - breaks (1 byte)

        Returns:
            Dictionary with system state or None if read fails
        """
        def read_single_packet():
            """Read a single state packet from serial buffer."""
            try:
                # Read byte by byte until we find 0xC3 0x3C 0xA5 pattern
                max_attempts = 200
                prev_prev_byte = 0
                prev_byte = 0
                
                for attempt in range(max_attempts):
                    byte = self.ser.read(1)
                    if len(byte) != 1:
                        continue
                        
                    current_byte = byte[0]
                    
                    # Check for 0xC3 0x3C 0xA5 header pattern
                    if prev_prev_byte == 0xC3 and prev_byte == 0x3C and current_byte == 0xA5:
                        # Found header, read the rest of the packet (14 bytes)
                        data = self.ser.read(14)
                        if len(data) != 14:
                            prev_prev_byte = prev_byte
                            prev_byte = current_byte
                            continue
                        
                        # 6 x uint16 + 2 x uint8 = 14 bytes
                        values = struct.unpack('<HHHHHHBB', data)
                        return {
                            'steering_angle': values[0],
                            'steering_angle_setpoint': values[1],
                            'steering_manipulate_variable': values[2],
                            'acceleration': values[3],
                            'acceleration_setpoint': values[4],
                            'acceleration_manipulate_variable': values[5],
                            'direction': values[6],
                            'breaks': values[7]
                        }
                    
                    prev_prev_byte = prev_byte
                    prev_byte = current_byte
                
                return None
            except Exception as e:
                print(f"Error reading system state: {e}")
                return None
        
        # Flush serial buffer to discard stale packets
        self.ser.reset_input_buffer()
        
        # Wait for fresh packet from next firmware cycle (10ms loop + margin)
        time.sleep(0.025)
        
        # Read twice and return the second packet to avoid sync errors
        first_packet = read_single_packet()  # Discard first packet (might be partial)
        if first_packet:
            print(f"[Discarded] {first_packet}")
        time.sleep(0.015)     # Wait for next packet
        second_packet = read_single_packet()  # Return fresh packet
        if second_packet:
            print(f"[Received] {second_packet}")
        return second_packet

    def test_initial_state(self):
        """Test that firmware initializes to safe defaults."""
        state = self.read_system_state()
        self.assertIsNotNone(state, "Failed to read initial system state")
        
        # Check initial steering (should be center position 9000)
        self.assertEqual(
            state['steering_angle_setpoint'],
            9000,
            "Initial steering setpoint should be 9000 (center)"
        )
        
        # Check initial acceleration (should be 0)
        self.assertEqual(
            state['acceleration_setpoint'],
            0,
            "Initial acceleration setpoint should be 0"
        )
        
        # Check brakes engaged
        self.assertEqual(
            state['breaks'],
            1,
            "Initial brakes should be engaged (1)"
        )

    def test_steering_command(self):
        """Test steering command updates steering setpoint."""
        test_values = [0, 9000, 18000, 4500, 13500]
        
        for steer_val in test_values:
            with self.subTest(steer_val=steer_val):
                self.send_steering_command(steer_val)
                
                # Read multiple state packets to get updated value
                state = None
                for _ in range(5):
                    state = self.read_system_state()
                    if state and state['steering_angle_setpoint'] == steer_val:
                        break
                    time.sleep(0.02)
                
                self.assertIsNotNone(state, "Firmware did not respond")
                self.assertEqual(
                    state['steering_angle_setpoint'],
                    steer_val,
                    f"Steering setpoint should be {steer_val}"
                )

    def test_acceleration_command(self):
        """Test acceleration command updates acceleration setpoint."""
        test_values = [0, 2500, 5000, 7500, 10000]
        
        for accel_val in test_values:
            with self.subTest(accel_val=accel_val):
                self.send_acceleration_command(accel_val)
                
                # Read multiple state packets to get updated value
                state = None
                for _ in range(5):
                    state = self.read_system_state()
                    if state and state['acceleration_setpoint'] == accel_val:
                        break
                    time.sleep(0.02)
                
                self.assertIsNotNone(state, "Firmware did not respond")
                self.assertEqual(
                    state['acceleration_setpoint'],
                    accel_val,
                    f"Acceleration setpoint should be {accel_val}"
                )

    def test_direction_command(self):
        """Test direction command updates direction state."""
        for direction in [0, 1]:
            with self.subTest(direction=direction):
                self.send_direction_command(direction)
                
                # Read multiple state packets to get updated value
                state = None
                for _ in range(5):
                    state = self.read_system_state()
                    if state and state['direction'] == direction:
                        break
                    time.sleep(0.02)
                
                self.assertIsNotNone(state, "Firmware did not respond")
                self.assertEqual(
                    state['direction'],
                    direction,
                    f"Direction should be {direction}"
                )

    def test_brakes_command(self):
        """Test brakes command updates brake state."""
        for brakes in [0, 1]:
            with self.subTest(brakes=brakes):
                self.send_brakes_command(brakes)
                
                # Read multiple state packets to get updated value
                state = None
                for _ in range(5):
                    state = self.read_system_state()
                    if state and state['breaks'] == brakes:
                        break
                    time.sleep(0.02)
                
                self.assertIsNotNone(state, "Firmware did not respond")
                self.assertEqual(
                    state['breaks'],
                    brakes,
                    f"Brakes should be {brakes}"
                )

    def test_multiple_commands_sequence(self):
        """Test sending multiple different commands in sequence."""
        # Clear buffer
        self.ser.reset_input_buffer()
        
        # Send sequence of commands
        self.send_steering_command(9000)
        time.sleep(0.1)
        self.send_acceleration_command(5000)
        time.sleep(0.1)
        self.send_direction_command(1)
        time.sleep(0.1)
        
        # Verify final state
        state = None
        for _ in range(10):
            state = self.read_system_state()
            if state:
                break
            time.sleep(0.02)
        
        self.assertIsNotNone(state, "No response after command sequence")
        self.assertEqual(state['steering_angle_setpoint'], 9000)
        self.assertEqual(state['acceleration_setpoint'], 5000)
        self.assertEqual(state['direction'], 1)

    def test_boundary_values(self):
        """Test boundary values for each command type."""
        boundary_tests = [
            ('steering', 0),
            ('steering', 18000),
            ('acceleration', 0),
            ('acceleration', 10000),
            ('direction', 0),
            ('direction', 1),
            ('brakes', 0),
            ('brakes', 1),
        ]
        
        for cmd_type, value in boundary_tests:
            with self.subTest(cmd_type=cmd_type, value=value):
                if cmd_type == 'steering':
                    self.send_steering_command(value)
                    field = 'steering_angle_setpoint'
                elif cmd_type == 'acceleration':
                    self.send_acceleration_command(value)
                    field = 'acceleration_setpoint'
                elif cmd_type == 'direction':
                    self.send_direction_command(value)
                    field = 'direction'
                elif cmd_type == 'brakes':
                    self.send_brakes_command(value)
                    field = 'breaks'
                
                # Read and verify
                state = None
                for _ in range(5):
                    state = self.read_system_state()
                    if state and state[field] == value:
                        break
                    time.sleep(0.02)
                
                self.assertIsNotNone(state, f"No response for {cmd_type}={value}")
                self.assertEqual(state[field], value)

    def test_communication_timeout_safety(self):
        """Test that firmware enters failsafe after communication timeout."""
        # Send command to establish communication
        self.send_acceleration_command(5000)
        time.sleep(0.1)
        
        # Verify acceleration is set
        state = self.read_system_state()
        self.assertIsNotNone(state)
        self.assertEqual(state['acceleration_setpoint'], 5000)
        
        # Wait for timeout (1 second + margin)
        print("\nWaiting for safety timeout (1.5s)...")
        time.sleep(1.5)
        
        # Read state - brakes should be engaged
        state = self.read_system_state()
        self.assertIsNotNone(state)
        self.assertEqual(
            state['breaks'],
            1,
            "Brakes should be engaged after communication timeout"
        )
        self.assertEqual(
            state['acceleration_manipulate_variable'],
            0,
            "Acceleration output should be 0 in failsafe"
        )


if __name__ == "__main__":
    unittest.main()
