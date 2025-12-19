//** Gestion du mode débug

//** Initialisation du port série en mode debug
void debugInit() {
  if(!isDebugMode()) { return; };
  Serial.begin(9600);
  delay(100);
  for (int i = 0; i < 30; i++) { Serial.println(""); };
  Serial.println("Event : -- Reboot --");
  Serial.println("Event : Serial port initialisation OK");
}

//** Envoi d'un message de debug vers le port série
void debugTrace(String debugTraceType, String debugTraceDescription) {
  if(isDebugMode()) { Serial.println(debugTraceType + " : " + debugTraceDescription); };
}

//** Activation du mode de test
int debugTestModeStatus = 0;
void IRAM_ATTR debugTestModeToggle() { debugTestModeStatus++; }

void debugTestModeEnable() { if(debugTestModeStatus == 0) { debugTestModeStatus = 1; }; }
bool debugTestMode() {
  if(debugTestModeStatus==1) { displayShowMonoLine("1. Test ecran",3); delay(3000); displayTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==2) { displayShowMonoLine("2. Test 4G",3); delay(3000); modem4GTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==3) { displayShowMonoLine("3. Test Wifi",3); delay(3000); modemWifiTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==4) { displayShowMonoLine("4. Test radar",3); delay(3000); radarTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==5) { displayShowMonoLine("5. Test son",3); delay(3000); soundTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==6) { displayShowMonoLine("6. Test tactils",3); delay(3000); btnTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==7) { displayShowMonoLine("7. Test Id",3); delay(3000); idTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==8) { displayShowMonoLine("8. Test Web Api",3); delay(3000); webApiTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==9) { displayShowMonoLine("9. Test storage",3); delay(3000); storageTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==10) { displayShowMonoLine("10. Test horloge",3); delay(3000); timeTest(); debugTestModeToggle(); return true; };
  debugTestModeStatus = 0; return false; 
}

uint8_t debugTestModeValues = 1;
void debugTestValues() {
  if(debugTestModeValues<=1) { counterTestValues(0); soundBip(); debugTestModeValues=2; return; }
  else if(debugTestModeValues==2) { counterTestValues(12); soundBip(); debugTestModeValues=3; return; }
  else if(debugTestModeValues==3) { counterTestValues(123); soundBip(); debugTestModeValues=4; return; }
  else if(debugTestModeValues==4) { counterTestValues(1234); soundBip(); debugTestModeValues=5; return; }
  else { counterTestValues(11345); soundBip(); debugTestModeValues=1; return; }
}