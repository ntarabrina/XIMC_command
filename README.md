# Program for sending commands to XIMC controller from AVR controller
### (Written in AtmelStudio7 for Atmel ATMEGA2560 16AU 1432 and used with XIMC controller and step motor ST5918L3008-B)

##### What does this program:
1. Gets current position of the step motor.
2. Moves it to the left for 3 seconds and stops the step motor.
3. Moves it to the init position (position from get position command (p.1)).
---
##### Before using you need to check if you have this:
1. XIMC controller.
2. Step motor.
3. AVR controller Atmel ATMEGA2560 16AU 1432.
4. USBASP ATMEGA8A AU 1605.

##### And this programs and drivers:
1. Atmel Studio 7 (or other program for writing code for AVR controller).
2. AvrDude (for broaching AVR controller).
3. Driver for USBASP.
---
##### For assembling you need:
1. To flash controller:
    * connect ATMEGA2560 to USBASP:
       - on ATMEGA2560: ICSP pins (1-6) ![alt text](https://pp.userapi.com/c846524/v846524467/a7dbc/gJnjHK02TFQ.jpg)
    * put the HEX-file (XIMC_command.hex) to the exact directory (for example: to the OS(C:\\))
    * open command line
    * change main directory to your chosen directory with HEX-file (write 'cd C:\\' (or name of your directory instead of 'C:\\'))
    * write 'avrdude -p atmega2560 -c usbasp -U flash:w:\XIMC_command.hex' (this will load the program code to the ATMEGA2560)
2. To use UART on XIMC controller see [Serial Port in XIMC support](https://doc.xisupport.com/en/8smc5-usb/8SMCn-USB/Technical_specification/Additional_features/Serial_port.html#serial-port)
3. Connect the XIMC controller UART (see serial port pins [BackPlane Connector in XIMC support](https://doc.xisupport.com/en/8smc5-usb/8SMCn-USB/Technical_specification/Appearance_and_connectors/Controller_board.html#backplane-connector)) to ATMEGA2560 UART3 (Pin 14 - TX3; Pin 15 - RX3)
4. Reset the ATMEGA2560 for pushing the commands.

##### Above View:
![alt text](https://pp.userapi.com/c844617/v844617467/ad289/7i6zREsGj2Q.jpg)
