// Gestion des requêtes serveurs
#include <HTTPClient.h>

String serverGetString(String dataKey) {
  if (!isWebAvailable()) { return "Error"; }
  if (wifiCheck()==false) { return "Error"; }
  // Préparation des données POST
  String postData = "deviceId="+idGet() + "&hwVersion="+hwVersionGet() + "&action="+dataKey + "&t=" + String(millis());
  // Ajout du hmac
  postData = postData + "&s=" + smallHmacCompute(postData);
  return serverQuery(postData); 
}

int32_t serverGetInt(String dataKey, uint32_t defaultValue) {
  String serverReturn = serverGetString(dataKey);
  if(serverReturn=="Error") { return defaultValue; }
  return serverReturn.toInt();
}

bool serverPostData(String action,String data) {
  if (!isWebAvailable()) { return false; }
  if (wifiCheck()==false) { return false; }
  // Préparation des données POST
  String random = String(millis());
  if(data.length()>0) { data = "&" + data; };
  String postData = "deviceId="+idGet() + "&hwVersion="+hwVersionGet() + "&action="+action + data + "&t=" + random;
  // Ajout du hmac
  postData = postData + "&s=" + smallHmacCompute(postData);
  // Envoi de la requête 
  String serverReturn = serverQuery(postData);
  if (serverReturn==("Ok-"+action+"-"+random)) { 
    return true;
  } else {
    if(serverReturn!="Erreur") { errorLog(1002,"serverPostData issue : Send = " + postData + " | Return = " + serverReturn); }
    return false;
  }
}  

String serverQuery(String postData) {
// Création de la ressource HTTP
  String response = "Error";
  HTTPClient http;
  const char* serverUrl = "http://tous-en-mobilite-active.fr/webapi";  // URL du serveur cible
  http.begin(serverUrl);          // Initialiser HTTP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  // Type des données envoyées
  int httpResponseCode = http.POST(postData);      // Envoyer POST
  if (httpResponseCode > 0) {
    response = http.getString();
  } else {
    errorLog(1001,"Erreur lors du POST : " + String(httpResponseCode));
  }
  http.end(); // Libérer ressources
  return response;
}

// ** Fonction test ****************** 
void webApiTest() {
  if(serverPostData("testApi",""))  { debugTrace("Test","Web test Ok"); } else { debugTrace("Test","Web test issue"); };
}
