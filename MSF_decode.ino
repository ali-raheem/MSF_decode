/*
 * MSF protocol
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
const int LED_PIN = 13;
const int NANOS_IN_MILLI = 1000;
bool LED_STATE = false;
const unsigned long JITTER = 50L * NANOS_IN_MILLI;
const int DEBUG = false;

byte A[64], B[64]; // FIXME should be bitfield.
unsigned int bitIndex;
unsigned int year = 0, month = 0, dayOfMonth = 0, dayOfWeek = 0, hours = 0, minutes = 0;
bool TIME_GOOD = false;

unsigned int get_bit(byte *bits, int i) {
  return bits[i];
}
inline bool approxEq(const unsigned long X, const unsigned long Y, const unsigned long J) {
  return (X + J > Y && X - J < Y);
}
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(MSF_IN_PIN, INPUT_PULLUP);
  pinMode(MSF_EN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(MSF_EN_PIN, LOW); // inverted
  bitIndex = 0;
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
  if(approxEq(low_time, 500, 50) && approxEq(high_time, 500, 50)) {
    Serial.println("Minute marker");
    bitIndex = 0;
    return;
  }else{
    bitIndex = (++bitIndex) % 64;
  }
  if (approxEq(low_time, 100, 50)) {
    if (DEBUG) {
      Serial.print(bitIndex);
      Serial.print(" BitA 0,");
    }
    A[bitIndex] = 0;
    if (approxEq(high_time, 900, 50)) {
      if (DEBUG) Serial.println("BitB 0");
      B[bitIndex] = 0;
    }else {
      B[bitIndex] = 1;
    }
  }else if (approxEq(low_time, 200, 50)) {
    if (DEBUG) {
      Serial.print(bitIndex);
      Serial.println(" BitA 1, BitB 0");
    }
    A[bitIndex] = 1;
    B[bitIndex] = 0;
  }else if (approxEq(low_time, 300, 50)){
    if (DEBUG) {
      Serial.print(bitIndex);
      Serial.println(" BitA 1, BitB 1");
    }
    A[bitIndex] = 1;
    B[bitIndex] = 1;
  }else {
          Serial.print(low_time);
      Serial.print(",");
      Serial.println(high_time);
    Serial.print(bitIndex);
    Serial.println(" Bad bits");
  }

  if(bitIndex == 58) {
    printTimeDate();
    Serial.println(TIME_GOOD ? "" : "Bad time");
  }
}

void printTimeDate() {
  year = 80 * get_bit(A, 17) + 40 * get_bit(A, 18) + 20 * get_bit(A, 19) + 10 * get_bit(A, 20) + 
         8 * get_bit(A, 21) + 4 * get_bit(A, 22) + 2 * get_bit(A, 23) + 1 * get_bit(A, 24);
  month = 10 * get_bit(A, 25) + 8 * get_bit(A, 26) + 4 * get_bit(A, 27) + 2 * get_bit(A, 28) + 1 * get_bit(A, 29);
  dayOfMonth = 20 * get_bit(A, 30) + 10 * get_bit(A, 31) + 8 * get_bit(A, 32) + 
               4 * get_bit(A, 33) + 2 * get_bit(A, 34) + 1 * get_bit(A, 35);
  dayOfWeek = 4 * get_bit(A, 36) + 2 * get_bit(A, 37) + 1 * get_bit(A, 38);
  hours = 20 * get_bit(A, 39) + 10 * get_bit(A, 40) + 8 * get_bit(A, 41) + 
          4 * get_bit(A, 42) + 2 * get_bit(A, 43) + 1 * get_bit(A, 44);
  minutes = 40 * get_bit(A, 45) + 20 * get_bit(A, 46) + 10 * get_bit(A, 47) + 
            8 * get_bit(A, 48) + 4 * get_bit(A, 49) + 2 * get_bit(A, 50) + 1 * get_bit(A, 51);

  int i;
  int yearParity = 0;
  for(i = 17; i <= 24; i++)
    yearParity += A[i];
  yearParity += B[54];
  yearParity %= 2;
  if(yearParity)
    Serial.println("Year Parity GOOD");
  int dateParity = 0;
  for(i = 25; i <= 35; i++)
    dateParity += A[i];
  dateParity += B[55];
  dateParity %= 2;
  if(dateParity && DEBUG)
    Serial.println("Date Parity GOOD");
  int dayParity = 0;
  for(i = 36; i <= 38; i++)
    dayParity += A[i];
  dayParity += B[56];
  dayParity %= 2;
  if(dayParity && DEBUG)
    Serial.println("Day Parity GOOD");
  int timeParity = 0;
  for(i = 39; i <= 51; i++)
    timeParity += A[i];
  timeParity += B[57];
  timeParity %= 2;
  if(timeParity && DEBUG)
    Serial.println("Time Parity GOOD");

  TIME_GOOD = (yearParity && dateParity && dayParity && timeParity);

  char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  char *days[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  Serial.print(days[(dayOfWeek-1) % 7]);
  Serial.print(" ");
  Serial.print(dayOfMonth);
  Serial.print("-");
  Serial.print(months[(month-1) % 12]);
  Serial.print("-");
  Serial.print(year);
  Serial.print(" ");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.println(bitIndex);
}

