<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> de59284... Fixing publisher names and adding copyright message
"""
Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.

   The MIT License(MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal 
    in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject tothe following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
"""
<<<<<<< HEAD
=======
>>>>>>> c992184... Adding python weather station
=======
>>>>>>> de59284... Fixing publisher names and adding copyright message
import _wini2c as i2c
import time

sample_temp_hold = bytes([0xe3])
sample_humidity_hold = bytes([0xe5])

htdu21d = i2c.i2cdevice('I2C1', 0x40, i2c.FASTSPEED, i2c.SHAREDMODE)
<<<<<<< HEAD
<<<<<<< HEAD
=======
gpio.setup(5,gpio.OUT)
>>>>>>> c992184... Adding python weather station
=======
>>>>>>> d351786... Fixes python samples, adding license text and Microsoft as publisher for others

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