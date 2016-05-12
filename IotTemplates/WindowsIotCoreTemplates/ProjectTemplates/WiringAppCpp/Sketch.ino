// The Arduino Wiring application template is documented at 
// http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

// Use GPIO pin 5
const unsigned int LED_PIN = GPIO5;

void setup()
{
    // put your setup code here, to run once:

    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    // put your main code here, to run repeatedly:

    digitalWrite(LED_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(500);
}
