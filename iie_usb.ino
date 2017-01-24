/*
  Modified version of RetroConnector IIe-to-USB keyboard adapter, changed
  to work with regular Teensy 2.0 (32u4, not Teensy++) and a 74HC165N (or
  equivalent) shift register

  Original: https://github.com/option8/RetroConnector/blob/master/IIe-USB
*/

/* requires keypad library by Mark Stanley, Alexander Brevig from:
http://www.arduino.cc/playground/Code/Keypad

Uses the PJRC Teensy++ 2.0 and Teensyduino libraries, specifically USB HID and Keyboard():
http://www.pjrc.com/teensy/teensyduino.html
*/

#include <Keypad.h>


/*
Declares the matrix rows/cols of the Apple IIe keyboard.

More information here:
http://apple2.info/wiki/index.php?title=Pinouts#Apple_.2F.2Fe_Motherboard_keyboard_connector

*/

// Prevent compiler warning about casts from unsigned ints to chars
#define C(x) ((char) (x))

const byte ROWS = 10; // rows
const byte COLS = 8; // columns
char keys[ROWS][COLS] = {


  { C(KEY_ESC), C(KEY_TAB), C(KEY_A), C(KEY_Z), C(KEYPAD_SLASH), 0, C(KEYPAD_ASTERIX), C(KEY_ESC)},

  { C(KEY_1), C(KEY_Q), C(KEY_D), C(KEY_X),  0,  0,  0,  0},

  { C(KEY_2), C(KEY_W), C(KEY_S), C(KEY_C), C(KEYPAD_0), C(KEYPAD_4), C(KEYPAD_8),  0},

  { C(KEY_3), C(KEY_E), C(KEY_H), C(KEY_V), C(KEYPAD_1), C(KEYPAD_5), C(KEYPAD_9), C(KEYPAD_MINUS)},

  { C(KEY_4), C(KEY_R), C(KEY_F), C(KEY_B), C(KEYPAD_2), C(KEYPAD_6), C(KEYPAD_PERIOD), C(KEYPAD_ENTER)},

  { C(KEY_6), C(KEY_Y), C(KEY_G), C(KEY_N), C(KEYPAD_3), C(KEYPAD_7), C(KEYPAD_PLUS),  0},

  { C(KEY_5), C(KEY_T), C(KEY_J), C(KEY_M), C(KEY_BACKSLASH), C(KEY_TILDE), C(KEY_ENTER), C(KEY_BACKSPACE)},

  { C(KEY_7), C(KEY_U), C(KEY_K), C(KEY_COMMA), C(KEY_EQUAL), C(KEY_P), C(KEY_UP), C(KEY_DOWN) },

  { C(KEY_8), C(KEY_I), C(KEY_SEMICOLON), C(KEY_PERIOD), C(KEY_0), C(KEY_LEFT_BRACE), C(KEY_SPACE), C(KEY_LEFT) },

  { C(KEY_9), C(KEY_O), C(KEY_L), C(KEY_SLASH), C(KEY_MINUS), C(KEY_RIGHT_BRACE), C(KEY_QUOTE), C(KEY_RIGHT) },

};


char Fkeys[2][10] = {
  {C(KEY_1), C(KEY_2), C(KEY_3), C(KEY_4), C(KEY_5), C(KEY_6), C(KEY_7), C(KEY_8), C(KEY_9), C(KEY_0)},
  {C(KEY_F1), C(KEY_F2), C(KEY_F3), C(KEY_F4), C(KEY_F5), C(KEY_F6), C(KEY_F7), C(KEY_F8), C(KEY_F9), C(KEY_F10)}
};




/*
        XO      X1      X2      X3  |   X4      X5      X6      X7
------------------------------------+--------------------------------
YO      ESC     TAB     A       Z   |   /       )       *       ESC
                                    |
Y1      1!      Q       D       X   |   DOWN    UP      LEFT    RIGHT
                                    |
Y2      2@      W       S       C   |   0       4       8       (
                                    |
Y3      3#      E       H       V   |   1       5       9       -
                                    |
Y4      4$      R       F       S   |   2       6       .       RETURN
                                    |
Y5      6^      Y       G       N   |   3       7       +       ,
                                    +----------------------------------
Y6      5%      T       J       M       \|      `~      RETURN  DELETE

Y7      7&      U       K       ,<      +=       P      UP       DOWN

Y8      8*      I       ;:      .>      0)       [{     SPACE   LEFT

Y9      9(      O       L       /?      -_       ]}      '"      RIGHT

The RetroConnector IIe maps the 26 Apple IIe pins to Teensy 2.0 pins. "--"
denotes connection to the shift register.

IIe  Col/Row   Arduino  Teensy 2.0
1     Y0       0        B0
2     Y1	     1        B1
3     +5V      +5v      +5v
4     Y2	     2        B2
5     SW1/CAPL --       --
6     Y3	     3        B3
7     SW0/OAPL --       --
8     Y4	     24       E6
9     CAPLOCK* --       --
10    Y5	     4        B7
11    CNTL*    --       --
12    Y8	     5        D0
13    GND      GND      GND
14    X0	     6        D1
15    RESET*   --       --
16    X2       9        C6
17    X7       17       F6
18    X1	     10       C7
19    X5       16       F7
20    X3       23       D5
21    X4       15       B6
22    Y9       22       D4
23    Y6       14       B5
24    SHFT*	   --       --
25    Y7       13       B4
26    X6       12       D7

Other Pins:

7/D2    USB RX
8/D3    USB TX
11/D6   LED_BUILTIN
18/F5   Shift Register Parallel Load/Latch  (IC Pin 1, active low)
19/F4   Shift Register Clock Enable/Inhibit (IC Pin 15, active low)
20/F1   Shift Register Clock Pulse          (IC Pin 2, low-to-high edge triggered)
21/F0   Shift Register Data Pin (Q7)        (IC Pin 9)

Shift Register Pins-to-Keys:
0: open-apple, pull-down, active high
1: closed-apple, pull-down, active high
2: unused
3: unused
4: Caps-Lock, pull-up, active low
5: Shift, pull-up, active low
6: Reset, pull-up, active low (only active with Control)
7: Control, pull-up, active low

NOTE: Could unify Parallel Load and Clock Enable pins using a single-transistor
NOT Gate/Inverter.

Shift Register http://playground.arduino.cc/Code/ShiftRegSN74HC165N
http://www.ti.com/lit/ds/symlink/sn74ls165a.pdf
*/

byte rowPins[ROWS] = { // Y0 - Y9
  0, 1, 2, 3, 24, 4, 14, 13, 5, 22}; //connect to the row pinouts of the keypad

byte colPins[COLS] = { // X0 - X7
   6, 10, 9, 23, 15, 16, 12, 17}; //connect to the column pinouts of the keypad

Keypad KPD = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// // these pins are special in that they are dis/connected to ground, instead of to a row/col
// const int  SHIFTPin = 21;    // the pin that the shift key is attached to
// const int  CTRLPin = 5;    // the pin that the control key is attached to
// const int  APPLEPin1 = 8;    // the pin that the open-apple key is attached to
// const int  APPLEPin2 = 9;    // the pin that the closed-apple key is attached to
// const int  CAPSPin = 7;

// Shift register pins for reading modifier keys

#define PARALLEL_LOAD_PIN 18 // F5, IC Pin 1, active low
#define CLOCK_ENABLE_PIN 19  // F4, IC Pin 15, active low
#define CLOCK_PULSE_PIN 20  // F1, IC Pin 2, low-to-high edge triggered
#define SHIFT_DATA_PIN 21 // F0, IC Pin 9

/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   1

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5
#define BYTES_VAL_T unsigned int
#define POLL_DELAY_MSEC   10

BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;

#define LED  11

uint16_t modifierKeys[4];


#define KEY_CAPS_UNLOCK  0

boolean resetCapsLock = false;  // Allows one caps unlock signal.
unsigned long dTime = 0;
boolean CAPSLock = false;  // Initialize this to a reasonable value.

boolean FKEYS = false; // used to set numbers to F-Key equivalent. currently tied to caps lock

void setup()
{
  // pinMode(SHIFTPin, INPUT_PULLUP);
  // digitalWrite(SHIFTPin, HIGH);
  //
  // pinMode(CTRLPin, INPUT_PULLUP);
  // digitalWrite(CTRLPin, HIGH);
  //
  // pinMode(APPLEPin1, INPUT);
  // digitalWrite(APPLEPin1, LOW);
  //
  // pinMode(APPLEPin2, INPUT);
  // digitalWrite(APPLEPin2, LOW);
  //
  // pinMode(CAPSPin, INPUT_PULLUP);
  // digitalWrite(CAPSPin, HIGH);

  // Configure Shift Register pins
  pinMode(PARALLEL_LOAD_PIN, OUTPUT);
  digitalWrite(PARALLEL_LOAD_PIN, HIGH);

  pinMode(CLOCK_ENABLE_PIN, OUTPUT);
  digitalWrite(CLOCK_ENABLE_PIN, HIGH);

  pinMode(CLOCK_PULSE_PIN, OUTPUT);
  digitalWrite(CLOCK_PULSE_PIN, LOW);

  pinMode(SHIFT_DATA_PIN, INPUT);

  // DEBUG
  Serial.begin(9600);
  pinMode(LED, OUTPUT);

  // This gives time for the keyboard to hook up to the PC.
  // Otherwise the caps lock state may be incorrect.
  delay(1000);

  pinValues = read_shift_regs();
  display_pin_values();
  oldPinValues = pinValues;
}


/* This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer (or long).
*/
BYTES_VAL_T read_shift_regs()
{
    long bitVal;
    BYTES_VAL_T bytesVal = 0;

    /* Trigger a parallel Load to latch the state of the data lines,
    */
    digitalWrite(CLOCK_ENABLE_PIN, HIGH);
    digitalWrite(PARALLEL_LOAD_PIN, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(PARALLEL_LOAD_PIN, HIGH);
    digitalWrite(CLOCK_ENABLE_PIN, LOW);

    /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(SHIFT_DATA_PIN);

        /* Set the corresponding bit in bytesVal.
        */
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));

        /* Pulse the Clock (rising edge shifts the next bit).
        */
        digitalWrite(CLOCK_ENABLE_PIN, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(CLOCK_ENABLE_PIN, LOW);
    }

    return(bytesVal);
}

void display_pin_values()
{
    Serial.print("Pin States:\r\n");

    for(int i = 0; i < DATA_WIDTH; i++)
    {
        Serial.print("  Pin-");
        Serial.print(i);
        Serial.print(": ");

        if((pinValues >> i) & 1)
            Serial.print("HIGH");
        else
            Serial.print("LOW");

        Serial.print("\r\n");
    }

    Serial.print("\r\n");
}



void loop()
{

  /* Read the state of all zones.
  */
  pinValues = read_shift_regs();

  /* If there was a chage in state, display which ones changed.
  */
  if(pinValues != oldPinValues)
  {
      Serial.print("*Pin value change detected*\r\n");
      display_pin_values();
      oldPinValues = pinValues;


  }

  delay(POLL_DELAY_MSEC);



//probably should be on an interrupt, to catch high->low transition

  // Only do something if the pin is different from previous state.
  // boolean newCaps = digitalRead(CAPSPin) ? false : true;
  boolean newCaps = !bitRead(pinValues, 4);

  if ( (CAPSLock != newCaps) && !resetCapsLock)
  {
    CAPSLock = newCaps;    // Remember new CAPSLock.
    Keyboard.set_key6((uint8_t) KEY_CAPS_LOCK);    // Send KEY_CAPS_LOCK.
    dTime = millis();                    // Reset delay timer.
    resetCapsLock = true;

   Serial.print("Caps = ");
   Serial.println(CAPSLock);

    // Turn on the LED for caps lock.
    digitalWrite(LED, CAPSLock ? HIGH : LOW);

  }

  if ( resetCapsLock && (millis()-dTime) > 10)
  {
    Keyboard.set_key6(KEY_CAPS_UNLOCK);
    resetCapsLock = false;
  }

  // If caps lock is set, then turn number keys into function keys.
  FKEYS = CAPSLock;

/*char CAPSLock = digitalRead(CAPSPin);
    if (CAPSLock == LOW) {
      Keyboard.set_key6(KEY_CAPS_LOCK);
    } else {
      Keyboard.set_key6(0);89
    }

  */
  //  char SHIFTState = digitalRead(SHIFTPin);
  char SHIFTState = bitRead(pinValues, 5);

    if (SHIFTState == LOW) {
      modifierKeys[0] = MODIFIERKEY_SHIFT;
    } else {
      modifierKeys[0] = 0;
    }

  //  char CTRLState = digitalRead(CTRLPin);
  char CTRLState = bitRead(pinValues, 7);

    if (CTRLState == LOW) {
      modifierKeys[1] = MODIFIERKEY_CTRL;
    } else {
      modifierKeys[1] = 0;
    }

   char OAPPLEState = bitRead(pinValues, 0);
   char CAPPLEState = bitRead(pinValues, 1);

/*    if (OAPPLEState == HIGH) {
      modifierKeys[2] =  MODIFIERKEY_GUI;
    } else if (CAPPLEState == HIGH) {
      modifierKeys[2] =  MODIFIERKEY_GUI;
    } else {
      modifierKeys[2] = 0;
    }
      digitalWrite(APPLEPin1, LOW);
      digitalWrite(APPLEPin2, LOW);
*/


// *** NOW USING CLOSED APPLE AS ALT/OPTION
    if (OAPPLEState == HIGH) {
      modifierKeys[2] =  MODIFIERKEY_GUI;
    } else {
      modifierKeys[2] = 0;
    }

    if (CAPPLEState == HIGH) {
      modifierKeys[3] =  MODIFIERKEY_ALT;
    } else {
      modifierKeys[3] = 0;
    }


// to use the TILDE key as ALT/OPTION
/*    modifierKeys[3] = 0;
    if( KPD.isPressed(KEY_TILDE) ) {
      modifierKeys[3] = MODIFIERKEY_ALT;
    }
*/

// *** NOW USING CLOSED APPLE AS ALT/OPTION;

  Keyboard.set_modifier( modifierKeys[0] | modifierKeys[1] | modifierKeys[2] | modifierKeys[3] );

  KPD.getKeys(); // Scan for all pressed keys. 6 Max, + 4 modifiers. Should be plenty, but can be extended to 10+

	// Set keyboard keys to default values.
	Keyboard.set_key1(0);
	Keyboard.set_key2(0);
	Keyboard.set_key3(0);
	Keyboard.set_key4(0);
	Keyboard.set_key5(0);
	//Keyboard.set_key6(0);


  /* based on suggestion from Craig Brooks <s.craig.brooks@gmail.com>
     uses CAPS LOCK to turn number keys into F-Key equivalent.
  */


	// Update keyboard keys to active values.
	if ( KPD.key[0].kchar && ( KPD.key[0].kstate==PRESSED || KPD.key[0].kstate==HOLD ))
	{
    Serial.println(FKEYS);

    if (FKEYS)
    {
      // number keys 1 through 0 for f1 - f10
      if ((KPD.key[0].kchar >= 0x1E) &&  (KPD.key[0].kchar <= 0x27))
      {
        KPD.key[0].kchar += 0x1C;
//      Serial.println( KPD.key[0].kchar, HEX );
      }
      else if ( KPD.key[0].kchar == 0x2D || KPD.key[0].kchar == 0x2E )
      {
//      - and = for f11 and f12
        KPD.key[0].kchar += 0x17;
      }
    }

    Keyboard.set_key1( KPD.key[0].kchar );
  }


	if ( KPD.key[1].kchar && ( KPD.key[1].kstate==PRESSED || KPD.key[1].kstate==HOLD ))
		Keyboard.set_key2( KPD.key[1].kchar );

	if( KPD.key[2].kchar && ( KPD.key[2].kstate==PRESSED || KPD.key[2].kstate==HOLD ))
		Keyboard.set_key3( KPD.key[2].kchar );

	if( KPD.key[3].kchar && ( KPD.key[3].kstate==PRESSED || KPD.key[3].kstate==HOLD ))
		Keyboard.set_key4( KPD.key[3].kchar );

	if( KPD.key[4].kchar && ( KPD.key[4].kstate==PRESSED || KPD.key[4].kstate==HOLD ))
		Keyboard.set_key5( KPD.key[4].kchar );

	//if( KPD.key[5].kchar && ( KPD.key[5].kstate==PRESSED || KPD.key[5].kstate==HOLD ))
	//	Keyboard.set_key6( KPD.key[5].kchar );


	Keyboard.send_now();
	Keyboard.set_modifier(0);
}
