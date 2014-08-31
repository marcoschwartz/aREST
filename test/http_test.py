# Test for the aREST library using HTTP

# Imports
import pycurl
import time
import json
import unittest
import StringIO

# Target
#target = '192.168.1.102'
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

  # Mode basic test
  def test_mode(self):
      
    # Input
    answer = json.loads(curl_call(target,"/mode/6/i"))
    self.assertEqual(answer['message'],"Pin D6 set to input")

    # Output
    answer = json.loads(curl_call(target,"/mode/6/o"))
    self.assertEqual(answer['message'],"Pin D6 set to output")

  # Digital write basic test
  def test_digital_write(self):

    # HIGH
    answer = json.loads(curl_call(target,"/digital/6/1"))
    self.assertEqual(answer['message'],"Pin D6 set to 1")

    # LOW
    answer = json.loads(curl_call(target,"/digital/6/0"))
    self.assertEqual(answer['message'],"Pin D6 set to 0")

  # Digital read basic test
  def test_digital_read(self):

    # Set to LOW
    answer = json.loads(curl_call(target,"/digital/6/0"))
  
    # Read
    answer = json.loads(curl_call(target,"/digital/6"))
    self.assertEqual(answer['return_value'],0) 

  # Analog write basic test
  def test_analog_write(self):

    # Set to 100
    answer = json.loads(curl_call(target,"/analog/6/100"))
    self.assertEqual(answer['message'],"Pin D6 set to 100")
  
    # Set to 0
    answer = json.loads(curl_call(target,"/analog/6/0"))
    self.assertEqual(answer['message'],"Pin D6 set to 0")

  # Analog read basic test
  def test_analog_read(self):

    # Read
    answer = json.loads(curl_call(target,"/analog/6"))
    self.assertGreaterEqual(answer['return_value'],0)
    self.assertLessEqual(answer['return_value'],1023) 

  # Digital write + check test
  def test_digital_check(self):

    # Set to Output
    answer = json.loads(curl_call(target,"/mode/6/o"))
      
    # Set to HIGH
    answer = json.loads(curl_call(target,"/digital/6/1"))

    # Read
    answer = json.loads(curl_call(target,"/digital/6"))
    self.assertEqual(answer['return_value'],1)

  # Variable write check
  def test_variable(self):

    # Get variable
    answer = json.loads(curl_call(target,"/temperature"))
    self.assertGreaterEqual(answer['temperature'],0)
    self.assertLessEqual(answer['temperature'],40)

  # Function call check
  def test_function(self):

    # Call function
    answer = json.loads(curl_call(target,"/led?params=1"))
    self.assertEqual(answer['return_value'],1)

    # Read
    answer = json.loads(curl_call(target,"/digital/6"))
    self.assertEqual(answer['return_value'],1)

    # Call function
    answer = json.loads(curl_call(target,"/led?params=0"))
    self.assertEqual(answer['return_value'],1)

    # Read
    answer = json.loads(curl_call(target,"/digital/6"))
    self.assertEqual(answer['return_value'],0)    

if __name__ == '__main__':
  unittest.main()