// Gestion de la connexion internet
#if GPRS_HARDWARE
//TODO

#define TINY_GSM_MODEM_SIM800
#define SIM800L_IP5306_VERSION_20200811
#include "modemUtilities.h"

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to the module)
#define SerialAT  Serial1

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Define how you're planning to connect to the internet.
// This is only needed for this example, not in other code.
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

// Your GPRS credentials, if any
const char apn[]      = "iot.1nce.net";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

#include <TinyGsmClient.h>
//#include <ArduinoHttpClient.h>

// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

void modemInit() {
    debugTrace("Event","start modem initialisation");

    // Set GSM module baud rate and UART pins
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
#ifdef MODEM_RST
    // Keep reset high
    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, HIGH);
#endif
    pinMode(MODEM_PWRKEY, OUTPUT);
    pinMode(MODEM_POWER_ON, OUTPUT);

    // Turn on the Modem power first
    digitalWrite(MODEM_POWER_ON, HIGH);

    // Pull down PWRKEY for more than 1 second according to manual requirements
    digitalWrite(MODEM_PWRKEY, HIGH); delay(100);
    digitalWrite(MODEM_PWRKEY, LOW); delay(1000);
    digitalWrite(MODEM_PWRKEY, HIGH);

    // Initialize the indicator as an output
    pinMode(LED_GPIO, OUTPUT);
    digitalWrite(LED_GPIO, LED_OFF); delay(6000);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    modem.restart();
    // modem.init();

    String modemInfo = modem.getModemInfo();
    debugTrace("Event","Modem Info: " + modemInfo);

#if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if (GSM_PIN && modem.getSimStatus() != 3) {
        modem.simUnlock(GSM_PIN);
    }
#endif
}

bool modemEx() {
#if TINY_GSM_USE_WIFI
    // Wifi connection parameters must be set before waiting for the network
    debugTrace("Event","Setting WIFI SSID/password...");
    if (!modem.networkConnect(wifiSSID, wifiPass)) { errorLog(401,"Wifi connection fails"); return false; };
    debugTrace("Event","Wifi connection success");
#endif

#if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
    // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);
#endif

    debugTrace("Event","Waiting for mobile network...");
    if (!modem.waitForNetwork()) { errorLog(402,"Mobile network NOT available"); return false; };
    debugTrace("Event","Mobile network available");

    if (!modem.isNetworkConnected()) { errorLog(403,"Mobile network NOT connected"); return false; };
    debugTrace("Event","Mobile network connected");

#if TINY_GSM_USE_GPRS
    // GPRS connection parameters are usually set after network registration
    debugTrace("Event","Connecting to " + String(apn));
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) { errorLog(404,"Apn connection failed"); return false; };
    errorLog(405,"Apn connection success");

    if (!modem.isGprsConnected()) { errorLog(406,"GPRS NOT connected"); return false; };
    debugTrace("Event","GPRS connected");
#endif

    debugTrace("Event","Performing HTTPS GET request... ");
    if (Bearing_set() == false) { errorLog(407,"Bearing set fall"); return false; };
    if (Https_get() == false) { errorLog(408,"https get fall"); return false; };
    Close_serve();

#if TINY_GSM_USE_WIFI
    modem.networkDisconnect();
    debugTrace("Event","WiFi disconnected");
#endif
#if TINY_GSM_USE_GPRS
    modem.gprsDisconnect();
    debugTrace("Event","GPRS disconnected");
#endif
}

//Bearing set
bool Bearing_set() {
    modem.sendAT(GF("+HTTPTERM")); // Termine toute session HTTP active
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPTERM")); /*return false;*/ }

    modem.sendAT(GF("+SAPBR=0,1")); // Ferme le bearer identifié par l’ID 1.
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+SAPBR=0,1")); return false; }
    delay(1000);

    modem.sendAT(GF("+SAPBR=3,1,\"Contype\",\"GPRS\"")); // Configure le type de bearer en GPRS.
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+SAPBR=3,1,\"Contype\",\"GPRS\"")); return false; }

    modem.sendAT(GF("+SAPBR=1,1")); // Ouvre le bearer ID 1.
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+SAPBR=1,1")); /*return false;*/ }

    modem.sendAT(GF("+SAPBR=2,1")); // Lit les paramètres du bearer (vérifie si la connexion est active et affiche IP, etc.).
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+SAPBR=2,1")); /*return false;*/ }
    delay(2000);
    return true;
}

bool Https_get() {
    modem.sendAT(GF("+HTTPINIT"));// Initialize the HTTP service
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPINIT")); return false; };

    modem.sendAT(GF("+HTTPPARA=\"CID\",1"));//Set HTTP session parameters
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPPARA=\"CID\",1")); return false; };

   modem.sendAT(GF("+HTTPPARA=\"URL\",\"ouvrigo.ovh/test.php?aa=3\""));//Set HTTP session parameters
   if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPPARA=\"URL\",\"ouvrigo.ovh/test.php?aa=3\"")); return false; };

/*
    modem.sendAT(GF("+HTTPPARA=\"URL\",\"http://188.165.53.185/test.php?aa=2\""));//Set HTTP session parameters
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPPARA=\"URL\",\"http://188.165.53.185/test.php?aa=2\"")); return false; };

    modem.sendAT(GF("+HTTPPARA=\"USERDATA\",\"Host: ouvrigo.ovh\""));//Set HTTP session parameters
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPPARA=\"USERDATA\",\"Host: ouvrigo.ovh\"")); return false; };
*/
    /*
    modem.sendAT(GF("+HTTPPARA=\"URL\",\"www.baidu.com\""));//Set HTTP session parameters
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPPARA=\"URL\",\"www.baidu.com\"")); return false; };

    modem.sendAT(GF("+HTTPPARA=\"REDIR\",1"));//Set HTTP session parameters
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPPARA=\"REDIR\",1")); return false; };

    modem.sendAT(GF("+HTTPSSL=1")); //Enabling the HTTPS function
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPSSL=1")); return false; };
    */

    modem.sendAT(GF("+HTTPACTION=0")); //Get
    if (modem.waitResponse(60000L) != 1) { DBG(GF("+HTTPACTION=0")); return false; };
    delay(10000); // a vérifier pour plus rapide

    modem.sendAT(GF("+HTTPREAD")); //Read data from the HTTP server
    if (modem.waitResponse(60000L) != 1) { DBG(GF("+HTTPREAD")); return false; };

    debugTrace("Event","Http done !!!");
    return true;
}

bool Close_serve() {
    modem.sendAT(GF("+HTTPTERM")); //close https
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+HTTPTERM")); return false; };

    modem.sendAT(GF("+SAPBR=0,1")); //close GPRS
    if (modem.waitResponse(10000L) != 1) { DBG(GF("+SAPBR=0,1")); return false; }

    return true;
}

void modem4GTest() {
  debugTrace("TODO","modem4GTest not dev");
}

void modemWifiTest() {
  debugTrace("TODO","modemWifiTest not dev");
}

#endif // GPRS_HARDWARE