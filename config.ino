//** Gestion de la configuration

bool configFactorySetupMode = true; 
bool configSoundActif = true;
String configWifiSsid = "NotDefined"; 
String configWifiPwd = "NotDefined"; 
bool webAvailable = false; 
bool wifiAvailable = false; 

void configInit() {
  storageOpen();
  configFactorySetupMode = storageReadBool(storageKeyFactorySetupMode, configFactorySetupMode);
  configSoundActif = storageReadBool(storageKeySoundActif, configSoundActif);
  configWifiSsid = storageReadString(storageKeyWifiSsid, configWifiSsid);
  configWifiPwd = storageReadString(storageKeyWifiPwd, configWifiPwd);
  storageClose();
  debugTrace("Event","Config initialisation OK");
  watchDogReset();
}

bool isFactorySetupMode() { return configFactorySetupMode; }
void setFactorySetupDone() { configFactorySetupMode=false; storageWriteBool(storageKeyFactorySetupMode, false); webStorageWriteBool(storageKeyFactorySetupMode, false, ""); }
bool isSoundActif() { return configSoundActif; }
void soundSetActif() { configSoundActif = true; storageWriteBool(storageKeySoundActif, true); webStorageWriteBool(storageKeySoundActif, true, ""); }
void soundSetInactif() { configSoundActif = false; storageWriteBool(storageKeySoundActif, false); webStorageWriteBool(storageKeySoundActif, false, ""); }
bool isWebAvailable() { return webAvailable; }
bool isWifiAvailable() { return wifiAvailable; }
void setWifiAvailable() { wifiAvailable=true; webAvailable=true;  }
void setWifiNotAvailable() { wifiAvailable=false; webAvailable=false; }
const char* wifi_SSID () { return configWifiSsid.c_str(); }
const char* wifi_PWD () { return configWifiPwd.c_str(); }