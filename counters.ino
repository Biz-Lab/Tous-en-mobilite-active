//** Gestion des compteurs de mobilité
#include <nvs_flash.h>
#include <nvs.h>
#include <Preferences.h>

// Agrégation par intervalles multiples pour optimiser l'espace
const uint32_t SLOT_1H_DURATION = 3600;     // 1 heure en secondes (pour compteur 24h)
const uint32_t SLOT_3H_DURATION = 3*3600;   // 3 heures en secondes (pour compteur 7j)
const uint32_t SLOT_6H_DURATION = 6*3600;   // 6 heures en secondes (pour compteur 30j)

int counterUnit = 1;
uint32_t nextCounterUpdate = 0;

// Obtenir les slots actuels pour chaque durée
#define timeStampStart 1735686000 // Timestamp hardcodé = 01/01/2025
uint32_t getCurrentSlot1H() { return (time()-timeStampStart) / SLOT_1H_DURATION; }
uint32_t getCurrentSlot3H() { return (time()-timeStampStart) / SLOT_3H_DURATION; }
uint32_t getCurrentSlot6H() { return (time()-timeStampStart) / SLOT_6H_DURATION; }

// Obtenir les clés de stockage pour chaque type de slot
String getSlotKey1H(uint32_t slot) { return "h1_" + String(slot); }
String getSlotKey3H(uint32_t slot) { return "h3_" + String(slot); }
String getSlotKey6H(uint32_t slot) { return "h6_" + String(slot); }

void countersInit() {
  // Chargement des données
  storageOpen();
  
  // Recueil des compteurs (gardés pour compatibilité affichage)
  dailyCounter = storageReadUShort(storageKeyDailyCounter, 0);
  weeklyCounter = storageReadUShort(storageKeyWeeklyCounter, 0);
  monthlyCounter = storageReadUShort(storageKeyMonthlyCounter, 0);
  
  // Recueil de l'unité d'affichage
  counterUnit = storageReadUShort(storageKeyCounterUnit, 7);
  if(counterUnit==30) { setMonthlyMode(); } 
  else if(counterUnit==7) { setWeeklyMode(); } 
  else { setDailyMode(); }
  
  storageClose();
  debugTrace("Event","Counters initialisation OK");
  watchDogReset();
}

void countersInc() {
  if(!radarNewUser()) { countersRefuseInc(); return; }
  
  storageOpen();
  
  // Incrémenter le slot 1h (pour compteur 24h)
  uint32_t currentSlot1H = getCurrentSlot1H();
  String slotKey1H = getSlotKey1H(currentSlot1H);
  uint16_t slotCount1H = storageReadUShort(slotKey1H.c_str(), 0);
  storageWriteUShort(slotKey1H.c_str(), slotCount1H + 1);
  
  // Incrémenter le slot 3h (pour compteur 7j)
  uint32_t currentSlot3H = getCurrentSlot3H();
  String slotKey3H = getSlotKey3H(currentSlot3H);
  uint16_t slotCount3H = storageReadUShort(slotKey3H.c_str(), 0);
  storageWriteUShort(slotKey3H.c_str(), slotCount3H + 1);
  
  // Incrémenter le slot 6h (pour compteur 30j)
  uint32_t currentSlot6H = getCurrentSlot6H();
  String slotKey6H = getSlotKey6H(currentSlot6H);
  uint16_t slotCount6H = storageReadUShort(slotKey6H.c_str(), 0);
  storageWriteUShort(slotKey6H.c_str(), slotCount6H + 1);
  
  storageClose();
  
  // Mise à jour des compteurs affichés
  dailyCounter++; 
  weeklyCounter++; 
  monthlyCounter++;
  
  radarDisableUser();
  displayPlusOne();
  soundOk();
  countersSave();
  serverPostData("setCounterInc","radar : "+radarDistanceGet());
  nextCounterUpdate = time() + 30; 
}

void countersRefuseInc() {
  displayAlreadyDone();
  soundKo();
  serverPostData("setCounterRefuseInc","radar : "+radarDistanceGet());
}

void countersUpdate() {
  uint32_t currentTime = time();
  
  // Slots actuels
  uint32_t currentSlot1H = getCurrentSlot1H();
  uint32_t currentSlot3H = getCurrentSlot3H();
  uint32_t currentSlot6H = getCurrentSlot6H();
  
  // Calcul des parts des derniers slots à prendre en compte
  uint16_t latestSlot1HPartToAdd = ((((currentSlot1H+1)*SLOT_1H_DURATION) - currentTime) * 100) / SLOT_1H_DURATION;
  uint16_t latestSlot3HPartToAdd = ((((currentSlot3H+1)*SLOT_3H_DURATION) - currentTime) * 100) / SLOT_3H_DURATION;
  uint16_t latestSlot6HPartToAdd = ((((currentSlot6H+1)*SLOT_6H_DURATION) - currentTime) * 100) / SLOT_6H_DURATION;
  
  // Calcul des seuils de slots
  uint32_t dailySlotThreshold = currentSlot1H - 24;      // 24 heures (slots de 1h)
  uint32_t weeklySlotThreshold = currentSlot3H - 56;     // 7 jours (56 slots de 3h)
  uint32_t monthlySlotThreshold = currentSlot6H - 120;   // 30 jours (120 slots de 6h)
  
  storageOpen();
  
  // ===== Recalcul du compteur journalier (slots 1h) =====
  dailyCounter = 0;
  for (uint32_t slot = dailySlotThreshold + 1; slot <= currentSlot1H; slot++) {
    String slotKey = getSlotKey1H(slot);
    uint16_t slotCount = storageReadUShort(slotKey.c_str(), 0);
    if (slotCount > 0) {
      if (slot > dailySlotThreshold) { 
        dailyCounter += slotCount; 
      }
      if (slot == dailySlotThreshold) { 
        dailyCounter += (int)round((float)slotCount * latestSlot1HPartToAdd / 100.0); 
  } } }
  
  // ===== Recalcul du compteur hebdomadaire (slots 3h) =====
  weeklyCounter = 0;
  for (uint32_t slot = weeklySlotThreshold + 1; slot <= currentSlot3H; slot++) {
    String slotKey = getSlotKey3H(slot);
    uint16_t slotCount = storageReadUShort(slotKey.c_str(), 0);
    if (slotCount > 0) {
      if (slot > weeklySlotThreshold) { 
        weeklyCounter += slotCount; 
      }
      if (slot == weeklySlotThreshold) { 
        weeklyCounter += (int)round((float)slotCount * latestSlot3HPartToAdd / 100.0); 
  } } }
  
  // ===== Recalcul du compteur mensuel (slots 6h) =====
  monthlyCounter = 0;
  for (uint32_t slot = monthlySlotThreshold + 1; slot <= currentSlot6H; slot++) {
    String slotKey = getSlotKey6H(slot);
    uint16_t slotCount = storageReadUShort(slotKey.c_str(), 0);
    if (slotCount > 0) {
      if (slot > monthlySlotThreshold) { 
        monthlyCounter += slotCount; 
      }
      if (slot == monthlySlotThreshold) { 
        monthlyCounter += (int)round((float)slotCount * latestSlot6HPartToAdd / 100.0); 
  } } }
  
  // ===== Suppression des slots obsolètes =====
  if (isTimeReliable()) {
    nvs_iterator_t it = NULL;
    esp_err_t res = nvs_entry_find("nvs", "stockage", NVS_TYPE_ANY, &it);
    while (res == ESP_OK) {
      nvs_entry_info_t info;
      nvs_entry_info(it, &info);
      String key = String(info.key);
      // Nettoyage des slots 1h (> 24h)
      if (key.startsWith("h1_")) {
        String suffix = key.substring(3);
        int slot = suffix.toInt();
        if((slot<(dailySlotThreshold - 2))||(slot>currentSlot1H)) {
          storageRemove(key.c_str()); 
      };};
      // Nettoyage des slots 3h (> 7j)
      if (key.startsWith("h3_")) {
        String suffix = key.substring(3);
        int slot = suffix.toInt();
        if((slot<(weeklySlotThreshold - 2))||(slot>currentSlot3H)) {
          storageRemove(key.c_str()); 
      };};
      // Nettoyage des slots 6h (> 30j)
      if (key.startsWith("h6_")) {
        String suffix = key.substring(3);
        int slot = suffix.toInt();
        if((slot<(monthlySlotThreshold - 2))||(slot>currentSlot6H)) {
          storageRemove(key.c_str()); 
      };};
      res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
    watchDogReset();  
  }

  storageClose();
  
  // Mise à jour des compteurs sauvegardés
  countersSave();

  debugTrace("Debug","dailyCounter = " + String(dailyCounter));
  debugTrace("Debug","weeklyCounter = " + String(weeklyCounter));
  debugTrace("Debug","monthlyCounter = " + String(monthlyCounter));
}

void countersAutoUpdate() {
  // Évitement des relancements trop fréquents
  uint32_t currentTime = time();
  if(currentTime < nextCounterUpdate) { return; }
  
  // Mise à jour des compteurs
  nextCounterUpdate = currentTime + 300; // Toutes les 5 minutes
  countersUpdate();
}

void countersSave() {
  storageOpen();
  storageWriteUShort(storageKeyDailyCounter, dailyCounter);
  storageWriteUShort(storageKeyWeeklyCounter, weeklyCounter);
  storageWriteUShort(storageKeyMonthlyCounter, monthlyCounter);
  storageClose();
}

void setDailyMode() {
  digitalWrite(ledDailyPin,HIGH);
  digitalWrite(ledWeeklyPin,LOW);
  digitalWrite(ledMonthlyPin,LOW);
  counterUnit = 1;
  storageWriteUShort(storageKeyCounterUnit, counterUnit);
  webStorageWriteUShort(storageKeyCounterUnit, counterUnit, "&radar="+radarDistanceGet());
  debugTrace("Event","Set daily mode / radar : " + radarDistanceGet());
}

void setWeeklyMode() {
  digitalWrite(ledDailyPin,LOW);
  digitalWrite(ledWeeklyPin,HIGH);
  digitalWrite(ledMonthlyPin,LOW);
  counterUnit = 7;
  storageWriteUShort(storageKeyCounterUnit, counterUnit);
  webStorageWriteUShort(storageKeyCounterUnit, counterUnit, "&radar="+radarDistanceGet());
  debugTrace("Event","Set weekly mode / radar : "+radarDistanceGet());
}

void setMonthlyMode() {
  digitalWrite(ledDailyPin,LOW);
  digitalWrite(ledWeeklyPin,LOW);
  digitalWrite(ledMonthlyPin,HIGH);
  counterUnit = 30;
  storageWriteUShort(storageKeyCounterUnit, counterUnit);
  webStorageWriteUShort(storageKeyCounterUnit, counterUnit, "&radar="+radarDistanceGet());
  debugTrace("Event","Set monthly mode / radar : "+radarDistanceGet());
}

int counterToDisplay() {
  countersAutoUpdate();
  if(counterUnit==1) { return dailyCounter; }
  if(counterUnit==7) { return weeklyCounter; }
  if(counterUnit==30) { return monthlyCounter; }
  setDailyMode();
  return dailyCounter;
}

// ** Fonction test ****************** 
#if MODE_DEBUG
void counterTestValues(int16_t valuesCount) {
  if(valuesCount >= 0) {
    debugTrace("debug","Chargement test de " + String(valuesCount) + " events");
    countersResetSlots();
    if(valuesCount > 0) {
      storageOpen();

      uint32_t slot1H = getCurrentSlot1H();
      int16_t reste = valuesCount;
      int16_t toAdd =0;
      for (int i = 0; i < 24; i++) {
        String slotKey1H = getSlotKey1H(slot1H);
        toAdd = min(reste,(int16_t)ceil(valuesCount / 30.0 / 24.0));
        storageWriteUShort(slotKey1H.c_str(), toAdd);
        reste = reste-toAdd;
        slot1H--;
      }
      watchDogReset();  

      uint32_t slot3H = getCurrentSlot3H();
      reste = valuesCount;
      for (int i = 0; i < 56; i++) {
        String slotKey3H = getSlotKey3H(slot3H);
        toAdd = min(reste,(int16_t)ceil(valuesCount / 30.0 / 24.0 * 3.0));
        storageWriteUShort(slotKey3H.c_str(), toAdd);
        reste = reste-toAdd;
        slot3H--;
      }
      watchDogReset();  

      uint32_t slot6H = getCurrentSlot6H();
      reste = valuesCount;
      for (int i = 0; i < 120; i++) {
        String slotKey6H = getSlotKey6H(slot6H);
        toAdd = min(reste,(int16_t)ceil(valuesCount / 30.0 / 24.0 * 6.0));
        storageWriteUShort(slotKey6H.c_str(), toAdd);
        reste = reste-toAdd;
        slot6H--;
      }
      watchDogReset();  

      storageClose();
      countersUpdate();
} } }

// Fonction pour nettoyer tous les slots (utile pour reset)
void countersResetSlots() {
  debugTrace("Event", "Resetting all counter slots...");
  storageOpen();
  nvs_iterator_t it = NULL;
  esp_err_t res = nvs_entry_find("nvs", "stockage", NVS_TYPE_ANY, &it);
  while (res == ESP_OK) {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    String key = String(info.key);
    if (key.startsWith("h1_")) { storageRemove(key.c_str()); };
    if (key.startsWith("h3_")) { storageRemove(key.c_str()); };
    if (key.startsWith("h6_")) { storageRemove(key.c_str()); };
    res = nvs_entry_next(&it);
  }
  nvs_release_iterator(it);
  watchDogReset();
  dailyCounter = 0;
  weeklyCounter = 0;
  monthlyCounter = 0;
  countersSave();
  storageClose();
  debugTrace("Event", "All counter slots reset");
}

void counterDetailsDisplay() {
  // Slots actuels
  uint32_t currentSlot1H = getCurrentSlot1H();
  uint32_t currentSlot3H = getCurrentSlot3H();
  uint32_t currentSlot6H = getCurrentSlot6H();
  // Calcul des seuils de slots
  uint32_t dailySlotThreshold = currentSlot1H - 24;      // 24 heures (slots de 1h)
  uint32_t weeklySlotThreshold = currentSlot3H - 56;     // 7 jours (56 slots de 3h)
  uint32_t monthlySlotThreshold = currentSlot6H - 120;   // 30 jours (120 slots de 6h)
  storageOpen();  
  // ===== Recalcul du compteur journalier (slots 1h) =====
  debugTrace("Debug","dailyCounter = " + String(dailyCounter));
  dailyCounter = 0;
  for (uint32_t slot = dailySlotThreshold + 1; slot <= currentSlot1H; slot++) {
    String slotKey = getSlotKey1H(slot);
    uint16_t slotCount = storageReadUShort(slotKey.c_str(), 0);
    if (slotCount > 0) { debugTrace("Debug", "SlotKey1H;"+slotKey+";"+String(slot*SLOT_1H_DURATION+timeStampStart)+";"+slotCount); } 
  } 
  // ===== Recalcul du compteur hebdomadaire (slots 3h) =====
  debugTrace("Debug","weeklyCounter = " + String(weeklyCounter));
  weeklyCounter = 0;
  for (uint32_t slot = weeklySlotThreshold + 1; slot <= currentSlot3H; slot++) {
    String slotKey = getSlotKey3H(slot);
    uint16_t slotCount = storageReadUShort(slotKey.c_str(), 0);
    if (slotCount > 0) { debugTrace("Debug", "SlotKey3H;"+slotKey+";"+String(slot*SLOT_3H_DURATION+timeStampStart)+";"+slotCount); } 
  }
  // ===== Recalcul du compteur mensuel (slots 6h) =====
  debugTrace("Debug","monthlyCounter = " + String(monthlyCounter));
  monthlyCounter = 0;
  for (uint32_t slot = monthlySlotThreshold + 1; slot <= currentSlot6H; slot++) {
    String slotKey = getSlotKey6H(slot);
    uint16_t slotCount = storageReadUShort(slotKey.c_str(), 0);
    if (slotCount > 0) { debugTrace("Debug", "SlotKey6H;"+slotKey+";"+String(slot*SLOT_6H_DURATION+timeStampStart)+";"+slotCount); } 
  } 
  storageClose();
}  
#endif // MODE_DEBUG