/*This is my crack at a state-based approach to automating a Big Mouth Billy Bass.
 This code was built on work done by both Donald Bell and github user jswett77. 
 See links below for more information on their previous work.

 In this code you'll find reference to the MX1508 library, which is a simple 
 library I wrote to interface with the extremely cheap 2-channel H-bridges that
 use the MX1508 driver chip. It may also work with other H-bridges that use different
 chips (such as the L298N), so long as you can PWM the inputs.

 This code watches for a voltage increase on input A0, and when sound rises above a
 set threshold it opens the mouth of the fish. When the voltage falls below the threshold,
 the mouth closes.The result is the appearance of the mouth "riding the wave" of audio
 amplitude, and reacting to each voltage spike by opening again. There is also some code
 which adds body movements for a bit more personality while talking.

 Most of this work was based on the code written by jswett77, and can be found here:
 https://github.com/jswett77/big_mouth/blob/master/billy.ino

 Donald Bell wrote the initial code for getting a Billy Bass to react to audio input,
 and his project can be found on Instructables here:
 https://www.instructables.com/id/Animate-a-Billy-Bass-Mouth-With-Any-Audio-Source/

 Author: Jordan Bunker <jordan@hierotechnics.com> 2019
 License: MIT License (https://opensource.org/licenses/MIT)
*/

#include <MX1508.h>

MX1508 bodyMotor(6, 9); // Sets up an MX1508 controlled motor on PWM pins 6 and 9
MX1508 mouthMotor(5, 3); // Sets up an MX1508 controlled motor on PWM pins 5 and 3

bool debug = false;

// SOUND INPUT
int low_freq_pin = A1;
int high_freq_pin = A2;

int low_vol = 0;
int high_vol = 0;

int low_threshold = 10; // threshold for baseline
int high_threshold = 10; // threshold for vocals

// MOTOR VARIABLES
int bodySpeed = 0; // body motor speed initialized to 0

// ROLLING ARRAY VARIABLES

// int counter = 0;
// int arr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// FISH VARIABLES
int fishState = 0; // variable to indicate the state Billy is in
bool talking = false; //indicates whether the fish should be talking or not

int tailState = 0;
int headState = 0;

int state_move_tail = 0;
int state_move_mouth = 0;

int time_mouth_init = 0;
int time_tail_init = 0;

int yapping = false;
int flapping = false;

//these variables are for storing the current time, scheduling times for actions to end, and when the action took place
long currentTime;
long mouthActionTime;
long bodyActionTime;
long lastActionTime;

void setup() {
  
  //make sure both motor speeds are set to zero
  bodyMotor.setSpeed(0); 
  mouthMotor.setSpeed(0);

  //input mode for sound pin
  pinMode(low_freq_pin, INPUT);
  pinMode(high_freq_pin, INPUT);

  if (debug) {

    Serial.begin(9600);
    Serial.print("Billy is running ...");

  }
  
}

void loop() {
  currentTime = millis(); //updates the time each time the loop is run
  updateSoundInput(); //updates the volume level detected
  // SMBillyBass(); //this is the switch/case statement to control the state of the fish
  
  Head();
  moveMouth();

  Tail();
  moveTail();

}

void Head(){

  switch(headState) {

    case 0: // Start and Wait

      // threshold reached
      if (high_vol > high_threshold) {

        headState = 1;

      }

    case 1: // yapping

      yapping = true;

      if (high_vol > high_threshold) {

        headState = 1; // stay in talking mode

      } else {

        headState = 2; // move to stop talking

      }
    
  case 2: // stop motion

    yapping = false;
    headState = 0; 

  }

}

void Tail(){}

  switch(tailState) {

    case 0: // Start and Wait

      // threshold reached
      if (low_vol > low_threshold) {

        tailState = 1;

      }

    case 1: // flapping

      flapping = true;

      if (low_vol > low_threshold) {

        tailState = 1; // stay in flapping mode

      } else {

        tailState = 2; // move to stop flapping

      }
    
  case 2: // stop motion

    flapping = false;
    tailState = 0; 

}

void moveMouth(){
  
  switch state_move_mouth {

    case 0: // waiting

      time_mouth_init = currentTime;

      if (yapping){

        mouthMotor.setSpeed(100);    
        mouthMotor.forward();

        state_move_mouth = 1;

      }

    case 1: // opening

      if (currentTime - time_mouth_init > 500){
        
        mouthMotor.halt();    
        mouthMotor.backward();

        state_move_mouth = 2;

      }

    case 2: // closing

      if (currentTime - time_mouth_init > 1000){
        
        mouthMotor.halt();    
        mouthMotor.setSpeed(0);

        state_move_mouth = 0;

      }
  }

}

void moveTail(){

    case 0: // waiting

      time_tail_init = currentTime;

      if (yapping){

        bodyMotor.setSpeed(100);    
        bodyMotor.forward();

        state_move_tail = 1;

      }

    case 1: // opening

      if (currentTime - time_tail_init > 500){
        
        bodyMotor.halt();    
        bodyMotor.backward();

        state_move_tail = 2;

      }

    case 2: // closing

      if (currentTime - time_tail_init > 1000){
        
        bodyMotor.halt();    
        bodyMotor.setSpeed(0);

        state_move_tail = 0;

      }
  }
}

/*
void SMBillyBass() {
  switch (fishState) {
    case 0: //START & WAITING
      if (s2 > silence) { //if we detect audio input above the threshold
        if (currentTime > mouthActionTime) { //and if we haven't yet scheduled a mouth movement
          talking = true; //  set talking to true and schedule the mouth movement action
          mouthActionTime = currentTime + 100;
          fishState = 1; // jump to a talking state
        }
      } else if (currentTime > mouthActionTime + 100) { //if we're beyond the scheduled talking time, halt the motors
        bodyMotor.halt();
        mouthMotor.halt();
      }
      if (currentTime - lastActionTime > 1500) { //if Billy hasn't done anything in a while, we need to show he's bored
        lastActionTime = currentTime + floor(random(30, 60)) * 1000L; //you can adjust the numbers here to change how often he flaps
        fishState = 2; //jump to a flapping state!
      }
      break;

    case 1: //TALKING
      if (currentTime < mouthActionTime) { //if we have a scheduled mouthActionTime in the future....
        if (talking) { // and if we think we should be talking
          openMouth(); // then open the mouth and articulate the body
          lastActionTime = currentTime;
          articulateBody(true);
        }
      }
      else { // otherwise, close the mouth, don't articulate the body, and set talking to false
        closeMouth();
        articulateBody(false);
        talking = false;
        fishState = 0; //jump back to waiting state
      }
      break;

    case 2: //GOTTA FLAP!
      //Serial.println("I'm bored. Gotta flap.");
      flap();
      fishState = 0;
      break;
  }
}
*/

int updateSoundInput() {

  // read pins
  low_vol = analogRead(low_freq_pin);
  high_vol = analogRead(high_freq_pin);

  if (debug) {

    Serial.print(low_vol);
    Serial.print(",");
    Serial.println(high_vol);
    
  }

  // rolling array
  //if (counter > 20){
  //  counter = 0;
  //}

  //arr[counter] = sV;
  //counter = counter+1;

  //soundVolume = average(arr, 20);

}

float average (int * array, int len) {  // assuming array is int.
  long sum = 0L ;  // sum will be larger than an item, long for safety.
  for (int i = 0 ; i < len ; i++)
    sum += array [i] ;
  return  ((float) sum) / len ;  // average will be fractional, so float may be appropriate.
}

/*
void openMouth() {
  mouthMotor.halt(); //stop the mouth motor
  mouthMotor.setSpeed(220); //set the mouth motor speed
  mouthMotor.forward(); //open the mouth
}

void closeMouth() {
  mouthMotor.halt(); //stop the mouth motor
  mouthMotor.setSpeed(180); //set the mouth motor speed
  mouthMotor.backward(); // close the mouth
}

void articulateBody(bool talking) { //function for articulating the body
  if (talking) { //if Billy is talking
    if (currentTime > bodyActionTime) { // and if we don't have a scheduled body movement
      int r = floor(random(0, 8)); // create a random number between 0 and 7)
      if (r < 1) {
        bodySpeed = 0; // don't move the body
        bodyActionTime = currentTime + floor(random(500, 1000)); //schedule body action for .5 to 1 seconds from current time
        bodyMotor.forward(); //move the body motor to raise the head

      } else if (r < 3) {
        bodySpeed = 150; //move the body slowly
        bodyActionTime = currentTime + floor(random(500, 1000)); //schedule body action for .5 to 1 seconds from current time
        bodyMotor.forward(); //move the body motor to raise the head

      } else if (r == 4) {
        bodySpeed = 200;  // move the body medium speed
        bodyActionTime = currentTime + floor(random(500, 1000)); //schedule body action for .5 to 1 seconds from current time
        bodyMotor.forward(); //move the body motor to raise the head

      } else if ( r == 5 ) {
        bodySpeed = 0; //set body motor speed to 0
        bodyMotor.halt(); //stop the body motor (to keep from violent sudden direction changes)
        bodyMotor.setSpeed(255); //set the body motor to full speed
        bodyMotor.backward(); //move the body motor to raise the tail
        bodyActionTime = currentTime + floor(random(900, 1200)); //schedule body action for .9 to 1.2 seconds from current time
      }
      else {
        bodySpeed = 255; // move the body full speed
        bodyMotor.forward(); //move the body motor to raise the head
        bodyActionTime = currentTime + floor(random(1500, 3000)); //schedule action time for 1.5 to 3.0 seconds from current time
      }
    }

    bodyMotor.setSpeed(bodySpeed); //set the body motor speed
  } else {
    if (currentTime > bodyActionTime) { //if we're beyond the scheduled body action time
      bodyMotor.halt(); //stop the body motor
      bodyActionTime = currentTime + floor(random(20, 50)); //set the next scheduled body action to current time plus .02 to .05 seconds
    }
  }
}

void flap() {
  bodyMotor.setSpeed(180); //set the body motor to full speed
  bodyMotor.backward(); //move the body motor to raise the tail
  delay(500); //wait a bit, for dramatic effect
  bodyMotor.halt(); //halt the motor
}
*/