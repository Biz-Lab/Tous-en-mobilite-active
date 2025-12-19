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

int smoothedDistance = 999;
int smoothedStationaryDistance = 999;
int smoothedMovingDistance = 999;
int radarStart = -1;
void radarLoop() {
  // Activation du radar uniquement après une actions utilisateur
  // if(radarIsNewUser) { return; };
  // Lecture des données du radar et lissage de la mesure pour éviter les faux positifs
  radar.read();
  uint16_t realTimeStationaryDistance = 300;
  if(radar.stationaryTargetDetected()) { realTimeStationaryDistance = radar.stationaryTargetDistance(); }
  smoothedStationaryDistance = floor(0.1 * realTimeStationaryDistance + 0.9 * smoothedStationaryDistance);
  uint16_t realTimeMovingDistance = 300;
  if(radar.movingTargetDetected()) { realTimeMovingDistance = radar.movingTargetDistance(); }
  smoothedMovingDistance = floor(0.1 * realTimeMovingDistance + 0.9 * smoothedMovingDistance);
  // Lissage de la mesure pour éviter les faux positifs
  smoothedDistance = min(smoothedStationaryDistance,smoothedMovingDistance); 
  // Détection de la présence d'une personne
  radarIsUserPresent = (smoothedDistance<RadarProximityEntryThreshold());
  // Réactivation de la possibilité d'un nouveau +1 après un délais minimum, si la personne sort de la zône radar ou après un délais maximum
  uint32_t delay = (millis()-radarStart)/1000;
  if(delay<newUserMinDelay()) { return; }
  if((smoothedDistance>RadarProximityExitThreshold())||(delay>newUserMaxDelay())) { 
    radarIsNewUser = true; 
    //debugTrace("Debug", "New user : " + String(smoothedDistance) + "cm after " + String(delay) + "s");  soundDebug(); 
} }

String radarDistanceGet() {
  return String(smoothedStationaryDistance) + " / " +  String(smoothedMovingDistance);
}

bool radarNewUser() { return radarIsNewUser; }
void radarDisableUser() { radarIsNewUser = false; radarStart = millis(); };
bool radarUserPresence() { return true; } //{ return radarIsUserPresent; }

// ** Fonction test ****************** 
int testStationaryDistance = 0;
int testMovingDistance = 0;
void radarTest() {
  uint32_t currentTime = millis();
  if(currentTime < radarNextRead) { return; }; 
  radarNextRead = currentTime+500;
  radar.read();
  if(!radar.isConnected()) { debugTrace("Test","Radar : Not connected"); displayShow2Lines("Radar","Ko",0); return; }
  if(!radar.presenceDetected()) { debugTrace("Test","Radar : No presence detected"); displayShow2Lines("Radar","Out 1",0); return; }
  int currentStationaryDistance = 999;
  int currentMovingDistance = 999;
  if(radar.stationaryTargetDetected()) { currentStationaryDistance = 10*floor(radar.stationaryTargetDistance()/10); }
  if(radar.movingTargetDetected()) { currentMovingDistance = 10*floor(radar.movingTargetDistance()/10); }
  if((currentStationaryDistance!=testStationaryDistance)||(currentMovingDistance!=testMovingDistance)) { 
    debugTrace("Test","Radar : "+String(currentStationaryDistance)+" / "+String(currentMovingDistance));
    displayShow2Lines(" " + String(currentStationaryDistance)," " + String(currentMovingDistance),0); 
    testStationaryDistance=currentStationaryDistance;
    testMovingDistance!=currentMovingDistance;
  } else {
    displayShow2Lines("Radar","Out 2",0);
  }
  delay(500);
}
