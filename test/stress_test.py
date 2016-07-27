# Test for the aREST library using HTTP

# Imports
import pycurl
import time
import json
import StringIO

# Target
# target = 'http://192.168.0.106'
target = 'https://cloud.arest.io/01e47c'
# target = 'arduino.local'

# Function to make cURL call
def curl_call(target, command):

  buf = StringIO.StringIO()

  c = pycurl.Curl()
  c.setopt(c.URL, target + command)
  c.setopt(c.WRITEFUNCTION, buf.write)
  c.perform()
  c.close()

  return buf.getvalue()

# Output
i = 0
print(curl_call(target, "/mode/5/o"))

while True:

  print(curl_call(target,"/digital/5/1"))
  time.sleep(1)
  print(curl_call(target,"/digital/5/0"))
  time.sleep(1)
  i = i + 1
  print i
