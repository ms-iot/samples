void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}
int incoming=0;

void loop() {
  // put your main code here, to run repeatedly:

   //Serial.write("hello world");
   //delay(1000);

 while(Serial.available()>0)
  {
    incoming=Serial.read();
    Serial.write(incoming);

    
  }
  
}
