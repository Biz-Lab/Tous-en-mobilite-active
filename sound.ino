//** Gestion des sons

void soundOk(bool forceSoundActif) {
  int notes[] = {262, 330, 392, 523};      // do - mi - sol - do+
  int durations[] = {100, 100, 100, 200};  
  soundPlay(4, notes, durations, forceSoundActif);
}

void soundKo(bool forceSoundActif) {
  int notes[] = {220,180};
  int durations[] = {150, 200};
  soundPlay(2, notes, durations, forceSoundActif);
}

void soundBip(bool forceSoundActif) {
  int notes[] = {1500};
  int durations[] = {150};
  soundPlay(1, notes, durations, forceSoundActif);
}

void soundDebug() {
  int notes[] = {1500};
  int durations[] = {150};
  soundPlay(1, notes, durations, true);
}

void soundPlay(int size, const int* notes, const int* durations, bool forceSoundActif) {
  if (!isSoundActif() && !forceSoundActif) { return; }
  for (int i = 0; i < size; i++) {
    tone(soundPin, notes[i], durations[i]);
    delay(durations[i] + 30);
  }
  noTone(soundPin);
}

void soundConfigToggle() { 
  if(configSoundActif) { 
    soundSetInactif();
    displayShow2Lines("Son","OFF",5);
  } else {
    soundSetActif();
    displayShow2Lines("Son","ON",5);
  }
  soundBip(true); 
}

// ** Fonction test ****************** 
void soundTest() {
  displayShow2Lines("Son","Ok",3); soundOk(); delay(2000); watchDogReset();
  displayShow2Lines("Son","Ko",3); soundKo(); delay(2000); watchDogReset();
  displayShow2Lines("Son","Bip",3); soundBip(); delay(2000); watchDogReset();
}
