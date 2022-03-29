#include "extractTar.h"
#include "util.h"
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>
#define BUFF_SIZE 512
#define OCTAL 8

int ensurePath(char *pathname, int len) {
  char target[len];
  int i, lastSlash;
  for (i = len - 2; i >= 0; --i) {
    if (pathname[i] == '/') {
      break;
    }
  }
  if (i < 1) {
    return 0;
  }
  lastSlash = i;
  for (i = 0; i < lastSlash; ++i) {
    target[i] = pathname[i];
  }
  target[i] = '\0';
  if (NULL == opendir(target)) {
    ensurePath(target, lastSlash);
    if (-1 == mkdir(target, 0700)) {
      perror(target);
      return -1;
    }
  }
  return 0;
}

void createSpecified(int tarFile, char *readBuff, char **listFiles,
                     int numListFiles, int vFlag) {
  int fileSizeInt; /* size of file */
  int offsetBlock; /* num of data blocks to read */
  char *strtoulPtr;
  char fileName[MPATH_MAX] = {0};
  /* read data from header into struct */
  Header *h = calloc(1, sizeof(Header));
  readHeaderIntoStruct(h, readBuff);
  /* calculate file Size and offsetBlock */
  fileSizeInt = strtoul(h->size, &strtoulPtr, OCTAL);
  offsetBlock = fileSizeInt / BUFF_SIZE;
  if (fileSizeInt % BUFF_SIZE) {
    offsetBlock++;
  }
  /* constructing file name */
  if (strlen(h->prefix)) {
    strcat(fileName, h->prefix);
    strcat(fileName, "/");
  }
  strcat(fileName, h->name);

  int sec = strtoul(h->mtime, &strtoulPtr, OCTAL);

  if (canPrint(fileName, listFiles, numListFiles)) {
    if (0 == ensurePath(fileName, strlen(fileName))) {
      if (vFlag) {
        printf("%s\n", fileName);
      }
      mode_t mode = getModeFromStr(h->mode, h->typeflag[0]);
      switch (h->typeflag[0]) {
      case TYPEFLAG_REG:
      case TYPEFLAG_REGALT: {
        int fd;
        if (-1 == (fd = open(fileName, O_CREAT | O_WRONLY | O_TRUNC, mode))) {
          perror(fileName);
        }
        int i;
        char writeBuf[BUFF_SIZE];
        for (i = 0; i < offsetBlock; ++i) {
          if (-1 == read(tarFile, writeBuf, BUFF_SIZE)) {
            perrWith(fileName);
          }
          if (-1 == write(fd, writeBuf,
                          fileSizeInt < BUFF_SIZE ? fileSizeInt : BUFF_SIZE)) {
            perrWith(fileName);
          }
          fileSizeInt -= BUFF_SIZE;
        }
        if (-1 == close(fd)) {
          perror(fileName);
        }
        break;
      }
      case TYPEFLAG_LNK: {
        int fd;
        if (-1 == (fd = creat(h->linkname, 0700))) {
          perror(h->linkname);
        }
        if (-1 == close(fd)) {
          perror(fileName);
        }
        if (-1 == symlink(h->linkname, fileName)) {
          perror(h->linkname);
        }
        if (-1 == remove(h->linkname)) {
          perror(h->linkname);
        }
        break;
      }
      case TYPEFLAG_DIR:
        if (-1 == mkdir(fileName, mode)) {
          perror(fileName);
        }
        break;
      }
      struct utimbuf *buf = calloc(1, sizeof(struct utimbuf));
      buf->modtime = sec;
      if (-1 == utime(fileName, buf)) {
        perror(fileName);
      }
      free(buf);
    }
  } else {
    lseek(tarFile, offsetBlock * BUFF_SIZE, SEEK_CUR);
  }
  free(h);
}

void extractTar(int TarFile, char vFlag, char sFlag, char **listFiles,
                int numListFiles, char *tarFileName) {
  char readBuf[BUFF_SIZE];
  while (1) {
    int r = read(TarFile, readBuf, BUFF_SIZE);
    if (-1 == r) {
      perrWith(tarFileName);
    } else if (r == 0 || readBuf[0] == 0) {
      break;
    } else if (512 != r) {
      printf("Bad tar file\n");
      exit(EXIT_FAILURE);
    }
    checkHeader(readBuf, sFlag);
    createSpecified(TarFile, readBuf, listFiles, numListFiles, vFlag);
  }
}
