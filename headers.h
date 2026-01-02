// Déclaration des Pins du microcontroller
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

// Clés de stockages
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

// storage.ino
void storageOpen();
void storageClose();
void storageInit();
bool storageReadBool(const char *key, bool defaultValue = false);
bool storageWriteBool(const char *key, bool value);
bool webStorageReadBool(const char *key, bool defaultValue);
bool webStorageWriteBool(const char *key, bool value, String additionalData = "");
uint16_t storageReadUShort(const char *key, uint16_t defaultValue = 0);
bool storageWriteUShort(const char *key, uint16_t value);
uint16_t webStorageReadUShort(const char *key, uint16_t defaultValue = 0);
bool webStorageWriteUShort(const char *key, uint16_t value, String additionalData = "");
uint32_t storageReadUInt(const char *key, uint32_t defaultValue = 0);
bool storageWriteUInt(const char *key, uint32_t value);
uint32_t webStorageReadUInt(const char *key, uint32_t defaultValue = 0);
bool webStorageWriteUInt(const char *key, uint32_t value, String additionalData = "");
String storageReadString(const char *key, String defaultValue = "");
bool storageWriteString(const char *key, String value);
String webStorageReadString(const char *key, String defaultValue = "");
bool webStorageWriteString(const char *key, String value, String additionalData = "");
bool storageRemove(const char *key);

// id.ino
String idGet();
void idShow();
void idPwdAction();
String hwVersionGet();
String smallHmacCompute(String messageToHash);
void factorySetup();

// counters.ino
void countersInit();
void countersSave();
void countersInc();
void countersRefuseInc();
void countersPrint();
void counterTestValues(int16_t valuesCount);

// display.ino
void displayShowMonoLine(String text, int duration=0);
void displayShow2Lines(String line1, String line2, int duration);
void displayTurnOff();
void displayTurnOn();
void displaySetIntensity(uint8_t intensity);

// displayControl.ino
class LedControl {
  private :
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
      LedControl(int dataPin, int clkPin, int csPin1, int csPin2, int numDevices=1);
      int getDeviceCount();
      void shutdown(int addr, bool status);
      void setScanLimit(int addr, int limit);
      void setIntensity(int addr, int intensity);
      void clearDisplay(int addr);
      void setLed(int addr, int row, int col, boolean state);
      void setRow(int addr, int row, byte value);
      void setColumn(int addr, int col, byte value);
};

// powerSaving.ino
#define powerSavingAttenuationTime 180 // 180 secondes = 3 minutes
#define powerSavingIdleTime 900 // 900 secondes = 15 minutes
void powerSavingOnIdle(uint32_t idleSeconds);

// radar.ino
#define RadarProximityExitThreshold 160 // cm
#define newUserMinDelay 3 // secondes
#define newUserMaxDelay 30 // secondes
void radarInit();
void radarLoop();
String radarDistanceGet();
bool radarNewUser();
void radarDisableUser();
bool radarUserPresence();

// sound.ino 
void soundOk(bool forceSoundActif = false);
void soundKo(bool forceSoundActif = false);
void soundBip(bool forceSoundActif = false);
void soundConfigToggle();

// display.ino Structure pour définir une police
struct FontDefStruct {
  int height;
  int space;
  int charCount;
  const void* fontData;  // Pointeur générique vers les données de la police
};

// web.ino
String serverGetString(String dataKey);
int32_t serverGetInt(String dataKey, uint32_t defaultValue=-1);
String serverPostData(String action, String data = "", bool getServerReturn = false);

// time.ino
void timeInit();
void timeSelfMaintenance();
uint32_t time();
bool isTimeReliable();
