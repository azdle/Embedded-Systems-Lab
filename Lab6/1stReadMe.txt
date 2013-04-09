PIC32 USB Starter Kit II: USB Device - CDC - Basic Demo
---------------------------------------------------------
 
Required Development Resources:
-------------------------------
a. PIC32 USB Starter Kit II (DM320003-2)
b. MPLAB IDE.
c. C Compiler.

Optional Development Resources:
-------------------------------
Microsoft Applications Library (MAL)


Compiling and Downloading the Demo:
-----------------------------------
1. Load the demo code into MPLAB by double clicking the appropriate
   .mcp project file:

     USB Device - CDC - Basic Demo -  PIC32 ESK with MAL.mcp
     USB Device - CDC - Basic Demo -  PIC32 ESK wo MAL.mcp

   depending on whether the MAL is installed or not.

   If the Microchip Applications Library is installed, follow the
   instructions in ./Source/WhereToFindFiles.txt to copy the needed
   MAL files into the ./Source directory.  You may also follow the
   instructions in ./USB/WhereToFindIncludeFiles.txt to update the
   header (.h) files found the ./USB directory.

2. Connect the mini-B debugger port on-board the Starter board to an
   USB port on the development computer using the USB cable provided
   in the kit.

3. Choose the PIC32 Starter Kit debugger tool in MPLAB IDE by
   selecting Debugger>Select Tool and then click on PIC32 Starter
   Kit.

4. Build the project by selecting Project>Build All.

5. Download your code into the evaluation board microcontroller by
   selecting Debugger>Programming>Program All Memories.

6. Run the demo code by selecting Debugger>Run.


Running the Demo:
-----------------
This demo allows the device to appear like a serial (COM) port to the
host via a USB port on your PC.

1. Once demo is loaded and running, connect the micro-B USB port on-
   board the Starter board to an USB port ont eh development computer
   using the USB cable provided in the kit.

2. If prompted, install driver located in the .\inf folder included
   with the demo project.  If the device driver is not after this
   step, use the Device Manager found under Control Panel to
   manually install the driver.

3. Open a terminal program, such as Hyperterminal.  Select the
   appropriate COM port.  On most machines this will be COM5 or
   higher.

   If you are using Windows 7, you will need to download and install a
   Hyperterminal replacement such as "Hyperterminal Private Edition"
   or copy Hyperterminal software from a PC with Windows XP.

   For more information:
      http://helpdeskgeek.com/windows-7/windows-7-hyperterminal/

4. Once connected to the device, the firmware responds in two ways:

   1) Typing a key in the terminal window will result in the device
      echoing the next key in the ASCII character set.  So if you
      press the "a" key, the device will echo "b".

   2) If the SW1 pushbutton is pressed the device will echo
      "Button Pressed" back to the Hyperterminal window.

Note: Some terminal programs, like hyperterminal, require users to
click the disconnect button before removing the device from the
computer.  Failing to do so may result in having to close and open the
program again in order to reconnect to the device


There are three USB connectors on the edge of the USB Starter Kit II board:

    1) Mini-B for the MPLAB debugger
    2) Standard Type-A when board serves as a USB host 
       (example: plugging in a memory stick)
    3) Micro-B for plugging the board into a host (such as your PC)
       as a USB device, as with this demo

Here's a simple diagram of the edge of the board:

         Debug               USB Host
        (Mini-B)             (Std-A)
        ============================   USB Starter Kit II Board
                            (Micro-B)
                            USB Device

There are three sets of switches and LEDs on the right hand edge of
the top of the board:

 +-----------------------------+
 + Debug                       +
 + USB                 SW/LED1 +  Switch 1 toggles cursor gyrations
 +                             +
 +          =========          +
 +          |       |  SW/LED2 +  LED2 glows when board is powered.
 +          | PIC32 |          +
 +          | CHIP  |          +  LED1/LED2 alternate when the board
 +          |       |          +  is recognized as USB device.
 +          =========          +
 +                     SW/LED3 +
 + Host                        +
 + USB                         +
 +-----------------------------+

After starting the demo application on the board (Step 6. above)
simply plug the board into your PC with a Standard A to Micro-B USB
cable.  LEDs 1 and 2 will start alternating.  Your PC will then
attempt to install the driver for the new device the ESK board
presents on the USB port.  After the driver is installed (either
automatically or through Device Manager, move to the Hyperterminal
application on your PC to see the firmware in action.


