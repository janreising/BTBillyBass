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

bool debug = true;

// SOUND INPUT
int low_freq_pin = A1;
int high_freq_pin = A2;

int low_vol_read = 0;
int high_vol_read = 0;

float low_vol = 0;
float high_vol = 0;

int low_threshold = 2; // threshold for baseline
int high_threshold = 100; // threshold for vocals

// MOTOR VARIABLES
int bodySpeed = 0; // body motor speed initialized to 0

// ROLLING ARRAY VARIABLES

int counter = 0;
//int low_arr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// int high_arr[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int arr_size = 100;
int low_arr[100] = {0};
int high_arr[100] = {0};

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

  switch (headState) {

    case 0: // Start and Wait

      //Serial.println("vol: "+String(high_vol)+" thresh: "+String(high_threshold)+" "+String(high_vol > high_threshold));

      // threshold reached
      if (high_vol > high_threshold) {

        //Serial.println("head state 1");

        headState = 1;

      }

    case 1: // yapping

      yapping = true;

      if (high_vol > high_threshold) {

        headState = 1; // stay in talking mode

      } else {

        //Serial.println("stopping");
        headState = 2; // move to stop talking

      }
    
    case 2: // stop motion

      //Serial.println("head state 2");
      yapping = false;
      headState = 0; 

    }

}

void Tail(){

  //Serial.println("vol: "+String(low_vol)+" thresh: "+String(low_threshold)+" "+String(low_vol > low_threshold)+" "+String(flapping));

  switch (tailState) {

    case 0: // Start and Wait

      // threshold reached
      if (low_vol > low_threshold) {

        tailState = 1;

      }

      break;

    case 1: // flapping

      flapping = true;
      
      if (low_vol > low_threshold) {
        
        tailState = 1; // stay in flapping mode

      } else {
        //Serial.println("flapping: "+String(flapping));
        tailState = 2; // move to stop flapping

      }

      break;
    
    case 2: // stop motion
      //Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> stopping");
      flapping = false;
      tailState = 0; 

      break;

  }

}

void moveMouth(){
  
  switch (state_move_mouth) {

    case 0: // waiting

      time_mouth_init = currentTime;

      

      if (yapping){

        // Serial.println("start yapping");

        mouthMotor.halt();
        mouthMotor.setSpeed(250);    
        mouthMotor.forward();

        state_move_mouth = 1;

      }

    case 1: // opening

      if (currentTime - time_mouth_init > 500){
        
        // Serial.println("Stop yapping");

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

  switch (state_move_tail) {

    case 0: // waiting

      time_tail_init = currentTime;

      if (flapping){
        Serial.println("start flapping");
        

        state_move_tail = 1;

      }

      break;

    case 1: // opening

      if (currentTime - time_tail_init > 100){
        Serial.println("now we are here");
        //bodyMotor.halt();    
        //bodyMotor.backward();

        bodyMotor.halt();        
        bodyMotor.setSpeed(200);    
        bodyMotor.backward();

        state_move_tail = 2;

      }

      break;

    case 2: // closing

      if (currentTime - time_tail_init > 500){
        
        bodyMotor.halt();    
        bodyMotor.setSpeed(0);

        state_move_tail = 3;

      }

      break;

      case 3: // waiting time

      if (currentTime - time_tail_init > 2000){
        
        state_move_tail = 0;
        Serial.println("waiting over.");

      }

      break;
  }
}


int updateSoundInput() {

  // read pins
  low_vol_read = analogRead(low_freq_pin);
  high_vol_read = analogRead(high_freq_pin);

  if (debug) {

    //Serial.print(low_vol);
    //Serial.print(",");
    //Serial.println(high_vol);
    
  }

  // rolling array
  if (counter > arr_size){
    counter = 0;
  }

  low_arr[counter] = low_vol_read;
  high_arr[counter] = high_vol_read;
  counter = counter+1;

  low_vol = average(low_arr, arr_size);
  high_vol = average(high_arr, arr_size);

}

float average (int * array, int len) {  // assuming array is int.
  long sum = 0L ;  // sum will be larger than an item, long for safety.
  for (int i = 0 ; i < len ; i++)
    sum += array [i] ;
  return  ((float) sum) / len ;  // average will be fractional, so float may be appropriate.
}
