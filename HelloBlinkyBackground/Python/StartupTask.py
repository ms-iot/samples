""" Copyright (c) Microsoft. All rights reserved."""

import _wingpio as gpio
import time

led_pin = 5
pinValue = gpio.HIGH

gpio.setup(led_pin, gpio.OUT, gpio.PUD_OFF, gpio.HIGH)

while True:
    if pinValue == gpio.HIGH:
        pinValue = gpio.LOW
        gpio.output(led_pin, pinValue)
    else:
        pinValue = gpio.HIGH
        gpio.output(led_pin, pinValue)

    time.sleep(0.5)

gpio.cleanup()
