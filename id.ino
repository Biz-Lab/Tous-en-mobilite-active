//** Gestion de l'identifiant de l'appareil
#include <esp_efuse.h>
#include <esp_efuse_table.h>
#include <esp_hmac.h>
String idCacheData = "";
String hwVersionCacheData = "";
// Structure pour stocker les données dans l'eFuse
struct EfuseData {
    char productId[6];      // 5 caractères + '\0'
    char hwVersion[4];      // 3 caractères + '\0'
    uint8_t reserved[22];   // Padding pour 32 octets
};

/**
 * Obtient l'ID produit depuis l'eFuse
 * @return String de 5 caractères, ou "" si non défini
 */
String idGet() {
    if(idCacheData.length() == 5) { return idCacheData; };
    EfuseData data;
    memset(&data, 0, sizeof(EfuseData));
    esp_err_t result = esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA,(uint8_t*)&data,256);
    if (result != ESP_OK) { return ""; };
    // Vérifier si le premier caractère est valide
    if (data.productId[0] == 0x00 || data.productId[0] == 0xFF || data.productId[0] < 32 || data.productId[0] > 126) { return ""; }
    // Construire la string
    String productId = "";
    for (int i = 0; i < 5; i++) {
        if (data.productId[i] >= 32 && data.productId[i] <= 126) { productId += data.productId[i]; } else { break; }
    };
    idCacheData = productId;
    return productId;
}

/**
 * Obtient la version hardware depuis l'eFuse
 * @return String de 3 caractères, ou "" si non défini
 */
String hwVersionGet() {
    if(hwVersionCacheData.length() == 3) { return hwVersionCacheData; };
    EfuseData data;
    memset(&data, 0, sizeof(EfuseData));
    esp_err_t result = esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA,(uint8_t*)&data,256);
    if (result != ESP_OK) { return ""; }
    // Vérifier si le premier caractère est valide
    if (data.hwVersion[0] == 0x00 || data.hwVersion[0] == 0xFF || data.hwVersion[0] < 32 || data.hwVersion[0] > 126) { return ""; }
    // Construire la string
    String hwVersion = "";
    for (int i = 0; i < 3; i++) { 
        if (data.hwVersion[i] >= 32 && data.hwVersion[i] <= 126) { hwVersion += data.hwVersion[i]; } else { break; }
    }
    hwVersionCacheData = hwVersion;
    return hwVersion;
}

void factorySetup() {
  // Vérification si les paramètres usines sont déjà bien charger
  String efuseProductId = idGet();
  String efuseHwVersion = hwVersionGet();
  bool hmacTokenAvailable = !esp_efuse_key_block_unused(EFUSE_BLK_KEY1);
  if ((efuseProductId.length() == 5) && (efuseHwVersion.length() == 3) && hmacTokenAvailable) { debugTrace("Event","Factory Setup OK"); return; }
  debugTrace("Event","Factory Setup TO PERFORM");
  bool setupDone = true;
  // Vérification si le micro-controleur est bien en mode de configuration usine
  if(!isFactorySetupMode()) { return; }
  // Mise en place des codes wifi usine
  if(wifi_PWD()=="NotDefined") {
    storageWriteString(storageKeyWifiSsid, FACTORY_WIFI_SSID);
    storageWriteString(storageKeyWifiPwd, FACTORY_WIFI_PWD);
    reboot();
  }
  // Vérification de la connexion internet
  if(!isWebAvailable()) { return; }
  // Vérification que l'id produit et version hardware sont bien à charger
  if (efuseProductId.length() == 0) {
    // Recherche des données sur le serveur
    String productId = serverGetString("getProductId");
    if (productId.length() != 5) { errorLog(301,String(productId.length()) + " caractères : " + productId); setupDone=false; return; }
    idCacheData = productId;
    String hwVersion = serverGetString("getHwVersion");
    if (hwVersion.length() != 3) { errorLog(302,String(hwVersion.length()) + " caractères : " + hwVersion); setupDone=false; return; }
    // Préparation des données
    EfuseData data;
    memset(&data, 0, sizeof(EfuseData));
    productId.toCharArray(data.productId, 6);
    hwVersion.toCharArray(data.hwVersion, 4);
    esp_err_t result = esp_efuse_write_field_blob(ESP_EFUSE_USER_DATA, (uint8_t*)&data, 256);
    if (result == ESP_OK) { debugTrace("Event","Factory Setup - productId + hwVersion written"); } 
    else { errorLog(303,"ESP EFUSE write error"); setupDone=false; }
  }
  // Vérification si la clé hmac est bien chargée
  if(!hmacTokenAvailable) {
    String hmacString = serverGetString("getProductHmac");
    if (hmacString.length() != 64) { errorLog(304,String(hmacString.length()) + " caractères : " + hmacString); setupDone=false; return; }
    for (size_t i = 0; i < hmacString.length(); i++) {
      char c = hmacString.charAt(i);
      if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) { errorLog(305,"Not hex value : " + hmacString); setupDone=false; return; } 
    };
    uint8_t hmac_key1[32];
    for (size_t i = 0; i < 32; i++) {
      String byte_str = hmacString.substring(i * 2, i * 2 + 2);
      hmac_key1[i] = (uint8_t) strtol(byte_str.c_str(), NULL, 16);
    };
    esp_err_t result = esp_efuse_write_key(EFUSE_BLK_KEY1, ESP_EFUSE_KEY_PURPOSE_HMAC_UP, hmac_key1, 32);
    if (result == ESP_OK) { debugTrace("Event","Factory Setup - hmac written"); } 
    else { errorLog(306,"ESP EFUSE write error"); setupDone=false; }

    // Test de la clé
    String localHmac = hmacCompute("Hello World");
    String serverHmac = serverGetString("hmacCheck");
    if(localHmac == serverHmac) {
      debugTrace("Event","Factory Setup - hmac Ok");
    } else {
      errorLog(307,"Factory Setup - Erreur HMAC : " + localHmac + " vs " + serverHmac);
  } }
  if(setupDone) { setFactorySetupDone(); };
}

void idShow() {
  soundBip(); 
  displayShow2Lines("Login",idGet(),15);
}

String hmacCompute(String messageToHash) {
  uint8_t hmac[32];
  esp_err_t result = esp_hmac_calculate(HMAC_KEY1,messageToHash.c_str(),messageToHash.length(),hmac);
  if (result == ESP_OK) {
    String result = "";
    for (int i = 0; i < 32; i++) {
      if (hmac[i] < 0x10) { result += "0"; }
      result += String(hmac[i], HEX);
    }
    return result;
  } else {
    errorLog(308,"Erreur HMAC : "+String(esp_err_to_name(result)));
    return "";
  }
}

String smallHmacCompute(String messageToHash) {
  uint8_t hmac[32];
  esp_err_t result = esp_hmac_calculate(HMAC_KEY1,messageToHash.c_str(),messageToHash.length(),hmac);
  if (result == ESP_OK) {
    const char CHARSET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const int CHARSET_SIZE = 62;
    uint8_t output_length = 8;
    String result = "";
    result.reserve(output_length); 
    int bit_position = 0;
    for (int i = 0; i < output_length; i++) {
      int byte_index = bit_position / 8;
      int bit_offset = bit_position % 8;
      uint16_t bits;
      if (bit_offset <= 2) {
        bits = (hmac[byte_index] >> (2 - bit_offset)) & 0x3F;
      } else {
        bits = ((hmac[byte_index] << (bit_offset - 2)) | (hmac[byte_index + 1] >> (10 - bit_offset))) & 0x3F;
      }
      result += CHARSET[bits % CHARSET_SIZE];
      bit_position += 6;
    }
    return result;
  } else {
    errorLog(309,"Erreur HMAC : "+String(esp_err_to_name(result)));
    return "";
  }
}

void idPwdAction() {
  serverPostData("idPwdAction");
  soundBip(); 
  displayShow2Lines("Action","Ok",3);
}

// ** Fonction test ****************** 
#if MODE_DEBUG
void idTest() {
  displayShow2Lines("Login",idGet(),10);
}
#endif // MODE_DEBUG
