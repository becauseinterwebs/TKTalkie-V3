/****
 * TK TALKIE by TK-81113 (Brent Williams) <becauseinterwebs@gmail.com>
 * www.tktalkie.com / www.tk81113.com 
 * 
 * Version 3.1 (June 4, 2017)
 *
 * WhiteArmor.net User ID: lerxstrulz
 * 
 * This sketch is meant to use a Teensy 3.2 with a Teensy Audio Shield and 
 * reads sounds from an SD card and plays them after the user stops talking 
 * to simulate com=m noise such as clicks and static. This version adds a lot  
 * of new features, including the ability to be controlled via a mobile app 
 * and Bluetooth.  This release also introduces memory optimizations and other 
 * improvements.
 * 
 * You are free to use this code in your own projects, provided they are for 
 * personal, non-commercial use.
 * 
 * The audio components and connections were made using the GUI tool 
 * available at http://www.pjrc.com/teensy/gui.  If you want to modify 
 * the signal path, add more effects, etc., you can copy and paste the 
 * code marked by 'GUITool' into the online editor at the above URL.
 * 
 * WHAT'S NEW:
 * 
 * V3.1 (5/30/2017)
 *  1.  Added new setting to the SETTINGS.TXT control file to alert the 
 *      mobile app which input/volume controls should be presented.
 *      Valid values are:
 *        MIC  = Show the microphone jack controls only
 *        LINE = Show the line-in controls only 
 *        BOTH = Show mic/line-in controls and input switcher
 *      By default this is set to BOTH for backwards compatibility.  As of 
 *      version 3.1 of the TKTalkie hardware, inputs are wired to mic only
 *      as it will support both wired and wireless microphones.
 *      
 *  2.  Added new settings to the SETTINGS.TXT control file to alert the     
 *      mobile app which output/volume controls should be presented.
 *      Valid values are:
 *         SPKR = Show only headphones/speaker controls 
 *         LINE = Show only line-out controls
 *         BOTH = Show both
 *      By default this is set to BOTH for backwards compatibility
 *      
 *  3.  Add new option (all versions) to allow background loop to continue 
 *      playing while talking (loop is paused by default)
 *      
 *  4.  Fixed bug that prevented PTT button setting up properly.
 *  
 * V3.0 (4/10/2017)
 *  1.  Modified to be able to communicate via Bluetooth Low Energy (BLE) 
 *      serial adapter with mobile app for control of settings and profile 
 *      switching.
 *  2.  Began optimizations on usage of C-type strings to reduce memory usage.
 *  3.  Added new commands specific to BLE adapter usage.
 *  4.  Added new commands to manage multiple configuration profiles.
 *  5.  Reorganized SD card files and folders
 *  
 *  Please visit http://www.tktalkie.com/changelog for prior changes.
 */
 
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce2.h>
#include "globals.h"
#include "utils.h"
#include "files.h"
#include "sounds.h"
#include "calibrate.h"
#include "settings.h"
#include "ble.h"

/*** 
 * Check if the PTT button was pressed 
 */
boolean checkButton() 
{
  PTT.update();
  if (PTT.fell()) {
    playEffect(BUTTON_WAV);
    return true;
  } else {
    return false;
  }
}

/**
 * Read the SETTINGS.TXT file
 */
void startup() 
{

  // make sure we have a profile to load
  memset(PROFILE_FILE, 0, sizeof(PROFILE_FILE));

  // Load all entries in PROFILES.TXT file.
  // The file to load will be in the format default=filename.
  // All other entries are in the format profile=filename.
  int total = loadSettingsFile(SETTINGS_FILE, STARTUP_SETTINGS, STARTUP_SETTINGS_COUNT);

  char buffer[SETTING_ENTRY_MAX];
  char *buf;
  
  if (total > 0) {

    buf = getSettingValue(buffer, "debug");
    DEBUG = (strcasecmp(buf, "1") == 0) ? true : false;
    debug(F("Got startup value DEBUG: %s\n"), (DEBUG == true ? "true" : "false"));
    memset(buffer, 0, sizeof(buffer));
    buf[0] = '\0';
    
    buf = getSettingValue(buffer, "profile");
    strcpy(PROFILE_FILE, buf);
    debug(F("Got startup value PROFILE: %s\n"), PROFILE_FILE);
    memset(buffer, 0, sizeof(buffer));
    buf[0] = '\0';
    
    buf = getSettingValue(buffer, "echo");
    ECHO = (strcasecmp(buf, "1") == 0) ? true : false;
    debug(F("Got startup  value ECHO: %s\n"), (ECHO == true ? "true" : "false"));
    memset(buffer, 0, sizeof(buffer));
    buf[0] = '\0';
    
    buf = getSettingValue(buffer, "input");
    strcpy(INPUT_TYPE, buf);
    if (strcasecmp(INPUT_TYPE, "") == 0) {
      strcpy(INPUT_TYPE, "BOTH");
    }
    debug(F("Got startup value INPUT TYPE: %s\n"), INPUT_TYPE);
    memset(buffer, 0, sizeof(buffer));
    buf[0] = '\0';

    buf = getSettingValue(buffer, "output");
    strcpy(OUTPUT_TYPE, buf);
    if (strcasecmp(OUTPUT_TYPE, "") == 0) {
      strcpy(OUTPUT_TYPE, "BOTH");
    }
    debug(F("Got startup value OUTPUT TYPE: %s\n"), OUTPUT_TYPE);
    memset(buffer, 0, sizeof(buffer));
    buf[0] = '\0';
    
  }

  Serial.println(F("\n----------------------------------------------"));
  Serial.println(F("TKTalkie v3.0"));
  Serial.println(F("(c) 2017 TK81113/Because...Interwebs!\nwww.TKTalkie.com"));
  Serial.print(F("Debugging is "));
  Serial.println(DEBUG == true ? "ON" : "OFF");
  if (DEBUG == false) {
    Serial.println(F("Type debug=1 [ENTER] to enable debug messages"));
  } else {
    Serial.println(F("Type debug=0 [ENTER] to disable debug messages"));
  }
  Serial.println(F("----------------------------------------------\n"));
  
  if (strcasecmp(PROFILE_FILE, "") == 0) {
    // No profile specified, try to find one and load it
    char files[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
    total = listFiles(PROFILES_DIR, files, MAX_FILE_COUNT, FILE_EXT, false, false);
    if (total > 0) {
      memset(PROFILE_FILE, 0, sizeof(PROFILE_FILE));
      strcpy(PROFILE_FILE, files[0]);
      // If no startup profiles were found, set default and save
      if (total < 1) {
        setSettingValue("profile", PROFILE_FILE);
        saveSettings();
      }
    } else {
      debug(F("NO PROFILES LISTED\n"));
    }
  }

  if (strcasecmp(PROFILE_FILE, "") == 0) {
    debug(F("NO PROFILE FILE FOUND!\n"));
    return;
  }

  debug(F("PROFILE: %s\n"), PROFILE_FILE);

  buf[0] = '\0';
  memset(buffer, 0, sizeof(buffer));
  buf = getSettingValue(buffer, "access_code");
  debug(F("Read access code %s\n"), buf);
  if (strcasecmp(buf, "") != 0) {
    memset(ACCESS_CODE, 0, sizeof(ACCESS_CODE));
    strcpy(ACCESS_CODE, buf);
  }
  
  char profile_settings[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
  
  // Load settings from specified file
  char path[100];
  strcpy(path, PROFILES_DIR);
  strcat(path, PROFILE_FILE);
  total = loadSettingsFile(path, profile_settings, MAX_FILE_COUNT);

  // Parse all of the settings
  processSettings(profile_settings, total);

  // apply the settings so we can do stuff
  applySettings();

  // set the volume, either by config or volume pot
  readVolume();

  // turn on outputs
  audioShield.unmuteLineout();
  audioShield.unmuteHeadphone();

  // play startup sound
  long l = playSound(STARTUP_WAV);

  // add a smidge of delay ;)
  delay(l+100); 

  // play background loop
  playLoop();

  STATE = STATE_RUNNING;

}

/***
 * Initial setup...runs only once when the board is turned on.
 */
void setup() 
{
  STATE = STATE_BOOTING;
  
  // You really only need the Serial connection 
  // for output while you are developing, so you 
  // can uncomment this and use Serial.println()
  // to write messages to the console.
  Serial.begin(57600);
  
  Serial1.begin(9600);
  
  delay(500);

  // Always allocate memory for the audio shield!
  AudioMemory(16);
  
  // turn on audio shield
  audioShield.enable();
  
  // disable volume and outputs during setup to avoid pops
  audioShield.muteLineout();
  audioShield.muteHeadphone();
  audioShield.volume(0);

  // turn on post processing
  audioShield.audioPostProcessorEnable();

  // Initialize sound generator for warning tones when we switch modes from PTT back to Voice Activated
  waveform1.begin(WAVEFORM_SINE);
  
  // Check SD card
  SPI.setMOSI(7);  // Set to 7 for Teensy
  SPI.setSCK(14);  // Set to 14 for Teensy
  // CS Pin is 10..do not change for Teensy!
  if (!(SD.begin(10))) {
     Serial.println("Unable to access the SD card");
     beep(2);
  }
  // this just makes sure we get truly random numbers each time
  // when choosing a file to play from the list later on...
  randomSeed(analogRead(0));
  
  // load startup settings
  startup();
  
}

/***
 * Main program loop
 */

void loop() 
{

  switch (STATE) {
    case STATE_NONE:
    case STATE_BOOTING:
      // do nothing
      break;
    case STATE_RUNNING:
      // normal program operation
      run();
      break;
  }
  
}

/***
 * Called from main loop
 */
void run() {

  // check loop
  if (STATE == STATE_RUNNING) {

    if (loopLength && loopMillis > loopLength) {
        playLoop();
    }

    if (Serial.available() > 0) { 
      Serial.readBytesUntil('\n', received, MAX_DATA_SIZE);
      memset(cmd_key, 0, sizeof(cmd_key));
      memset(cmd_val, 0, sizeof(cmd_val));
      char *key, *val, *buf;
      key = strtok_r(received, "=", &buf);
      val = strtok_r(NULL, "=", &buf);
      strcpy(cmd_key, key);
      strcpy(cmd_val, val);
    } else if (Serial1.available() > 0) {
      char *key, *val, *buf, *buf2, *uid;      
      Serial1.readBytesUntil('\n', received, MAX_DATA_SIZE);
      debug(F("RX: %s\n"), received);
      key = strtok_r(received, "=", &buf);
      val = strtok_r(NULL, "=", &buf);
      uid = strtok_r(val, "|", &buf2);
      val = strtok_r(NULL, "|", &buf2);
      strcpy(cmd_key, key);
      strcpy(cmd_val, val);
      debug(F("BLE Cmd: %s Value: %s Uid: %s\n"), cmd_key, cmd_val, uid);
      // validate data received from mobile device!
      if (strcasecmp(cmd_key, "connect") == 0) {
          debug(F("Received access code %s\n"), cmd_val);
          if (strcmp(ACCESS_CODE, cmd_val) == 0) {
            connectSound();
            BT_CONNECTED = true;
            memset(DEVICE_ID, 0, sizeof(DEVICE_ID));
            strcpy(DEVICE_ID, uid);
            debug(F("DEVICE ID %s...Send Access OK\n"), DEVICE_ID);
            sendToApp("access", "1");
          } else {
            BT_CONNECTED = true;
            sendToApp("access", "0");
            BT_CONNECTED = false;
          }
          strcpy(cmd_key, "");
      } else {
          if (strcmp(DEVICE_ID, uid) == 0) {
            // Process remote commands
            if (strcasecmp(cmd_key, "disconnect") == 0) {
                BT_CONNECTED = false;
                disconnectSound();
                memset(DEVICE_ID, 0, sizeof(DEVICE_ID));
                memset(cmd_key, 0, sizeof(cmd_key));
                memset(cmd_val, 0, sizeof(cmd_val));
            } else if (strcasecmp(cmd_key, "config") == 0) {
                sendConfig();
                memset(cmd_key, 0, sizeof(cmd_key));
                memset(cmd_val, 0, sizeof(cmd_val));
            }
          } else {
            // The UUID does not match the one connected, 
            // so clear the command.            
            memset(cmd_key, 0, sizeof(cmd_key));
            memset(cmd_val, 0, sizeof(cmd_val));
          }
      }
    }
    
    if (strcasecmp(cmd_key, "") != 0) {
      Serial.print(">");
      Serial.print(cmd_key);
      if (strcasecmp(cmd_val, "") != 0) {
        Serial.print("=");
        Serial.print(cmd_val);
      }
      Serial.println("<");
      // Check if there is a parameter and process 
      // commands with values first
      if (strcasecmp(cmd_key, "config") == 0) {
        for (int i = 0; i < STARTUP_SETTINGS_COUNT; i++) {
          Serial.println(STARTUP_SETTINGS[i]);
        }
      } else if (strcasecmp(cmd_key, "save") == 0) {
        if (strcasecmp(cmd_val, "") != 0) {
            char *ptr, *pfile, *pname;
            pfile = strtok_r(cmd_val, ";", &ptr);
            if (strcasecmp(pfile, "") != 0) {
              memset(PROFILE_FILE, 0, sizeof(PROFILE_FILE));
              strcpy(PROFILE_FILE, pfile);
            }
            pname = strtok_r(NULL, ";", &ptr);
            if (strcasecmp(pname, "") != 0) {
              memset(PROFILE_NAME, 0, sizeof(PROFILE_NAME));
              strcpy(PROFILE_NAME, pname);
            }
         }
         addFileExt(PROFILE_FILE);
         debug(F("Save settings file %s with description %s\n"), PROFILE_FILE, PROFILE_NAME);
         if (saveSettingsFile(PROFILE_FILE) == true) {
          sendToApp("save", "1");
         } else {
          sendToApp("save", "0");
         }
      } else if (strcasecmp(cmd_key, "access_code") == 0) {
           if (strcasecmp(cmd_val, "") != 0) {
              memset(ACCESS_CODE, 0, sizeof(ACCESS_CODE));
              strcpy(ACCESS_CODE, cmd_val);
              setSettingValue("access_code", ACCESS_CODE);
              saveSettings();
          }
      } else if (strcasecmp(cmd_key, "debug") == 0) {
           if (strcasecmp(cmd_val, "") == 0) {
              Serial.print("DEBUG=");
              Serial.println(DEBUG == true ? "1" : "0");
           } else {
              int i = atoi(cmd_val);
              char val[2] = "0";
              if (i == 0) {
                DEBUG = false;
              } else {
                strcpy(val, "1");
                DEBUG = true;
              }
              setSettingValue("debug", val);
              saveSettings();
          }
      } else if (strcasecmp(cmd_key, "echo") == 0) {
           if (strcasecmp(cmd_val, "") == 0) {
              Serial.print("ECHO=");
              Serial.println(ECHO == true ? "1" : "0");
           } else {
              int i = atoi(cmd_val);
              char val[2] = "0";
              if (i == 0) {
                ECHO = false;
              } else {
                strcpy(val, "1");
                ECHO = true;
              }
              setSettingValue("echo", val);
              saveSettings();
          }
      } else if (strcasecmp(cmd_key, "default") == 0) {
          if (strcasecmp(cmd_val, "") == 0) {
            strcpy(cmd_val, PROFILE_FILE);
          }
          char ret[SETTING_ENTRY_MAX];
          if (setDefaultProfile(cmd_val)) {
            strcpy(ret, "1;");
          } else {
            strcpy(ret, "0;");
          }
          strcat(ret, cmd_val);
          sendToApp("default", ret);
      } else if (strcasecmp(cmd_key, "delete") == 0) {
          if (strcasecmp(cmd_val, "") != 0) {
            char ret[SETTING_ENTRY_MAX];
            if (deleteProfile(cmd_val)) {
              strcpy(ret, "1;");
              strcat(ret, cmd_val);
            } else {
              strcpy(ret, "0;Could not remove profile");
            }
            sendToApp("delete", ret);
          }
      } else if (strcasecmp(cmd_key, "load") == 0) {
          loopPlayer.stop();
          if (strcasecmp(cmd_val, "") != 0) {
            memset(PROFILE_FILE, 0, sizeof(PROFILE_FILE));
            strcpy(PROFILE_FILE, cmd_val);
          } 
          addFileExt(PROFILE_FILE);
          char buf[100];
          strcpy(buf, PROFILES_DIR);
          strcat(buf, PROFILE_FILE);
          loadSettings(buf);
          applySettings();
          long l = playSound(STARTUP_WAV);
          delay(l+100);
          playLoop();
          // send to remote if connected
          sendConfig();
      } else if (strcasecmp(cmd_key, "play") == 0) {
          //playSoundFile(EFFECTS_PLAYER, cmd_val);
          effectsPlayer.play(cmd_val);
      } else if (strcasecmp(cmd_key, "play_effect") == 0) {
          playEffect(cmd_val);
      } else if (strcasecmp(cmd_key, "play_sound") == 0) {
          playSound(cmd_val);
      } else if (strcasecmp(cmd_key, "play_loop") == 0) {
          if (strcasecmp(cmd_val, "") != 0) {
            memset(LOOP_WAV, 0, sizeof(LOOP_WAV));
            strcpy(LOOP_WAV, cmd_val);
          }
          playLoop();
      } else if (strcasecmp(cmd_key, "stop_loop") == 0) {
         loopPlayer.stop();
      } else if (strcasecmp(cmd_key, "beep") == 0) {
          int i = atoi(cmd_val);
          if (i < 1) {
            i = 1;
          }
          beep(i);
      } else if (strcasecmp(cmd_key, "mute") == 0) {
         audioShield.muteHeadphone();
         audioShield.muteLineout();
         MUTED = true;
      } else if (strcasecmp(cmd_key, "unmute") == 0) {
         audioShield.unmuteHeadphone();
         audioShield.unmuteLineout();
         MUTED = false;
      } else if (strcasecmp(cmd_key, "backup") == 0) {
         if (strcasecmp(cmd_val, "") == 0) {
           strcpy(cmd_val, PROFILE_FILE);
         }
         addBackupExt(cmd_val);
         saveSettingsFile(cmd_val, false); 
      } else if (strcasecmp(cmd_key, "restore") == 0) {
         loopPlayer.stop();
         if (strcasecmp(cmd_val, "") == 0) {
           strcpy(cmd_val, PROFILE_FILE);
         }
         addBackupExt(cmd_val);
         char *ret = strstr(cmd_val, PROFILES_DIR);
         if (ret == NULL) {
           char buf[SETTING_ENTRY_MAX];
           strcpy(buf, PROFILES_DIR);
           strcat(buf, cmd_val);
           memset(cmd_val, 0, sizeof(cmd_val));
           strcpy(cmd_val, buf);
           memset(buf, 0, sizeof(buf));
         }
         loadSettings(cmd_val);    
         applySettings();
         long l = playSound(STARTUP_WAV);
         delay(l+100);
         playLoop();
      } else if (strcasecmp(cmd_key, "settings") == 0) {
          Serial.println(F(""));
          Serial.println(PROFILE_FILE);
          Serial.println(F("--------------------------------------------------------------------------------"));
          char buffer[1024];
          char *p = settingsToString(buffer);
          Serial.println(p);
          Serial.println(F("--------------------------------------------------------------------------------"));
          Serial.println(F(""));
      } else if (strcasecmp(cmd_key, "files") == 0) {
          char temp[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
          listFiles("/", temp, MAX_FILE_COUNT, "", true, true);
      } else if (strcasecmp(cmd_key, "show") == 0) {
          showFile(cmd_val);
      } else if (strcasecmp(cmd_key, "sounds") == 0) {
          char temp[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
          int count = listFiles(SOUNDS_DIR, temp, MAX_FILE_COUNT, SOUND_EXT, false, true);
          if (strcasecmp(cmd_val, "1") == 0) {
            char buffer[1024];
            char *files = arrayToStringJson(buffer, temp, count);
            sendToApp("sounds", files);
          }
      } else if (strcasecmp(cmd_key, "effects") == 0) {
          char temp[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
          int count = listFiles(EFFECTS_DIR, temp, MAX_FILE_COUNT, SOUND_EXT, false, true);
          if (strcasecmp(cmd_val, "1") == 0) {
            char buffer[1024];
            char *files = arrayToStringJson(buffer, temp, count);
            sendToApp("effects", files);
          }
      } else if (strcasecmp(cmd_key, "loops") == 0) {
          char temp[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
          int count = listFiles(LOOP_DIR, temp, MAX_FILE_COUNT, SOUND_EXT, false, true);
          if (strcasecmp(cmd_val, "1") == 0) {
            char buffer[1024];
            char *files = arrayToStringJson(buffer, temp, count);
            sendToApp("loops", files);
          }
      } else if (strcasecmp(cmd_key, "profiles") == 0) {
          char temp[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
          int count = listFiles(PROFILES_DIR, temp, MAX_FILE_COUNT, FILE_EXT, false, false);
          char buffer[1024];
          char *def = getSettingValue(buffer, "profile");
          for (int i = 0; i < count; i++) {
            Serial.print(temp[i]);
            if (strcasecmp(temp[i], PROFILE_FILE) == 0) {
              Serial.print(" (Loaded)");
            }
            if (strcasecmp(temp[i], def) == 0) {
              Serial.print(" (Default)");
            }
            Serial.println("");
          }
      } else if (strcasecmp(cmd_key, "ls") == 0) {
          char paths[MAX_FILE_COUNT][SETTING_ENTRY_MAX];
          char buffer[1024];
          // return a list of directories on the card
          int count = listDirectories("/", paths);
          char *dirs = arrayToStringJson(buffer, paths, count);
          sendToApp(cmd_key, dirs);
      } else if (strcasecmp(cmd_key, "help") == 0) {
          showFile("HELP.TXT");
      } else if (strcasecmp(cmd_key, "calibrate") == 0) {
          #ifdef CALIBRATE
            calibrate();
          #endif
      } else if (strcasecmp(cmd_key, "reset") == 0) {
        softreset();
      } else { 
        parseSetting(cmd_key, cmd_val);
        applySettings();
        if (strcasecmp(cmd_key, "loop") == 0) {
          playLoop();
        }  
      }
      Serial.println("");
      memset(cmd_key, 0, sizeof(cmd_key));
      memset(cmd_val, 0, sizeof(cmd_val));
    }
  
    if (BUTTON_PIN && button_initialized == false) {
      button_initialized = checkButton();
      if (button_initialized) {
        // turn voice on with background noise
        voiceOn();
      }
    } else {
      PTT.update();
    }

    
    if (BUTTON_PIN && button_initialized) {
  
      // Check if there is silence.  If not, set a flag so that
      // we don't switch back to Voice Activated mode accidentally ;)
      if (speaking == true && silent == true) {
          if (rms1.available()) {
            float val = rms1.read();
            // This check is here to make sure we don't get false readings
            // when the button click noise is played which would prevent 
            // the switch back to Voice Activated mode
            if ((val-VOICE_STOP) >= .1) {
              silent = false;
            }
          }
      }
  
      // Switch back to Voice Activated mode if:
      //    1.  PTT button was pushed
      //    2.  There has been silence for at least 2 seconds
      // NOTE:  If you start talking before the 2 second time limit
      //        it will NOT switch back...or if you talk and pause for 
      //        2 seconds or more it will NOT switch back.
      if (speaking == true && silent == true && stopped > 2000) {
        voiceOff();
        button_initialized = false;
        connectSound();
        return;
      }
  
      // Button press
      
      if (PTT.fell()) {
        playEffect(BUTTON_WAV);
        //ms = 0;
        voiceOn();
      }
      
      // Button release
      if (PTT.rose()) {
        stopped = 0;
        while (stopped < SILENCE_TIME) {
        }
          voiceOff();
        // Random comm sound
        addSoundEffect();
      }
      
    } else {
  
        // Check if we have audio
        if (rms1.available()) {
          
          // get the input amplitude...will be between 0.0 (no sound) and 1.0
          float val = rms1.read();
    
          // Uncomment this to see what your constant signal is
          //Serial.println(val);
          
          // Set a minimum and maximum level to use or you will trigger it accidentally 
          // by breathing, etc.  Adjust this level as necessary!
          if (val >= VOICE_START) {
  
             debug(F("Voice start: %4f\n"), val);
             
            // If user is not currently speaking, then they just started talking :)
            if (speaking == false) {
  
              voiceOn();
          
            }
  
          } else if (speaking == true) {

              debug(F("Voice val: %4f\n"), val);
              
              if (val < VOICE_STOP) {
    
                // If the user has stopped talking for at least the required "silence" time and 
                // the mic/line input has fallen below the minimum input threshold, play a random 
                // sound from the card.  NOTE:  You can adjust the delay time to your liking...
                // but it should probably be AT LEAST 1/4 second (250 milliseconds.)
    
                if (stopped >= SILENCE_TIME) {
                  debug(F("Voice stop: %4f\n"), val);
                  voiceOff();
                  // play random sound effect
                  addSoundEffect();
                }
    
              } else {
                  
                  // Reset the "silence" counter 
                  stopped = 0;
                  
              }
    
            }
          
         }
  
  
    }
  
    readVolume();

  }
  
}

/**
 * Perform a restart without having to cycle power
 */
#define RESTART_ADDR       0xE000ED0C
#define READ_RESTART()     (*(volatile uint32_t *)RESTART_ADDR)
#define WRITE_RESTART(val) ((*(volatile uint32_t *)RESTART_ADDR) = (val))

void softreset() {
 WRITE_RESTART(0x5FA0004);
}

// END


