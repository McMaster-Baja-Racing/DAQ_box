// digital pin 2 is the hall pin
int hall_pin = 2;
// set number of hall trips for RPM reading (higher improves accuracy)
unsigned int hall_thresh = 4;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(115200);
  // make the hall pin an input:
  pinMode(hall_pin, INPUT);
}

// the loop routine runs over and over again forever:
void loop() {
//  Serial.println(digitalRead(hall_pin));
//  delay(10);
//  
  // preallocate values for tach
  unsigned int hall_count = 0;
  unsigned long start = micros();
  bool on_state = false;
  // counting number of times the hall sensor is tripped
  // but without double counting during the same trip
  while (true){
    if (digitalRead(hall_pin)==LOW){
      if (on_state==false){
        on_state = true;
        hall_count+=1;
      }
    } else{
      on_state = false;
    }
    
    if (hall_count>=hall_thresh){
      break;
    }
  }
  
  // print information about Time and RPM
  unsigned long end_time = micros();
  float time_passed = (end_time-start)/1000000.0;
  //Serial.print("Time Passed: ");
  //Serial.print(time_passed);
//  Serial.println("s");
  float rpm_val = (hall_count/time_passed)*60;
  Serial.print(" RPM: ");
  Serial.println(rpm_val);
  delay(1);        // delay in between reads for stability
}
