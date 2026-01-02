//** Gestion du mode débug

//** Initialisation du port série en mode debug
void debugInit() {
  if(!MODE_DEBUG) { return; };
  Serial.begin(9600);
  delay(100);
  for (int i = 0; i < 30; i++) { Serial.println(""); };
  Serial.println("Event : -- Reboot --");
  Serial.println("Event : Serial port initialisation OK");
}

//** Envoi d'un message de debug vers le port série
void debugTrace(String debugTraceType, String debugTraceDescription) {
  if(MODE_DEBUG) { Serial.println(debugTraceType + " : " + debugTraceDescription); };
}

//** Activation du mode de test
#if MODE_DEBUG
int debugTestModeStatus = 0;
void IRAM_ATTR debugTestModeToggle() { debugTestModeStatus++; }

void debugTestModeEnable() { if(debugTestModeStatus == 0) { debugTestModeStatus = 1; }; }
bool debugTestMode() {
  int debugTestModeId = 0;
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("1. Test ecran",3); delay(3000); displayTest(); debugTestModeToggle(); return true; };
  #if GPRS_HARDWARE 
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("2. Test 4G",3); delay(3000); modem4GTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("3. Test Wifi",3); delay(3000); modemWifiTest(); debugTestModeToggle(); return true; };
  #endif // GPRS_HARDWARE
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("4. Test radar",3); delay(3000); radarTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("5. Test son",3); delay(3000); soundTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("6. Test tactils",3); delay(3000); btnTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("7. Test Id",3); delay(3000); idTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("8. Test Web Api",3); delay(3000); webApiTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("9. Test storage",3); delay(3000); storageTest(); debugTestModeToggle(); return true; };
  if(debugTestModeStatus==(++debugTestModeId)) { displayShowMonoLine("10. Test horloge",3); delay(3000); timeTest(); debugTestModeToggle(); return true; };
  debugTestModeStatus = 0; return false; 
}

#endif // MODE_DEBUG