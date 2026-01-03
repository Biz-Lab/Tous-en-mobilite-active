//** Gestion du radar de présence
#define RADAR_SERIAL Serial1
#include <ld2410.h>
ld2410 radar;

uint32_t radarNextRead = 0;
bool radarClosePresenceDetected = false;
bool radarWidePresenceDetected = false;
bool radarIsNewUser = true;
bool radarIsUserPresent = false;

void radarInit() {
  RADAR_SERIAL.begin(256000, SERIAL_8N1, radarRxPin, radarTxPin); //UART for monitoring the radar
  delay(1000);
  radar.begin(RADAR_SERIAL,true);
  delay(1000);
  radar.read();
  if(!radar.isConnected()) { errorLog(601,"Radar issue"); };
  debugTrace("Event","Radar initialisation OK");
  watchDogReset();
}

int smoothedDistance = 300;
int smoothedStationaryDistance = 300;
int smoothedMovingDistance = 300;
int previousSmoothedStationaryDistance = 300;
int previousSmoothedMovingDistance = 300;
uint32_t radarLatestPresenceDetected = 0;
uint32_t radarLatestUserActionDetected = 0;
uint32_t radarPresenceIdleSeconds = 0;

#define smoothedRatio 0.05
void radarLoop() {
  // Lecture des données du radar et lissage de la mesure pour éviter les faux positifs
  radar.read();
  uint16_t realTimeStationaryDistance = 300;
  if(radar.stationaryTargetDetected()) { realTimeStationaryDistance = radar.stationaryTargetDistance(); }
  smoothedStationaryDistance = floor(smoothedRatio * realTimeStationaryDistance + (1 - smoothedRatio) * smoothedStationaryDistance);
  uint16_t realTimeMovingDistance = 300;
  if(radar.movingTargetDetected()) { realTimeMovingDistance = radar.movingTargetDistance(); }
  smoothedMovingDistance = floor(smoothedRatio * realTimeMovingDistance + (1 - smoothedRatio) * smoothedMovingDistance);
  // Lissage de la mesure pour éviter les faux positifs
  smoothedDistance = min(smoothedStationaryDistance,smoothedMovingDistance); 
  // Détection de la présence d'une personne
  if((abs(previousSmoothedStationaryDistance - smoothedStationaryDistance) >= 10) || (abs(previousSmoothedMovingDistance - smoothedMovingDistance) >= 10)) {
    radarIsUserPresent = true;
    previousSmoothedStationaryDistance = smoothedStationaryDistance;
    previousSmoothedMovingDistance = smoothedMovingDistance;
    radarLatestPresenceDetected = millis();
    powerSavingOnIdle(0);
  } else {
    radarPresenceIdleSeconds = (int)((millis() - radarLatestPresenceDetected)/1000.0);
    powerSavingOnIdle(radarPresenceIdleSeconds);
  }
  // Réactivation de la possibilité d'un nouveau +1 après un délais minimum, si la personne sort de la zône radar ou après un délais maximum
  uint32_t delaySeconds = (millis()-radarLatestUserActionDetected)/1000;
  if(delaySeconds < newUserMinDelay) { return; }
  if((smoothedDistance > RadarProximityExitThreshold)||(delaySeconds > newUserMaxDelay)) { 
    radarIsNewUser = true; 
} }

String radarDistanceGet() {
  return String(previousSmoothedStationaryDistance) + " / " +  String(previousSmoothedMovingDistance);
}

bool radarNewUser() { return radarIsNewUser; }
void radarDisableUser() { radarIsNewUser = false; radarLatestUserActionDetected = millis(); }
bool radarUserPresence() { return (radarPresenceIdleSeconds < 300); } 

// ** Fonction test ****************** 
#if MODE_DEBUG
void radarTest() {
  displayShow2Lines(String(previousSmoothedStationaryDistance),String(previousSmoothedMovingDistance) + " " + String(radarPresenceIdleSeconds),60);
}
#endif // MODE_DEBUG