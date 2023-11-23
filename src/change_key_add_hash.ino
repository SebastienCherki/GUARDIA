#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D1 // D8
#define RST_PIN D0 // D4

MFRC522 mfrc522(SS_PIN, RST_PIN);


void setup() {
	Serial.begin(115200);
	SPI.begin();
	mfrc522.PCD_Init();
	Serial.println("Scan the RFID card to change the keys...");
  
}

void loop() {
  // CHECK IF NEW CARD
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Card detected!");

    // GET UID
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    Serial.print(F("UID: "));
    Serial.println(uid);

    // ADD HASH IN BLOCKS
    MFRC522::MIFARE_Key currentKey = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    char *hardHash = "kjdhfgquhnqvoivghut345768y\0";
    add_hash(&currentKey, hardHash);

    int cur = 1;
    // IN BLOCK 1 | 3 | 5
    while (cur < 6) {
      // CHANGE KEY A
      mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, cur, &currentKey, &(mfrc522.uid));
      MFRC522::MIFARE_Key newKeyA = {0x4F, 0x2E, 0x7A, 0x91, 0xC8, 0x3F};
      changeKey(&currentKey, &newKeyA, MFRC522::PICC_CMD_MF_AUTH_KEY_A, cur);
      cur += 2;
    }
    Serial.println("Keys A changed successfully!");

    cur = 3;
    // IN BLOCK 3 | 6 | 9
    while (cur < 13) {
      // CHANGE KEY B
      mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, cur, &currentKey, &(mfrc522.uid));
      MFRC522::MIFARE_Key newKeyB = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
      changeKey(&currentKey, &newKeyB, MFRC522::PICC_CMD_MF_AUTH_KEY_B, cur);
      cur += 3;
    }
    Serial.println("Keys B changed successfully!");

    // Halt PICC (put the card into sleep mode | signal that the interaction is complete)
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();

    delay(1000);
    Serial.println("Scan the RFID card to change the keys...");
  }
}

void changeKey(MFRC522::MIFARE_Key *currentKey, MFRC522::MIFARE_Key *newKey, byte authKeyType, int cur) {
  for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
    mfrc522.PCD_WriteRegister(MFRC522::SectorTrailBlock, i, *newKey[i]);
    currentKey[i] = newKey[i];
	}
  mfrc522.PCD_Authenticate(authKeyType, cur, currentKey, &(mfrc522.uid));
}

void add_hash(MFRC522::MIFARE_Key *key, char *hash) {
  int i = 0;
  byte  block = 1;
  MFRC522::StatusCode status;

  while (hash[i] && block <= 15) {
    mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    status = mfrc522.MIFARE_Write(block, (byte *)&hash[i], 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
    i += 16;
    block++;
  }
}