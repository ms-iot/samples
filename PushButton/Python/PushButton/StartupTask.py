import _wingpio as gpio

led_pin = 6;
button_pin = 5;

ledPinValue = gpio.HIGH;

gpio.setup(button_pin, gpio.IN, gpio.PUD_UP, None);
gpio.setup(led_pin, gpio.OUT);
print("GPIO pins initialized correctly.")

gpio.output(led_pin, gpio.HIGH);

def buttonPin_ValueChanged(sender):
    if gpio.input(button_pin):
        if ledPinValue == gpio.HIGH:
            ledPinValue = gpio.LOW;
        else:
            ledPinValue = gpio.HIGH;

        print("Button released")
    else:
        print("Button Pressed")

gpio.add_event_detect(button_pin, gpio.BOTH, callback=buttonPin_ValueChanged, bouncetime=50);
print("GPIO event registered")