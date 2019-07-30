/*  
The circuit:
Audio output on digital pin 9 on a Uno or similar, or
DAC/A14 on Teensy 3.1, or 
check the README or http://sensorium.github.com/Mozzi/
*/

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <EventDelay.h>
#include <ADSR.h>
#include <Metronome.h>
#include <mozzi_rand.h>
#include <tables/sin2048_int8.h> 
#include <tables/square_no_alias_2048_int8.h>
#include <tables/saw2048_int8.h>
#include <tables/triangle2048_int8.h>
#include "muxlib.h"

#define MUX_MASK B11100011 // For clearing bits of LED mux before setting select lines

// use: Oscil <table_size, update_rate> oscilName (wavetable), look in .h file of table #included above
/* Oscillators */
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil <SQUARE_NO_ALIAS_2048_NUM_CELLS, AUDIO_RATE> aSqu(SQUARE_NO_ALIAS_2048_DATA);
Oscil <SAW2048_NUM_CELLS, AUDIO_RATE> aSaw(SAW2048_DATA);
Oscil <TRIANGLE2048_NUM_CELLS, AUDIO_RATE> aTri(TRIANGLE2048_DATA);

ADSR <CONTROL_RATE, CONTROL_RATE> envelope;
unsigned int duration, attack, decay, sustain, release_ms;
                                        
//Potentiometers
const char INPUT_PIN = 5; // set the input for the knob to analog pin 0
const char TEMPO_POT = 6;
const char FREQ_POT;

// LED pins
const char MUX_S0 = A4; //Mux select for leds
const char MUX_S1 = A3;
const char MUX_S2 = A2;

const char STEP_EVEN_LED = A0;
const char STEP_ODD_LED = A1;

const char OCT_LED = A7;
const char OCT2X_LED = 2;

//Buttons
const char BTN_1 = 7;     // Set step 0/1
const char BTN_2 = 6;     // Set step 2/3
const char BTN_3 = 5;     // Set step 4/5
const char BTN_4 = 4;     // Set step 6/7
const char SEQ_TOG = 3;   // Toggle odd/even indexes of sequence
const char OCT_TOG = 10;  // Toggle octave
const char OSC_SEL = 8;   // Select oscillator
const char MODE_SEL = 11; // Select mode
const char ROOT_SEL = 12; // Select root note

// Constants
const char MAX_OCT = 1;
const char MIN_OCT = 2;
const int MAX_TEMPO = 500;
const int MIN_TEMPO = 150;
const int SEQ_LENGTH = 8;

// Variables
int tog_btn;
int oct_btn;
int osc_btn;
int mode_btn;
int root_btn;
//int btn1;
//int btn2;
//int btn3;
//int btn4;

bool oct_tog = false;   // Octave toggle state
bool seq_even = false;  // Odd / Even indexes of sequence
int freq = 440;
int tempo = 200;
int seq_point = 0;
int myNote;
float note;
int freq_control;
int tempo_control;

int current_root = 1;
int current_mode = 0;

const float NOTE_MULT = 1.059463094359; // 1 semitone in Hz, used to set root note

                   //a     a#      b        c       c#      d       d#      e       f       f#      g      g#
float notes[24] = {110.0, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.0, 196.0, 207.65,
                   220.0, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.0, 415.3};// A2 to A3

float sequence[SEQ_LENGTH];
int sequence_steps[SEQ_LENGTH]  = {0,2,3,5,6,7,8,10};

float modes[7][SEQ_LENGTH] = {{55.0, 61.785, 65.405, 73.415, 82.405, 87.305, 98.0000, 110.0},//aeolian
                              {55.0, 58.270, 65.405, 73.415, 77.780, 87.305, 98.0000, 110.0},//locrian
                              {55.0, 61.785, 69.275, 73.415, 82.405, 92.500, 103.825, 110.0},//ionian
                              {55.0, 61.785, 65.405, 73.415, 82.405, 92.500, 98.0000, 110.0},//dorian
                              {55.0, 58.270, 65.405, 73.415, 82.405, 87.305, 98.0000, 110.0},//phrygian
                              {55.0, 61.785, 69.275, 77.780, 82.405, 92.500, 103.825, 110.0},//lydian
                              {55.0, 61.785, 69.275, 73.415, 82.405, 92.500, 98.0000, 110.0}};//mixolydian                              

float current_mode_seq[SEQ_LENGTH];

typedef enum{SINE, SQUARE, SAW, TRI} OSCILLATOR;
typedef enum{odd, even, sine, squ, saw, tri} mux2_states;
 
EventDelay buttonDelay;
EventDelay noteDelay;
Metronome metro;

int current_osc = 0;
OSCILLATOR osc;
MUX mux;
MUX seq_mux;

boolean note_is_on = true;
byte gain;

void setup(){
    mux.enable = INACTIVE;
    mux.msize = 8;
    mux.s0 = MUX_S0;
    mux.s1 = MUX_S1;
    mux.s2 = MUX_S2;

    osc = current_osc;

    // set initial mode and root
//    set_mode(current_mode);
    set_mode();
    for(int i = 0; i < SEQ_LENGTH; i++){
        current_mode_seq[i] = modes[current_mode][i]; 
        sequence[i] = current_mode_seq[i];         
    }

    //Serial.begin(9600); // for Teensy 3.1, beware printout can cause glitches
    Serial.begin(115200); // set up the Serial output so we can look at the piezo values // set up the Serial output so we can look at the input values
    pinMode(BTN_1, INPUT);
    pinMode(BTN_2, INPUT);  
    pinMode(BTN_3, INPUT);
    pinMode(BTN_4, INPUT);
    pinMode(SEQ_TOG, INPUT);
    pinMode(OCT_TOG, INPUT);
    pinMode(OSC_SEL, INPUT);
    pinMode(ROOT_SEL, INPUT);
    pinMode(MODE_SEL, INPUT);
    
    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);
    pinMode(MUX_S2, OUTPUT);
    // LED outputs
    pinMode(STEP_EVEN_LED, OUTPUT);
    pinMode(STEP_EVEN_LED, OUTPUT);
    pinMode(OCT_LED, OUTPUT);
    pinMode(OCT2X_LED, OUTPUT);

    PORTC |= B00000010; // init step led
   
    aSin.setFreq(freq);
    randSeed();
    noteDelay.set(2000); // 2 second countdown
    buttonDelay.set(200);
//    metro.setBPM(120);
    startMozzi();
}


void updateControl(){   
    
    freq_control = mozziAnalogRead(INPUT_PIN);
    tempo_control = mozziAnalogRead(TEMPO_POT);
    
    freq = map(freq_control, 5, 1020, 0, 7);
    tempo = map(tempo_control, 0, 1023, MIN_TEMPO, MAX_TEMPO);
    
    if(noteDelay.ready()){
        // choose envelope levels
        envelope.setADLevels(255,255);
               
        if(seq_point == 8){seq_point = 0;}  // Loop to start of sequence
      
        osc = current_osc;
        switch(osc){
            case SINE:    
                aSin.setFreq(sequence[seq_point]);
            case SQUARE:    
                aSqu.setFreq(sequence[seq_point]);
            case SAW:    
                aSaw.setFreq(sequence[seq_point]);
            case TRI:    
                aTri.setFreq(sequence[seq_point]);        
        }
        
        set_mux(seq_point);   //set current channel of mux
        seq_point++;
        
        //attack,decay,sustain,release_ms
        envelope.setTimes(100,100,100,100);
        envelope.noteOn();
        noteDelay.start(tempo);
    }
    
    envelope.update();
    gain = envelope.next();
   
    tog_btn = digitalRead(SEQ_TOG);
    oct_btn = digitalRead(OCT_TOG);
    osc_btn = digitalRead(OSC_SEL);
    mode_btn = digitalRead(MODE_SEL);
    root_btn = digitalRead(ROOT_SEL);

    myNote = current_mode_seq[freq];

//  Set odd indexes of sequence  
   if(!seq_even){
        if(PIND & B00010000){set_step(0, freq);} // Sequence pin 1
        if(PIND & B00100000){set_step(2, freq);} // Sequence pin 2
        if(PIND & B01000000){set_step(4, freq);} // Sequence pin 3
        if(PIND & B10000000){set_step(6, freq);} // Sequence pin 4
    }
    //set even indexes of sequence
    else{
        if(PIND & B00010000){set_step(1, freq);} // Sequence pin 1
        if(PIND & B00100000){set_step(3, freq);} // Sequence pin 2
        if(PIND & B01000000){set_step(5, freq);} // Sequence pin 3
        if(PIND & B10000000){set_step(7, freq);} // Sequence pin 4
    }
    
    if(buttonDelay.ready()){
        /* Oscillator button */
        if(osc_btn){
            current_osc++;
            if(current_osc == 3){current_osc = 0;}
//            Serial.println(osc_btn);
        }
//        Serial.println(current_mode);

        /********************
         ** Handle buttons **
        *********************/
        /* Mode button */   
        if(mode_btn){set_mode();}
        /* Root note button */
        if(root_btn){set_root();}   

        /* Odd / even sequence toggle */
        if(tog_btn){
//        if(PIND & B00000100){
            if(!seq_even){
                seq_even = true;
                PORTC &= B01111100; //clear step bits
                PORTC |= B10000010; //set even led high
            }
            else if(seq_even){
                seq_even = false;
                PORTC &= B01111100; //clear step bits
                PORTC |= B10000001; //set odd led high
            }
        }

        /* Octave button */
        if(oct_btn && !oct_tog){              
              PORTC &= B01111100; //clear step bits
              PORTC |= B10000000;
            for(int i = 0; i<8; i++){
              sequence[i] = sequence[i]*2;
              oct_tog = true;

            }
        }
        else if(oct_btn && oct_tog){
            for(int i = 0; i<8; i++){
              sequence[i] = sequence[i]/2;
              oct_tog = false;
              // PORTD = B00000100;
            }
        }                 
        buttonDelay.start();
    }
}

int updateAudio(){
    switch(osc){
        case SINE:    
            return ((int)aSin.next() * gain)>>8; // shift back into range after multiplying by 8 bit value
        case SQUARE:    
            return ((int)aSqu.next() * gain)>>8; // shift back into range after multiplying by 8 bit value
        case SAW:    
            return ((int)aSaw.next() * gain)>>8; // shift back into range after multiplying by 8 bit value
        case TRI:    
            return ((int)aTri.next() * gain)>>8; // shift back into range after multiplying by 8 bit value
    }
    return ((int)aSin.next() * gain)>>8; // shift back into range after multiplying by 8 bit value
}

void loop(){
    audioHook(); // required here
}

void set_pinmodes(){
    
}


/* Set current step to freq determined to potentiometer*/
void set_step(int curr_step, int new_step){
    sequence_steps[curr_step] = new_step;
    sequence[curr_step] = modes[current_mode][sequence_steps[curr_step]];    
}

/* Set freq sequence based on current steps, getting current index sequence, then getting the freq from that index in current mode */
void set_mode(){
    current_mode ++;
    if(current_mode == 7){
      current_mode = 0;
    }
    for(int i = 0; i < 8; i++){ 
      sequence[i] = modes[current_mode][sequence_steps[i]];
    }
}

/* Set root note for sequence. Multiplies current sequence by NOTE_MULT to incr in semitones */
void set_root(){
    current_root ++;
    /* Increment root note */
    for(int i = 0; i<SEQ_LENGTH; i++){
        sequence[i] = sequence[i] * NOTE_MULT;
        current_mode_seq[i] = current_mode_seq[i] * NOTE_MULT;
    }
    if(current_root == 12){
        current_root = 0;
        /* Loop back around from G# to A */
        for(int i = 0; i<SEQ_LENGTH; i++){
            sequence[i] = sequence[i] / 2;
            current_mode_seq[i] = current_mode_seq[i] / 2;
        }
    }
}

/* Set MUX select pins with registers*/
/* MUX_MASK used to clear mux pins, without affecting others*/
void set_mux(int seq_point){
    PORTC &= MUX_MASK;
    if(seq_point == 0){     //s0=0, s1=1, s2=0
      PORTC &= MUX_MASK;
      PORTC |= B0001000;
    }      
    else if(seq_point == 1){ //s0=1, s1=0, s2=0
      PORTC &= MUX_MASK;
      PORTC |= B00000100;
    } 
    else if(seq_point == 2){ //s0=0, s1=0, s2=0
      PORTC &= MUX_MASK;
      PORTC |= B00000000;
    } 
    else if(seq_point == 3){ //s0=1, s1=1, s2=0
      PORTC &= MUX_MASK;
      PORTC |= B00001100;
    } 
    else if(seq_point == 4){ //s0=1, s1=0, s2=1
      PORTC &= MUX_MASK;
      PORTC |= B00010100;
    } 
    else if(seq_point == 5){ //s0=1, s1=1, s2=1
      PORTC &= MUX_MASK;
      PORTC |= B00011100;
    } 
    else if(seq_point == 6){ //s0=0, s1=1, s2=1
      PORTC &= MUX_MASK;
      PORTC |= B00011000;
    } 
    else if(seq_point == 7){ //s0=0, s1=0, s2=1
      PORTC &= MUX_MASK;
      PORTC |= B00010000;
    } 
}





