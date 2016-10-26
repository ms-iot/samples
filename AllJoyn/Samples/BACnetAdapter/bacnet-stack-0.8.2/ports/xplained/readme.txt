BACnet MS/TP on Atmel XMEGA-A3BU XPLAINED evaluation board.

An RS-485 add-on board (daughterboard, shield) was designed
to handle the RS-485 interface and some LEDs.  See rs485-shield/
folder for EAGLE CAD design files.

Use the Configuration "Debug-XPLAINED" and not in "Debug" or "Release".
"CONF_BOARD_ENABLE_RS485_XPLAINED" is defined and used in led.c, rs485.c,
and main.c for specific board I/O. When it is not defined, the I/O is
either removed (i.e. led.c, main.c) or altered (rs485.c).

There are other defines in the "Debug-XPLAINED" Configuration
which include other parts of the XPLAINED platform code.

For your own board, you could just change rs485.c, main.c, and led.c
to use the I/O that you want to use, and not worry about the
"CONF_BOARD_ENABLE_RS485_XPLAINED". Or you can leave the
"CONF_BOARD_ENABLE_RS485_XPLAINED" in the files so that you can
always test on the XPLAINED platform with Debug-XPLAINED, and use
"Debug" or "Release" for your project.