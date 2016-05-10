""" Copyright (c) Microsoft. All rights reserved."""

import _wini2c as i2c
import time

sample_temp_hold = bytes([0xe3])
sample_humidity_hold = bytes([0xe5])

htdu21d = i2c.i2cdevice(0, 0x40, i2c.FASTSPEED, i2c.SHAREDMODE)

while True:
    temp_data = htdu21d.writeread(sample_temp_hold, 3)
    tempReading = temp_data[0] << 8 | temp_data[1]
    tempRatio = tempReading/65536.0
    temperature = (-46.85 + (175.72 * tempRatio)) * 9 / 5 + 32

    print("Temperature: ",temperature)

    humidity_data = htdu21d.writeread(sample_humidity_hold, 3)
    humidityReading = humidity_data[0] << 8 | humidity_data[1]
    humidityRatio = humidityReading / 65536.0
    humidity = -6 + (125 * humidityRatio)
    print("Humidity: " , humidity)

    time.sleep(1)
