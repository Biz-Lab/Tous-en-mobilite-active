//** Gestion des mises à jour logiciel
#include <Update.h>
#include <HTTPClient.h>

//** Détection si le système est à jour 
void otaInit() {
  if(!isWifiAvailable()) { return; };
  int currentVersion = CURRENT_SOFTWARE_VERSION;
  int webLatestVersion = serverGetInt("getLatestFirmwareVersion");
  if(currentVersion>=webLatestVersion) {
    debugTrace("Event","Software version OK (v" + String(currentVersion) + ")");
    webStorageWriteUInt(storageKeyCurrentFirmwareVersion,CURRENT_SOFTWARE_VERSION,"");
  } else {
    debugTrace("Event","A new version of software is available (v" + String(webLatestVersion) + " vs " + String(currentVersion) + ")");
    otaUpdatePerform();
  }
  watchDogReset(); 
}

void otaUpdatePerform() {
  // Précontrôle de la signature du fichier par mesure de sécurité
  String firmwareURL = serverGetString("getLatestFirmwareURL");
  String firmwareHmac = serverGetString("getLatestFirmwareHmac");
  int firmwareSize = serverGetInt("getLatestFirmwareSize");
  if(hmacCompute(firmwareURL)!=firmwareHmac) { errorLog(507,"firmwareUpdate - firmwareURL Hmac issue "+firmwareHmac+" vs "+hmacCompute(firmwareURL)); return; }
  // ---
  serverPostData("firmwareUpdate");
  watchDogStop();
  HTTPClient http;
  http.begin(firmwareURL);
  int httpCode = http.GET();
  if (httpCode != 200) { errorLog(501,"Software update issue : " + String(httpCode)); }
  else {
    int contentLength = http.getSize();
    if(contentLength!=firmwareSize) { errorLog(502,"firmwareUpdate - file size issue : " + String(contentLength)+ " vs " + String(firmwareSize)); }
    else {
      if (!Update.begin(contentLength)) { errorLog(503,"firmwareUpdate - no enough room to update"); } 
      else {
        Stream& stream = http.getStream();
        size_t written = Update.writeStream(stream);
        if (written != contentLength) { errorLog(504,"firmwareUpdate - written issue"); }
        else {
          debugTrace("Event","Firmware uploaded"); 
          if (!Update.end()) { errorLog(505,"firmwareUpdate - update issue : " + String(Update.getError())); }
          else {
            if (!Update.isFinished()) { errorLog(506,"firmwareUpdate - update not Finished"); }
            else {
              debugTrace("Event","Firmware updated");
              debugTrace("Event","Restarting ...");
              delay(1000);
              ESP.restart();
  } } } } } }
  http.end();
}
