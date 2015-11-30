
// The Arduino Wiring application template is documented at 
// http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

void setup()
{
    // put your setup code here, to run once:

    pinMode(GPIO_5, OUTPUT);
}

void loop()
{
    // put your main code here, to run repeatedly:

    digitalWrite(GPIO_5, LOW);
    delay(500);
    digitalWrite(GPIO_5, HIGH);
    delay(500);
}
