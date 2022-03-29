#include "readTar.h"
#include "util.h"
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void printTimeFormat(int seconds) {
  /* generating a struct that holds current date since epoch */
  time_t sec = seconds;
  struct tm *tk = localtime(&sec);
  /* data to print */
  int year = tk->tm_year + START_OF_YEAR;
  int month = tk->tm_mon + START_OF_MONTH;
  int mday = tk->tm_mday;
  int hour = tk->tm_hour;
  int min = tk->tm_min;
  /* prints in format */
  printf("%d-%02d-%02d %02d:%02d ", year, month, mday, hour, min);
}

void printVerbosePermission(char typeflag, char *permissions) {
  char buf[MODE_BUFSIZE];
  getStrMode(getModeFromStr(permissions, typeflag), buf);
  printf("%s", buf);
}

int printFile(char *readBuf, char **fileList, int numSize, int vFlag) {
  int offsetBlock;  /* # of blocks till next Header */
  int fileSizeInt;  /* size of the file data */
  char *strtoulPtr; /* used for strtoul() */

  /* create header and read into it */
  Header *r = calloc(1, sizeof(Header));
  readHeaderIntoStruct(r, readBuf);
  char fileName[SIZE_NAME + SIZE_PREFIX + 2] = {0};

  /* Construct fileName */
  if (strlen(r->prefix)) {
    strcat(fileName, r->prefix);
    strcat(fileName, "/");
  }
  strcat(fileName, r->name);

  /* calculate offset */
  fileSizeInt = strtoul(r->size, &strtoulPtr, OCTAL);
  offsetBlock = fileSizeInt / BUFF_SIZE;
  if (fileSizeInt % BUFF_SIZE) {
    offsetBlock++;
  }

  /* if file can be printed */
  if (canPrint(fileName, fileList, numSize)) {
    /* verbose version */
    if (vFlag) {
      int timeSec = strtoul(r->mtime, &strtoulPtr, OCTAL);
      printVerbosePermission(r->typeflag[0], r->mode);
      printf(" %s/%s %8d ", r->uname, r->gname, fileSizeInt);
      printTimeFormat(timeSec);
      printf("%s\n", fileName);
    }
    /* standard version */
    else {
      printf("%s\n", fileName);
    }
  }

  free(r);
  return offsetBlock;
}

void printTableContent(int tarFile, char vFlag, char sFlag, char **fileList,
                       int numSize, char *tarFileName) {
  int offset;
  char readBuf[BUFF_SIZE];
  /* reads header into Buff */
  readBlock(tarFile, readBuf, tarFileName);
  /* loop till EOA */
  while (readBuf[0] != 0) {
    if (-1 == checkHeader(readBuf, sFlag)) {
      perrWith("Bad tar file");
    }
    /* print file */
    offset = printFile(readBuf, fileList, numSize, vFlag);
    /* seek the next header block */
    lseek(tarFile, offset * BUFF_SIZE, SEEK_CUR);
    /* read the header */
    readBlock(tarFile, readBuf, tarFileName);
  }
}
