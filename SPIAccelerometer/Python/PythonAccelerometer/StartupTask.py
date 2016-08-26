""" Copyright (c) Microsoft. All rights reserved."""

import time
import sys
import _winspi as spi

ACCEL_REG_POWER_CONTROl = 0x2d      # Address of the Power Control register
ACCEL_REG_DATA_FORMAT = 0x31        # Addres of the Data Format register
ACCEL_REG_X = 0x32                  # Address of the X Axis data register
ACCEL_REG_Y = 0x34                  # Address of the Y Axis data register
ACCEL_REG_Z = 0x36                  # Address of the Z Axis data register

SPI_CHIP_SELECT_LINE = 0            # Chip select line to use
ACCEL_SPI_RW_BIT = 0x80             # Bit used in SPI transaction to indicate read write
ACCEL_SPI_MB_BIT = 0x40             # Bit used to indicate multi-byte SPI transactions

SPI_CLOCK_FREQUENCY = 5000000       # 5Mhz is the rated speed of the ADXL345 accelerometer
SPI_MODE = spi.MODE3;               # The accelerometer expects an idle-high clock polarity, we use Mode3
                                    # to set the clock polarity and phase to: CPOL = 1, CPHA = 1

# Initialize the spi accelerometer
def spi_init():
    spi_accel = spi.spidevice(0, SPI_CHIP_SELECT_LINE, SPI_CLOCK_FREQUENCY, 8, SPI_MODE)
    if spi_accel is None :
        raise RuntimeError("Cannot get Spi device")

    """
      Initialize the accelerometer:

      For this device, we create 2-byte write buffers:
      The first byte is the register address we want to write to.
      The second byte is the contents that we want to write to the register.
    """
    data_format_buffer = bytes([ACCEL_REG_DATA_FORMAT, 0x1])        # 0x01 sets range to +- 4Gs
    power_control_buffer = bytes([ACCEL_REG_POWER_CONTROl, 0x8])    # 0x08 puts the accelerometer into measurement mode

    spi_accel.write(data_format_buffer)
    spi_accel.write(power_control_buffer)

    return spi_accel


def display_data(x, y, z):
    print("X Axis: %5.2f G       Y Axis: %5.2f G       Z Axis: %5.2f G" % (x, y, z))

def read_data(spi_accel):
    reg_addr_buffer = bytearray(1 + 6)  # Register address buffer of size 1 byte + 6 bytes padding
    reg_addr_buffer[0] = ACCEL_REG_X | ACCEL_SPI_RW_BIT | ACCEL_SPI_MB_BIT
    read_buffer = spi_accel.transfer(reg_addr_buffer, 6 + 1) # Read buffer of size 6 bytes (2 bytes * 3 axes) + 1 byte padding

    # Discard first dummy byte from read
    read_buffer = read_buffer[1:7]

    # In order to get the raw 16-bit data values, we need to concatenate two 8-bit bytes for each axis
    x_raw = int.from_bytes(read_buffer[0:2], byteorder=sys.byteorder, signed = True)
    y_raw = int.from_bytes(read_buffer[2:4], byteorder=sys.byteorder, signed = True)
    z_raw = int.from_bytes(read_buffer[4:6], byteorder=sys.byteorder, signed = True)

    # Convert raw values to G's
    ACCEL_RES = 1024;                               # The ADXL345 has 10 bit resolution giving 1024 unique values
    ACCEL_DYN_RANGE_G = 8;                          # The ADXL345 had a total dynamic range of 8G, since we're configuring it to +-4G
    UNITS_PER_G = ACCEL_RES / ACCEL_DYN_RANGE_G;    # Ratio of raw int values to G units

    x = x_raw / UNITS_PER_G
    y = y_raw / UNITS_PER_G
    z = z_raw / UNITS_PER_G

    return x, y, z

try:
    spi_accel = spi_init()
    for i in range(0, 10):
        x, y, z = read_data(spi_accel)
        display_data(x, y, z)

        time.sleep(1)
except Exception as e:
    print("Error: ", e)
