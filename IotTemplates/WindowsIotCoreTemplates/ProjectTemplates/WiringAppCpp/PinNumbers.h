#pragma once

// This file contains the pin numbers mappings for MinnowBoard Max and Raspberry Pi2 controllers

#if defined(_M_IX86) || defined(_M_X64)

// Pin numbers mappings for MinnowBoard Max

const UCHAR GPIO_0 = 21;
const UCHAR GPIO_1 = 23;
const UCHAR GPIO_2 = 25;
const UCHAR GPIO_3 = 14;
const UCHAR GPIO_4 = 16;
const UCHAR GPIO_5 = 18;
const UCHAR GPIO_6 = 20;
const UCHAR GPIO_7 = 22;
const UCHAR GPIO_8 = 24;
const UCHAR GPIO_9 = 26;
const UCHAR I2C_SCL = 13;
const UCHAR I2C_SDA = 15;
const UCHAR SPI_CS0 = 5;
const UCHAR SPI_MISO = 7;
const UCHAR SPI_MOSI = 9;
const UCHAR SPI_SCLK = 11;
const UCHAR UARAT1_CTS = 10;
const UCHAR UARAT1_RTS = 12;
const UCHAR UARAT1_RX = 8;
const UCHAR UARAT1_TX = 6;
const UCHAR UARAT2_RX = 19;
const UCHAR UARAT2_TX = 17;

#elif defined (_M_ARM)

// Pin numbers mappings for Raspberry Pi2

const UCHAR GPIO_2 = 3;
const UCHAR GPIO_3 = 5;
const UCHAR GPIO_4 = 7;
const UCHAR GPIO_5 = 29;
const UCHAR GPIO_6 = 31;
const UCHAR GPIO_7 = 26;
const UCHAR GPIO_8 = 24;
const UCHAR GPIO_9 = 21;
const UCHAR GPIO_10 = 19;
const UCHAR GPIO_11 = 23;
const UCHAR GPIO_12 = 32;
const UCHAR GPIO_13 = 33;
const UCHAR GPIO_14 = 8;
const UCHAR GPIO_15 = 10;
const UCHAR GPIO_16 = 36;
const UCHAR GPIO_17 = 11;
const UCHAR GPIO_18 = 12;
const UCHAR GPIO_19 = 35;
const UCHAR GPIO_20 = 38;
const UCHAR GPIO_21 = 40;
const UCHAR GPIO_22 = 15;
const UCHAR GPIO_23 = 16;
const UCHAR GPIO_24 = 18;
const UCHAR GPIO_25 = 22;
const UCHAR GPIO_26 = 37;
const UCHAR GPIO_27 = 13;
const UCHAR GPIO_GCLK = 7;
const UCHAR GPIO_GEN0 = 11;
const UCHAR GPIO_GEN1 = 12;
const UCHAR GPIO_GEN2 = 13;
const UCHAR GPIO_GEN3 = 15;
const UCHAR GPIO_GEN4 = 16;
const UCHAR GPIO_GEN5 = 18;
const UCHAR I2C_SCL1 = 5;
const UCHAR I2C_SDA1 = 3;
const UCHAR SPI_CS0 = 24;
const UCHAR SPI_CS1 = 26;
const UCHAR SPI_CLK = 23;
const UCHAR SPI_MISO = 21;
const UCHAR SPI_MOSI = 19;
const UCHAR RXD0 = 10;
const UCHAR TXD0 = 8;

#endif
