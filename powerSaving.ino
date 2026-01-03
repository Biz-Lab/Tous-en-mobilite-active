//** Gestion du mode d'énomomie d'énergie

//#include "esp_bt.h"

// Initialisation du port série en mode debug
void powerSavingInit() {
  //debugTrace("Event","Power Saving initialisation OK");
  debugTrace("Event","Power Saving TO DEV ...");

  // Test désactivation Bluetooth : disableBluetooth();
  // Test désactivation wifi
  // passer l'écran en mode économie d'énergie
  // éteindre l'écran sur historique de présence + détection radar
  // passer l'esp32 en veille avec wake-up via les zones tactiles, voir activer la sortie radar  
  // Tester de ralentir l'esp32 via delay ou via son horloge

  watchDogReset();
}

void disableBluetooth() {
  //esp_bluedroid_disable();
  //esp_bluedroid_deinit();
  //esp_bt_controller_disable();
  //esp_bt_controller_deinit();
}

int previousIdleStatus = 0;
void powerSavingOnIdle(uint32_t idleSeconds) {
  if(idleSeconds > powerSavingIdleTime) {
    if(previousIdleStatus!=2) { previousIdleStatus = 2; displayTurnOff(); serverPostData("powerSaving","Off"); };
  } else if(idleSeconds > powerSavingAttenuationTime) {
    if(previousIdleStatus!=1) { previousIdleStatus = 1; displaySetIntensity(7); };
  } else {
    if(previousIdleStatus!=0) { previousIdleStatus = 0; displayTurnOn(); serverPostData("powerSaving","On"); displayShow2Lines("Power","On",5); };
  }
}