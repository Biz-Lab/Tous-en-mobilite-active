//** Gestion de la configuration

//** Configuration dynamique
bool configFactorySetupMode = true; 
bool configDebugMode = true;
bool configSoundActif = true;
bool hasModemOnShip = false;
uint16_t configRadarProximityEntryThreshold = 100;
uint16_t configRadarProximityExitThreshold = 160;
uint16_t configNewUserMinDelay = 3;
uint16_t configNewUserMaxDelay = 30;
bool configWifiReset = false; 
String configWifiSsid = "NotDefined"; 
String configWifiPwd = "NotDefined"; 
bool webAvailable = false; 
bool wifiAvailable = false; 
bool configStorageToReset = false;

void configInit() {
  storageOpen();
  configFactorySetupMode = storageReadBool(storageKeyFactorySetupMode, configFactorySetupMode);
  configDebugMode = storageReadBool(storageKeyDebugMode, configDebugMode);
  configSoundActif = storageReadBool(storageKeySoundActif, configSoundActif);
  configRadarProximityExitThreshold = storageReadUShort(storageKeyRadarProximityExitThreshold, configRadarProximityExitThreshold);
  configNewUserMinDelay = storageReadUShort(storageKeyNewUserMinDelay, configNewUserMinDelay);
  configNewUserMaxDelay = storageReadUShort(storageKeyNewUserMaxDelay, configNewUserMaxDelay);
  configWifiReset = storageReadBool(storageKeyWifiReset, configWifiReset);
  if(configWifiReset!=true) {
    configWifiSsid = storageReadString(storageKeyWifiSsid, configWifiSsid);
    configWifiPwd = storageReadString(storageKeyWifiPwd, configWifiPwd);
  };
  if(hwVersionGet()=="TODO") { hasModemOnShip=true; } // TODO ?????????????????????????????
  configStorageToReset = storageReadBool(storageKeyStorageToReset, configStorageToReset);
  storageClose();
  debugTrace("Event","Config initialisation OK");
  watchDogReset();
}

void checkConfigOverTheWeb() {
  if(!isWebAvailable()) { return; };
  storageOpen();
  storageWriteBool(storageKeyFactorySetupMode,webStorageReadBool(storageKeyFactorySetupMode,configFactorySetupMode));
  storageWriteBool(storageKeyDebugMode,webStorageReadBool(storageKeyDebugMode,configDebugMode));
  storageWriteBool(storageKeySoundActif,webStorageReadBool(storageKeySoundActif,configSoundActif));
  storageWriteUShort(storageKeyRadarProximityEntryThreshold,webStorageReadUShort(storageKeyRadarProximityEntryThreshold,configRadarProximityEntryThreshold));
  storageWriteUShort(storageKeyRadarProximityExitThreshold,webStorageReadUShort(storageKeyRadarProximityExitThreshold,configRadarProximityExitThreshold));
  storageWriteUShort(storageKeyNewUserMinDelay,webStorageReadUShort(storageKeyNewUserMinDelay,configNewUserMinDelay));
  storageWriteUShort(storageKeyNewUserMaxDelay,webStorageReadUShort(storageKeyNewUserMaxDelay,configNewUserMaxDelay));
  storageWriteBool(storageKeyWifiReset,webStorageReadBool(storageKeyWifiReset,configWifiReset));
  storageWriteBool(storageKeyStorageToReset,webStorageReadBool(storageKeyStorageToReset,configStorageToReset));
  storageClose();
  configInit();
  watchDogReset();
}

bool isFactorySetupMode() { return configFactorySetupMode; }
void setFactorySetupDone() { configFactorySetupMode=false; storageWriteBool(storageKeyFactorySetupMode, false); webStorageWriteBool(storageKeyFactorySetupMode, false); }
bool resetStorage() { return configStorageToReset; }
void setStorageReseted() { configStorageToReset=false; storageWriteBool(storageKeyStorageToReset, false); webStorageWriteBool(storageKeyStorageToReset, false); }
bool isDebugMode() { return configDebugMode; }
int RadarProximityEntryThreshold() { return configRadarProximityEntryThreshold; }
int RadarProximityExitThreshold() { return configRadarProximityExitThreshold; }
int newUserMinDelay() { return configNewUserMinDelay; }
int newUserMaxDelay() { return configNewUserMaxDelay; }
bool isSoundActif() { return configSoundActif; }
void soundSetActif() { configSoundActif = true; storageWriteBool(storageKeySoundActif, true); webStorageWriteBool(storageKeySoundActif, true); }
void soundSetInactif() { configSoundActif = false; storageWriteBool(storageKeySoundActif, false); webStorageWriteBool(storageKeySoundActif, false); }
bool hasModem() { return hasModemOnShip; }
bool isWebAvailable() { return webAvailable; }
bool isWifiAvailable() { return wifiAvailable; }
void setWifiAvailable() { wifiAvailable=true; webAvailable=true;  }
void setWifiNotAvailable() { wifiAvailable=false; webAvailable=false; }
const char* factoryWifi_SSID () { return "Martenot_RDJ"; }
const char* factoryWifi_PWD () { return "wifi-martenot-2012"; }
const char* wifi_SSID () { return configWifiSsid.c_str(); }
const char* wifi_PWD () { return configWifiPwd.c_str(); }