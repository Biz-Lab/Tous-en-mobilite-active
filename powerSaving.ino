//** Gestion du mode d'énomomie d'énergie

//#include "esp_bt.h"

// Initialisation du port série en mode debug
void powerSavingInit() {
  //debugTrace("Event","Power Saving initialisation OK");
  debugTrace("Event","Power Saving TO DEV ...");

  // Test désactivation Bluetooth : disableBluetooth();
  // Test désactivation radar hors action
  // Test désactivation wifi
  // passer l'écran en mode économie d'énergie
  // éteindre l'écran sur historique de présence + détection radar
  // passer l'esp32 en veille avec wake-up via les zones tactiles, voir activer la sortie radar 
  // Passer le radar en mode out  https://github.com/ncmreynolds/ld2410
  // Logger les commandes 
  // Tester de ralentir l'esp32 via delay ou via son horloge

  watchDogReset();
}

void disableBluetooth() {
  //esp_bluedroid_disable();
  //esp_bluedroid_deinit();
  //esp_bt_controller_disable();
  //esp_bt_controller_deinit();
}

void powerSavingLoop() {
  debugTrace("TODO","powerSavingLoop not dev");
}