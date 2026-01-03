// ============================================================================
// DÉCLARATION DES PINS DU MICROCONTRÔLEUR
// ============================================================================
#define btnWifiSetup 33
#define btnIdPwd 34
#define btnSoundToggle 35
#define btnDailyPin 10
#define btnWeeklyPin 11
#define btnMonthlyPin 13
#define btnPlusOnePin 5
#define displayClkPin 42
#define displayDataInPin 8
#define displayLoadPin1 39
#define displayLoadPin2 9
#define ledDailyPin 41
#define ledWeeklyPin 40
#define ledMonthlyPin 38
#define radarRxPin 7
#define radarTxPin 6
#define soundPin 37

// ============================================================================
// CLÉS DE STOCKAGE NON-VOLATILE
// ============================================================================
#define storageKeySoundActif "k2"
#define storageKeyWifiSsid "k8"
#define storageKeyWifiPwd "k9"
#define storageKeyDailyCounter "kA"
#define storageKeyWeeklyCounter "kB"
#define storageKeyMonthlyCounter "kC"
#define storageKeyPushDataId "kD"
#define storageKeyPushData "kE"
#define storageKeyTimeStampOffset "kF"
#define storageKeyTestArea "kG"
#define storageKeyCounterUnit "kH"
#define storageKeyFactorySetupMode "kI"
#define storageKeyStorageUsedEntries "kN"
#define storageKeyStorageFreeEntries "kO"
#define storageKeyStorageTotalEntries "kP"
#define storageKeyStorageNamespaceCount "kQ"
#define storageKeyCurrentFirmwareVersion "kS"


// ============================================================================
// STORAGE.INO - Gestion du stockage non-volatile
// ============================================================================
void storageOpen();
void storageClose();
void storageInit();
void storageCompact();
void storageSelfMaintenance();
void storageReset();
void storageFullReset();

// Lecture/Écriture Bool
bool storageReadBool(const char *key, bool defaultValue = false);
bool storageWriteBool(const char *key, bool value);
bool webStorageReadBool(const char *key, bool defaultValue = false);
bool webStorageWriteBool(const char *key, bool value, String additionalData = "");

// Lecture/Écriture UShort
uint16_t storageReadUShort(const char *key, uint16_t defaultValue = 0);
bool storageWriteUShort(const char *key, uint16_t value);
uint16_t webStorageReadUShort(const char *key, uint16_t defaultValue = 0);
bool webStorageWriteUShort(const char *key, uint16_t value, String additionalData = "");

// Lecture/Écriture UInt
uint32_t storageReadUInt(const char *key, uint32_t defaultValue = 0);
bool storageWriteUInt(const char *key, uint32_t value);
uint32_t webStorageReadUInt(const char *key, uint32_t defaultValue = 0);
bool webStorageWriteUInt(const char *key, uint32_t value, String additionalData = "");

// Lecture/Écriture String
String storageReadString(const char *key, String defaultValue = "");
bool storageWriteString(const char *key, String value);
String webStorageReadString(const char *key, String defaultValue = "");
bool webStorageWriteString(const char *key, String value, String additionalData = "");

// Suppression
bool storageRemove(const char *key);

#if MODE_DEBUG
void storageTest();
#endif

// ============================================================================
// CONFIG.INO - Gestion de la configuration
// ============================================================================
void configInit();
bool isFactorySetupMode();
void setFactorySetupDone();
bool isSoundActif();
void soundSetActif();
void soundSetInactif();
bool isWebAvailable();
bool isWifiAvailable();
void setWifiAvailable();
void setWifiNotAvailable();
const char* wifi_SSID();
const char* wifi_PWD();

// ============================================================================
// ID.INO - Gestion de l'identifiant de l'appareil
// ============================================================================
String idGet();
String hwVersionGet();
void idShow();
void idPwdAction();
void factorySetup();
String hmacCompute(String messageToHash);
String smallHmacCompute(String messageToHash);

#if MODE_DEBUG
void idTest();
#endif

// ============================================================================
// COUNTERS.INO - Gestion des compteurs de mobilité
// ============================================================================
uint16_t dailyCounter;
uint16_t weeklyCounter;
uint16_t monthlyCounter;
void countersInit();
void countersInc();
void countersRefuseInc();
void countersUpdate();
void countersAutoUpdate();
void countersSave();
void setDailyMode();
void setWeeklyMode();
void setMonthlyMode();
int counterToDisplay();

#if MODE_DEBUG
void counterTestValues(int16_t valuesCount);
void countersResetSlots();
void counterDetailsDisplay();
#endif

// ============================================================================
// DISPLAY.INO - Gestion de l'affichage LED
// ============================================================================
void displayInit();
void displayShowMonoLine(String text, int duration = 0);
void displayShow2Lines(String line1, String line2, int duration = 0);
void displayCounter();
void displayPlusOne();
void displayAlreadyDone();
void displayTurnOff();
void displayTurnOn();
void displaySetIntensity(uint8_t intensity);

// Structure pour définir une police
struct FontDefStruct {
  int height;
  int space;
  int charCount;
  const void* fontData;
};

#if MODE_DEBUG
void displayTest();
void displaytest99();
#endif

// ============================================================================
// DISPLAYCONTROL.INO - Contrôle bas niveau des matrices LED
// ============================================================================
class LedControl {
  private:
    byte spidata[16];
    void spiTransfer(int addr, byte opcode, byte data);
    void myShiftOut(int dataPin, int clockPin, int bitOrder, int val);
    byte status[64];
    int SPI_MOSI;
    int SPI_CLK;
    int SPI_CS1;
    int SPI_CS2;
    int maxDevices;

  public:
    LedControl(int dataPin, int clkPin, int csPin1, int csPin2, int numDevices = 1);
    int getDeviceCount();
    void shutdown(int addr, bool status);
    void setScanLimit(int addr, int limit);
    void setIntensity(int addr, int intensity);
    void clearDisplay(int addr);
    void setLed(int addr, int row, int col, boolean state);
    void setRow(int addr, int row, byte value);
    void setColumn(int addr, int col, byte value);
};

// ============================================================================
// BTN.INO - Gestion des zones tactiles et boutons
// ============================================================================
void btnInit();
void btnLoop();

#if MODE_DEBUG
void btnTest();
#endif

// ============================================================================
// RADAR.INO - Gestion du radar de présence
// ============================================================================
#define RadarProximityExitThreshold 160  // cm
#define newUserMinDelay 3                // secondes
#define newUserMaxDelay 30               // secondes

void radarInit();
void radarLoop();
String radarDistanceGet();
bool radarNewUser();
void radarDisableUser();
bool radarUserPresence();

#if MODE_DEBUG
void radarTest();
#endif

// ============================================================================
// SOUND.INO - Gestion des sons
// ============================================================================
void soundOk(bool forceSoundActif = false);
void soundKo(bool forceSoundActif = false);
void soundBip(bool forceSoundActif = false);
void soundDebug();
void soundConfigToggle();
void soundPlay(int size, const int* notes, const int* durations, bool forceSoundActif);

#if MODE_DEBUG
void soundTest();
#endif

// ============================================================================
// WIFI.INO - Gestion de la connexion WiFi
// ============================================================================
bool wifiInit();
bool wifiCheck();
void wifiOff();
void wifiSetup();
void startConfigPortal();
void stopConfigPortal();
void handleRoot();
void handleNetworks();
void handleScan();
void handleSave();
void handleResult();
String scanWiFiNetworks();
String wifiStatusToString(int status);

// ============================================================================
// WEB.INO - Gestion des requêtes serveur
// ============================================================================
String serverGetString(String dataKey);
int32_t serverGetInt(String dataKey, uint32_t defaultValue = -1);
String serverPostData(String action, String data = "", bool getServerReturn = false);
String serverQuery(String postData);

#if MODE_DEBUG
void webApiTest();
#endif

// ============================================================================
// TIME.INO - Gestion de l'horloge
// ============================================================================
void timeInit();
void timeSelfMaintenance();
void timeStore();
uint32_t time();
bool isTimeReliable();
uint32_t timeFromModem();
uint32_t timeFromWifi();
void setLocalTimeFromTimestamp(uint32_t timestamp);
String timeFormat();

#if MODE_DEBUG
void timeTest();
#endif

// ============================================================================
// ERROR.INO - Gestion des erreurs
// ============================================================================
void errorLog(int errorId, String errorDescription);
void errorQueueProcess();

// ============================================================================
// WATCHDOG.INO - Gestion du WatchDog
// ============================================================================
void watchDogInit();
void watchDogReset();
void watchDogStop();
void watchDogAutoReboot();
void reboot();
void lifeSignalToServer();

// ============================================================================
// POWERSAVING.INO - Gestion de l'économie d'énergie
// ============================================================================
#define powerSavingAttenuationTime 180  // 180 secondes = 3 minutes
#define powerSavingIdleTime 900         // 900 secondes = 15 minutes

void powerSavingInit();
void powerSavingOnIdle(uint32_t idleSeconds);
void disableBluetooth();

// ============================================================================
// OTA.INO - Gestion des mises à jour logiciel
// ============================================================================
void otaInit();
void otaUpdatePerform();

// ============================================================================
// DEBUG.INO - Gestion du mode debug
// ============================================================================
void debugInit();
void debugTrace(String debugTraceType, String debugTraceDescription);

#if MODE_DEBUG
void debugTestModeEnable();
bool debugTestMode();
#endif

// ============================================================================
// MODEM.INO - Gestion de la connexion 4G (si GPRS_HARDWARE activé)
// ============================================================================
#if GPRS_HARDWARE
void modemInit();
bool modemEx();
bool Bearing_set();
bool Https_get();
bool Close_serve();
void modem4GTest();
void modemWifiTest();
#endif
