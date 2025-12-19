//** Gestion de l'horloge
#include <time.h>

// Variables internes
bool timeStampOffsetSynchronized = false;
uint32_t timeStampOffset = 0;
uint32_t timeLatestBackup = 0;
uint32_t timeBackupFrequency = 15000; // 15s

void timeInit() {
// Demande l'heure au modem si présent
  if(hasModem()) { timeStampOffset = timeFromModem(); if(timeStampOffset!=0) { timeStampOffsetSynchronized=true; };};
// En cas d'échec, tentative de chargement via le Wifi  
  if((timeStampOffset==0) && isWebAvailable()) { timeStampOffset = timeFromWifi(); if(timeStampOffset!=0) { timeStampOffsetSynchronized=true; };};
// En cas d'échec, chargement de la dernière heure connue depuis le stockage non-volatile
  if(timeStampOffset==0) { timeStampOffset = storageReadUInt(storageKeyTimeStampOffset,0); setLocalTimeFromTimestamp(timeStampOffset); };  
// En cas d'echec, traçage de l'erreur et abandon
  if(timeStampOffset==0) { errorLog(801,"Clock initialisation failure"); return; };
// En cas de succès, mise à jour de l'heure système
  timeStore();
  debugTrace("Event","Time initialisation OK (" + timeFormat() +")"); 
  watchDogReset();
}

void setLocalTimeFromTimestamp(uint32_t timestamp) {
    struct timeval tv;
    tv.tv_sec = timestamp;
    tv.tv_usec = 0;
    // Définir l'heure système
    settimeofday(&tv, NULL);
    // Configurer le fuseau horaire (exemple pour Paris, UTC+1/+2)
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
}

String timeFormat() {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char buffer[80];
  strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

//** Stokage de la dernière heure connue
void timeStore() {
  if(millis() < (timeLatestBackup + timeBackupFrequency)) { return; }; // Limitation de la fréquence d'appel
  storageWriteUInt(storageKeyTimeStampOffset,time());
  timeLatestBackup = millis();
}

//** Renvoi du timestamp Unix
uint32_t time() {
  return timeStampOffset + (millis()/1000);  
}

//** Obtention du timestamp Unix via le modem
uint32_t timeFromModem() {
 // Appel de la commande d'heure au modem
  String cclk = "+CCLK: \"25/07/09,16:40:11+04\""; //SerialAT.println("AT+CCLK?"); 
  errorLog(802,"Obtaining time via modem not available");
  // TODO xxxxxxxxxxx
  return 0;

  // Extraire la partie entre les guillemets
  int debut = cclk.indexOf("\"") + 1;
  int fin = cclk.lastIndexOf("\"");
  String dateTime = cclk.substring(debut, fin);

  // Séparer la date et l'heure
  int an = dateTime.substring(0, 2).toInt() + 2000;
  int mois = dateTime.substring(3, 5).toInt();
  int jour = dateTime.substring(6, 8).toInt();
  int heure = dateTime.substring(9, 11).toInt();
  int minute = dateTime.substring(12, 14).toInt();
  int seconde = dateTime.substring(15, 17).toInt();

  // Fuseau horaire
  int tzSign = dateTime.indexOf('+') >= 0 ? 1 : -1;
  int tzPos = dateTime.indexOf('+');
  if (tzPos == -1) tzPos = dateTime.indexOf('-');
  int tzHeure = dateTime.substring(tzPos + 1, tzPos + 3).toInt();

  // Construire la structure tm
  struct tm t = {0};
  t.tm_year = an - 1900;
  t.tm_mon = mois - 1;
  t.tm_mday = jour;
  t.tm_hour = heure - tzSign * tzHeure;
  t.tm_min = minute;
  t.tm_sec = seconde;

  // Convertir en timestamp Unix UTC
  time_t timestamp = mktime(&t);
  return (uint32_t) timestamp;
}

//** Obtention du timestamp Unix via le wifi
uint32_t timeFromWifi() {
  // Appel du serveur NTP
  const char* ntpServer = "fr.pool.ntp.org";
  configTime(3600, 3600, ntpServer);
  // Attente de la synchronisation
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) { return 0; }
  // Résolution et validation du timestamp
  time_t now;
  time(&now);
  uint32_t timeStamp = (uint32_t)now;
  if(now<timeStamp) { return 0; } // Supérieur au 01/01/2025
  if(2082754800<timeStamp) { return 0; } // Inférieur au 01/01/2036
  return timeStamp;
}

bool isTimeReliable() { return timeStampOffsetSynchronized; }

// ** Fonction test ****************** 
void timeTest() {
  if(timeStampOffsetSynchronized) { 
    debugTrace("Test", "Time test OK : " + String(time()));
  } else {
    debugTrace("Test", "Time test issue");
} }
