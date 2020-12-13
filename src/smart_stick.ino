#include <Arduino.h>
#include <Mouse.h>
#include <Keyboard.h>

/*INPUT*/
#define SWITCH_ON_OFF_I 2  // button switch to turn on and off mouse control
#define INCLINE_SWITCH_I 3 // button switch to set inclint mode active
#define MOVE_SWITCH_I 4    // button switch to set move mode active
#define ZOOM_SWITCH_I 5    // button switch to set zoom mode active
#define X_AXIS_I A0        // joystick X axis
#define Y_AXIS_I A1        // joystick Y axis

/*PARAMS*/
#define RANGE 8                     // output RANGE of X or Y
#define THRESHOLD RANGE / 4         // resting THRESHOLD
#define CENTER RANGE / 2            // resting position value
#define RESPONSE_DELAY 5            // response delay of the mouse, in ms
#define SENSITIVITY 2               // higher sensitivity value = slower mouse
#define LEFT_STEPS_ITERATION_QTY 11 // iterations quantity to reach left end of the screen
#define UP_STEPS_ITERATION_QTY 7    // iterations quantity to reach left upper corner of the screen
#define RIGHT_STEPS_ITERATION_QTY 5 // iterations quantity to reach upper ~center of the screen
#define DOWN_STEPS_ITERATION_QTY 3  // iterations quantity to reach ~center of the screen

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

bool deviceIsActive = false;       // whether or not to control the HID
bool lastSwitchOnOffState = LOW;   // previous ON\OFF switch state
bool lastInclineSwitchState = LOW; // previous incline switch state
bool lastMoveSwitchState = LOW;    // previous move switch state
bool lastZoomSwitchState = LOW;    // previous zoom switch state
bool wasMoved = false;
int switchMode = 1; // joystick mode (default == 1 (incline))

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

void toConsoleJoystickModeState(int modeNum)
{
    Serial.print("Mode is active: ");
    Serial.print(modeNum);
    Serial.println(".");
}

void setup()
{
    Serial.begin(9600);
    while (Serial.available())
        ;

    Serial.println("Initializing...");

    pinMode(SWITCH_ON_OFF_I, INPUT);  // the ON\OFF switch pin
    pinMode(INCLINE_SWITCH_I, INPUT); // the inclint switch pin
    pinMode(MOVE_SWITCH_I, INPUT);    // the move switch pin
    pinMode(ZOOM_SWITCH_I, INPUT);    // the zoom switch pin

    digitalWrite(SWITCH_ON_OFF_I, HIGH);  // pull button select pin high
    digitalWrite(INCLINE_SWITCH_I, HIGH); // pull button incline switch high
    digitalWrite(MOVE_SWITCH_I, HIGH);    // pull button move switch high
    digitalWrite(ZOOM_SWITCH_I, HIGH);    // pull button zoom switch high

    delay(1000); // short delay to let outputs settle

    Serial.println("Variables initialization has finished.");

    // take control of the mouse:
    Mouse.begin();
    // take control of the keyboard:
    Keyboard.begin();

    Serial.println("Setup has finished.");
}

void loop()
{
    // debounced value of ON\OFF button (pressed or released)
    bool switchOnOffState = debounce(lastSwitchOnOffState, SWITCH_ON_OFF_I);

    // device ON\OFF state
    if (switchOnOffState != lastSwitchOnOffState && switchOnOffState == LOW)
    {
        deviceIsActive = !deviceIsActive;
        toConsoleDeviceState(deviceIsActive);
    }

    // if the HID control state is active, do actions
    if (deviceIsActive)
    {
        // debounced value of incline mode button (pressed or released)
        bool switchInclineSwitchState = debounce(lastInclineSwitchState, INCLINE_SWITCH_I);
        // incline mode state
        if (switchInclineSwitchState != lastInclineSwitchState && switchInclineSwitchState == LOW)
        {
            switchMode = 1;
            toConsoleJoystickModeState(switchMode);
        }

        // debounced value of move mode button (pressed or released)
        bool switchMoveSwitchState = debounce(lastMoveSwitchState, MOVE_SWITCH_I);
        // move mode state
        if (switchMoveSwitchState != lastMoveSwitchState && switchMoveSwitchState == LOW)
        {
            switchMode = 2;
            toConsoleJoystickModeState(switchMode);
        }

        // debounced value of zoom mode button (pressed or released)
        bool switchZoomSwitchState = debounce(lastZoomSwitchState, ZOOM_SWITCH_I);
        // zoom mode state
        if (switchZoomSwitchState != lastZoomSwitchState && switchZoomSwitchState == LOW)
        {
            switchMode = 3;
            toConsoleJoystickModeState(switchMode);
        }

        int xReading = readAxis(X_AXIS_I, RANGE, CENTER, THRESHOLD);
        int yReading = readAxis(Y_AXIS_I, RANGE, CENTER, THRESHOLD);

        if (xReading != 0 || yReading != 0)
        {
            Serial.print("Joystick data: x=");
            Serial.print(xReading);
            Serial.print(", y=");
            Serial.print(yReading);
            Serial.println(".");

            switch (switchMode)
            {
            case 1:
                Keyboard.press(F4_KEY);
                Mouse.press(MOUSE_LEFT);
                Mouse.move(xReading, yReading, 0);

                break;
            case 2:
                Mouse.press(MOUSE_MIDDLE_KEY);
                Mouse.move(xReading, yReading, 0);

                break;
            case 3:
                Keyboard.press(F3_KEY);
                Mouse.press(MOUSE_LEFT);
                Mouse.move(0, yReading, 0);

                break;

            default:
                Serial.print("UNEXPECTED MODE: ");
                Serial.print(switchMode);
                Serial.print("!!!!!");

                break;
            }

            wasMoved = true;
        }
        else if (wasMoved)
        {
            Keyboard.releaseAll();
            Mouse.release(MOUSE_LEFT);
            Mouse.release(MOUSE_MIDDLE_KEY);
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

        // save current switch mode buttons states to use on the next loop
        lastInclineSwitchState = switchInclineSwitchState;
        lastMoveSwitchState = switchMoveSwitchState;
        lastZoomSwitchState = switchZoomSwitchState;

        delay(RESPONSE_DELAY);
    }

    // save current ON\OFF button state to use on the next loop
    lastSwitchOnOffState = switchOnOffState;
}