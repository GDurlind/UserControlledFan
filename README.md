# UserControlledFan
C++ code for an embedded system that allows for a user to control a fan


# EE30185 INTEGRATED ENGINEERING - README 

—————————————————————————————————————————————————————————————————————————————

This embedded system allows for a user to control the speed of a fan, as well as selecting which control system the fan uses.  

—————————————————————————————————————————————————————————————————————————————

# HARDWARE REQUIREMENTS
1. 12V Power Supply
2. STM32 F070RB NUCLEO Board with Push Button
3. Extension board
4. 16x2 LCD Screen
5. 12cm Auto Thermal Fan
6. Bi-directional LED
—————————————————————————————————————————————————————————————————————————————

# DEPENDENCIES

‘cstdio’: This is a standard C library that provides input and output functions for providing basic operations.

‘PinNamesTypes.h’: Header file that defines the data types and constants used in the mbed library for representing pin modes and pin names.

‘PwmOut.h’: This library is used to generate PWM signals to control the fan.

‘mbed.h’: This is the main library for the mbed studio platform, which provides access to various hardware peripherals and functions.

‘queue’: This library is used to implement a queue data structure. 

‘TextLCD.h’: This library is used to interface with an LCD screen.

‘chromo’: This library is used to measure time in the programme.

—————————————————————————————————————————————————————————————————————————————

# USER MANUAL

1. Connect the 12V Power Supply to the extension board and turn it on.

2. Plug the NUCLEO Board + extension board setup into a computer.

3. Start the Bin file.

4. The system will initially enter an open-loop control system, at 50% of its max speed.

5. This information should be seen on the LCD screen, along with the green LED on the extension board that signifies the system is using open-loop control.

6. The rotary encoder can now be used to increase/decrease this %, with any changes being updated on the LCD.

7. To switch the system to closed-loop control, the push button on the NUCLEO Board can be pressed.

8. When pressed, the LCD will indicate closed-loop control is now being used, as well as the LED on the extension board switching to red. 

9. The rotary encoder can now be used again to increase/decrease the RPM of the fan up to its maximum of 1750 RPM. With the 'S' representing the speed of the fan and the 'T' representing the target speed. 

10. The fan will typically stop spinning around 15%/650rpm.
