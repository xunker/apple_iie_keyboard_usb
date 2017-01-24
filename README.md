## Apple IIe Keyboard USB

Arduino project to convert Apple IIe keyboard to USB using Teensy 2.0.

This is a modified version of [RetroConnector's IIe-to-USB keyboard adapter code](https://github.com/option8/RetroConnector/blob/master/IIe-USB),
changed to work with the Teensy 2.0 (atmega32u4) instead of the Teensy++. This
modification also requires a parallel-in serial-out shift register such as as
the 74HC165 or 74LS165.

### Differences from the original code

The Apple IIe keyboard requires 24 pins, but the Teensy 2.0 only has 23 pins
available while it's in HID USB mode.

This modified code works around this limitation by using 18 pins for the X/Y
lines, and 4 pins to drive the shift register to read CONTROL, SHIFT, CAPS LOCK,
OPEN/CLOSED APPLE and RESET.

This leaves one pin free that we give to the built-in LED to show the Caps-Lock
state.

### Wiring

#### Keyboard connector to Teensy 2.0

Connections from the 26-pin connector on the Apple IIe keyboard to the pins on
the Teensy 2.0 pins. "--" denotes connection to the shift register, covered
below.

```
IIe Connector   Col/Row   Arduino IDE Pin   Teensy 2.0 Pin

1               Y0        0                 B0
2               Y1        1                 B1
3               +5V       +5v               +5v
4               Y2        2                 B2
5               SW1/CAPL  --                --
6               Y3        3                 B3
7               SW0/OAPL  --                --
8               Y4        24                E6
9               CAPLOCK   --                --
10              Y5        4                 B7
11              CNTL      --                --
12              Y8        5                 D0
13              GND       GND               GND
14              X0        6                 D1
15              RESET     --                --
16              X2        9                 C6
17              X7        17                F6
18              X1        10                C7
19              X5        16                F7
20              X3        23                D5
21              X4        15                B6
22              Y9        22                D4
23              Y6        14                B5
24              SHIFT     --                --
25              Y7        13                B4
26              X6        12                D7
```

Wiring note: With the keyboard sitting in front of you as though you were typing
on it, pins 25/26 will be on the end of the connector closest to you, and pins
1/2 will be on the end of the connector farthest from you:

```
+----------------+
|                |  +-------+
|                |  |  1/2  |
|    KEYBOARD    |  |   |   |
|                |  | 25/26 |
|                |  +-------+
+----------------+  
```

#### Modifier keys to shift register

Follows DIP-8 package: http://www.ti.com/lit/ds/symlink/sn74ls165a.pdf

```
IIe Connector   Col/Row   Shift Register Input  Notes

5               SW1/CAPL  B (pin 12)            Active High
7               SW0/OAPL  A (pin 11)            Active High
9               CAPLOCK   E (pin 3)             Active Low, needs pull-up
11              CNTL      H (pin 6)             Active Low, needs pull-up
15              RESET     G (pin 5)             Active Low, needs pull-up, active low (only active with Control)
24              SHIFT     F (pin 4)             Active Low, needs pull-up
```

#### Shift register to Teensy 2.0

```
Teensy 2.0 Pin    Shift Register Pin

18/F5             1:  Parallel Load / SH/LD   (active low)
19/F4             15: Clock Enable / CLK INH  (active low)
20/F1             2:  Clock Pulse / CLK       (low-to-high edge triggered)
21/F0             9:  Data Pin (Q7)
+5v               16: VCC
GND               8:  GND
```

Follows DIP-8 package: http://www.ti.com/lit/ds/symlink/sn74ls165a.pdf

#### Other Pins

Leave these unconnected.

```
Teensy 2.0  Purpose

7/D2        USB RX
8/D3        USB TX
11/D6       Caps Lock LED (LED_BUILTIN)
```

### Changes

Jan 24, 2017: Initial Commit

### Credits

This modification is by Matthew Nielsen, and can be found at:
https://github.com/xunker/apple_iie_keyboard_usb

This is based on the original code by RetroConnector:
https://github.com/option8/RetroConnector/blob/master/IIe-USB

Shift Register code is pulled from the Arduino Playground:
http://playground.arduino.cc/Code/ShiftRegSN74HC165N

It uses the Keypad library, by Mark Stanley and Alexander Brevig:
http://www.arduino.cc/playground/Code/Keypad

Keyboard pinout information is from Apple.info:
http://apple2.info/wiki/index.php?title=Pinouts#Apple_.2F.2Fe_Motherboard_keyboard_connector
