/****
 * Global variables/constants
 */
 
// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=196,125
AudioPlaySdWav           loopPlayer;     //xy=199,402
AudioSynthWaveform       waveform1;      //xy=202,316
AudioPlaySdWav           effectsPlayer;  //xy=208,266
AudioAnalyzeRMS          rms1;           //xy=325,174
AudioSynthNoisePink      pink1;          //xy=473,176
AudioEffectBitcrusher    bitcrusher1;    //xy=487,90
AudioEffectBitcrusher    bitcrusher2;    //xy=487,133
AudioMixer4              loopMixer;      //xy=656,412
AudioMixer4              voiceMixer;     //xy=659,142
AudioMixer4              effectsMixer;   //xy=660,280
AudioOutputI2S           i2s2;           //xy=836,139
AudioConnection          patchCord1(i2s1, 0, rms1, 0);
AudioConnection          patchCord2(i2s1, 0, bitcrusher1, 0);
AudioConnection          patchCord3(i2s1, 0, bitcrusher2, 0);
AudioConnection          patchCord4(loopPlayer, 0, loopMixer, 0);
AudioConnection          patchCord5(loopPlayer, 1, loopMixer, 1);
AudioConnection          patchCord6(waveform1, 0, effectsMixer, 2);
AudioConnection          patchCord7(effectsPlayer, 0, effectsMixer, 0);
AudioConnection          patchCord8(effectsPlayer, 1, effectsMixer, 1);
AudioConnection          patchCord9(pink1, 0, voiceMixer, 2);
AudioConnection          patchCord10(bitcrusher1, 0, voiceMixer, 0);
AudioConnection          patchCord11(bitcrusher2, 0, voiceMixer, 1);
AudioConnection          patchCord12(loopMixer, 0, effectsMixer, 3);
AudioConnection          patchCord13(voiceMixer, 0, i2s2, 0);
AudioConnection          patchCord14(voiceMixer, 0, i2s2, 1);
AudioConnection          patchCord15(effectsMixer, 0, voiceMixer, 3);
AudioControlSGTL5000     audioShield;    //xy=846,428
// GUItool: end automatically generated code


// version flag
const char VERSION[5] = "3.11";
      
//elapsedMillis ms;                         // running timer...inputs are checked every 24 milliseconds
elapsedMillis stopped;                      // used to tell how long user has stopped talking
boolean speaking = false;                   // flag to let us know if the user is speaking or not

const int MAX_FILE_COUNT    = 99;
const int SETTING_ENTRY_MAX = 50;

int SOUND_EFFECTS_COUNT = 0;                             // This keeps count of how many valid WAV files were found.
char SOUND_EFFECTS[MAX_FILE_COUNT][SETTING_ENTRY_MAX];   // This will hold an array of the WAV files on the SD card.
                                                         // 99 is an arbitrary number.  You can change it as you need to.
int lastRnd  = -1;                                       // Keeps track of the last file played so that it is different each time

Bounce PTT = Bounce();                       // Used to read the PTT button (if attached)

boolean silent = false;                      // used for PTT and to switch back to Voice Activated mode
boolean button_initialized = false;          // flag that lets us know if the PTT has been pushed or not to go into PTT mode

const int EFFECTS_PLAYER = 1;
const int LOOP_PLAYER = 2;

const char SETTINGS_FILE[SETTING_ENTRY_MAX]    = "SETTINGS.TXT";   // Global settings file

// These are the paths where files are stored for the profile.  This can be changed via the app or
// edited manually so that each profile can have its own files (or you can share them...)
char EFFECTS_DIR[SETTING_ENTRY_MAX] = "/effects/";      // effects sounds (mic pop, clicks, static, etc.)
char SOUNDS_DIR[SETTING_ENTRY_MAX]  = "/sounds/";       // general sound files 
char LOOP_DIR[SETTING_ENTRY_MAX]    = "/loops/";        // sound loops
const char PROFILES_DIR[11]         = "/profiles/";     // Location of config profiles

const int STARTUP_SETTINGS_COUNT    = 6;                                 // counter for number of entries in global settings file
char STARTUP_SETTINGS[STARTUP_SETTINGS_COUNT][SETTING_ENTRY_MAX];        // Holds settings values

int STATE;                                             // current operational state of application

// operation states
const int STATE_NONE     = 0;
const int STATE_BOOTING  = 2;
const int STATE_RUNNING  = 3;

// Other defaults
const char SOUND_EXT[5]  = ".WAV";
const char FILE_EXT[5]   = ".TXT";
const char BACKUP_EXT[5] = ".BAK";

// Default settings - can be modified through settings file
char     PROFILE_NAME[50] = "Default";
char     PROFILE_FILE[SETTING_ENTRY_MAX] = "TKCONFIG.TXT";
float    MASTER_VOLUME    = 0.5; 
int      LINEOUT          = 29; // Valid values 13 to 31. Default teensy setting is 29.
int      LINEIN           = 5;  // Value values 0 to 15. Default teensy setting is 5;
int      HIPASS           = 0;  // off by default, 1 = on
int      MIC_GAIN         = 3;
char     STARTUP_WAV[SETTING_ENTRY_MAX];
char     LOOP_WAV[SETTING_ENTRY_MAX];
char     BUTTON_WAV[SETTING_ENTRY_MAX];
int      AUDIO_INPUT      = AUDIO_INPUT_MIC;
int      EQ               = 0;
int      EQ_BANDS_SIZE    = 5;
float    EQ_BANDS[5]      = { -1.0,0,1,0,-1.0 };
int      BITCRUSHER_SIZE  = 4;
int      BITCRUSHER[4]    = { 12,16384,10,10240 };
float    LOOP_GAIN        = 2;
float    VOICE_GAIN       = 1;
float    NOISE_GAIN       = 0;
float    EFFECTS_GAIN     = 5;
uint16_t SILENCE_TIME     = 350;    // The minimum time to wait before playing a sound effect after talking has stopped
float    VOICE_START      = 0.07;   // The amplitude needed to trigger the sound effects process
float    VOICE_STOP       = 0.02;   // The minimum amplitude to use when determining if you've stopped talking.
                                    // Depending upon the microphone you are using, you may get a constant signal
                                    // that is above 0 or even 0.01.  Use the Serial monitor and add output to the 
                                    // loop to see what signal amplitude you are receiving when the mic is "quiet."
int      BUTTON_PIN;                // The pin to which a PTT button is connected (not required.) Change it if you 
                                    // attach it to a different pin (should use 0, 1 or 2, though...as not all pins
                                    // are available since they are used by the Audio Adaptor.
boolean MUTE_LOOP = 1;              // mute loop while talking on/off
boolean DEBUG     = false;          // Set to true to have debug messages printed out...useful for testing
boolean ECHO      = false;          // Set to true to have BLE TX/RX messages displayed in serial monitor

// Bluetooth App 
char DEVICE_ID[50];                 // the uuid of the device connecting 
char ACCESS_CODE[25] = "1138";      // the password for remote apps to access this device (specified in global settings file)
boolean BT_CONNECTED = false;       // flag to indicate whether a remote app is connected or not
boolean MUTED        = false;       // flag to indicate whether all sounds should be muted

char INPUT_TYPE[5]         = "BOTH";  // Set to "MIC", "LINE" or "BOTH" (default)
char OUTPUT_TYPE[5]        = "BOTH";  // Set to "SPKR", "LINE" or "BOTH" (default)

// Loops
elapsedMillis loopMillis = 0;
unsigned int loopLength;

// loop and serial command handlers
char cmd_key[SETTING_ENTRY_MAX] = "";
char cmd_val[SETTING_ENTRY_MAX] = "";
const int MAX_DATA_SIZE = 100;
char received[MAX_DATA_SIZE] = "";

