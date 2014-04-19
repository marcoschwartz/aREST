// Global variables
var selected_pin;
var selected_command;
var board_type = "uno";
var comm_type = "wifi";
var comm_target = "arduino.local";

// Define pins
var pins = ["A0","A1","A2","A3","A4","A5","13","12","11","10","9","8","7","6","5","4","3","2","1","0"];
var pwmPins = ["3","5","6","9","10","11"];
var pinsCommand = [];
var pinsStatus= [];

// Init pins commands & status
for (var i=0;i<pins.length;i++)
{ 
    pinsCommand[pins[i]] = "notSet";
}
for (var i=0;i<pins.length;i++)
{ 
    pinsStatus[pins[i]] = "0";
}

// Build interface
window.onload = function() {

  // Pins
  for (var i=0;i<pins.length;i++)
  { 
    if (pins[i].charAt(0) == "A"){
      $( "#leftPart" ).append("<div class='pinBlock'><div class='pin' id=" + pins[i] + ">" + pins[i]+ "</div><span class='pinText' id='text" + pins[i] + "'></span></div>" );
    }
    else {
      $( "#rightPart" ).append( "<div class='pinBlock'><span class='pinText' id='text" + pins[i] + "'></span><div class='pin' id="+ pins[i] + ">" + pins[i]+ "</div></div>" );
    }
  }

  // Communication choices
  var communicationChoices = "<span id='wifi' class='commChoices'>WiFi</span>" + 
  "<span id='serial' class='commChoices'>Serial</span>" + 
  "<input type='text' value='arduino.local' id='commTarget'></input>" + 
  "<span id='targetSelector'>&#10003</span>";
  $("#communication").html(communicationChoices);
}

// Handle events
$(document).ready ( function () {

  // Handle click on communication choice (like WiFi or Serial)
  $(document).on("click", ".commChoices", function(e){

    // Reset other choices
    $(".commChoices").css("color","gray");

    // Select choice
    $("#" + e.target.id).css("color","red");
    comm_type = e.target.id;

  });

  // Handle click on comm target
  $(document).on("click", "#targetSelector", function(e){
    
    // Get value from target
    comm_target = $("#commTarget").val();

    // Replace by simple text
    $("#commTarget").replaceWith("<div id='commTarget'>" + comm_target + "</div>");
    $("#commTarget").css("color","red");

    // Remove check button
    $("#targetSelector").html("");

  });

  // Handle click on a pin
  $(document).on("click", ".pin", function(e){

    // Propose pin configuration if not set yet
    if (pinsCommand[e.target.id] == "notSet") {
        
        // Build menu
        var menu = "Selected pin: " + e.target.id + 
            "<div><span class='selectorElement' id='digitalWrite'>digitalWrite</span>" + 
            "<span class='selectorElement' id='digitalRead'>digitalRead</span>";

        // Analog pin ?
        if (e.target.id.charAt(0) == "A") {menu = menu + "<span class='selectorElement' id='analogRead'>analogRead</span>"}

        // PWM pin ?
        if (pwmPins.indexOf(e.target.id) > -1) {menu = menu + "<span class='selectorElement' id='analogWrite'>analogWrite</span>"}

        // Close menu with /div tag
        menu = menu + "</div>";

        // Create menu & show
        $("#selector").html(menu);
        selected_pin = e.target.id;  
        $('#selector').show();  
    }

    // Toggle between HIGH and LOW if digitalWrite selected
    if (pinsCommand[e.target.id] == "digitalWrite") { 
        if (pinsStatus[e.target.id] == "0") {
            pinsStatus[e.target.id] = "1";
            $("#" + e.target.id).css("background-color","red");
            $("#text" + e.target.id).html("HIGH");
        } 
        else {
            pinsStatus[e.target.id] = "0";
            $("#" + e.target.id).css("background-color","white");
            $("#text" + e.target.id).html("LOW");
        }
        // Send digital command
        $.get( "comm.php", {type: comm_type, target: comm_target, command: "/digital/" + e.target.id + "/" + pinsStatus[e.target.id]} );
    }

    // Get value from the slider & send command if analogWrite command selected
    if (pinsCommand[e.target.id] == "analogWrite") { 
        
        // Get value
        var value = $("#value" + e.target.id).val();

        // Send command
        $.get( "comm.php", {type: comm_type, target: comm_target, command: "/analog/" + e.target.id + "/" + value} );
    }

    // Read digital value if digitalRead selected
    if (pinsCommand[e.target.id] == "digitalRead") {

      // Get data
      $.get( "comm.php", {type: comm_type, target: comm_target, command: "/digital/" + e.target.id + "/r"}, function(data) {
        obj = JSON.parse(data);

        // Display 
        if (obj.return_value == 1){
          $("#text" + e.target.id).html("HIGH");
        }
        else {
          $("#text" + e.target.id).html("LOW");
        }
      } );
    }

    // Read analog value if analogRead selected
    if (pinsCommand[e.target.id] == "analogRead") {

      // Get data & display
      $.get( "comm.php", {type: comm_type, target: comm_target, command: "/analog/" + e.target.id + "/r"}, function(data) {
        obj = JSON.parse(data);
        $("#text" + e.target.id).html(obj.return_value);
      });
    }
  });

  // Handle clicks on main configuration menu
  $(document).on("click", ".selectorElement", function(e){

    selected_command = e.target.id;
    pinsCommand[selected_pin] = e.target.id;

    if (selected_command == "digitalWrite") {
      $("#" + selected_pin).css("border-color","red");
      $("#" + selected_pin).css("background-color","white");
      $.get( "comm.php", {type: comm_type, target: comm_target, command: "/mode/" + selected_pin + "/o"} );
    }

    if (selected_command == "digitalRead") {
      $("#" + selected_pin).css("border-color","green");
      $("#" + selected_pin).css("background-color","white");
      $.get( "comm.php", {type: comm_type, target: comm_target, command: "/mode/" + selected_pin + "/i"} );
    }

    if (selected_command == "analogRead") {
      $("#" + selected_pin).css("border-color","orange");
      $("#" + selected_pin).css("background-color","white");
    }

    if (selected_command == "analogWrite") {
      $("#" + selected_pin).css("border-color","blue");
      $("#" + selected_pin).css("background-color","white");
      $.get( "comm.php", {type: comm_type, target: comm_target, command: "/mode/" + selected_pin + "/o"} );
      $("#text" + selected_pin).html("<input id='value" + selected_pin + "' type='range' min='0' max='255' value='0'>");
    }

    $('#selector').hide();

  });

});
