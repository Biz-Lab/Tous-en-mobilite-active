//** Gestion du WatchDog

#include <esp_task_wdt.h>
esp_err_t ESP32_ERROR;
uint32_t watchDogLatestReset = 0;

void watchDogInit() {
  esp_task_wdt_deinit();
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 15 * 1000,                 
    .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, 
    .trigger_panic = true
  };
  ESP32_ERROR = esp_task_wdt_init(&wdt_config);
  String latestError = String(esp_err_to_name(ESP32_ERROR));
  if(latestError != "ESP_OK") { errorLog(901,"Reset on error : " + latestError); }
  esp_task_wdt_add(NULL);
  watchDogLatestReset = millis();
  debugTrace("Event","WatchDog initialisation OK");
}

void watchDogReset() {
  if(millis() < (watchDogLatestReset+5000)) { return; }; // Limitation de la fréquence d'appel
  esp_task_wdt_reset();
  watchDogLatestReset = millis();
  delay(1);
}

void watchDogStop() {
  esp_task_wdt_delete(NULL);
  esp_task_wdt_deinit();
  debugTrace("Event", "WatchDog arrêté");
}

void reboot() {
  esp_restart();
}

void watchDogAutoReboot() {
  if(millis()>4*60*60*1000) { // Toutes les 4h
    esp_restart();
  }
}