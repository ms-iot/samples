import _wingpio as gpio
import time

led_pin = 5
ledstatus = 0

gpio.setup(led_pin, gpio.OUT, gpio.PUD_OFF, gpio.HIGH)

while True:
    if ledstatus == 0:
        ledstatus = 1
        gpio.output(led_pin, gpio.HIGH)
    else:
        ledstatus = 0
        gpio.output(led_pin, gpio.LOW)

    time.sleep(0.5)

gpio.cleanup()
