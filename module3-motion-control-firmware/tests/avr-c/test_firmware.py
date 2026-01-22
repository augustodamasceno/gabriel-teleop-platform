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
    TIMEOUT = 1

    def setUp(self):
        """Set up serial connection before each test."""
        self.ser = serial.Serial(self.PORT_NAME, self.BAUD_RATE, timeout=self.TIMEOUT)
        self.header = 0xFF
        time.sleep(2)  # Wait for Arduino to reset

    def tearDown(self):
        """Close serial connection after each test."""
        if self.ser.is_open:
            self.ser.close()

    def send_packet(self, steer_val, accel_val):
        """
        Send binary packet to firmware and read response.

        Args:
            steer_val: Steering value (0-18000)
            accel_val: Acceleration value (0-10000)

        Returns:
            Response string from firmware
        """
        packet = struct.pack('<BHH', self.header, steer_val, accel_val)
        self.ser.write(packet)
        response = self.ser.readline().decode('ascii').strip()
        return response

    def test_firmware_communication(self):
        """Test firmware responds with correct format after multiple commands."""
        steer_val = 12345
        accel_val = 65535

        # Send commands in loop
        for _ in range(2):
            response = self.send_packet(steer_val, accel_val)
            self.assertIsNotNone(response, "Firmware did not respond")
            self.assertTrue(len(response) > 0, "Empty response received")

        expected_response = "00090 00000 12345 65535"
        self.assertEqual(response, expected_response,
                        f"Expected '{expected_response}', got '{response}'")


if __name__ == "__main__":
    unittest.main()