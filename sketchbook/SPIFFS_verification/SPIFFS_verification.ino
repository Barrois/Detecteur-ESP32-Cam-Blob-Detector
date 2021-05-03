#include "SPIFFS.h"

void listFilesInDir(File dir, int numTabs = 1);

void setup() {

  Serial.begin(112500);

  delay(500);

  Serial.println(F("Inizializing FS..."));
  if (SPIFFS.begin()){
      Serial.println(F("SPIFFS mounted correctly."));
  }else{
      Serial.println(F("!An error occurred during SPIFFS mounting"));
      //SPIFFS.begin(true); // pour formater SPIFFS
  }

  // Get all information of SPIFFS
  unsigned int totalBytes = SPIFFS.totalBytes();
  unsigned int usedBytes = SPIFFS.usedBytes();

  Serial.println("===== File system info =====");

  Serial.print("Total space:      ");
  Serial.print(totalBytes);
  Serial.println("byte");

  Serial.print("Total space used: ");
  Serial.print(usedBytes);
  Serial.println("byte");

  Serial.println();

  // Open dir folder
  File dir = SPIFFS.open("/");
  // List file at root
  listFilesInDir(dir);
}

void listFilesInDir(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files in the folder
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      listFilesInDir(entry, numTabs + 1);
    } else {
      // display zise for file, nothing for directory
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }

}

void loop() {

  if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  bool formatted = SPIFFS.format();
  if ( formatted ) {
    Serial.println("SPIFFS formatted successfully");
  } else {
    Serial.println("Error formatting");
  }

}
