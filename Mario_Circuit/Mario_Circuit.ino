/* 
 Educational BoosterPack MK II - Konami/Contra Intro
http://boosterpackdepot.info/wiki/index.php?title=Educational_BoosterPack_MK_II
 
 Play Konami/Contra Intro through the buzzer, demonstrating
 buzzer tune() API and pitch/tone/simple music generation
 
 Dec 2012 - Created for Educational BoosterPack
            buzzer Pin = 19
 Dec 2013 - Modified for Educational BoosterPack MK II
            buzzer Pin = 40
 by Dung Dang
 
 */
#include "pitches.h"


int buzzerPin = 40;

 
// notes in the melody:
int melody[] = {
  NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A5, //Contador de inicio de carrera
  
  NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_AS4, NOTE_B4, NOTE_C5, NOTE_CS5, NOTE_CS5, NOTE_CS5, NOTE_CS5,   //Intro 

  NOTE_C3, NOTE_C3, NOTE_C4, //Timbales

  NOTE_G5, NOTE_A5, NOTE_FS5, NOTE_C5, NOTE_D5, NOTE_G5, NOTE_A5, NOTE_FS5, NOTE_A5, NOTE_B5,
  NOTE_F5, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_D5, 
  NOTE_G5, NOTE_A5, NOTE_FS5, NOTE_A5, NOTE_AS5,

  NOTE_G5, NOTE_A5, NOTE_FS5, NOTE_C5, NOTE_D5, NOTE_G5, NOTE_A5, NOTE_FS5, NOTE_A5, NOTE_B5,
  NOTE_F5, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_D5, 
  NOTE_D4, NOTE_G4, NOTE_B4, NOTE_B4
  
};
   
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
 2, 2, 2, 1,
  
 4, 16, 16, 14, 16, 6, 6, 6, 6, 8, 8, 16, 3,    // 11 start screen  notes

 24, 24 ,24,

 7, 7, 7, 6, 2, 7, 7, 7, 7, 2,
 7, 7, 7, 6, 2,
 7, 7, 5, 5, 3,

 7, 7, 7, 6, 2, 7, 7, 7, 7, 2,
 7, 7, 7, 6, 2,
 7, 7, 4, 4
};

void setup() 
{
pinMode(buzzerPin,OUTPUT);
}
void loop() 
{
  for (int thisNote = 0; (sizeof(melody)/sizeof(int)); thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    // Had to down tempo to 100/150 
    int noteDuration = 1500/noteDurations[thisNote];
    tone(buzzerPin, melody[thisNote],noteDuration);

    int pauseBetweenNotes = noteDuration + 50;      //delay between pulse
    delay(pauseBetweenNotes);
    
    noTone(buzzerPin);                // stop the tone playing
  }
}
