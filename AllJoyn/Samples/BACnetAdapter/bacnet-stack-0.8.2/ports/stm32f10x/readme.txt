This port was done with the STM32 ARM Cortex-M3 STM32F103RGT6 on 
a STM32 Discovery Kit using the STM32 CMSIS library and drivers
and IAR EWARM 6.10 compiler.

The CMSIS library was 21MiB compressed, so I didn't include it
as part of this project.  The CMSIS and drivers 
can be found by following the 'Click here for STM32
embedded firmware' link from the resources page:
http://www.st.com/stonline/stappl/resourceSelector/app?page=resourceSelector&doctype=FIRMWARE&SubClassID=1169
There will be a list of firmware resources.  
The library you are looking for is in the 
‘ARM-based 32-bit MCU STM32F10xxx standard peripheral library’.
Download the 
‘ARM-based 32-bit MCU STM32F10xxx standard peripheral library’ 
and the CMSIS library can be found in 
'…\STM32F10x_StdPeriph_Lib_V3.4.0\Libraries\CMSIS\CM3'. 
Copy the contents of 'CMSIS' to the 'CMSIS' folder in this project.
and the drivers library can be found in 
'…\STM32F10x_StdPeriph_Lib_V3.4.0\Libraries\STM32F10x_StdPeriph_Driver'. 
Copy the contents of 'STM32F10x_StdPeriph_Driver' to the 
'drivers' folder in this project.

The hardware interface only uses the USART and a peripheral pin
(RTS) for the MS/TP RS-485 interface, and the System Clock for 
the millisecond timer.

It was created for the STM32 Design Challenge on March 20, 2011,
by Steve Karg.  Although the design didn't win any awards, 
it was one of the six finalists and was on display at the 
STM booth at the 2010 Embedded Systems Conference West.
http://www.stm32challenge.com/
