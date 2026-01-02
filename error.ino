//** Gestion des erreurs

#define MAX_ERROR_QUEUE 100
#define MAX_ERROR_LENGTH 128

// Structure pour la pile d'erreurs
struct ErrorQueue {
  String errors[MAX_ERROR_QUEUE];
  int head;  // Position d'écriture
  int tail;  // Position de lecture
  int count; // Nombre d'éléments
} errorQueue = {{}, 0, 0, 0};

// Fonction de traçage d'erreur évoluée
void errorLog(int errorId, String errorDescription) {
  debugTrace("Error " + String(errorId), errorDescription);
  
  // Vérifier si la pile n'est pas pleine
  if (errorQueue.count >= MAX_ERROR_QUEUE) {
    debugTrace("Error","ATTENTION: Pile d'erreurs pleine, erreur ignorée!");
    return;
  }
  
  // Limiter la longueur du message
  errorDescription.replace("\"", " ");
  errorDescription = "Error " + String(errorId) + " : " + errorDescription;
  if (errorDescription.length() > MAX_ERROR_LENGTH) { errorDescription = errorDescription.substring(0, MAX_ERROR_LENGTH); }
  
  // Ajouter l'erreur à la pile
  errorQueue.errors[errorQueue.head] = errorDescription;
  errorQueue.head = (errorQueue.head + 1) % MAX_ERROR_QUEUE;
  errorQueue.count++;
}

uint32_t notSendErrorBefore = 0;
// Fonction à appeler régulièrement dans loop()
void errorQueueProcess() {
  // Vérifier s'il y a des erreurs à traiter
  if (errorQueue.count == 0) { return; }
  if (time()<notSendErrorBefore) { return; }

  // Récupérer l'erreur la plus ancienne
  String errorToSend = errorQueue.errors[errorQueue.tail];
  
  // Tenter d'envoyer au serveur  
  if (serverPostData("errorLog", "value='" + errorToSend + "'")=="Ok") {
    errorQueue.errors[errorQueue.tail] = "";
    errorQueue.tail = (errorQueue.tail + 1) % MAX_ERROR_QUEUE;
    errorQueue.count--;
    debugTrace("Event","Erreur envoyée au serveur. Restant: " + String(errorQueue.count));
  } else {
    debugTrace("Error","Échec d'envoi au serveur. Nouvelle tentative plus tard.");
    notSendErrorBefore = time()+30;
  }
}
