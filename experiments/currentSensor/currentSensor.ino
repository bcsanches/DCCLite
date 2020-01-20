
bool inputState = false;
bool currentState = false;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(5, INPUT);

  Serial.begin(9600);  
}

bool changing = false;
long time = 0;

// the loop function runs over and over again forever
void loop() {  
  
  if(changing && (time < millis()))
  {
    changing = false;
    bool inputState = digitalRead(5) == HIGH;    

    if(inputState != currentState)
    {
      currentState = inputState;

      if(currentState)
      {
        Serial.println("HIGH");
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      }
      else
      {
        Serial.println("LOW");
        digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
      }
    }

    return;
  }
  else if(changing)
    return;

  bool inputState = digitalRead(5) == HIGH;    
  if((inputState != currentState) && (!changing))
  {
    changing = true;
    time = millis() + 20;
  }
  
}
