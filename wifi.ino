// Gestion de la connexion wifi avec portail captif et timeout d'inactivité

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

/* Configuration Wifi */
WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

bool setupPortalActive = false;
unsigned long restartTime = 0;
unsigned long lastActivityTime = 0;  
unsigned long notReconnectBefore = 0;  
int wifiConnectErrorCount = 0;
const unsigned long INACTIVITY_TIMEOUT = 60000; 

bool wifiInit() {
  const char* currentSsid = "";
  const char* currentPwd = "";
  // Connexion au WiFi client
  if(String(wifi_SSID())!="NotDefined") { 
    currentSsid = wifi_SSID();
    currentPwd = wifi_PWD();
  } else {
  // Sinon tentative de connexion au Wifi usine   
    currentSsid = factoryWifi_SSID();
    currentPwd = factoryWifi_PWD();
  }
  WiFi.begin(currentSsid, currentPwd);
// Attente de la connexion au maximum 10 secondes
  int tryCount = 0;
  int wifiStatus = WL_CONNECTED;
  while ((wifiStatus = WiFi.status()) != WL_CONNECTED) {
    if ((tryCount++>50) || (wifiStatus == WL_NO_SSID_AVAIL)) { 
      wifiConnectErrorCount++;
      if(wifiConnectErrorCount<=1) { notReconnectBefore = time()+5; errorLog(1101,"Wifi connexion error to " + String(currentSsid)); displayShow2Lines("WIFI","Ko",5); }
      else if(wifiConnectErrorCount<=10) { notReconnectBefore = time()+30; }
      else { notReconnectBefore = time()+300; }
      return false; 
    } 
    delay(200);
    watchDogReset();
  } 
 // Connexion réussie
  setWifiAvailable();
  debugTrace("Event","Connexion WiFi OK");
  serverPostData("Restart","");
  notReconnectBefore = 0;
  wifiConnectErrorCount = 0;
  watchDogReset();
  return true;
}

bool wifiCheck() {
  // En cas de configuration du Wifi
  if (setupPortalActive) { 
    displayShow2Lines("WiFi","SETUP",3600);
    dnsServer.processNextRequest();
    server.handleClient();
    if (millis() - lastActivityTime > INACTIVITY_TIMEOUT) {
      debugTrace("Event","Timeout d'inactivité atteint ("+String(INACTIVITY_TIMEOUT/1000)+"s) - sortie du mode paramétrage");
      stopConfigPortal();
      displayShow2Lines("SETUP","EXIT",3);
      return false;
    }
    // Vérifier si c'est le moment de redémarrer
    if (restartTime > 0 && millis() >= restartTime) {
      debugTrace("Event","Redémarrage de la borne ...");
      dnsServer.stop();
      ESP.restart();
    }
    return true; 
  }
  // Vérification de la connexion
  if (WiFi.status() != WL_CONNECTED) {
    setWifiNotAvailable();
    if(time()<notReconnectBefore) { return false; }
    return wifiInit();
  }
  return true;
}

void wifiOff() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}

// Variables globales
String ssid_stored = "";
String password_stored = "";
String ssid_test = "";
String password_test = "";
bool connectionSuccess = false;
String networksJson = "[]";

// Fonction pour scanner les réseaux et générer le JSON
String scanWiFiNetworks() {
    debugTrace("Event", "Scan des réseaux WiFi...");
    
    int n = WiFi.scanNetworks();
    String json = "[";
    
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            if (i > 0) json += ",";
            
            json += "{";
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
            json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
            json += "}";
        }
        debugTrace("Event", String(n) + " réseaux trouvés");
    } else {
        debugTrace("Event", "Aucun réseau trouvé");
    }
    
    json += "]";
    WiFi.scanDelete();
    
    return json;
}

// Page HTML du portail de configuration avec scan WiFi
const char* configPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate">
    <meta http-equiv="Pragma" content="no-cache">
    <meta http-equiv="Expires" content="0">
    <title>Configuration WiFi de la Borne de Mobilité Active</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 500px;
            margin: 50px auto;
            padding: 20px;
            background: #f0f0f0;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 10px;
        }
        .subtitle {
            text-align: center;
            color: #666;
            font-size: 14px;
            margin-bottom: 25px;
        }
        .input-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #333;
            font-weight: bold;
        }
        input, select {
            width: 100%;
            padding: 12px;
            border: 1px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
            font-size: 14px;
        }
        select {
            background: white;
            cursor: pointer;
        }
        .network-option {
            padding: 8px;
        }
        .signal-icon {
            margin-right: 8px;
        }
        .manual-input {
            display: none;
            animation: fadeIn 0.3s;
        }
        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }
        button {
            width: 100%;
            padding: 12px;
            background: #4CAF50;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            font-weight: bold;
        }
        button:hover {
            background: #45a049;
        }
        .refresh-btn {
            background: #2196F3;
            margin-bottom: 15px;
        }
        .refresh-btn:hover {
            background: #0b7dda;
        }
        .info {
            color: #666;
            font-size: 14px;
            margin-top: 20px;
            text-align: center;
        }
        .lock-icon {
            color: #f39c12;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📶 Configuration WiFi de la Borne de Mobilité Active</h1>
        <div class="subtitle">Sélectionnez un réseau ou entrez manuellement</div>
        
        <button class="refresh-btn" onclick="refreshNetworks()">🔄 Rafraîchir la liste</button>
        
        <form action="/save" method="POST">
            <div class="input-group">
                <label>Réseaux disponibles:</label>
                <select id="networkSelect" onchange="selectNetwork()">
                    <option value="">-- Chargement des réseaux... --</option>
                </select>
            </div>
            
            <div class="input-group manual-input" id="manualSsid">
                <label>Nom du réseau (SSID):</label>
                <input type="text" id="ssidInput" name="ssid" placeholder="Votre SSID WiFi">
            </div>
            
            <div class="input-group">
                <label>Mot de passe:</label>
                <input type="text" id="passwordInput" name="password" placeholder="Mot de passe WiFi">
            </div>
            
            <button type="submit">Enregistrer et tester</button>
        </form>
    </div>

    <script>
        let networks = [];
        
        function loadNetworks() {
            fetch('/networks')
                .then(response => response.json())
                .then(data => {
                    networks = data;
                    updateNetworkList();
                })
                .catch(error => {
                    console.error('Erreur:', error);
                    document.getElementById('networkSelect').innerHTML = 
                        '<option value="">Erreur de chargement</option>';
                });
        }
        
        function updateNetworkList() {
            const select = document.getElementById('networkSelect');
            select.innerHTML = '<option value="">-- Sélectionnez un réseau --</option>';
            select.innerHTML += '<option value="manual">✏️ Saisie manuelle</option>';
            
            networks.forEach(network => {
                const signalIcon = getSignalIcon(network.rssi);
                const lockIcon = network.secure ? ' 🔒' : '';
                const option = document.createElement('option');
                option.value = network.ssid;
                option.textContent = signalIcon + ' ' + network.ssid + lockIcon;
                select.appendChild(option);
            });
        }
        
        function getSignalIcon(rssi) {
            if (rssi > -50) return '📶';
            if (rssi > -60) return '📶';
            if (rssi > -70) return '📡';
            return '📡';
        }
        
        function selectNetwork() {
            const select = document.getElementById('networkSelect');
            const manualDiv = document.getElementById('manualSsid');
            const ssidInput = document.getElementById('ssidInput');
            const passwordInput = document.getElementById('passwordInput');
            
            if (select.value === 'manual') {
                manualDiv.style.display = 'block';
                ssidInput.value = '';
                ssidInput.required = true;
                passwordInput.focus();
            } else if (select.value !== '') {
                manualDiv.style.display = 'none';
                ssidInput.value = select.value;
                ssidInput.required = false;
                passwordInput.focus();
            } else {
                manualDiv.style.display = 'none';
                ssidInput.value = '';
                ssidInput.required = false;
            }
        }
        
        function refreshNetworks() {
            const select = document.getElementById('networkSelect');
            select.innerHTML = '<option value="">⏳ Scan en cours...</option>';
            
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    networks = data;
                    updateNetworkList();
                })
                .catch(error => {
                    console.error('Erreur:', error);
                    select.innerHTML = '<option value="">Erreur de scan</option>';
                });
        }
        
        // Charger les réseaux au démarrage
        window.onload = loadNetworks;
    </script>
</body>
</html>
)rawliteral";

// Page de vérification en cours
const char* testingPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Test de connexion...</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 400px;
            margin: 50px auto;
            padding: 20px;
            background: #f0f0f0;
            text-align: center;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        .spinner {
            border: 4px solid #f3f3f3;
            border-top: 4px solid #3498db;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        h1 { color: #3498db; }
        p { color: #666; }
    </style>
    <script>
        setTimeout(function() {
            window.location.href = '/result';
        }, 10000);
    </script>
</head>
<body>
    <div class="container">
        <h1>⏳ Test en cours...</h1>
        <div class="spinner"></div>
        <p>Vérification de la connexion WiFi...</p>
        <p><small>Veuillez patienter</small></p>
    </div>
</body>
</html>
)rawliteral";

// Page de succès
const char* successPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Connexion réussie</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 400px;
            margin: 50px auto;
            padding: 20px;
            background: #f0f0f0;
            text-align: center;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 { color: #4CAF50; }
        p { color: #666; }
        .countdown {
            font-size: 24px;
            font-weight: bold;
            color: #4CAF50;
            margin: 20px 0;
        }
    </style>
    <script>
        var seconds = 10;
        function countdown() {
            document.getElementById('timer').innerText = seconds;
            if (seconds > 0) {
                seconds--;
                setTimeout(countdown, 1000);
            }
        }
        window.onload = countdown;
    </script>
</head>
<body>
    <div class="container">
        <h1>✓ Connexion réussie !</h1>
        <p>Les identifiants WiFi sont corrects.</p>
        <p>La borne va redémarrer dans :</p>
        <div class="countdown"><span id="timer">10</span> secondes</div>
    </div>
</body>
</html>
)rawliteral";

// Page d'échec
const char* errorPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Échec de connexion</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 400px;
            margin: 50px auto;
            padding: 20px;
            background: #f0f0f0;
        }
        .container {
            background: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #e74c3c;
            text-align: center;
        }
        .error-msg {
            background: #ffe6e6;
            border-left: 4px solid #e74c3c;
            padding: 15px;
            margin: 20px 0;
            color: #c0392b;
        }
        button {
            width: 100%;
            padding: 12px;
            background: #3498db;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        button:hover {
            background: #2980b9;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>✗ Échec de connexion</h1>
        <div class="error-msg">
            <strong>Impossible de se connecter au réseau WiFi</strong><br>
            Vérifiez que :
            <ul style="margin: 10px 0; padding-left: 20px;">
                <li>Le SSID est correct</li>
                <li>Le mot de passe est correct</li>
                <li>Le réseau est accessible</li>
            </ul>
        </div>
        <button onclick="window.location.href='/'">↻ Réessayer</button>
    </div>
</body>
</html>
)rawliteral";

void wifiSetup() {
  if (!setupPortalActive) {
    debugTrace("Event","Lancement du portail de configuration WiFi...");
    displayShow2Lines("WiFi","SETUP",3600);
    startConfigPortal();
    setupPortalActive = true;
    lastActivityTime = millis();
} }

void startConfigPortal() {
    // Démarrer en mode Point d'Accès
    IPAddress local_IP(192, 168, 5, 1);
    IPAddress gateway(192, 168, 5, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Borne Mobilite Active", "");

    IPAddress IP = WiFi.softAPIP();
    debugTrace("Event","=== Point d'Accès démarré ===");
    debugTrace("Event","SSID: Borne Mobilite Active");
    debugTrace("Event","Adresse IP: " + String(IP.toString()));
    debugTrace("Event","Ouvrez http://192.168.5.1 dans votre navigateur");
    
    // Démarrer le serveur DNS pour le portail captif
    dnsServer.start(DNS_PORT, "*", local_IP);
    debugTrace("Event","Serveur DNS démarré (portail captif actif)");
    
    // Scanner les réseaux WiFi disponibles
    WiFi.mode(WIFI_AP_STA);
    networksJson = scanWiFiNetworks();
    WiFi.mode(WIFI_AP);
    
    // Configuration des routes du serveur web
    server.on("/", HTTP_GET, handleRoot);
    server.on("/networks", HTTP_GET, handleNetworks);
    server.on("/scan", HTTP_GET, handleScan);
    server.on("/save", HTTP_POST, handleSave);
    server.on("/result", HTTP_GET, handleResult);
    
    // Routes pour la détection du portail captif
    // iOS
    server.on("/hotspot-detect.html", HTTP_GET, handleRoot);
    server.on("/library/test/success.html", HTTP_GET, handleRoot);
    
    // Android
    server.on("/generate_204", HTTP_GET, handleRoot);
    server.on("/gen_204", HTTP_GET, handleRoot);
    
    // Windows
    server.on("/connecttest.txt", HTTP_GET, [](){
        lastActivityTime = millis(); 
        server.send(200, "text/plain", "Microsoft Connect Test");
    });
    server.on("/ncsi.txt", HTTP_GET, [](){
        lastActivityTime = millis();  
        server.send(200, "text/plain", "Microsoft NCSI");
    });
    
    // Route catch-all pour capturer toutes les autres requêtes
    server.onNotFound(handleRoot);
    
    server.begin();
    debugTrace("Event","Serveur Web démarré");
}

void stopConfigPortal() {
    debugTrace("Event","Arrêt du portail de configuration WiFi...");
    server.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    setupPortalActive = false;
    restartTime = 0;
    connectionSuccess = false;
    debugTrace("Event","Portail de configuration arrêté - retour au mode normal");
}

void handleRoot() {
    lastActivityTime = millis(); 
    server.send(200, "text/html", configPage);
}

void handleNetworks() {
    lastActivityTime = millis(); 
    server.send(200, "application/json", networksJson);
}

void handleScan() {
    lastActivityTime = millis();  
    debugTrace("Event", "Demande de scan WiFi...");
    WiFi.mode(WIFI_AP_STA);
    networksJson = scanWiFiNetworks();
    WiFi.mode(WIFI_AP);
    server.send(200, "application/json", networksJson);
}

void handleSave() {
    lastActivityTime = millis(); 
    ssid_test = server.arg("ssid");
    password_test = server.arg("password");
    
    debugTrace("Event","=== Test des identifiants WiFi ===");
    debugTrace("Event","SSID: " + ssid_test);
    
    // Envoyer la page de test en cours
    server.send(200, "text/html", testingPage);
    
    // GARDER le mode AP+STA actif pendant le test
    WiFi.mode(WIFI_AP_STA);
    
    // Démarrer la connexion en mode Station
    WiFi.begin(ssid_test.c_str(), password_test.c_str());
    
    debugTrace("Event","Test de connexion");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        debugTrace("Event","WiFi.status : " + wifiStatusToString(WiFi.status()));
        delay(500);
        watchDogReset();
        dnsServer.processNextRequest();
        server.handleClient();
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        debugTrace("Event","✓ Connexion réussie !");
        debugTrace("Event","Adresse IP: " + String(WiFi.localIP().toString()));
        storageOpen();
        storageWriteString(storageKeyWifiSsid, ssid_test);
        storageWriteString(storageKeyWifiPwd, password_test);
        storageWriteUShort(storageKeyWifiReset, 0);
        storageClose();
        connectionSuccess = true;
        WiFi.disconnect();
        
    } else {
        debugTrace("Event","✗ Échec de connexion");
        connectionSuccess = false;
        WiFi.disconnect();
    }
}

void handleResult() {
    lastActivityTime = millis(); 
    if (connectionSuccess) {
        server.send(200, "text/html", successPage);
        // Programmer le redémarrage dans 10 secondes
        restartTime = millis() + 10000;
        debugTrace("Event","Redémarrage programmé dans 10 secondes...");
    } else {
        server.send(200, "text/html", errorPage);
    }
}

String wifiStatusToString(int status) {
    switch (status) {
        case WL_IDLE_STATUS:
            return "Inactif";
        case WL_NO_SSID_AVAIL:
            return "SSID introuvable";
        case WL_SCAN_COMPLETED:
            return "Scan terminé";
        case WL_CONNECTED:
            return "Connecté";
        case WL_CONNECT_FAILED:
            return "Échec de connexion";
        case WL_CONNECTION_LOST:
            return "Connexion perdue";
        case WL_DISCONNECTED:
            return "Déconnecté";
        default:
            return "Statut inconnu";
    }
}