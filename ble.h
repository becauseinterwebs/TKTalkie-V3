/****
 * Routines for BLE 
 */

/**
 * Send output to BLE
 */
void btprint(const char *str) {
  if (ECHO == true) {
    Serial.print("TX: ");
    Serial.println(str);
  }
  if (BT_CONNECTED == true) {
    Serial1.print(str);
  }
}

/**
 * Send output to BLE
 */
void btprintln(const char *str) {
  if (ECHO == true) {
    Serial.print("TX: ");
    Serial.println(str);
  }
  if (BT_CONNECTED == true) {
    Serial1.print(str);
    Serial1.print("\n");
  }
}

void btprint(const __FlashStringHelper *fmt, ... ) {
  char buf[1025]; // resulting string limited to 1M chars
  va_list args;
  va_start (args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);
  if (ECHO == true) {
    Serial.print(buf);
  }
  Serial1.print(buf);
}

/**
 * Shortcut to send output to in JSON format
 */
void sendToApp(const char *cmd, const char *value) 
{
  if (value[0] == '[') {
    btprint(F("{\"cmd\":\"%s\",\"data\":%s}\n"), cmd, value);
  } else {
    btprint(F("{\"cmd\":\"%s\",\"data\":\"%s\"}\n"), cmd, value);
  }
}

/**
 * Sends config via Bluetooth Serial.  Used for TKTalkie App
 */
void sendConfig() 
{

  debug(F("Sending config\n"));

  btprint(F("{\"cmd\":\"config\", \"data\": { \"ver\":\"%s\","), VERSION);

  // get sound files
  char files[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
  
  int count = listFiles(SOUNDS_DIR, files, MAX_FILE_COUNT, SOUND_EXT, false, false);

  // Add sound files
  char buffer[1024];
  char *sounds = arrayToStringJson(buffer, files, count);
  btprint(F("\"sounds\":%s,"), sounds);
  memset(buffer, 0, sizeof(buffer));

  // Clear array
  memset(files, 0, sizeof(files));

  // Add effects 
  char *effects = arrayToStringJson(buffer, SOUND_EFFECTS, SOUND_EFFECTS_COUNT);
  btprint(F("\"effects\":%s,"), effects);
  memset(buffer, 0, sizeof(buffer));

  // get loop files 
  count = listFiles(LOOP_DIR, files, MAX_FILE_COUNT, SOUND_EXT, false, false);
  
  // Add loops 
  char *loops = arrayToStringJson(buffer, files, count);
  btprint(F("\"loops\":%s,"), loops);
  memset(buffer, 0, sizeof(buffer));

  // Clear array
  memset(files, 0, sizeof(files));

  char *profile = getSettingValue(buffer, "profile");
  btprint(F("\"default\":\"%s\","), profile);
  memset(buffer, 0, sizeof(buffer));
  
  btprint(F("\"current\": {\"name\":\"%s\",\"desc\":\"%s\"},"), PROFILE_FILE, PROFILE_NAME);

  // This is already formatted correctly
  char *json = settingsToJson(buffer);
  btprint(F("%s,"), json);
  memset(buffer, 0, sizeof(buffer));

  btprint(F("\"profiles\":["));
  
  // get config profile files 
  count = listFiles(PROFILES_DIR, files, MAX_FILE_COUNT, FILE_EXT, false, false);
  
  for (int i = 0; i < count; i++) {
     char entries[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
     char filename[SETTING_ENTRY_MAX];
     strcpy(filename, PROFILES_DIR);
     strcat(filename, files[i]);
     int total = loadSettingsFile(filename, entries, MAX_FILE_COUNT);
     for (int x = 0; x < total; x++) {
        char *key, *value, *ptr;
        char entry[SETTING_ENTRY_MAX];
        strcpy(entry, entries[x]);
        key = strtok_r(entry, "=", &ptr);
        value = strtok_r(NULL, "=", &ptr);
        if (strcasecmp(key, "name") == 0) {
           btprint(F("{\"name\":\"%s\",\"desc\":\"%s\"}"), files[i], value);
           if (i < count-1) {
             btprint(F(","));
           }
           break;
        }
     }
  }
  
  btprint(F("],\"mute\":%s,"), MUTED == true ? "1" : "0");

  btprint(F("\"bg\":%s,"), loopPlayer.isPlaying() ? "1" : "0");

  // list directories on the card
  memset(buffer, 0, sizeof(buffer));
  count = listDirectories("/", files);
  char *dirs = arrayToStringJson(buffer, files, count);
  btprint(F("\"ls\":%s"), dirs);
            
  // end
  btprint("}}\n");
}

