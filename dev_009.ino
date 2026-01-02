#include <Arduino.h>
#include <headers.h>
#define CURRENT_SOFTWARE_VERSION 142
#define MODE_DEBUG false // TODO A DESACTIVER
#define GPRS_HARDWARE false
#define FACTORY_WIFI_SSID "xxxxxx"
#define FACTORY_WIFI_PWD "xxxxxxx"

void setup() {
  debugInit();               // Initialisation du port série en mode debug
  watchDogInit();            // Initialisation du WatchDog
  displayInit();             // Initialisation de l'affichage
  storageInit();             // Initialisation du stockage
  configInit();              // Chargement de la configuration locale
  wifiInit();                // Connexion au Wifi
  factorySetup();            // Chargement de la configuration usine
  //modemInit();             // Connexion 4G ## A DEVELOPER ##
  timeInit();                // Initialisation de l'horloge interne
  radarInit();               // Initialisation du radar
  powerSavingInit();         // Initialisation du mode d'économie d'énergie ## A DEVELOPER ##
  otaInit();                 // Chargement des mises à jours automatiques
  btnInit();                 // Initialisation des boutons et zones tactiles
  countersInit();            // Chargement des données depuis la mémoire non volatile
  debugTrace("Event","-- End of initialisations --");
  //counterTestValues((int16_t)serverGetInt("getTestSize",1));
  // TODO : durcir et tester les  boucles d'erreurs pour éviter de tomber en wathcdog
  //displaytest99();
  //counterDetailsDisplay();
  //counterTestValues(360);
}

void loop() {
  radarLoop();
  btnLoop();
  displayCounter();
  watchDogReset();
  timeStore();
  wifiCheck();
  storageSelfMaintenance();
  errorQueueProcess();
  watchDogAutoReboot();
  lifeSignalToServer();
}
