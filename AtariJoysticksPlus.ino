#include <Joystick.h>
// Joystick library taken from https://github.com/MHeironimus/ArduinoJoystickLibrary

// To get multiple joysticks working in Ubuntu, one needs to first remove the usbhid module:
// sudo rmmod usbhid
// Then, we add the module back, with a quirk telling it that our joystick is in fact multiple joysticks
// For an Arduino Leonardo, the command
// sudo modprobe usbhid quirks=0x2341:0x8036:0x40
// The general command is sudo modprobe usbhid quirks=0xVID:0xPID:0x40
// and you can find your device's VID and PID by running lsusb
// The above is taken from
// http://stackoverflow.com/questions/29358179/usb-possible-to-define-multiple-distinct-hid-joysticks-on-one-interface


/* D-SUB9 pin number, wire color, function
1    White    Up/Driving controller MSB
2    Blue     Down/Driving controller LSB
3    Green    Left/Paddle A button
4    Brown    Right/Paddle B button
5    Red      Paddle B potentiometer
6    Orange   Joystick button
7    Yellow   +5V (needed for paddles)
8    Black    Ground
9    Purple   Paddle A potentiometer
I was not aware of the above wire coloring scheme when starting this project, and do not currently have all the above colors anyway. So, my choice of colors for wiring from the D-SUB9 to the Arduino is the following (change this to keep track of your wires):
+++ 1    White
+++ 2    Blue
+++ 3    Green
+++ 4    Yellow
+++ 5    Orange
+++ 6    Red
+++ 7    Light green
+++ 8    Black
+++ 9    Brown
Pressing a button/moving the joystick in a direction shorts the relevant pin with ground. Otherwise, the pin is floating.
Button B:
On the EPYX 500XJ (by Konix) this is pin 9
On the Wico Three Way Deluxe and Super Three way deluxe, turning the switch to F1 and pressing the base button shorts pins 5 and 9
Paddle potentiometer is rated as 1M Ohm, linear. One end is connected to the
+5V power supply (pin 7). Full right is 0 Ohm. Full left should nominally be
1 Mega-Ohm, but I have measured 760K Ohm on one paddle and 900K Ohm
on another.
There are tutorials on how to clean Atari paddles. Maybe I should do this...

=============================================
Truglodite's comments below:
Requires 3x resistors, 2x male DB-9 plugs, a pro-micro, bits of wire, and some Atari controllers.
Works with Atari joysticks (1 or 2), paddles (1 pair only... ~10 games exist that need 3-4 paddles), or driving controllers (1 or 2).
If doing DIY paddles, use 10k pots and resistors for cleaner signals (1M resistors for stock paddles).
Failed to compile in PIO... compiles fine in Arduino 1.8.9 (add library manually)
Most emulators are going to need the analog deadband set to zero to prevent "hole in the middle" behavior.
Windows 10 see's this as 2 joysticks with no needed changes (add usbhid.quirks=0x2341:0x8036:0x40 to retropie /boot/cmdline.txt)
Modify VID/PID so no usb quirks are needed... C:\Users\Kevin\AppData\Local\Arduino15\packages\arduino\hardware\avr\1.8.2\boards.txt
change the 3 lines:
  micro.build.mcu=atmega32u4
  micro.build.f_cpu=16000000L
  micro.build.vid=0x2341
  micro.build.pid=0x8037
  micro.build.usb_product="Arduino Micro"
  micro.build.board=AVR_MICRO
  micro.build.core=arduino
  micro.build.variant=micro
  micro.build.extra_flags={build.usb_flags}
to:
  ...
  micro.build.vid=0x8282
  micro.build.pid=0x3201
  micro.build.usb_product="Atari Retro Joystick Adapter"
  ...
This makes the Micro show up as a Mojo Retro Adapter named "Atari Retro..."

*Note this makes driving controllers not work with stella!!! Instead, use the stelladaptor PID/VID.
*This allows stella to properly recognize the output of the driving controller.
*Instead, use this in boards.txt to make the micro appear as a stelladaptor:
* micro.build.vid=0x04D8
* micro.build.pid=0xBEEF
* micro.build.usb_product="Stelladaptor Atari 2600-to-USB Interface"
*Note stelladaptors only have 1 port. Use the usbhid quirks for linux to maybe see it as 2 joysticks?

Pinouts =====================================
All digital pins use internal pullups (short to ground = active)
Arduino Pin   DB9 port-pin          Function  
0             1-2                   p1-down / p1-drive LSB
1             1-1                   p1-up /  p1-drive MSB
2             1-4                   p1-right / p2-paddle button
3             1-3                   p1-left / p1-paddle button
4             1-6                   p1-joy button
A3            1-9                   p1-paddle (A)
A2            1-5                   p2-paddle (B)
5V            1-7                   paddle supply
gnd           1-8                   common ground
5             2-2                   p2-down / p2-drive LSB
6             2-1                   p2-up / p2-drive MSB
7             2-4                   p2-right
8             2-3                   p2-left
9             2-6                   p2-joy button
-             2-9                   -
-             2-5                   -
-             2-7                   -
gnd           2-8                   common gnd
10            -                     S1 (device dependent output)
==================================================

Overall schematic for the added resistors (uses original Atari equipment, simple wiring and all) ===============
+5V --- poti A (1M)  ---  |--- 1M --- GND
                          |
                          |--- 1M  --- D10
                          |
                          A5
                         
+5V --- poti B (1M)  ---  |--- 1M --- GND
                          |
                          A4

...just the stuff you solder inside the adapter box ======================================

(DB9 1-9) ---|--- R1 --- GND
             |
             |--- R2 --- D10
             |
             |
             A5
                        
(DB9 1-5) ---|--- R3 --- GND
             |
             A4

*R1=R2=R3=RpotiA=RpotiB, original poti's are ~1Mohm, CW to zero.
*1Mohm is noisy with arduino inputs. Therefore it is better to use 10k poti's and resistors if possible.
*/
/*
const int minAxisValue = -127; // left/up
const int maxAxisValue = 127;  // right/down
const int midAxisValue = 0;
const int clicksInFullRotation = 16; // driving controller has 4 * 4 = 16 state changes in a 360 degree turn
*/
const int minAxisValue = 0; // left/up
const int maxAxisValue = 255;  // right/down
const int midAxisValue = 127;
const int clicksInFullRotation = 16; // driving controller has 4 * 4 = 16 state changes in a 360 degree turn
const int drivingMaxYvalue = 192; //trug... for stelladapter driving output (0x7F-0x00-0xFF-0xCO...)

// digital must come before analog!
enum joyFunc {
    // digital
    fireA  = 0,
    left  = 1,
    right = 2,
    up    = 3,
    down  = 4,

    // analog
    paddleA_pot  = 5,
    paddleB_pot  = 6,

    // note the dual use
    paddleA_button = left,
    paddleB_button = right,

    driving_MSB = up,
    driving_LSB = down,

    // hybrid (fireB)
    fireB = 7,
    fireB_pullup = 7, // S1 in the schematic below

};

const int joyDigitalFuncCount = 5;
const int joyAnalogFuncCount = 2;
const int joyHybridFuncCount = 1;

const int joyTotalFuncCount = 8;

const int analogReadMaxValue = 1023;

const int analogReadTolerance = 1; // if |curr - prev| <= tolerance, treat as no change
// spec gives +- 2LSB as absolute accuracy
// Trug: No delay needed, since I'm using 10kohm potis and resistors.
// const int millisecondsBetweenReads = 2; // Since the 1M Ohm resistor is larger than the maximum suggested resistance going into the A2D converter, some delay is needed so that the position of one paddle does not interfere with the value read from the other one.
/*
Let us first talk about how to read the paddle position. This is done by using a voltage divider.
The voltage divider is +5V --- R1 = potentiometer (paddle) ---|--- R2 --- GND
                                                              |
                                                     tap for analog read
The nominal range of Atari paddles is 0 to 1M Ohm, linear.
So, to get the best precision, it turns out we should take R2 = 1M Ohm,
the maximum resistance of the potentiometer.
If the paddle is not connected, the voltage measured should be 0.
If the paddle is connected, the voltage range should nominally be
between +5V (R1 = 0) and +2.5v (R1 = 1M Ohm).
We take the threshold to decide if the paddle is connected to be +1.25V = 5V/4
The above schematic was a simplified introduction to paddles. In fact, to get both the paddles and fire button B working (pin 9 shorted to ground/pins 5 and 9 shorted), we use the following schematic:
+5V --- R1A  --- joystick pin 9 --- |--- R2A --- GND
     (paddle A)                     |
                                    |--- R3  --- S1 = +5V/not connected (pin joyFuncPins[?][fireB_pullup] on Arduino)
                                    |
                               analog tap A (pin joyFuncPins[?][paddleA_pot] on Arduino)
+5V --- R1B  --- joystick pin 5 --- |--- R2B --- GND
     (paddle B)                     |
                               analog tap B (pin joyFuncPins[?][paddleB_pot] on Arduino)
If the paddles are connected, both R1A and R2A are nominally between 0 and 1M Ohm. Otherwise, both R1A and R2A are infinity.
We start by checking if the paddles are connected.
* S1 is set to "not connected" (the corresponding pin is set to "input").
* If analog tap B is above the threshold of +1.25V (explained above), we know that the paddles are connected.
  + We make use of the reading just taken of analog tap B.
  + We then read analog tap A.
* Otherwise, the paddles are not connected.
  + We set S1 to +5V by setting   the corresponding pin to "output" and "high". The value of R3 is, also, 1M Ohm.
  + We read joystick up/down/left/right, as well as button A.
  + We make another read of analog tap A.
    # If button B is off, the read should be 5/2 = 2.5 V.
    # If button B is on, by shorting pin 9 to ground, then the analog read should be 0 V.
    # If button B is on, by shorting pins 5 and 9, then the analog read should be 5/3 = 1.66 V.
  + Thus, we set the threshold to be 5 * (5/12) = 2.08 V. If we are above the threshold, button B is off. Otherwise, button B is on.
*/

/* The driving controller uses the up and down joystick pins to encode direction via a Gray code. Let up be the MSB and down be LSB.
                              v---------------
   A left rotation is         00->01->11->10-|
   There are 4 such length 4 cycles in a 360 degree turn of the controller. So, we have a resolution of 360/16 = 22.5 degrees.
   If we read the input and see that both up and down directions are on, then we have clearly connected a driving controller.
   Conversely, if we read the input and see that either the left or right directions are on, then we have clearly connected a joystick.
*/

/* To calibrate a paddle:
1) Press and release the fire button.
2) Move the paddle to an extreme position (full left/right), and stay there for at least three seconds.
3) Repeat for the second extreme position.
*/

const int analogReadThreshold = analogReadMaxValue/4;
const int analogJumpThreshold = analogReadMaxValue/2;
const int fireBReadThreshold = (analogReadMaxValue * 5)/12;

// Nominal analog max should be analogReadMaxValue.
// Nominal analog min should be analogReadMaxValue/2.
// True max should be very close to analogReadMaxValue (R1 ~= 0, also for old paddles)
// True min might be noticeably less than analogReadMaxValue/2 (my paddles have R1 max equal to 900K Ohm and 760K Ohm).
// So, set initial max to 7/8 analogReadMaxValue,
// and set initial min to 6/8 analogReadMaxValue.

const int minAnalogInitialValue = 6 * (analogReadMaxValue/8);
const int maxAnalogInitialValue = 7 * (analogReadMaxValue/8);

// Start updating smoothedAnalog only after paddle button has been pushed. This way, transient behavior during paddle plugging is discarded.
const float smoothedAnalogNoUpdate = -1;

// Set the initial smoothed value to somewhere in between initial max and min.
const float smoothedAnalogInitialValue = (minAnalogInitialValue + maxAnalogInitialValue)/2.0;
const float smoothingPastWeight = 0.99;
const float smoothingMaxBuffer = 1.0;  // eliminate "dancing" on the right
const float smoothingMinBuffer = 12.0; // eliminate "dancing" on the left

const int joystickCount = 2;

int currJoyFuncVals[joystickCount][joyTotalFuncCount];
int prevJoyFuncVals[joystickCount][joyTotalFuncCount];

bool isDriving[joystickCount]; // true if a driving controller is connected
unsigned int drivingPos[joystickCount];

int minAnalogJoystickVals[joystickCount][joyAnalogFuncCount];
int maxAnalogJoystickVals[joystickCount][joyAnalogFuncCount];
float smoothedAnalogJoystickVals[joystickCount][joyAnalogFuncCount];

int joyFuncPins[joystickCount][joyTotalFuncCount];
/*
uint8_t fireAVal;
uint8_t fireBVal;
uint8_t leftVal;
uint8_t rightVal;
uint8_t upVal;
uint8_t downVal;
*/
bool firstTimeFlag;

Joystick_* Joystick[joystickCount];

void setup() {

    joyFuncPins[0][fireA]  = 4;
    joyFuncPins[0][left]  = 3;
    joyFuncPins[0][right] = 2;
    joyFuncPins[0][up]    = 1;
    joyFuncPins[0][down]  = 0;
    joyFuncPins[0][paddleA_pot]  = A3;
    joyFuncPins[0][paddleB_pot]  = A2;
    joyFuncPins[0][fireB_pullup]  = 10;

    joyFuncPins[1][fireA]  = 9;
    joyFuncPins[1][left]  = 8;
    joyFuncPins[1][right] = 7;
    joyFuncPins[1][up]    = 6;
    joyFuncPins[1][down]  = 5;
    joyFuncPins[1][paddleA_pot]  = -1;  // not connected
    joyFuncPins[1][paddleB_pot]  = -1;  // not connected
    joyFuncPins[1][fireB_pullup]  = -1; // not connected

    uint8_t i, joystickIndex;

    // Set pin modes and initial analog min/max values

    for ( joystickIndex = 0; joystickIndex < joystickCount; joystickIndex++ )
    {
        // first take care of digital
        for ( i = 0; i < joyDigitalFuncCount; i++ )
        {
            pinMode( joyFuncPins[joystickIndex][i], INPUT_PULLUP );
        }

        isDriving[joystickIndex] = false;  // assume a joystick is connected, as is typically the case
        drivingPos[joystickIndex] = 0;

        // then take care of analog
        for ( i = joyDigitalFuncCount; i < joyDigitalFuncCount + joyAnalogFuncCount; i++ )
        {
            if ( joyFuncPins[joystickIndex][i] == -1 )
            {
                // It seems there is nothing that needs to be done for the pin,
                // just leave it alone: it is already in analog read mode

                continue;
            }


            // Take care of initial max/min values.

            maxAnalogJoystickVals[joystickIndex][i - joyDigitalFuncCount] = maxAnalogInitialValue;
            minAnalogJoystickVals[joystickIndex][i - joyDigitalFuncCount] = minAnalogInitialValue;

            // Smoothing should not be updated until paddle button has been pressed.
            smoothedAnalogJoystickVals[joystickIndex][i - joyDigitalFuncCount] = smoothedAnalogNoUpdate;

        }

        // nothing to do for hybrid (fireB)

        // keep the compiler happy
        for ( i = 0; i < joyTotalFuncCount; i++ )
        {
            prevJoyFuncVals[joystickIndex][i] = 0;
        }

        // Initialize joystick object
        Joystick[joystickIndex] =
            new Joystick_(0x03+joystickIndex, JOYSTICK_TYPE_JOYSTICK, /*buttonCount*/ 2, /*hatSwitchCount*/  0, /*includeXAxis*/ true, /*includeYAxis*/ true, /*includeZAxis*/ false, /*includeRxAxis*/ true, false, false, false, false, false, false, false);

        // set min and max joystick axis values
        Joystick[joystickIndex]->setXAxisRange(minAxisValue, maxAxisValue);
        //Joystick[joystickIndex]->setYAxisRange(minAxisValue, maxAxisValue);
        Joystick[joystickIndex]->setYAxisRange(minAxisValue, maxAxisValue);

        // set RX range
        Joystick[joystickIndex]->setRxAxisRange(0, clicksInFullRotation - 1 );

        // start the joystick
        Joystick[joystickIndex]->begin(/*initAutoSendState*/ false);

    }


    firstTimeFlag = true;

    Serial.begin(9600);          //  setup serial (!!!debug!!!)

}

unsigned int regularToGray[4] = {0,1,3,2};
unsigned int *grayToRegular = regularToGray;

// return true if drivingGray implies a change of at most 1 in drivingPos, and return the increment in "increment"
bool validGrayIncrement(unsigned int drivingPos, unsigned int currentGray, int &increment )
{
    unsigned int prevGray = regularToGray[drivingPos % 4];
    unsigned int xorOfGrays = prevGray ^ currentGray;

    if ( xorOfGrays == 3 )
    {
        increment = 100; // some illegal entry
        return false;
    }

    increment = grayToRegular[currentGray] - grayToRegular[prevGray];

    if ( increment == -3 )
    {
        increment = 1;
    }
    else if (increment == 3 )
    {
        increment = -1;
    }


    return true;

}

int paddlePotRead( int joyFuncPin )
{

    if ( joyFuncPin == -1 )
    {
        return 0; // if paddles are not connected, simulate a read of 0
    }
    else
    {
        return myAnalogRead( joyFuncPin );
    }
}

// assume prevJoyFuncVals are set to previous values
// read new values into currJoyFuncVals
// digital: copy currJoyFuncVals to prevJoyFuncVals, return true if there was a change
// analog: copy currJoyFuncVals to prevJoyFuncVals if we are above the threshold of change, and return true if so
// return false otherwise
bool readJoystickVals( uint8_t joystickIndex, int *prevJoyFuncVals, int *currJoyFuncVals, int *joyFuncPins )
{
    uint8_t i;
    bool changedFlag;

    changedFlag = false;

    // First, take care of the simple digital reads
    for ( i = 0; i < joyDigitalFuncCount; i++ )
    {
        currJoyFuncVals[i] = !digitalRead( joyFuncPins[i] );
        if ( currJoyFuncVals[i] != prevJoyFuncVals[i] )
        {
            changedFlag = true;
        }
        prevJoyFuncVals[i] = currJoyFuncVals[i];

    }

    // Check if paddles are connected or not.
    // Start by setting S1 to "not connected"
    if ( joyFuncPins[fireB_pullup] != -1 )
    {
        pinMode( joyFuncPins[fireB_pullup], INPUT );
    }

    // Now, check if paddles are connected by reading digital tap B
    currJoyFuncVals[paddleB_pot] = paddlePotRead( joyFuncPins[paddleB_pot] );

    if (currJoyFuncVals[paddleB_pot] > analogReadThreshold )
    {
        // Paddles are connected

        // Read digital tap A as well
        currJoyFuncVals[paddleA_pot] = paddlePotRead( joyFuncPins[paddleA_pot] );


        for ( i = joyDigitalFuncCount; i < joyDigitalFuncCount + joyAnalogFuncCount; i++ )
        {
            // First, check that we are above the nominal min value.
            // If not, then this was typically a momentary "jump" while the paddle was moved
            // (infinite resistance), and should thus be discarded.
            if ( currJoyFuncVals[i] < analogJumpThreshold )
            {
                continue;
            }

            // update the max/min values
            updateAnalogMaxMin(currJoyFuncVals[i-joyDigitalFuncCount + paddleA_button], currJoyFuncVals[paddleA_button] || currJoyFuncVals[paddleB_button], currJoyFuncVals[i], smoothedAnalogJoystickVals[joystickIndex][i-joyDigitalFuncCount], minAnalogJoystickVals[joystickIndex][i-joyDigitalFuncCount], maxAnalogJoystickVals[joystickIndex][i-joyDigitalFuncCount]);


            // Check if we are above the change threshold
            if ( (currJoyFuncVals[i] - prevJoyFuncVals[i]) > analogReadTolerance ||
                    (prevJoyFuncVals[i] - currJoyFuncVals[i]) > analogReadTolerance )
            {
                // move current to prev
                prevJoyFuncVals[i] = currJoyFuncVals[i];

                changedFlag = true;
            }
        }
    }
    else // Joystick connected, check for fireB
    {
        // to be on the safe side, simulate a read of 0 on paddle A
        currJoyFuncVals[paddleA_pot] = 0;

        // Have we yanked out the paddles?
        if ( prevJoyFuncVals[paddleA_pot] > analogReadThreshold || prevJoyFuncVals[paddleB_pot] > analogReadThreshold )
        {
            prevJoyFuncVals[paddleA_pot] = prevJoyFuncVals[paddleB_pot] = 0;
            changedFlag = true;
        }


        if ( joyFuncPins[fireB_pullup] != -1 && joyFuncPins[paddleA_pot] != -1 )
        {
            // Start by setting S1 to +5V
            pinMode( joyFuncPins[fireB_pullup], OUTPUT );
            digitalWrite( joyFuncPins[fireB_pullup], HIGH );

            // Now, read digital tap B again, and check if we are above or bellow the threshold

            if( paddlePotRead( joyFuncPins[paddleA_pot] ) > fireBReadThreshold )
            {
                currJoyFuncVals[fireB] = 0;
            }
            else
            {
                currJoyFuncVals[fireB] = 1;
            }

            if ( currJoyFuncVals[fireB] != prevJoyFuncVals[fireB] )
            {
                changedFlag = true;
            }
        }
        else
        {
            currJoyFuncVals[fireB] = 0;
        }

        prevJoyFuncVals[fireB] = currJoyFuncVals[fireB];




    }

    /*
            Serial.print("in loop\n");
            Serial.println(i);
            Serial.println(joyFuncPins[i]);
            Serial.println(currJoyFuncVals[i]);
    */

    return changedFlag;
}

void updateAnalogMaxMin( int buttonVal, int eitherButtonPressed, int currAnalogVal, float &smoothedAnalogJoystickVal, int &minAnalogJoystickVal, int &maxAnalogJoystickVal)
{
    int smoothedMaxInt;
    int smoothedMinInt;

    if ( smoothedAnalogJoystickVal == smoothedAnalogNoUpdate && buttonVal == true )
    {
        smoothedAnalogJoystickVal = smoothedAnalogInitialValue;
    }

// Don't update if either button is pressed, or updating has yet to start.

    if ( eitherButtonPressed == true || smoothedAnalogJoystickVal == smoothedAnalogNoUpdate )
    {
        return;
    }

    // Update the smoothing value
    smoothedAnalogJoystickVal = smoothingPastWeight*(smoothedAnalogJoystickVal+ (1-smoothingPastWeight)*currAnalogVal);

    smoothedMaxInt = (int) (smoothedAnalogJoystickVal - smoothingMaxBuffer);
    smoothedMinInt = (int) (smoothedAnalogJoystickVal + smoothingMinBuffer);


    if ( smoothedMinInt < minAnalogJoystickVal )
    {
        minAnalogJoystickVal = smoothedMinInt;
    }

    if ( smoothedMaxInt > maxAnalogJoystickVal )
    {
        maxAnalogJoystickVal = smoothedMaxInt;
    }

}

float potTransform( int potVal )
{
    return ( (float) analogReadMaxValue ) / ( (float) potVal ) - 1.0;
}

int calculateAnalogAxisValue( int paddle_pot, int paddle_button, int *currJoyFuncVals, int *minAnalogJoystickVals, int *maxAnalogJoystickVals)
{
    int currPotVal, minPotVal, maxPotVal;
    float currTransformVal, maxTransformVal, minTransformVal;

    currPotVal = currJoyFuncVals[paddle_pot];
    minPotVal = minAnalogJoystickVals[paddle_pot - joyDigitalFuncCount];
    maxPotVal = maxAnalogJoystickVals[paddle_pot - joyDigitalFuncCount];

    if ( currPotVal < analogReadThreshold ) // paddle not connected, might as well return mid
    {
        return midAxisValue;
    }

    if ( currPotVal < minPotVal )
    {
        currPotVal = minPotVal;
    }

    if ( currPotVal > maxPotVal )
    {
        currPotVal = maxPotVal;
    }

    currTransformVal = potTransform( currPotVal );
    minTransformVal = potTransform( minPotVal );
    maxTransformVal = potTransform( maxPotVal );

    return ((currTransformVal - minTransformVal) * maxAxisValue + (maxTransformVal - currTransformVal) * minAxisValue)/(maxTransformVal - minTransformVal);
}

void writeJoystickVals( uint8_t joystickIndex, int *currJoyFuncVals, int *minAnalogJoystickVals, int *maxAnalogJoystickVals )
{
    unsigned int currentGray;
    int increment;

    // first, check for paddles
    if ( currJoyFuncVals[paddleA_pot] > analogReadThreshold || currJoyFuncVals[paddleB_pot] > analogReadThreshold )
    {
        // check buttons
        Joystick[joystickIndex]->setButton(0, currJoyFuncVals[paddleA_button]);
        Joystick[joystickIndex]->setButton(1, currJoyFuncVals[paddleB_button]);

        // set axis values
        Joystick[joystickIndex]->setXAxis( calculateAnalogAxisValue( paddleA_pot, paddleA_button, currJoyFuncVals, minAnalogJoystickVals, maxAnalogJoystickVals) );
        Joystick[joystickIndex]->setYAxis( calculateAnalogAxisValue( paddleB_pot, paddleB_button, currJoyFuncVals, minAnalogJoystickVals, maxAnalogJoystickVals) );

    }
    else // no paddles connected, so either a joystick or a driving controller
    {
        // is it a driving controller for sure?
        if ( currJoyFuncVals[up] && currJoyFuncVals[down] )
        {
            isDriving[joystickIndex]  = true;
        }

        // is it a joystick for sure?
        if ( currJoyFuncVals[left] || currJoyFuncVals[right] )
        {
            isDriving[joystickIndex]  = false;
        }

        // fireA button
        Joystick[joystickIndex]->setButton(0, currJoyFuncVals[fireA]);

        // fireB button
        Joystick[joystickIndex]->setButton(1, currJoyFuncVals[fireB]);

        if ( isDriving[joystickIndex] == true )
        {
            // start by centering the joystick, for good measure
            Joystick[joystickIndex]->setXAxis(midAxisValue);
            Joystick[joystickIndex]->setYAxis(midAxisValue);

            currentGray = currJoyFuncVals[driving_LSB] + 2 * currJoyFuncVals[driving_MSB];

            increment = 0; // keep the compiler happy

            if ( validGrayIncrement(drivingPos[joystickIndex], currentGray, increment ) == true )
            {
                drivingPos[joystickIndex] = (drivingPos[joystickIndex] + increment) % clicksInFullRotation;

                //Joystick[joystickIndex]->setRxAxis( drivingPos[joystickIndex] ); //trug, attempt to simulate stelladaptor
            }


            // for debugging, remove!
            if ( !currJoyFuncVals[up] && !currJoyFuncVals[down] )
            {
                Joystick[joystickIndex]->setYAxis(midAxisValue);
            }
            else if ( currJoyFuncVals[up] && !currJoyFuncVals[down] )
            {
                Joystick[joystickIndex]->setYAxis(minAxisValue);
            }
            else if ( !currJoyFuncVals[up] && currJoyFuncVals[down] )
            {
                Joystick[joystickIndex]->setYAxis(maxAxisValue);
            }
            else // currJoyFuncVals[up] && currJoyFuncVals[down], if we got here, there is a hardware problem!
            {
                Joystick[joystickIndex]->setYAxis(drivingMaxYvalue);  //trug, stelladapter
            }


        }
        else // it is a joystick
        {
            // start by nulling Rx, for good measure
            Joystick[joystickIndex]->setRxAxis(0);


            // left/right
            if ( !currJoyFuncVals[left] && !currJoyFuncVals[right] )
            {
                Joystick[joystickIndex]->setXAxis(midAxisValue);
            }
            else if ( currJoyFuncVals[left] && !currJoyFuncVals[right] )
            {
                Joystick[joystickIndex]->setXAxis(minAxisValue);
            }
            else if ( !currJoyFuncVals[left] && currJoyFuncVals[right] )
            {
                Joystick[joystickIndex]->setXAxis(maxAxisValue);
            }
            else // currJoyFuncVals[left] && currJoyFuncVals[right], we can't get here!
            {
                Joystick[joystickIndex]->setXAxis(100);
            }

            // up/down
            if ( !currJoyFuncVals[up] && !currJoyFuncVals[down] )
            {
                Joystick[joystickIndex]->setYAxis(midAxisValue);
            }
            else if ( currJoyFuncVals[up] && !currJoyFuncVals[down] )
            {
                Joystick[joystickIndex]->setYAxis(minAxisValue);
            }
            else if ( !currJoyFuncVals[up] && currJoyFuncVals[down] )
            {
                Joystick[joystickIndex]->setYAxis(maxAxisValue);
            }
            else // currJoyFuncVals[up] && currJoyFuncVals[down], if we got here, there is a hardware problem!
            {
                Joystick[joystickIndex]->setYAxis(100);
            }
        }
    }

    Joystick[joystickIndex]->sendState();

}

int myAnalogRead( int pin )
{
    // Need to read twice here, since we are generally changing pins.
    // Look at https://www.quora.com/Why-is-a-little-delay-needed-after-analogRead-in-Arduino
    // TODO: For setting the input pin beforehand, I should look at http://www.gammon.com.au/adc

    analogRead(pin);
    // delay(millisecondsBetweenReads); //trug no delay needed with 10kohm paddles and resistors.
    return analogRead(pin);
}

void loop()
{
    bool stateChange;

    uint8_t joystickIndex;

    for ( joystickIndex = 0; joystickIndex < joystickCount; joystickIndex++ )
    {

        stateChange = readJoystickVals( joystickIndex, prevJoyFuncVals[joystickIndex], currJoyFuncVals[joystickIndex], joyFuncPins[joystickIndex] );

        if ( stateChange == true || firstTimeFlag == true )
        {
            writeJoystickVals( joystickIndex, currJoyFuncVals[joystickIndex], minAnalogJoystickVals[joystickIndex], maxAnalogJoystickVals[joystickIndex] );
        }
    }

    firstTimeFlag = false;
}
