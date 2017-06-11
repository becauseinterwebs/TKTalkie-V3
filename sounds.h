/****
 * Sound utils
 */

/**
 * Emit a warning tone
 */
void beep(const int times = 1)
{
  audioShield.unmuteHeadphone();
  audioShield.unmuteLineout();
  for (int i=0; i<times; i++) {
    waveform1.frequency(720);
    waveform1.amplitude(0.7);
    delay(100);
    waveform1.frequency(1440);
    delay(50);
    waveform1.amplitude(0);
    delay(350);
  }
}

/***
 * Read the contents of the SD card and put any files ending with ".WAV" 
 * into the array.  It will recursively search directories.  
 */
void loadSoundEffects() 
{
  SOUND_EFFECTS_COUNT = listFiles(EFFECTS_DIR, SOUND_EFFECTS, MAX_FILE_COUNT, SOUND_EXT, false, false);
  debug(F("%d Sound effects loaded\n"), SOUND_EFFECTS_COUNT);
}

/***
 * Play the specified sound effect from the SD card
 */
long playSoundFile(int player, char *filename) 
{

  if (strcasecmp(filename, "") == 0) {
    debug(F("Exit play sound -> blank file name\n"));
    return 0;
  }
  char *ret = strstr(filename, ".");
  if (ret == NULL) {
    char ext[5];
    strcpy(ext, SOUND_EXT);
    strcat(filename, ext);
  }
  debug(F("Play sound file %s on player %d\n"), filename, player);
  long len = 0;
  switch (player) {
    case LOOP_PLAYER:
      loopPlayer.stop();
      loopPlayer.play(filename);
      delay(10);
      len = loopPlayer.lengthMillis();
      break;
    default:
      effectsPlayer.play(filename);
      delay(10);
      len = effectsPlayer.lengthMillis();
      break;
  }
  debug(F("Sound File Length: %d\n"), len);
  return len;
}

/**
 * Shortcut to play a sound from the SOUNDS directory
 */
long playSound(const char *filename)
{
  if (strcasecmp(filename, "") == 0) {
    return 0;
  }
  char buf[100];
  strcpy(buf, SOUNDS_DIR);
  strcat(buf, filename);
  return playSoundFile(EFFECTS_PLAYER, buf);
}

/**
 * Shortcut to play a sound from the EFFECTS directory
 */
long playEffect(const char *filename)
{
  if (strcasecmp(filename, "") == 0) {
    return 0;
  }
  char buf[100];
  strcpy(buf, EFFECTS_DIR);
  strcat(buf, filename);
  return playSoundFile(EFFECTS_PLAYER, buf);
}

/***
 * Play sound loop and set counters
 */
void playLoop() 
{
  loopLength = 0;
  if (strcasecmp(LOOP_WAV, "") != 0) {
    char buf[100];
    strcpy(buf, LOOP_DIR);
    strcat(buf, LOOP_WAV);
    loopLength = playSoundFile(LOOP_PLAYER, buf);
  }
  loopMillis = 0;
}

/***
 * Play a random sound effect from the SD card
 */
void addSoundEffect()
{
  if (speaking == true || SOUND_EFFECTS_COUNT < 1) return;
  // generate a random number between 0 and the number of files read - 1
  int rnd = 0;
  int count = 0;
  rnd = lastRnd;
  while (rnd == lastRnd && count < 50) {
   rnd = random(0, SOUND_EFFECTS_COUNT);
   count++;
  }
  lastRnd = rnd;
  // play the file
  playEffect(SOUND_EFFECTS[rnd]);
}

/***
 * Check the optional volume pot for output level
 */
float readVolume()
{
    float vol = 0;
    if (MASTER_VOLUME) {
      audioShield.volume(MASTER_VOLUME);
      vol = MASTER_VOLUME;
    } else {
      // comment these lines if your audio shield does not have the optional volume pot soldered on
      vol = analogRead(15);
      vol = vol / 1023;
      audioShield.volume(vol);
    }
    if (vol > 1.0) {
      vol = 1.0;
    } else if (vol < 0) {
      vol = 0;
    }
    return vol;
}


/***
 * This is played when switching from PTT back to Voice Activated mode.
 * It is also played when a device connects via Bluetooth. 
 */
void connectSound() 
{
  for (int i=0; i<3; i++) {
    waveform1.frequency(1440);
    waveform1.amplitude(0.3);
    delay(100);
    waveform1.amplitude(0);
    delay(100);
  }
  waveform1.amplitude(0);
}

/***
 * This is played when a mobile device connects via Bluetooth 
 */
void disconnectSound() 
{
  for (int i=0; i<3; i++) {
    waveform1.frequency(720);
    waveform1.amplitude(0.3);
    delay(100);
    waveform1.amplitude(0);
    delay(100);
  }
  waveform1.amplitude(0);
}

/***
 * Turns the volume down on the chatter loop
 */
void loopOff() 
{
  loopMixer.gain(0, 0);
  loopMixer.gain(1, 0);
}

/***
 * Turns the volume up on the chatter loop
 */
void loopOn() 
{
  // gradually raise level to avoid pops 
  if (LOOP_GAIN > 1) {
    for (int i=0; i<=LOOP_GAIN; i++) {
      loopMixer.gain(2, i);
      loopMixer.gain(3, i);
    }
  }
  loopMixer.gain(0, LOOP_GAIN);
  loopMixer.gain(1, LOOP_GAIN);
}

/***
 * Turns off the voice channels on the mixer
 */
void voiceOff() 
{
  speaking = false;
  silent = false;
  stopped = 0;
  pink1.amplitude(0);
  voiceMixer.gain(0, 0);
  voiceMixer.gain(1, 0);
  if (MUTE_LOOP == 1) {
    loopOn();
  }
}

/***
 * Turns on the voice channels on the mixer
 */
void voiceOn() 
{
  speaking = true;
  silent = true;
  if (MUTE_LOOP == 1) {
    loopOff();
  }
  // Reset the "user is talking" timer
  stopped = 0;
  // pops are ok here ;)
  pink1.amplitude(NOISE_GAIN);
  voiceMixer.gain(0, VOICE_GAIN);
  voiceMixer.gain(1, VOICE_GAIN);
}
