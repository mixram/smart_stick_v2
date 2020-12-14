#include <Arduino.h>
#include <Mouse.h>
#include <Keyboard.h>

/*INPUT*/
#define SWITCH_ON_OFF_I 2        // button to turn on and off device control
#define JOYSTICK_MODE_SWITCH_I 3 // button to switch between modes
#define ZOOM_IN_I 4              // button to zoom in
#define ZOOM_OUT_I 5             // button to zoom out
#define HOME_I 6                 // button to 'go home'
#define RESERVE_I 6              // button for reserve function
#define X_AXIS_I A0              // joystick X axis
#define Y_AXIS_I A1              // joystick Y axis

/*PARAMS*/
#define RANGE 8                     // output RANGE of X or Y
#define THRESHOLD RANGE / 4         // resting THRESHOLD
#define CENTER RANGE / 2            // resting position value
#define RESPONSE_DELAY 5            // response delay of the mouse, in ms
#define STEP_MOVE 4                 // step amount for X or Y movement
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
#define LEFT_SHIFT_KEY KEY_LEFT_SHIFT // KEY_LEFT_SHIFT key
#define ESC_KEY KEY_ESC               // ESC key
#define F3_KEY KEY_F3                 // zoom view
#define F4_KEY KEY_F4                 // rotate view
#define F6_KEY KEY_F6                 // home view

bool lastSwitchOnOffState = LOW;        // previous ON\OFF switch state
bool lastJoystickModeSwitchState = LOW; // previous joystick mode switch state
bool lastZoomInState = LOW;             // previous zoom in state
bool lastZoomOutState = LOW;            // previous zoom out state
bool lastGoHomeState = LOW;             // previous 'go home' state
bool lastReserveState = LOW;            // previous reserve state
bool deviceIsActive = false;            // whether or not to control the HID
bool wasMoved = false;
bool rotateModeActive = true; // joystick mode (true - rotate, false - move)

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

void toConsoleJoystickModeState(bool mode)
{
    if (mode)
    {
        Serial.println("Rotate mode is active.");
    }
    else
    {
        Serial.println("Move mode is active.");
    }
}

void toConsoleJoystickData(int xReading,
                           int yReading)
{
    Serial.print("Joystick data: x=");
    Serial.print(xReading);
    Serial.print(", y=");
    Serial.print(yReading);
    Serial.println(".");
}

void toConsoleJoystickData(int yReading)
{
    Serial.print("Zoom steps: y=");
    Serial.print(yReading);
    Serial.println(".");
}

void setup()
{
    Serial.begin(9600);
    while (Serial.available())
        ;

    Serial.println("Initializing...");

    pinMode(SWITCH_ON_OFF_I, INPUT);               // ON\OFF switch pin
    pinMode(JOYSTICK_MODE_SWITCH_I, INPUT_PULLUP); // joystick switch mode pin
    pinMode(ZOOM_IN_I, INPUT);                     // zoome in switch pin
    pinMode(ZOOM_OUT_I, INPUT);                    // zoome out switch pin
    pinMode(HOME_I, INPUT);                        // 'go home' switch pin
    pinMode(RESERVE_I, INPUT);                     // reserve switch pin

    digitalWrite(SWITCH_ON_OFF_I, HIGH);        // pull high button on\off
    digitalWrite(JOYSTICK_MODE_SWITCH_I, HIGH); // pull high button joystick mode
    digitalWrite(ZOOM_IN_I, HIGH);              // pull high button zoome in
    digitalWrite(ZOOM_OUT_I, HIGH);             // pull high button zoome out
    digitalWrite(HOME_I, HIGH);                 // pull high button home
    digitalWrite(RESERVE_I, HIGH);              // pull high button reserve

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
        // debounced value of rotate mode button (pressed or released)
        bool switchJoystickModeSwitchState = debounce(lastJoystickModeSwitchState, JOYSTICK_MODE_SWITCH_I);
        // rotate mode state
        if (switchJoystickModeSwitchState != lastJoystickModeSwitchState && switchJoystickModeSwitchState == LOW)
        {
            rotateModeActive = !rotateModeActive;
            toConsoleJoystickModeState(rotateModeActive);
        }

        int xReading = readAxis(X_AXIS_I, RANGE, CENTER, THRESHOLD);
        int yReading = readAxis(Y_AXIS_I, RANGE, CENTER, THRESHOLD);
        bool zoomInState = debounce(lastZoomInState, ZOOM_IN_I);
        bool zoomOutState = debounce(lastZoomOutState, ZOOM_OUT_I);
        
        int zoomReading = 0;
        if (zoomInState)
        {
            zoomReading = STEP_MOVE;
        }
        else if (zoomOutState)
        {
            zoomReading = -STEP_MOVE;
        }

        if (xReading != 0 || yReading != 0)
        {
            toConsoleJoystickData(xReading, yReading);

            if (rotateModeActive)
            {
                Keyboard.press(F4_KEY);
                Mouse.press(MOUSE_LEFT);
                Mouse.move(xReading, yReading, 0);
            }
            else
            {
                Mouse.press(MOUSE_MIDDLE_KEY);
                Mouse.move(xReading, yReading, 0);
            }

            wasMoved = true;
        }
        else if (zoomReading != 0)
        {
            toConsoleJoystickData(zoomReading);

            Keyboard.press(F3_KEY);
            Mouse.press(MOUSE_LEFT);
            Mouse.move(0, zoomReading, 0);

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
        lastJoystickModeSwitchState = switchJoystickModeSwitchState;
        lastZoomInState = zoomInState;
        lastZoomOutState = zoomOutState;

        delay(RESPONSE_DELAY);
    }

    // save current ON\OFF button state to use on the next loop
    lastSwitchOnOffState = switchOnOffState;
}