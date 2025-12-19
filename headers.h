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
#define storageKeyDebugMode "k1"
#define storageKeySoundActif "k2"
#define storageKeyRadarProximityEntryThreshold "kL"
#define storageKeyRadarProximityExitThreshold "k3"
#define storageKeyNewUserMinDelay "k4"
#define storageKeyNewUserMaxDelay "k5"
#define storageKeyWifiReset "k7"
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
#define storageKeyStorageToReset "kK"
#define storageKeyCurrentFirmwareVersion "kS"

// debug.ino
//void IRAM_ATTR debugTestModeToggle();

// storage.ino
void storageOpen();
void storageClose();
void storageInit();
bool storageReadBool(const char *key, bool defaultValue = false);
bool storageWriteBool(const char *key, bool value);
uint16_t storageReadUShort(const char *key, uint16_t defaultValue = 0);
bool storageWriteUShort(const char *key, uint16_t value);
uint32_t storageReadUInt(const char *key, uint32_t defaultValue = 0);
bool storageWriteUInt(const char *key, uint32_t value);
String storageReadString(const char *key, String defaultValue = "");
bool storageWriteString(const char *key, String value);
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
bool webStorageReadBool(const char *key, bool defaultValue);
bool webStorageWriteBool(const char *key, bool value);
uint16_t webStorageReadUShort(const char *key, uint16_t defaultValue);
bool webStorageWriteUShort(const char *key, uint16_t value);
String serverGetString(String dataKey);
int32_t serverGetInt(String dataKey, uint32_t defaultValue=-1);
bool serverPostData(String action, String data = "");

// time.ino
uint32_t time();
bool isTimeReliable();
