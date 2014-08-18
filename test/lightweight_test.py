# Test for the aREST library using HTTP

# Imports
import pycurl
import time
import json
import unittest
import StringIO

# Target
# arget = '192.168.1.105'
target = 'arduino.local'

# Function to make cURL call
def curl_call(target, command):
  
  buf = StringIO.StringIO()

  c = pycurl.Curl()
  c.setopt(c.URL, target + command)
  c.setopt(c.WRITEFUNCTION, buf.write)
  c.perform()
  c.close()

  return buf.getvalue()

# Test
class TestSequenceFunctions(unittest.TestCase):

  # Digital read basic test
  def test_digital_read(self):

    # Set to LOW
    answer = curl_call(target,"/digital/6/0").strip()
  
    # Read
    answer = curl_call(target,"/digital/6").strip()
    self.assertEqual(int(answer),0) 

  # Analog read basic test
  def test_analog_read(self):

    # Read
    answer = curl_call(target,"/analog/6")
    self.assertGreaterEqual(int(answer),0)
    self.assertLessEqual(int(answer),1023) 

  # Digital write + check test
  def test_digital_check(self):

    # Set to Output
    answer = curl_call(target,"/mode/6/o")
      
    # Set to HIGH
    answer = curl_call(target,"/digital/6/1")

    # Read
    answer = curl_call(target,"/digital/6")
    self.assertEqual(int(answer),1)

  # Variable write check
  def test_variable(self):

    # Get variable
    answer = curl_call(target,"/temperature")
    self.assertGreaterEqual(int(answer),0)
    self.assertLessEqual(int(answer),40)

  # Function call check
  def test_function(self):

    # Call function
    answer = curl_call(target,"/led?params=1")

    # Read
    answer = curl_call(target,"/digital/6")
    self.assertEqual(int(answer),1)

    # Call function
    answer = curl_call(target,"/led?params=0")

    # Read
    answer = curl_call(target,"/digital/6")
    self.assertEqual(int(answer),0)

if __name__ == '__main__':
  unittest.main()