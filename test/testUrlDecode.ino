#include <ESP8266WiFi.h>
#include "aREST.h"

aREST arest;
int initialMemory;


void setup() {

  Serial.begin(115200);
  Serial.println("\n\nBegin test");
}


bool output = true;

bool test(String test, String expected) {
    arest.urldecode(test);

    if(output) {
        Serial.print("|");
        Serial.print(test);
        Serial.print("| ");
        Serial.println(test == expected ? "Pass" : "Fail");
    }

    return test == expected;
}


int iters = 0;

void loop() {
    bool passed = 
        test("Hello+World", "Hello World") &&
        test("I+love+%25%25%25percents%25%25%25", "I love %%%percents%%%") &&
        test("", "") &&

        /* Malformed */
        test("%", "%") &&
        test("%1", "%1") &&
        test("I hate percents%", "I hate percents%") &&
        test("I hate percents%2", "I hate percents%2");

    if(!passed) {
        Serial.print("TESTS FAILED!");
    }

    if(iters == 0)
        initialMemory = ESP.getFreeHeap();

    int memEnd = ESP.getFreeHeap();
    iters++;

    Serial.printf("%d iterations; Memory loss [[[ %d ]]]\n", iters, memEnd - initialMemory);

    if(output)
        delay(1500);

    output = false;
}



