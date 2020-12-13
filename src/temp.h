#include <Arduino.h>
#include <Mouse.h>
#include <Keyboard.h>

/*INPUT*/
#define SWITCH_ON_OFF_I 2   // button switch to turn on and off mouse control
#define MOVE_DOWN_I 3       // button to move detail down
#define MOVE_UP_I 4         // button to move detail up
#define MOVE_RIGHT_I 5      // button to move detail right
#define MOVE_LEFT_I 6       // button to move detail left
#define X_AXIS_ZOOM_I A0    // joystick X axis (zoom)
#define Y_AXIS_ZOOM_I A1    // joystick Y axis (zoom)
#define X_AXIS_INCLINE_I A2 // joystick X axis (incline)
#define Y_AXIS_INCLINE_I A3 // joystick Y axis (incline)

/*PARAMS*/
#define RANGE_INCLINE 8                     // output RANGE_INCLINE of X or Y incline
#define THRESHOLD_INCLINE RANGE_INCLINE / 4 // resting THRESHOLD_INCLINE
#define CENTER_INCLINE RANGE_INCLINE / 2    // resting position value
#define RANGE_ZOOM 8                        // output RANGE_ZOOM of zoom
#define THRESHOLD_ZOOM RANGE_ZOOM / 4       // resting THRESHOLD_ZOOM
#define CENTER_ZOOM RANGE_ZOOM / 2          // resting position value
#define STEP_MOVE 4                         // step amount for X or Y movement
#define RESPONSE_DELAY 5                    // response delay of the mouse, in ms
#define SENSITIVITY 2                       // higher sensitivity value = slower mouse
#define LEFT_STEPS_ITERATION_QTY 11         // iterations quantity to reach left end of the screen
#define UP_STEPS_ITERATION_QTY 7            // iterations quantity to reach left upper corner of the screen
#define RIGHT_STEPS_ITERATION_QTY 5         // iterations quantity to reach upper ~center of the screen
#define DOWN_STEPS_ITERATION_QTY 3          // iterations quantity to reach ~center of the screen

/*MOUSE*/
#define MOUSE_LEFT_KEY MOUSE_LEFT     // MOUSE_LEFT key
#define MOUSE_RIGHT_KEY MOUSE_RIGHT   // MOUSE_RIGHT key
#define MOUSE_MIDDLE_KEY MOUSE_MIDDLE // MOUSE_MIDDLE key

/*KEYS*/
#define PAGE_DOWN_KEY KEY_PAGE_DOWN   // KEY_PAGE_DOWN key
#define PAGE_UP_KEY KEY_PAGE_UP       // KEY_PAGE_UP key
#define ESC_KEY KEY_ESC               // ESC key
#define LEFT_SHIFT_KEY KEY_LEFT_SHIFT // KEY_LEFT_SHIFT key
#define F3_KEY KEY_F3                 // zoom view
#define F4_KEY KEY_F4                 // rotate view
#define F6_KEY KEY_F6                 // home view

bool deviceIsActive = false;     // whether or not to control the mouse
bool keyboardIsActive = false;   // whether or not to control the keyboard
bool lastSwitchOnOffState = LOW; // previous ON\OFF switch state
bool lastMoveDownState = LOW;    // previous move down state
bool lastMoveUpState = LOW;      // previous move up state
bool lastMoveRightState = LOW;   // previous move right state
bool lastMoveLeftState = LOW;    // previous move left state
bool wasMoved = false;

void setup()
{
    Serial.begin(9600);
    while (Serial.available())
        ;

    Serial.println("Initializing...");

    pinMode(SWITCH_ON_OFF_I, INPUT); // the ON\OFF switch pin
    pinMode(MOVE_DOWN_I, INPUT);     // the move down pin
    pinMode(MOVE_UP_I, INPUT);       // the move up pin
    pinMode(MOVE_RIGHT_I, INPUT);    // the move right pin
    pinMode(MOVE_LEFT_I, INPUT);     // the move left pin

    digitalWrite(SWITCH_ON_OFF_I, HIGH); // Pull button select pin high

    delay(1000); // short delay to let outputs settle

    Serial.println("Initializing variables has finished.");

    // take control of the mouse:
    Mouse.begin();
    // take control of the keyboard:
    Keyboard.begin();
}

/*
  reads an axis (0 or 1 for x or y) and scales the analog input RANGE_INCLINE to a RANGE_INCLINE from 0 to <RANGE_INCLINE>
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

void toConsoleMouseState(bool deviceIsActive)
{
    Serial.print("Mouse is active: ");
    Serial.print(deviceIsActive);
    Serial.println(".");
}

void toConsoleKeyboardState(bool keyboardIsActive)
{
    Serial.print("Mouse is active: ");
    Serial.print(keyboardIsActive);
    Serial.println(".");
}

void loop()
{
    // debounced value of button (pressed or released)
    bool switchOnOffState = debounce(lastSwitchOnOffState, SWITCH_ON_OFF_I);

    // device ON\OFF state
    if (switchOnOffState != lastSwitchOnOffState && switchOnOffState == LOW)
    {
        deviceIsActive = !deviceIsActive;
        toConsoleMouseState(deviceIsActive);

        // keyboardIsActive = !keyboardIsActive;
        // toConsoleKeyboardState(keyboardIsActive);
    }

    // if the mouse control state is active, move the mouse
    if (deviceIsActive)
    {
        // read and scale axes
        bool moveDownState = debounce(lastMoveDownState, MOVE_DOWN_I);
        bool moveUpState = debounce(lastMoveUpState, MOVE_UP_I);
        bool moveRightState = debounce(lastMoveRightState, MOVE_RIGHT_I);
        bool moveLeftState = debounce(lastMoveLeftState, MOVE_LEFT_I);
        // int xZoomReading = readAxis(X_AXIS_ZOOM_I, RANGE_ZOOM, CENTER_ZOOM, THRESHOLD_ZOOM);
        int yZoomReading = readAxis(Y_AXIS_ZOOM_I, RANGE_ZOOM, CENTER_ZOOM, THRESHOLD_ZOOM);
        int xInclineReading = readAxis(X_AXIS_INCLINE_I, RANGE_INCLINE, CENTER_INCLINE, THRESHOLD_INCLINE);
        int yInclineReading = readAxis(Y_AXIS_INCLINE_I, RANGE_INCLINE, CENTER_INCLINE, THRESHOLD_INCLINE);
        int xMoveBtnReading = 0;
        int yMoveBtnReading = 0;

        if (moveDownState)
        {
            yMoveBtnReading = STEP_MOVE;
        }
        if (moveUpState)
        {
            yMoveBtnReading = -STEP_MOVE;
        }
        if (moveRightState)
        {
            xMoveBtnReading = STEP_MOVE;
        }
        if (moveLeftState)
        {
            xMoveBtnReading = -STEP_MOVE;
        }

        if (yZoomReading != 0)
        {
            // Serial.print("Zoom step: ");
            // Serial.print(yZoomReading);
            // Serial.println(".");

            Mouse.move(0, 0, (signed char) yZoomReading / 2);

            wasMoved = true;
        }
        else if (xInclineReading != 0 || yInclineReading != 0)
        {
            // Serial.print("Incline steps: x=");
            // Serial.print(xInclineReading);
            // Serial.print(", y=");
            // Serial.print(yInclineReading);
            // Serial.println(".");

            Keyboard.press(LEFT_SHIFT_KEY);
            Mouse.press(MOUSE_MIDDLE_KEY);
            Mouse.move(xInclineReading, yInclineReading, 0);

            // Keyboard.press(F4_KEY);
            // Mouse.press(MOUSE_LEFT);
            // Mouse.move(xInclineReading, yInclineReading, 0);

            wasMoved = true;
        }
        else if (xMoveBtnReading != 0 || yMoveBtnReading != 0)
        {
            // Serial.print("Move steps: x=");
            // Serial.print(xMoveBtnReading);
            // Serial.print(", y=");
            // Serial.print(yMoveBtnReading);
            // Serial.println(".");

            Mouse.press(MOUSE_MIDDLE_KEY);
            Mouse.move(xMoveBtnReading, yMoveBtnReading, 0);

            wasMoved = true;
        }
        else if (wasMoved)
        {
            Keyboard.releaseAll();
            Mouse.release(MOUSE_MIDDLE_KEY);
            Mouse.release(MOUSE_LEFT);
            Keyboard.write(ESC_KEY);

            /*
            * returm mouse to the ~center of the screen
            * step range: -128 to 127
            */
            //move mouse left to the end of the screen
            for (int a = 0; a < LEFT_STEPS_ITERATION_QTY; a++)
            {
                Mouse.move(-128, 0, 0);
            }
            //move mouse up to the left upper corner of the screen
            for (int a = 0; a < UP_STEPS_ITERATION_QTY; a++)
            {
                Mouse.move(0, -128, 0);
            }
            //move mouse right to the upper ~center of the screen
            for (int a = 0; a < RIGHT_STEPS_ITERATION_QTY; a++)
            {
                Mouse.move(127, 0, 0);
            }
            //move mouse down to the ~center of the screen
            for (int a = 0; a < DOWN_STEPS_ITERATION_QTY; a++)
            {
                Mouse.move(0, 127, 0);
            }

            wasMoved = false;
        }
    }

    // save current buttons states to use on the next loop
    lastSwitchOnOffState = switchOnOffState;

    delay(RESPONSE_DELAY);
}