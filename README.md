# TKTalkie V3
Firmware for TKTalkie v3 - www.tktalkie.com

**YOU ARE FREE TO USE THIS CODE IN YOUR OWN PROJECTS, PROVIDED THEY ARE FOR PERSONAL, NON-COMMERCIAL USE.**

## Latest Version: 3.11

**Visit www.tktalkie.com for support, downloads and more.**

## About
The TKTalkie software is part of the TKTalkie DIY project.  This revision has some significant changes designed to make it even easier to configure and use the system through the use of a BLE (Bluetooth Low Energy) device and a mobile app (iOS/Android) and contains the following improvements:

### New in version 3.11:
In order to support updates to the mobile app, the following updates were made:

  - Support for ability to select from different folders on the SD card when creating custom profiles
  - Support for new command key 'mute_loop=0/1' to set where or not the background loops mutes when talking (defaults to 1 = true)
  - Support for new command 'ls' which will list the folders on the SD card.
  - Fixed bug when requesting loop directory files be sent to app.
  
### New in version 3:
  - Support for BLE (Bluetooth Low Energy) boards used to communicate with TKTalkie App (iOS/Android) to control TKTalkie devices allowing you to modify settings, save profiles and more.
  - Ability to have mulitple profiles (allows you to use TKTalkie for different costume applications)
  - Ability to organize sound files in different directories, allowing different profile to use different sound collections
  - Default startup profile can be set (and changed via serial interface or mobile app.)
  - Removed support for different equalizer types.  Equalizer can be on/off and is a 5-band graphic.
  - Warning beeps added for startup and SD card errors (see Warning Beep section below)
  - Lots of code optimizations to better manage memory

### From version 2:
  - Text config file allows you to customize settings and options without having to recompile the code
  - Calibration wizard added to help find optimum settings for your particular use
  - Added support for an endless loop (background chatter) file
  - Added the ability to use a serial connection (via Arduino IDE Serial Monitor or Terminal software) to change settings, save config files and load config files to enable easier testing and the use of different configs for different applications
  - Cleaned up and optimized the code

## Warning Beeps
This version will sound warning beeps when errors are detected (to make is easier to be aware when the unit is in use.)  The error is determined by the number of beeps played:

- 2 Beeps = Error reading SD card
- 3 Beeps = Error opening a settings (profile) file
- 4 Beeps = Error updating the SETTINGS.TXT (system settings) file

## Startup Settings File

A new file, **SETTINGS.TXT**, is used to control startup functions of TKTalkie.  It has the following entries:

1. [profile=SAMPLE.TXT] - The default profile to load on startup
2. [access_code=xxxx] - Alphanumeric access code for the device that is needed to allow mobile app to connect.
3. [debug=0] - Turns debugging on (1) or off (0) for the unit.  Previously this was stored in the profile file.  Default is off.  When on, messages during program operation will be displayed to the serial interface.  This can be useful when testing configuration settings.

## Configuration (Profile) Files

You can now have multiple profile configuration files.  These are stored in the **/profiles** folder on the SD card. The **SETTINGS.TXT** holds the default profile to load at system startup.  You can easily swap profiles via the serial interface or mobile app.

Profile files have a **.TXT** file extension and have the following format for configuration values:

    [setting_key=setting_value]  *Note the use of [ and ] for the settings!

> Settings are wrapped in brackets due to the differences in the way editors will use the EOL (End-of-Line) character.  This method allows the use of any editor for your configuration file.

> **NOTE**: Any text *OUTSIDE* of the brackets will be ignored.

## Sound Collections

Each profile specifies the directories where the sound files to be used for that profile are located.  This allows you to either separate or share sounds between profiles.  By default, the directories are:

#### Sounds Directory
````
[sounds_dir=/sounds/]
````
This directory holds the startup sounds that can be played. It can be shared across profiles since only a  single startup sound is played and you can choose which sound to play in your configuration profile.

#### Loop Directory
```
[loop_dir=/loops/]
```
This holds the background loops.  Again, this can be shared since only one loop file is played and you can choose which file is used.

#### Effects Directory
```
[effects_dir=/effects/]
```
This directory holds the sounds (mic pops and clicks) that are played whenever you stop talking.  Although this directory can be shared, if you are using TKTalkie for other costumes (such as a Clone Trooper, for example, which does not use the mic pops) you can specify a different directory, or leave it blank.

## Configuration Values

The current settings and valid values are:

** name **
```
[name=This is the default profile]
```
Name of the profile.  This is displayed in the mobile app when loading/saving/managing profiles.

** volume **
```
[volume=.50]
```
Sets the initial volume level (If you have a volume pot soldered to your Audio Adaptor then leave this entry out of your config file and the sketch will read the volume pot instead.)  Valid values are decimal values from 0 to 1, although past .7 you may experience distortion.

> **NOTE**: This has no effect if you are using **LINEOUT** instead of the headphones jack

**linein**
```
[linein=5]
```
Set the level for the line-in input. Valid values are 0 to 15, with 5 being the default. Lower numbers decrease the input level, higher numbers increase it.

**lineout**
```
[lineout=29]
```
Set the level for the line-out signal.  Valid values are 13 to 31, with 29 being the default.  This works in the **OPPOSITE** direction, since by lowering the value you are lowering the attenuation of the signal making the output louder, and by raising it you are increasing the attenuation, making the signal quieter.

**startup**
```
[startup=STARTUP.WAV]
```
This is the sound that is played each time TKTalkie starts.

**loop**
```
[loop=CHATTER.WAV]
```
The background (chatter) file.  This file starts AFTER the startup file, then will continuously loop while TKTalkie is running.

> **NOTE**: The backgound loop will be silenced while you are speaking (but will continue running)

**loop_gain**
```
[loop_gain=.025]
```
This is the sound level of the loop while it is playing.

**silence_time**
```
[loop_gain=350]
```
The amount of time (in milliseconds...or *thousandths* of a second) to wait to make sure you are finished talking before playing an effect (static burst or mic click effect, etc.)

**voice_start**   (*formerly VOL_THRESHOLD_TRIGGER*)
```
[voice_start=0.07]
```
This is the input level received from the microphone that will trigger the Voice Activation when you begin talking.

> **NOTE"": You can use the new Calibration Wizard via the serial interface to help set this level

**voice_stop**  (*formerly VOL_THRESHOLD_MIN*)
```
[voice_stop=0.02]
```
The input level received from the microphone that indicates you are finished talking.

> **NOTE**: You can use the new Calibration Wizard via the serial interface to help set this level

*The following entries are for using the PTT (Push-to-Talk) function*

**button_pin**
```
[button_pin=2]
```
This setting tells TKTalkie which pin on the Teensy your PTT button is connected to.

**button_click**
```
[button_click=BUTTON.WAV]
```
The sound to play when the PTT button is pushed (before you start talking.)  When the button is released, a random sound will be played.

**button_gain**
```
[button_gain=1]
```
The output level of the button sound.

**input**
```
# 1 = microphone, 0 = line-in
[input=1]
```
This setting specifies which voice input will be used, the microphone input or the line-in input. Default is microphone.

**mic_gain**
```
[mic_gain=3]
```
The voice input level when using the microphone input. Values are whole numbers from 0 to 63. If you are experiencing feedback and using the microphone input (not line-in), try adjusting the *mic_gain* setting first.

> **NOTE**: This setting has no effect if you are using the line-in input for your microphone

**effects_gain**
```
[effects_gain=5]
```
This sets the output level of sound effects. Valid values are any decimal value from 0 to 10.

**eq**
```
[eq=1]
```
Specify whether the equalizer is on (1) or off (0). If set to on, a 5-band graphic equalizer is used.

**eq_bands**
```
[eq_bands=-1,0,1,0,-1]
```
Depending upon they type of equalizer selected, specify the settings for each band.  Bands are comma separated and valid decimal values from -1.00 (-11.75dB) to 1.00 (+12dB).

> **NOTE**: Bands (from left to right) are: Low (Bass), Low-Mid, Mid, High-Mid, High (Treble)

**voice_gain**
```
[voice_gain=1]
```
Set the output level of the voice channel on the mixer. Valid values are any decimal value from 0 to 32767, 1 is pass-thru, below 1 attenuates signal.

**bitcrushers**
```
# Format = bits1,rate1,bits2,rate2
# Set to 16,41000,16,41000 to just pass-thru (disable)
[bitcrushers=12,16384,10,10240]
```
There are two bitcrushers to process the voice input and make it sound a little more tinny and robotic as if it is coming through a radio.  These are comma separated values that denote **bits** and **sample rate**, hence *bits1,sample rate1,bits2,sample rate2*.  Valid values for bits are 1 to 16, and valid values for sample rate are from 1 to 44100.

> **NOTE**: You may need to disable these if using other voice processers such as a voice changer with TKTalkie.

**noise_gain**
```
[noise_gain=.15]
```
While talking, pink noise is played behind your voice to help simulate radio communications.  This setting sets the noise level.  Valid values are any decimal value from 0 to 32767, 1 is pass-thru, below 1 attenuates signal. Set to 0 to disable.

**sound_dir**
````
[sounds_dir=/sounds/]
````
This directory holds the startup sounds that can be played. It can be shared across profiles since only a  single startup sound is played and you can choose which sound to play in your configuration profile.

**loop_dir**
```
[loop_dir=/loops/]
```
This holds the background loops.  Again, this can be shared since only one loop file is played and you can choose which file is used.

**effects_dir**
```
[effects_dir=/effects/]
```
This directory holds the effects sounds that are applied when you stop talking.

## Serial Interface

As in version 2, this version of the sofware allows you to change configuration settings, save configuration files and load configuration files via a serial interface.  The Arduino IDE has a built-in serial monitor that can be used for this, or you can connect via a terminal program at 57600 baud.

When connected, the following commands are available:
````
help                  Show this help screen.
files                 Show a list of files on SD card.
profiles              Show a list of available profiles on SD card.
effects               Show directory listing of effects folder.
sounds                Show directory listing of sounds folder.
loops                 Show directory listing of sounds folder.
settings              Show current settings.
config                Displays the startup (system) settings
load                  Load settings from default file
save                  Save settings to currently loaded profile
access_code=value     Sets BLE access code for mobile app
debug=(1/0)           Turns debugging on/off
beep=n                Play warning beep sound n times
mute                  Mute all sounds
unmute                Unmute all sounds
reset                 Resets the TKTalkie unit

For the following commands, a file extension of .TXT (for settings files) or
.WAV (for sound files) is assumed and will be added if not specified.

load=filename         Load settings from specified file
save=filename         Save current settings to specified file
default=filename      Set the default settings profile to be loaded
                      when TKTalkie starts
delete=filename       Delete a settings file
play=filename         Plays the specified .WAV file (use full path/filename)
play_effect=filename  Play specified file from effects folder
play_sound=filename   Play specified file from sounds folder
play_loop=filename    Play specified file from loops folder
stop_loop             Stop currently playing loop

backup                Quick backup of settings file to SETTINGS.BAK
restore               Quick restore from SETTINGS.BAK

calibrate             Runs microphone calibration wizard
````

## Changing Configuration Settings

There are now **FOUR** ways to change configuration settings:

- Use a BLE and the mobile app
- Change the default values in the source code and recompile and upload the code to your device
- Change configuration values in the TKCONFIG.TXT file and restart the device
- Make live changes via the serial interface and then save them

### Use the Mobile App
If you have attached the BLE device (see the V3 tutorial at [http://www.tktalkie.com/tutorial/v3](http://www.tktalkie.com/tutorial/v3)), you can use the mobile app to control your device.  Please visit [http://www.tktalkie.com/app](http://www.tktalkie.com/app) for more information.

### Making Configuration Changes via Code

If you do not wish to use a config file, you can simply alter the default options located in the **globals.h** file and recompile, then upload, your changes to your device.

### Making Configuration Changes via Config File

Using this option, you can simply edit the config file, save your changes, then reinsert the card into the Teensy device and restart. Please see the **Configuration Values** section for a list of available options.

### Making Configuration Changes via Serial Interface

You can live, real-time changes via the serial interface.  Changes take effect immediately.  They will be LOST when the device restarts unless you save them using one of the '*save*' commands.  When making live changes, use the following format:

    setting_key=new_value [ENTER]

The new setting will take effect immediately.  This is a great way to allow you to tweak and test settings for your particular application of the TKTalkie system.  Please see the **Configuration Values** section for a list of available options.

For more information regarding TKTalkie, please visit [www.tktalkie.com](http://www.tktalkie.com).

