#include <Arduino.h>
#include <headers.h>
int currentSoftwareVersion() { return 122; }

void setup() {
  debugInit();               // Initialisation du port série en mode debug
  watchDogInit();            // Initialisation du WatchDog
  displayInit();             // Initialisation de l'affichage
  storageInit();             // Initialisation du stockage
  configInit();              // Chargement de la configuration locale
  wifiInit();                // Connexion au Wifi
  factorySetup();            // Chargement de la configuration usine
  //modemInit();             // Connexion 4G ## A DEVELOPER ##
  checkConfigOverTheWeb();   // Chargement de la configuration stockée sur le serveur
  timeInit();                // Initialisation de l'horloge interne
  radarInit();               // Initialisation du radar
  powerSavingInit();         // Initialisation du mode d'économie d'énergie ## A DEVELOPER ##
  otaInit();                 // Chargement des mises à jours automatiques
  btnInit();                 // Initialisation des boutons et zones tactiles
  countersInit();            // Chargement des données depuis la mémoire non volatile
  debugTrace("Event","-- End of initialisations --");
  //counterTestValues((int16_t)serverGetInt("getTestSize",1));
  // durcir et tester les  boucles d'erreurs pour éviter de tomber en wathcdog
  //displaytest99();
  //counterDetailsDisplay();
  //counterTestValues(360);
}

void loop() {
  radarLoop();
  btnLoop();
  displayCounter();
  //displaytest99();
  watchDogReset();
  timeStore();
  wifiCheck();
  storageSelfMaintenance();
  errorQueueProcess();
  watchDogAutoReboot();
}
