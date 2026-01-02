// Gestion des zônes tactiles et des leds associées

// Déclaration de la structure de données visant notamment à filtrer le bruit éventuel
struct btnStruct {
  int pin;
  int currentValue;  
  int previousValue;
  int stableValue;
  bool stableValueBool;
  bool newValue;
  int longStableValue;
  bool longStableValueBool;
  bool newLongValue;
  uint32_t latestChange;
};

// Prototype de la fonction de lecture de la valeur des zônes tactiles (obligatoire pour la compilation)
void btnReadStableValue(btnStruct &buttonRef);

// Déclaration des boutons
btnStruct plusOneButton = {btnPlusOnePin,LOW,LOW,LOW,false,false,LOW,false,false,0};
btnStruct dailyButton = {btnDailyPin,LOW,LOW,LOW,false,false,LOW,false,false,0};
btnStruct weeklyButton = {btnWeeklyPin,LOW,LOW,LOW,false,false,LOW,false,false,0};
btnStruct monthlyButton = {btnMonthlyPin,LOW,LOW,LOW,false,false,LOW,false,false,0};
btnStruct wifiSetupButton = {btnWifiSetup,HIGH,HIGH,HIGH,true,false,HIGH,true,false,0};
btnStruct idPwdButton = {btnIdPwd,HIGH,HIGH,HIGH,true,false,HIGH,true,false,0};
btnStruct soundToggle = {btnSoundToggle,HIGH,HIGH,HIGH,true,false,HIGH,true,false,0};
         
//** Fonction d'initialisation         
void btnInit() {
// Déclaration des boutons  
  pinMode(btnPlusOnePin, INPUT);
  pinMode(btnDailyPin, INPUT);
  pinMode(btnWeeklyPin, INPUT);
  pinMode(btnMonthlyPin, INPUT);
  pinMode(btnWifiSetup, INPUT_PULLUP);
  pinMode(btnIdPwd, INPUT_PULLUP);
  pinMode(btnSoundToggle, INPUT_PULLUP);
// Déclaration des leds  
  pinMode(ledDailyPin, OUTPUT);
  pinMode(ledWeeklyPin, OUTPUT);
  pinMode(ledMonthlyPin, OUTPUT);
// Fin
  debugTrace("Event","Btn initialisation OK");
  watchDogReset();
}

//** Fonction de production
void btnLoop() {
// Détection de l'état de chaque zône tactile
  btnReadStableValue(plusOneButton);
  btnReadStableValue(dailyButton);
  btnReadStableValue(weeklyButton);
  btnReadStableValue(monthlyButton);
  btnReadStableValue(wifiSetupButton);
  btnReadStableValue(idPwdButton);
  btnReadStableValue(soundToggle);
// En cas de détection +1  
  if(plusOneButton.newValue && plusOneButton.stableValueBool && radarUserPresence()) { countersInc(); };

// En cas de détection d'un changement de mode 
  if((dailyButton.newValue || weeklyButton.newValue || monthlyButton.newValue) && radarUserPresence()) {
    if(dailyButton.stableValueBool && !weeklyButton.stableValueBool && !monthlyButton.stableValueBool) {
    // Passage en mode jour  
      setDailyMode(); soundBip();
    }  
    if(weeklyButton.stableValueBool) {
    // Passage en mode 7 jours 
      setWeeklyMode(); soundBip();
    }
    if(!dailyButton.stableValueBool && !weeklyButton.stableValueBool && monthlyButton.stableValueBool) {
    // Passage en mode 30 jours  
      setMonthlyMode(); soundBip();
  } } 
// En cas d'appui sur le bouton de configuration du Wifi
  if(wifiSetupButton.newValue && !wifiSetupButton.stableValueBool) { wifiSetup(); };
// En cas d'appui sur le bouton d'affichage de l'identifiant de l'appareil
  if(idPwdButton.newValue && !idPwdButton.stableValueBool) { idShow(); };
 // En cas d'appui supérieur à 3s sur le bouton d'affichage de l'identifiant de l'appareil
  if(idPwdButton.newLongValue && !idPwdButton.longStableValueBool) { idPwdAction(); };
// En cas d'appui sur le bouton d'allumage / extinction du son
  if(soundToggle.newValue && !soundToggle.stableValueBool) { soundConfigToggle(); };
}

//** Fonction de lecture de la valeur des zônes tactiles
void btnReadStableValue(btnStruct &buttonRef) {
// Initialisation de la valeur de retour et lecture de la valeur courante
  buttonRef.newValue = false;
  buttonRef.newLongValue = false;
  buttonRef.currentValue = digitalRead(buttonRef.pin);
// Détection d'une nouvelle valeur
  if (buttonRef.currentValue != buttonRef.previousValue) {
    buttonRef.previousValue = buttonRef.currentValue;
    buttonRef.latestChange = millis(); 
  }
// Détection d'une nouvelle valeur stable durant plus de 0.05s
  if ((buttonRef.latestChange < (millis() - 50)) && (buttonRef.stableValue != buttonRef.previousValue)) {
    buttonRef.stableValue = buttonRef.previousValue;
    if(buttonRef.stableValue == LOW) { buttonRef.stableValueBool = false; } else { buttonRef.stableValueBool = true; }
    buttonRef.newValue = true;
  }
// Détection d'une nouvelle valeur stable durant plus de 3s
  if ((buttonRef.latestChange < (millis() - 3000)) && (buttonRef.longStableValue != buttonRef.previousValue)) {
    buttonRef.longStableValue = buttonRef.previousValue;
    if(buttonRef.longStableValue == LOW) { buttonRef.longStableValueBool = false; } else { buttonRef.longStableValueBool = true; }
    buttonRef.newLongValue = true;
} }

// ** Fonction test ****************** 
#if MODE_DEBUG
void btnTest() {
// Détection de l'état de chaque zône tactile et retranscription sur les leds correspondantes
  btnReadStableValue(plusOneButton); if(plusOneButton.newValue) { debugTrace("Test","btn plusOneButton "+ String(plusOneButton.stableValue)); soundBip(); };  // digitalWrite(plusOneButton.pin, plusOneButton.stableValue); soundBip();
  btnReadStableValue(dailyButton); if(dailyButton.newValue) { debugTrace("Test","btn dailyButton "+ String(plusOneButton.stableValue)); soundBip(); };
  btnReadStableValue(weeklyButton); if(weeklyButton.newValue) { debugTrace("Test","btn weeklyButton "+ String(plusOneButton.stableValue)); soundBip(); };
  btnReadStableValue(monthlyButton); if(monthlyButton.newValue) { debugTrace("Test","btn monthlyButton "+ String(plusOneButton.stableValue)); soundBip(); };
  btnReadStableValue(wifiSetupButton); if(wifiSetupButton.newValue) { debugTrace("Test","btn wifiSetupButton "+ String(wifiSetupButton.stableValue)); soundBip(); };
  btnReadStableValue(idPwdButton); if(idPwdButton.newValue) { debugTrace("Test","btn idPwdButton "+ String(idPwdButton.stableValue)); soundBip(); };
  btnReadStableValue(soundToggle); if(soundToggle.newValue) { debugTrace("Test","btn soundToggle "+ String(soundToggle.stableValue)); soundBip(); };
}
#endif // MODE_DEBUG