/*
 * MSFdecoder
 * Basic MSF time signal decoder for the NPL's radio clock
 * Ali Raheem 2024
 */
const int MSF_DIN = 2;
const int MSF_EN = 3;


unsigned long JITTER = 50;

// States
enum {
  MINUTE_LOW,
  MINUTE_HIGH,
  GET_A,
  GET_B,
  END_SECOND,
  CHECK_TIME,
  DONE
};

enum {
  TIME_GOOD,
  TIME_STALE,
  TIME_BAD
};

//ISR variables
volatile unsigned long counter = 0;
volatile byte A[64], B[64];
volatile unsigned int bitIdx = 0;
volatile unsigned int MSF_STATE = MINUTE_LOW;
volatile unsigned int TIME_STATE = TIME_BAD;
volatile unsigned long duration;
volatile bool high_low;

byte year = 0;
byte month = 0;
byte dayOfMonth = 0;
byte dayOfWeek = 0;
byte hours = 0;
byte minutes = 0;

bool aEq(const unsigned long Foo, const unsigned long Bar) {
  return ((Foo - JITTER) < Bar && (Foo + JITTER) > Bar);
}
unsigned int get_bit(byte *bits, int i) {
  return bits[i];
}
void setup() {
  // put your setup code here, to run once:
  pinMode(MSF_DIN, INPUT);
  pinMode(MSF_EN, OUTPUT);
  digitalWrite(MSF_EN, LOW);
  Serial.begin(115200);
  Serial.println("Start MSF decoder");
  delay(10);
  MSF_STATE = MINUTE_LOW;
  attachInterrupt(digitalPinToInterrupt(MSF_DIN), msf_isr, CHANGE);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(bitIdx);
  Serial.print(duration);
  Serial.println(high_low?" HIGH":" LOW");
  delay(50);
  if(bitIdx >= 58) {
    year = 80 * A[17] + 40 * A[18] + 20 * A[19] + 10 * A[20] + 8 * A[21] + 4 * A[22] + 2 * A[23] + A[24];
    Serial.print("Year: 20");
    Serial.println(year);
    printTimeDate();
  }
}

void msf_isr() {
  unsigned long time_now = millis();
  duration = time_now - counter;
  high_low = digitalRead(MSF_DIN);
  switch(MSF_STATE) {
    case MINUTE_LOW:
      if (!high_low && aEq(duration, 500)){
        MSF_STATE = MINUTE_HIGH;
      }
      break;
    case MINUTE_HIGH:
      if (high_low && aEq(duration, 500)){
        MSF_STATE = GET_A;
        TIME_STATE = TIME_STALE;
        bitIdx = 1;
      }
      break;
    case GET_A:
      if (!high_low && aEq(duration, 100)){
        A[bitIdx] = 0;
        B[bitIdx] = 0;
      } else if (!high_low && aEq(duration, 200)) {
        A[bitIdx] = 1;
        B[bitIdx] = 0;
      } else if (!high_low && aEq(duration, 300)) {
        A[bitIdx] = 1;
        B[bitIdx] = 1;
      }
      MSF_STATE = GET_B;
      break;
    case GET_B:
      MSF_STATE = (bitIdx++ < 58)? GET_A : MINUTE_LOW;
      break;
    case CHECK_TIME: // Second 59
     MSF_STATE = MINUTE_LOW;
    break;
  }
  counter = time_now;
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
  if(dateParity)
    Serial.println("Date Parity GOOD");
  int dayParity = 0;
  for(i = 36; i <= 38; i++)
    dayParity += A[i];
  dayParity += B[56];
  dayParity %= 2;
  if(dayParity )
    Serial.println("Day Parity GOOD");
  int timeParity = 0;
  for(i = 39; i <= 51; i++)
    timeParity += A[i];
  timeParity += B[57];
  timeParity %= 2;
  if(timeParity)
    Serial.println("Time Parity GOOD");

  TIME_STATE = (yearParity && dateParity && dayParity && timeParity) ? TIME_GOOD : TIME_BAD;

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
  Serial.println(bitIdx);
}
