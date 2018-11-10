#include "IRremote.h"

//___________________________________________________________________Variables and constants___________________________________________________________________//


// Speed
const int speedPin = A0;
bool speaker_on = false;

// Color Sensor
const int S0 = A1;
const int S1 = 2;
const int S2 = 3;
const int S3 = 4;
const int sensorOut = 5;
int frequency_red;
int frequency_green;
int frequency_blue;
const int color_delay = 10;

// Speaker
const int speakerPin = A2;
#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880

// IR Receiver
const int receiverPin = A3;

// LEDs
const int ledsPin = A4;
bool manual_leds = false;

// Lightsensor
const int lightCheck = A5;
const int light_threshold = 200;

// Left Motor
const int pwmleftPin = 6;
const int revleftPin = 7;

//  Right Motor
const int pwmrightPin = 9;
const int revrightPin = 8;

IRrecv irrecv(receiverPin);
decode_results results;

// Drive variables
unsigned long int lastKnownVal; //= 0x20DF906F ; // set value to stop_all() as default
unsigned long int usedVal;// = 0x20DF906F;  // set value to stop_all() as default
const unsigned long signal_threshold = 500; // threshold in milliseconds between signals, after which a total shutdown will occur if this value is exceeded
unsigned long int recv_counter; // each time a signal is recieved, the current time will be ascribed to this parameter
bool manual = true;
const int correction_speed = 50;  // the speed to which a wheel will be set to correct the car's path
unsigned int turn_speed = 150; // The speeds to which the wheels will be set to turn; necessary for turning on 3 or 5V

//___________________________________________________________________SETUP___________________________________________________________________//


void setup()
{
  

  pinMode(speedPin, OUTPUT);      // A0
  pinMode(S0, OUTPUT);            // A1
  pinMode(speakerPin, OUTPUT);    // A2
  pinMode(receiverPin, INPUT);    // A3
  pinMode(ledsPin, OUTPUT);       // A4
  pinMode(lightCheck, INPUT);     // A5
  pinMode(S1, OUTPUT);            // 2
  pinMode(S2, OUTPUT);            // 3
  pinMode(S3, OUTPUT);            // 4
  pinMode(sensorOut, INPUT);      // 5
  pinMode(pwmleftPin, OUTPUT);    // 6
  pinMode(revleftPin, OUTPUT);    // 7
  pinMode(pwmrightPin, OUTPUT);   // 8
  pinMode(revrightPin, OUTPUT);   // 9
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver

}



//___________________________________________________________________MOVEMENT COMMANDS___________________________________________________________________//

void forward()
{
  digitalWrite(revleftPin, LOW);
  analogWrite(pwmleftPin, 255);
  digitalWrite(revrightPin, LOW);
  analogWrite(pwmrightPin, 255);
}

void backward()
{
  digitalWrite(revleftPin, HIGH);
  analogWrite(pwmleftPin, 255);
  digitalWrite(revrightPin, HIGH);
  analogWrite(pwmrightPin, 255);
}

void right_turn()
{
  digitalWrite(revleftPin, LOW);
  analogWrite(pwmleftPin, turn_speed);
  digitalWrite(revrightPin, HIGH);
  analogWrite(pwmrightPin, turn_speed);
}

void left_turn()
{
  digitalWrite(revleftPin, HIGH);
  analogWrite(pwmleftPin, turn_speed);
  digitalWrite(revrightPin, LOW);
  analogWrite(pwmrightPin, turn_speed);
}

void slow() {
  digitalWrite(speedPin, HIGH);
  
}
void fast() {
  digitalWrite(speedPin, LOW);

}

void change_speed()
{ 
  switch(digitalRead(speedPin)){
    case 0:
    slow();
    turn_speed = 255;
    break;

    case 1:
    fast();
    turn_speed = 150;
    break;

  }
}

void stop_motors(){
  digitalWrite(revleftPin, LOW);
  analogWrite(pwmleftPin, 0);
  digitalWrite(revrightPin, LOW);
  analogWrite(pwmrightPin, 0);
}

void stop_all(){
  stop_motors();
  digitalWrite(speakerPin, LOW);
  digitalWrite(ledsPin, LOW);
}

//___________________________________________________________________LEDS___________________________________________________________________//

void leds_on() {
  digitalWrite(ledsPin, HIGH);
}
void leds_off() {
  digitalWrite(ledsPin, LOW);
}
void switch_leds()
{
  switch(digitalRead(ledsPin))
  {
    case 1:
    leds_off();
    manual_leds = false;
    break;

    case 0:
    leds_on();
    manual_leds = true;
    break;
  }

  }



bool is_dark() {

  
  if (analogRead(lightCheck) < light_threshold) {
    return true;
  }
  else {
    return false;
  }
}



//___________________________________________________________________COLOR SENSOR___________________________________________________________________//

void refresh_color() {
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  
  // Set red filter
  digitalWrite(S2, LOW); 
  digitalWrite(S3, LOW);
  // Read red frequency
  frequency_red = pulseIn(sensorOut, LOW);
  Serial.print("R= ");
  Serial.print(frequency_red);
  Serial.print("  ");
  delay(50);

//  // Set blue filter
//  digitalWrite(S2, LOW);
//  digitalWrite(S3, HIGH);
//  // Read blue frequency
//  frequency_blue = pulseIn(sensorOut, LOW);
//  Serial.print("B= ");
//  Serial.print(frequency_blue);
//  Serial.println("  ");
//  delay(50);
//
//  //Set green frequency
//  digitalWrite(S2, HIGH);
//  digitalWrite(S3, HIGH);
//  // Read green frequency
//  frequency_green = pulseIn(sensorOut, LOW);
//  Serial.print("G= ");
//  Serial.print(frequency_green);
//  Serial.println("  ");
//  delay(50);

}

void adjust_speed(){

  if (frequency_red <= 32)// && frequency_blue <= 30)
  {
    analogWrite(pwmrightPin, correction_speed);
    digitalWrite(pwmleftPin, HIGH);
  }
  else if (frequency_red <= 55) //&& frequency_blue <= 50)
  {
    analogWrite(pwmleftPin, correction_speed);
    digitalWrite(pwmrightPin, HIGH);
    
  }
  else if (frequency_red > 55)
  {
    stop_motors();
    manual_leds = true;
    leds_on();
    manual = true;
    
  }
  else {
    analogWrite(pwmrightPin, 255);
    analogWrite(pwmleftPin, 255);
  }
}



//___________________________________________________________________SPEAKER___________________________________________________________________//


void beep (unsigned char speakerPin, int frequencyInHertz, long timeInMilliseconds)
{
  int x;
  long delayAmount = (long)(1000000 / frequencyInHertz);
  long loopTime = (long)((timeInMilliseconds * 1000) / (delayAmount * 2));
  for (x = 0; x < loopTime; x++)
  {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(delayAmount);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(delayAmount);
  }
}

void march()
{
  beep(speakerPin, a, 500);
  beep(speakerPin, a, 500);
  beep(speakerPin, a, 500);
  beep(speakerPin, f, 350);
  beep(speakerPin, cH, 150);

  beep(speakerPin, a, 500);
  beep(speakerPin, f, 350);
  beep(speakerPin, cH, 150);
  beep(speakerPin, a, 1000);


  beep(speakerPin, eH, 500);
  beep(speakerPin, eH, 500);
  beep(speakerPin, eH, 500);
  beep(speakerPin, fH, 350);
  beep(speakerPin, cH, 150);

  beep(speakerPin, gS, 500);
  beep(speakerPin, f, 350);
  beep(speakerPin, cH, 150);
  beep(speakerPin, a, 1000);


  beep(speakerPin, aH, 500);
  beep(speakerPin, a, 350);
  beep(speakerPin, a, 150);
  beep(speakerPin, aH, 500);
  beep(speakerPin, gSH, 250);
  beep(speakerPin, gH, 250);

  beep(speakerPin, fSH, 125);
  beep(speakerPin, fH, 125);
  beep(speakerPin, fSH, 250);
  delay(250);
  beep(speakerPin, aS, 250);
  beep(speakerPin, dSH, 500);
  beep(speakerPin, dH, 250);
  beep(speakerPin, cSH, 250);


  beep(speakerPin, cH, 125);
  beep(speakerPin, b, 125);
  beep(speakerPin, cH, 250);
  delay(250);
  beep(speakerPin, f, 125);
  beep(speakerPin, gS, 500);
  beep(speakerPin, f, 375);
  beep(speakerPin, a, 125);

  beep(speakerPin, cH, 500);
  beep(speakerPin, a, 375);
  beep(speakerPin, cH, 125);
  beep(speakerPin, eH, 1000);


  beep(speakerPin, aH, 500);
  beep(speakerPin, a, 350);
  beep(speakerPin, a, 150);
  beep(speakerPin, aH, 500);
  beep(speakerPin, gSH, 250);
  beep(speakerPin, gH, 250);

  beep(speakerPin, fSH, 125);
  beep(speakerPin, fH, 125);
  beep(speakerPin, fSH, 250);
  delay(250);
  beep(speakerPin, aS, 250);
  beep(speakerPin, dSH, 500);
  beep(speakerPin, dH, 250);
  beep(speakerPin, cSH, 250);


  beep(speakerPin, cH, 125);
  beep(speakerPin, b, 125);
  beep(speakerPin, cH, 250);
  delay(250);
  beep(speakerPin, f, 250);
  beep(speakerPin, gS, 500);
  beep(speakerPin, f, 375);
  beep(speakerPin, cH, 125);

  beep(speakerPin, a, 500);
  beep(speakerPin, f, 375);
  beep(speakerPin, c, 125);
  beep(speakerPin, a, 1000);
}


//___________________________________________________________________IR SWITCH___________________________________________________________________//
void translateIR()
{
  if (results.value == 0xFFFFFFFF)
  {
    usedVal = lastKnownVal;
    recv_counter = millis();
  }
  else
  {
    usedVal = results.value;
    //lastKnownVal = results.value;
  }
  switch (usedVal)
  {

    case 0x20DF02FD: // Up Arrow key = forward
      Serial.println("Forward");
      forward();
      recv_counter = millis();
      lastKnownVal = results.value;
      break;

    case 0x20DF827D: // Down Arrow key = backward
      Serial.println("Backward");
      backward();
      recv_counter = millis();
      lastKnownVal = results.value;
      break;

    case 0x20DF609F: // Left Arrow key = turn left
      Serial.println("Left turn");
      right_turn();
      recv_counter = millis();
      lastKnownVal = results.value;
      break;

    case 0x20DFE01F: // Right Arrow key = turn right
      Serial.println("Right turn");
      left_turn();
      recv_counter = millis();
      lastKnownVal = results.value;
      break;

    case 0x20DF22DD: // Ok key = change speed
      Serial.println("Ok");
      change_speed();
      recv_counter = millis();
      lastKnownVal = results.value;
      break;

    case 0x20DF906F: // Mute key = Stop
      Serial.print("Stop");
      stop_all();
      recv_counter = millis();
      lastKnownVal = results.value;
      break;

    case 0x20DFA956: // Energy key = switch LEDs on/off
      Serial.print("LEDs");
      switch_leds();
      recv_counter = millis();
      lastKnownVal = results.value;
      break;

    case 0x20DF0DF2: // Play key = play the Imperial March
      Serial.println("Play");
      //switch_speaker();
      march();
      recv_counter = millis();
      lastKnownVal = results.value;
      break;

    case 0x20DF10EF: // Off Switch = go to recharge station
      Serial.println("Recharge");
      manual = false;
      break;
  }
  delay(100);
}




void loop()
{
  
  if (manual == true) {

    if (manual_leds == false){
    if (is_dark()==true) {
      leds_on();
      
    }
    else {
      leds_off();
      
    }
    }
    if (irrecv.decode(&results)) // If IR signal received
    {
      //Serial.println(results.value, HEX);
      translateIR();
      delay(200);
      irrecv.resume(); // Receive next value
    }
//    else // if no signal received
//  {
//    delay(150);
//    stop_all();
//  }
//}
    
    if ( (millis()) > (recv_counter + signal_threshold)) {
      stop_motors();
    }
  }

  else {
    backward();
    //slow();
    refresh_color();
    adjust_speed();
    delay(color_delay);

}
  
  
}

