/*
 * MSF for antenna tunning
 * Adjust coil over ferrite until you get the cleanest signal and then glue or 
 * wax the ferrite in plae.
 * 
 * Carrier on-off keying
 * Minute marker (second 00)
 *                 ________________
 * _______________|                
 *       500ms          500ms
 *       
 * Other seconds start with 100ms then Bit A and B are transmitted followed by 700ms of carrier on
 * __                         ______________
 *   |_______|   A   |   B   |
 *     100ms  100ms   100ms     700ms
 * 
 * Bits A and B have negative polarity
 * 
 * But my reciever inverts the output... confusing.
 */
const int MSF_IN_PIN = 2;
const int MSF_EN_PIN = 3;
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(MSF_IN_PIN, INPUT_PULLUP);
  pinMode(MSF_EN_PIN, OUTPUT);
  digitalWrite(MSF_EN_PIN, LOW); // inverted
  Serial.println("LOW, HIGH");
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long timer;

  timer = millis();
  while(digitalRead(MSF_IN_PIN)== HIGH);
  unsigned long low_time = millis() - timer;
  timer = millis();
  while(digitalRead(MSF_IN_PIN)== LOW);
  unsigned long high_time = millis() - timer;

    Serial.print(low_time);
    Serial.print(",");
    Serial.println(high_time);
}
