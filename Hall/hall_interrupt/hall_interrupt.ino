#define HALL_THRESH 2
#define HALL_PIN 2 //use external interrupt digital pin 2
unsigned int hall_count;
unsigned long hall_start_time;
unsigned long hall_end_time;
float rpm;

void setup() {
  pinMode(HALL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(HALL_PIN), HALL_ISR, RISING);
  Serial.begin(115200);
  hall_start_time = micros();
}

void HALL_ISR() {
 //increment hall_count every rotation
  hall_count +=1;
}

void loop() {

  if (hall_count >= HALL_THRESH){
        
    //Calculate and print RPM
    hall_end_time = micros();
    rpm = (60000000.0*hall_count/(hall_end_time-hall_start_time));
    Serial.println("\n");
    Serial.print(rpm);
    Serial.print(" RPM");

    //reset hall count and hall start time
    hall_count = 0;
    hall_start_time = micros();
  }
}
