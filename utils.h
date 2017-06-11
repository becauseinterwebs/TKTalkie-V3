/****
 * General utils
 */

/**
 * Uses the "F" FlashStringHelper (to help save memory)
 * to output a formatted string.
 * This code is adapted from http://playground.arduino.cc/Main/Printf
 */
void debug(const __FlashStringHelper *fmt, ... ) {
  if (DEBUG != true) {
    return;
  }
  char buf[256]; // resulting string limited to 1000 chars
  va_list args;
  va_start (args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);
  Serial.print("> ");
  Serial.print(buf);
}

/**
 * Convert char string to upcase
 */
void upcase(char *str)
{
   int i = 0;
   if (strcasecmp(str, "") == 0) {
    return;
   }
   while (str[i] != '\0') {
    toupper(str[i]);
    i++;
   }
}

/**
 * Convert array of char strings to comma-delimited string 
 */
char *arrayToString(char result[], const char arr[][SETTING_ENTRY_MAX], int len) 
{
  for (int i = 0 ; i < len; i++) {
    if (i == 0) {
      strcpy(result, arr[i]);
    } else {
      strcat(result, ",");
      strcat(result, arr[i]);
    }
  }
  return result;
}

/**
 * Convert array of char strings to comma-delimited char-based string 
 */
char *arrayToStringJson(char result[], const char arr[][SETTING_ENTRY_MAX], int len) 
{
  strcpy(result, "[");
  for (int i = 0 ; i < len; i++) {
    strcat(result, "\"");
    strcat(result, arr[i]);
    strcat(result, "\"");
    if (i < len-1) {
      strcat(result, ",");
    }
  }
  strcat(result, "]");
  return result;
}

/**
 * Convert array of int to comma-delimited string 
 */
char *arrayToString(char result[], int arr[], int len) 
{
  char buf[20];
  for (int i = 0 ; i < len; i++) {
    sprintf(buf, "%d", arr[i]);
    if (i == 0) {
      strcpy(result, buf);
    } else {
      strcat(result, ",");
      strcat(result, buf);
    }
    memset(buf, 0, sizeof(buf));
  }
  return result;
}

/**
 * Convert array of float to comma-delimited string
 */
char *arrayToString(char result[], float arr[], const int len) 
{
  char buf[20];
  for (int i = 0 ; i < len; i++) {
    dtostrf(arr[i], 0, 4, buf);
    if (i == 0) {
      strcpy(result, buf);
    } else {
      strcat(result, ",");
      strcat(result, buf);
    }
    memset(buf, 0, sizeof(buf));
  }
  return result;
}



