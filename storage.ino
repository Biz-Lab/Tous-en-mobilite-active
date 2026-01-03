//** Gestion du stockage non-volatile
#include <nvs_flash.h>
#include <Preferences.h>
#include <nvs.h>
Preferences disk;

bool storageIsOpened = false;
void storageOpen() { 
  if(disk.begin("stockage", false))  { storageIsOpened=true; }
  else {errorLog(713,"Non-volatile begin issue"); };
}

void storageClose() { disk.end(); storageIsOpened=false; }

void storageInit() {
  storageOpen();
  // Vérification de la validitée des données stockées
  const uint32_t diskValide = 431528172;
  if(disk.getUInt("diskValide",0)!=diskValide) {
    disk.clear();
    errorLog(703,"Non-volatile data not defined");
    disk.putUInt("diskValide",diskValide);
  };
  storageClose();
  debugTrace("Event","Storage initialisation OK");
  watchDogReset();
}

/*** Bool ***/
bool storageReadBool(const char *key, bool defaultValue) {
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  bool value = disk.getBool(key, defaultValue);
  if(autoOpenclose) { storageClose(); }
  return value;
}

bool webStorageReadBool(const char *key, bool defaultValue) { return (bool)(serverGetString("get-"+String(key))=="TRUE"); }

bool storageWriteBool(const char *key, bool value) {
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  bool oldValue = disk.getBool(key, false);
  bool changed = (oldValue != value);
  bool success = true;
  if(changed) {
    success = disk.putBool(key, value);
    if(!success) { errorLog(704, String("Failed to write ") + key + " = " + String(value)); } 
  }
  if(autoOpenclose) { storageClose(); }
  return success;
}

bool webStorageWriteBool(const char *key, bool value, String additionalData) { return serverPostData("set-"+String(key),"format=bool&value="+String(value?"TRUE":"FALSE")+additionalData); }

/*** UShort ***/
uint16_t storageReadUShort(const char *key, uint16_t defaultValue) {
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  uint16_t value = disk.getUShort(key, defaultValue);
  if(autoOpenclose) { storageClose(); }
  return value;
}

uint16_t webStorageReadUShort(const char *key, uint16_t defaultValue) { return serverGetInt("get-"+String(key),defaultValue); }

bool storageWriteUShort(const char *key, uint16_t value) {
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  uint16_t oldValue = disk.getUShort(key, UINT16_MAX); // Valeur impossible comme marqueur
  bool changed = (oldValue != value);
  bool success = true;
  if(changed) {
    success = disk.putUShort(key, value);
    if(!success) { errorLog(714, String("Failed to write ") + key + " = " + String(value)); } 
  }
  if(autoOpenclose) { storageClose(); }
  return success;
}

bool webStorageWriteUShort(const char *key, uint16_t value, String additionalData) { return serverPostData("set-"+String(key),"format=int&value="+String(value)+additionalData); }

/*** Int ***/
uint32_t storageReadUInt(const char *key, uint32_t defaultValue) {
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  uint32_t value = disk.getUInt(key, defaultValue);
  if(autoOpenclose) { storageClose(); }
  return value;
}

uint32_t webStorageReadUInt(const char *key, uint32_t defaultValue) { return serverGetInt("get-"+String(key),defaultValue); }

bool storageWriteUInt(const char *key, uint32_t value) {
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  uint32_t oldValue = disk.getUInt(key, UINT32_MAX); // Valeur impossible comme marqueur
  bool changed = (oldValue != value);
  bool success = true;
  if(changed) {
    success = disk.putUInt(key, value);
    if(!success) { errorLog(705, String("Failed to write ") + key + " = " + String(value)); };
  }
  if(autoOpenclose) { storageClose(); }
  return success;
}

bool webStorageWriteUInt(const char *key, uint32_t value, String additionalData) { return serverPostData("set-"+String(key),"format=int&value="+String(value)+additionalData); }

/*** String ***/
String storageReadString(const char *key, String defaultValue) {
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  String value = disk.getString(key, defaultValue);
  if(autoOpenclose) { storageClose(); }
  return value;
}

String webStorageReadString(const char *key) { return serverGetString("getData-"+String(key)); }

bool storageWriteString(const char *key, String value) {
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  String oldValue = disk.getString(key, "");
  bool changed = (oldValue != value);
  bool success = true;
  if(changed) {
    success = disk.putString(key, value);
    if(!success) { errorLog(712, String("Failed to write ") + key); }; 
  }
  if(autoOpenclose) { storageClose(); }
  return success;
}

bool webStorageWriteString(const char *key, String value, String additionalData) { return serverPostData("set-"+String(key),"value="+value+additionalData); }

bool storageRemove(const char *key) { 
  bool autoOpenclose = (storageIsOpened==false);
  if(autoOpenclose) { storageOpen(); }
  bool success = disk.remove(key); 
  if(autoOpenclose) { storageClose(); }
  storageCompact();
  return success;
}

void storageCompact() {
  // Fermer Preferences
  if(storageIsOpened) { storageClose(); }
  // Commit explicite pour forcer le nettoyage
  nvs_handle_t handle;
  esp_err_t err = nvs_open("stockage", NVS_READWRITE, &handle);
  if (err == ESP_OK) {
    nvs_commit(handle);
    nvs_close(handle);
  }
}

uint32_t nextStorageSelfMaintenance = 0;
void storageSelfMaintenance() {
  if(time()<nextStorageSelfMaintenance) { return; } // Limitation toutes les 12H
  nextStorageSelfMaintenance = time() + 12*3600;
  storageCompact();
  nvs_stats_t nvs_stats;
  esp_err_t err = nvs_get_stats("nvs", &nvs_stats);
  if (err == ESP_OK) {
    webStorageWriteUShort(storageKeyStorageUsedEntries,nvs_stats.used_entries,"");
    webStorageWriteUShort(storageKeyStorageFreeEntries,nvs_stats.free_entries,"");
    webStorageWriteUShort(storageKeyStorageTotalEntries,nvs_stats.total_entries,"");
    webStorageWriteUShort(storageKeyStorageNamespaceCount,nvs_stats.namespace_count,"");
  }
  uint16_t h1Entries = 0;
  uint16_t h3Entries = 0;
  uint16_t h6Entries = 0;
  uint16_t usedEntries = 0;
  nvs_iterator_t it = NULL;
  esp_err_t res = nvs_entry_find("nvs", "stockage", NVS_TYPE_ANY, &it);
  while (res == ESP_OK) {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    String key = String(info.key);
    if (key.startsWith("h1_")) { h1Entries++; };
    if (key.startsWith("h3_")) { h3Entries++; };
    if (key.startsWith("h6_")) { h6Entries++; };
    usedEntries++;
    res = nvs_entry_next(&it);
  }
  nvs_release_iterator(it);
  if(h1Entries>26) { errorLog(713,"Too many h1 keys :" + String(h1Entries)); }
  if(h3Entries>58) { errorLog(714,"Too many h3 keys :" + String(h3Entries)); }
  if(h6Entries>122) { errorLog(716,"Too many h6 keys :" + String(h6Entries)); }
}

void storageReset() { storageOpen(); disk.clear(); storageClose(); }

void storageFullReset() {
  esp_err_t err = nvs_flash_erase();
  if (err != ESP_OK) { errorLog(701, "Failed to erase NVS: " + String(err)); return; };
  err = nvs_flash_init();
  if (err != ESP_OK) { errorLog(702, "CRITICAL: Failed to reinit NVS!"); return; };
}

// ** Fonction test ****************** 
#if MODE_DEBUG
void storageTest() {
  bool testOk = true;
  // Test d'écriture / relecture local
  uint16_t aleaInit = (uint16_t) millis();
  storageWriteUInt(storageKeyTestArea, aleaInit);
  uint16_t aleaCheck = storageReadUInt(storageKeyTestArea, 0);
  if(aleaInit!=aleaCheck) { testOk = false; errorLog(706,"Storage test local write/read error"); };
  // Test d'écriture / relecture sur le serveur
  if(webStorageWriteUInt(storageKeyTestArea,aleaInit)==false) { testOk = false; errorLog(707,"Storage test web write error"); };
  aleaCheck = webStorageReadUInt(storageKeyTestArea,-1);
  if(aleaInit!=aleaCheck) { testOk = false; errorLog(708,"Storage test web write/read error"); };
  if(testOk) { debugTrace("Test","Storage test Ok"); } else { debugTrace("Test","Storage test issue"); };
}
#endif // MODE_DEBUG
