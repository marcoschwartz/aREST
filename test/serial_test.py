# Test for the aREST library using Serial

# Imports
import serial
import time
import json
import unittest

# Serial port parameters
serial_speed = 115200
serial_port = '/dev/tty.usbmodem1a12121'

class TestSequenceFunctions(unittest.TestCase):

  # Setup
  def setUp(self):
    
    # Open Serial connection
    self.serial = serial.Serial(serial_port, serial_speed, timeout=1)
    time.sleep(2)
    self.serial.write("\r\r")
    time.sleep(0.1)
    self.serial.reset_input_buffer()
    self.serial.reset_output_buffer()

  # Mode basic test
  def test_mode(self):
      
    # Input
    self.serial.write("/mode/6/i\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['message'],"Pin D6 set to input")

    # Output
    self.serial.write("/mode/6/o\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['message'],"Pin D6 set to output")

  # Digital write basic test
  def test_digital_write(self):
      
    # HIGH
    self.serial.write("/digital/6/1\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['message'],"Pin D6 set to 1")

    # LOW
    self.serial.write("/digital/6/0\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['message'],"Pin D6 set to 0")

  # Digital read basic test
  def test_digital_read(self):

  	# Set to LOW
    self.serial.write("/digital/6/0\r")
    answer = json.loads(self.serial.readline())
  
    # Read
    self.serial.write("/digital/6\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['return_value'],0) 

  # Analog write basic test
  def test_analog_write(self):

  	# Set to 100
    self.serial.write("/analog/6/100\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['message'],"Pin D6 set to 100")
  
    # Set to 0
    self.serial.write("/analog/6/0\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['message'],"Pin D6 set to 0")

  # Analog read basic test
  def test_analog_read(self):

  	# Read
    self.serial.write("/analog/6\r")
    answer = json.loads(self.serial.readline())
    self.assertGreaterEqual(answer['return_value'],0)
    self.assertLessEqual(answer['return_value'],1023) 

  # Digital write + check test
  def test_digital_check(self):

  	# Set to Output
    self.serial.write("/mode/6/o\r")
    answer = json.loads(self.serial.readline())
      
    # Set to HIGH
    self.serial.write("/digital/6/1\r")
    answer = json.loads(self.serial.readline())

    # Read
    self.serial.write("/digital/6\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['return_value'],1)

  # Variable write check
  def test_variable(self):

    # Get variable
    self.serial.write("/temperature\r")
    answer = json.loads(self.serial.readline())
    self.assertGreaterEqual(answer['temperature'],0)
    self.assertLessEqual(answer['temperature'],40)

   # Function call check
  def test_function(self):

    # Call function
    self.serial.write("/led?params=1\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['return_value'],1)

    # Read
    self.serial.write("/digital/6\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['return_value'],1)

    # Call function
    self.serial.write("/led?params=0\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['return_value'],1)

    # Read
    self.serial.write("/digital/6\r")
    answer = json.loads(self.serial.readline())
    self.assertEqual(answer['return_value'],0)

if __name__ == '__main__':
    unittest.main()