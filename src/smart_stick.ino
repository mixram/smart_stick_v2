#include <Arduino.h>
#include <Mouse.h>
#include <Keyboard.h>

/*INPUT*/
#define SWITCH_ON_OFF_I 2 // button switch to turn on and off mouse control
#define X_AXIS_I A2       // joystick X axis
#define Y_AXIS_I A3       // joystick Y axis

/*PARAMS*/
#define RANGE 8             // output RANGE of X or Y
#define THRESHOLD RANGE / 4 // resting THRESHOLD
#define CENTER RANGE / 2    // resting position value
#define RESPONSE_DELAY 5                    // response delay of the mouse, in ms
#define SENSITIVITY 2                       // higher sensitivity value = slower mouse
#define LEFT_STEPS_ITERATION_QTY 11         // iterations quantity to reach left end of the screen
#define UP_STEPS_ITERATION_QTY 7            // iterations quantity to reach left upper corner of the screen
#define RIGHT_STEPS_ITERATION_QTY 5         // iterations quantity to reach upper ~center of the screen
#define DOWN_STEPS_ITERATION_QTY 3          // iterations quantity to reach ~center of the screen

/*
  reads an axis (0 or 1 for x or y) and scales the analog input RANGE to a RANGE from 0 to <RANGE>
*/
int readAxis(int thisAxis,
             int range,
             int center,
             int threshold)
{
    // read the analog input
    int reading = analogRead(thisAxis);

    // map the reading from the analog input range to the output range
    reading = map(reading, 0, 1023, 0, range);

    // if the output reading is outside from the rest position threshold, use it
    int distance = reading - center;

    // do not move if distance is less then <threshold>
    if (abs(distance) < threshold)
    {
        distance = 0;
    }

    return distance;
}

bool debounce(bool last,
              int btn)
{
    bool current = digitalRead(btn);
    if (last != current)
    {
        delay(5);
        current = digitalRead(btn);
    }

    return current;
}

void toConsoleDeviceState(bool deviceIsActive)
{
    Serial.print("Device is active: ");
    Serial.print(deviceIsActive);
    Serial.println(".");
}

void setup() {}

void loop() {}