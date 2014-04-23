<?php

  // Includes
  include "php_serial.class.php";

  // Get data
  $type = $_GET['type'];
  $target = $_GET['target'];
  $command = $_GET['command'];

  // WiFi ?
  if ($type == "wifi") {

    // Create cURL call
    $service_url = 'http://'. $target . $command;
    $curl = curl_init($service_url);
     
    // Send cURL to board
    curl_setopt($curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4 ); 
    //curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($curl, CURLOPT_CONNECTTIMEOUT , 0.5);
    $curl_response = curl_exec($curl);
    curl_close($curl);

    //Print answer
    if ($curl_response == ""){
      echo "{\"connected\": false}";
    }
  }

  // Serial ?
  if ($type == "serial") {

    // Get configuration data
    $serial_port = $target;

    try {
      $serial = new phpSerial;
      $serial->deviceSet($serial_port);
      $serial->confBaudRate(9600);
      $serial->confParity("none");
      $serial->confCharacterLength(8);
      $serial->confStopBits(1);

      $h = popen('nohup sleep 5 < '. $serial_port . ' &', 'r');
      pclose($h);
      exec('stty -F '. $serial_port .' -hupcl');
      usleep(100000);

      // Open
      $serial->deviceOpen();

      // Send dummy command to clear previous commands
      $serial->sendMessage("dummy" . "\r");
      $dummy = $serial->readPort();

      // Send command
      $serial->sendMessage($command . "\r");
      $answer = $serial->readPort();

      // If empty, retry
      if ($answer == "") {
        sleep(1);
        $serial->sendMessage($command . "\r");
        $answer = $answer . $serial->readPort();  
      }

      $serial->deviceClose();

      // Return JSON
      if ($answer == "") {echo "{\"connected\": false}";}
      else {echo $answer;}
    }
    catch (Exception $e) {
      echo "{\"connected\": false}";
    }

  }
?>