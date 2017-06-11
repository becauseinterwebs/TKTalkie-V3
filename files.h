/****
 * Common file routines
 */

/**
 * Make sure the file has an extension
 */
void addFileExt(char *path)
{
  char buf[SETTING_ENTRY_MAX];
  strcpy(buf, path);
  upcase(buf);
  char *ret = strstr(buf, FILE_EXT);  
  if (ret == NULL) {
    strcat(path, FILE_EXT);
  }
}

/**
 * Make sure file has backup extension
 */
void addBackupExt(char *path)
{
  char *ret = strstr(path, ".");  
  if (ret != NULL) {
    char *ptr;
    ret = strtok_r(path, ".", &ptr);
  }
  strcat(path, BACKUP_EXT);
}

/**
 * Makes sure paths start and end with /
 */
void fixPath(char *path)
{
   if (strcasecmp(path, "") == 0) {
    return;
   }
   if (path[0] != '/') {
     char buf[SETTING_ENTRY_MAX] = "/";
     strcat(buf, path);
     strcpy(path, buf);
   }
   int i = 1;
   while (path[i] != '\0' && i < SETTING_ENTRY_MAX) {
    i++;
   }
   if (path[i-1] != '/') {
    path[i]   = '/';
    path[i+1] = '\0';
   }
}

/**
 * Delete the specified file
 */
boolean deleteFile(const char *filename) 
{
  if (SD.exists(filename)) {
    SD.remove(filename);
    return true;
  } else {
    return false;
  }
}

/**
 * Open a file on the SD card in the specified mode.
 */
File openFile(const char *filename, int mode) 
{
  // Thanks to TanRu !
  if (mode == FILE_WRITE) {
    deleteFile(filename);
  }
  debug(F("Opening file %s\n"), filename);
  return SD.open(filename, mode);  
}

/***
 * Show a text file
 */
void showFile(const char *filename) {
  debug(F("Showing file %s\n"), filename);
  File file = SD.open(filename);
  if (file) {
    char c;
    while (file.available()) {
      c = file.read();
      Serial.print(c);
    }
    file.close();
  } else {
    Serial.println(F("Could not find file!"));
  }
}

/***
 * recursively list all files on SD card
 */
String dirSep = "";

int listDirectories(const char *path, char directories[][SETTING_ENTRY_MAX])
{
   int index = 0;
   File dir = SD.open(path);
   dir.rewindDirectory();
   while(true && index < MAX_FILE_COUNT+1) {
     File entry = dir.openNextFile();
     if (! entry) {
       // no more files
       break;
     }
     if (entry.isDirectory()) {
       char *ret = strstr(entry.name(), "~");
       if (ret == NULL) {
         strcpy(directories[index], entry.name());
         //index += listDirectories(entry.name(), directories);
         index++;
       }
     }  
     entry.close();
   }
   return index;
}

/**
 * Return a directory listing.
 * If filter is specified, only file names containing
 * the filter text are returned.
 */
 int listFiles(const char *path, char files[][SETTING_ENTRY_MAX], int max, const char *match, boolean recurse, boolean echo) 
{
  char filter[SETTING_ENTRY_MAX];
  strcpy(filter, match);
  upcase(filter);
  boolean checkFilter = (strcasecmp(filter, "") == 0) ? false : true;
  if (checkFilter) {
    debug(F("Filter: %s\n"), filter);
  }
  int index = 0;
  File dir = SD.open(path);
  if (!dir) {
    return 0;
  }
  dir.rewindDirectory();
  while(true && index < max) {
     File entry = dir.openNextFile();
     if (! entry) {
       // no more files
       if (dirSep != "") {
          dirSep = dirSep.substring(0, dirSep.length()-2);
       }
       break;
     }
     if (entry.isDirectory() && recurse == true) {
       // Filter out folders with ~ (backups)
       char *ret = strstr(entry.name(), "~");
       if (ret == NULL) {
         if (echo || DEBUG) {
          Serial.print(dirSep);
          Serial.print(entry.name());
          Serial.println("/");
         }
         dirSep += "  ";
         index += listFiles(entry.name(), files, max-index, filter, recurse, echo);
       }
     } else {
        char *fname = entry.name();
        upcase(fname);
        // Filter out filenames with ~ (backups)
        char *ret = strstr(fname, "~");
        if (ret == NULL) {
            if (checkFilter) {
              ret = strstr(fname, filter);
              if (ret == NULL) {
                fname[0] = '\0';
              }
            }
        } else {
          fname[0] ='\0';
        }
       if (strcasecmp(fname, "") != 0) {
            if (echo || DEBUG) {
              Serial.println(dirSep + entry.name());
            }
            if (index < max) {
              strcpy(files[index], entry.name());
              index += 1;
            }
       }
     }
     entry.close();
   }
   return index;
}
