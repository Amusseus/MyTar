#include "util.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void readHeaderIntoStruct(Header *h, char *readBuf) {
  memcpy(h->name, &readBuf[OFFSET_NAME], SIZE_NAME);
  memcpy(h->mode, &readBuf[OFFSET_MODE], SIZE_MODE);
  memcpy(h->uid, &readBuf[OFFSET_UID], SIZE_UID);
  memcpy(h->gid, &readBuf[OFFSET_GID], SIZE_GID);
  memcpy(h->size, &readBuf[OFFSET_SIZE], SIZE_SIZE);
  memcpy(h->mtime, &readBuf[OFFSET_MTIME], SIZE_MTIME);
  memcpy(h->chksum, &readBuf[OFFSET_CHKSUM], SIZE_CHKSUM);
  memcpy(h->typeflag, &readBuf[OFFSET_TYPEFLAG], SIZE_TYPEFLAG);
  memcpy(h->linkname, &readBuf[OFFSET_LINKNAME], SIZE_LINKNAME);
  memcpy(h->magic, &readBuf[OFFSET_MAGIC], SIZE_MAGIC);
  memcpy(h->version, &readBuf[OFFSET_VERSION], SIZE_VERSION);
  memcpy(h->uname, &readBuf[OFFSET_UNAME], SIZE_UNAME);
  memcpy(h->gname, &readBuf[OFFSET_GNAME], SIZE_GNAME);
  memcpy(h->devmajor, &readBuf[OFFSET_DEVMAJOR], SIZE_DEVMAJOR);
  memcpy(h->devminor, &readBuf[OFFSET_DEVMINOR], SIZE_DEVMINOR);
  memcpy(h->prefix, &readBuf[OFFSET_PREFIX], SIZE_PREFIX);
}

void printErrorStr(char *filepath, char *strErr) {
  fprintf(stderr, "%s: %s\n", filepath, strErr);
}

void failWithUsage(char *filepath) {
  fprintf(stderr, "uasge: %s [ctxvS]f tarfile [ path [ ... ] ]\n", filepath);
  exit(EXIT_FAILURE);
}

/* prints with specified error */
void failWith(char *strErr) {
  printf("%s\n", strErr);
  exit(EXIT_FAILURE);
}

/* prints err to strerr and exits */
void perrWith(char *strErr) {
  perror(strErr);
  exit(EXIT_FAILURE);
}

void getOctalStringLeftAligned(int x, int bufLen, char *buf) {
  int numLen = 0, tmp = x;
  while (tmp > 0) {
    tmp /= 8;
    ++numLen;
  }
  int i = numLen - 1;
  while (i >= 0) {
    buf[i--] = '0' + x % 8;
    x /= 8;
  }
  for (i = numLen; i < bufLen; ++i) {
    buf[i] = '\0';
  }
}

void getOctalStringRightAligned(int x, int bufLen, char *buf) {
  int i = bufLen - 1;
  while (i >= 0) {
    buf[i--] = '0' + x % 8;
    x /= 8;
  }
}

void copyStr(char *dest, int destSize, char *src) {
  int i = 0;
  if (src != NULL) {
    while (src[i] && i < destSize) {
      dest[i] = src[i];
      ++i;
    }
  }
  for (; i < destSize; ++i) {
    dest[i] = '\0';
  }
}

void writeEmptyBlock(int fd) {
  char buf[BLOCK_SIZE];
  int i;
  for (i = 0; i < BLOCK_SIZE; ++i) {
    buf[i] = 0;
  }
  write(fd, buf, BLOCK_SIZE);
}

void copyStrWithOffset(char *dest, int destOffset, char *src, int srcSize) {
  int i = 0;
  for (; i < srcSize; ++i) {
    dest[i + destOffset] = src[i];
  }
}

void getStrMode(mode_t mode, char *buf) {
  buf[0] = S_ISDIR(mode) ? 'd' : (S_ISLNK(mode) ? 'l' : '-');
  buf[1] = mode & S_IRUSR ? 'r' : '-';
  buf[2] = mode & S_IWUSR ? 'w' : '-';
  buf[3] = mode & S_IXUSR ? 'x' : '-';
  buf[4] = mode & S_IRGRP ? 'r' : '-';
  buf[5] = mode & S_IWGRP ? 'w' : '-';
  buf[6] = mode & S_IXGRP ? 'x' : '-';
  buf[7] = mode & S_IROTH ? 'r' : '-';
  buf[8] = mode & S_IWOTH ? 'w' : '-';
  buf[9] = mode & S_IXOTH ? 'x' : '-';
  buf[10] = '\0';
}

mode_t modeUSR[8] = {
    0,       S_IXUSR,           S_IWUSR,           S_IXUSR | S_IWUSR,
    S_IRUSR, S_IRUSR | S_IXUSR, S_IRUSR | S_IWUSR, S_IRWXU,
};

mode_t modeGRP[8] = {
    0,       S_IXGRP,           S_IWGRP,           S_IXGRP | S_IWGRP,
    S_IRGRP, S_IRGRP | S_IXGRP, S_IRGRP | S_IWGRP, S_IRWXG,
};

mode_t modeOTH[8] = {
    0,       S_IXOTH,           S_IWOTH,           S_IXOTH | S_IWOTH,
    S_IROTH, S_IROTH | S_IXOTH, S_IROTH | S_IWOTH, S_IRWXO,
};

mode_t modeETC[8] = {
    0,       S_ISVTX,           S_ISGID,           S_ISVTX | S_ISGID,
    S_ISUID, S_ISUID | S_ISVTX, S_ISUID | S_ISGID, S_ISUID | S_ISGID | S_ISVTX,
};

mode_t getModeFromStr(char *buf, char typeflag) {
  int i = 0;
  mode_t mode = 0;
  while (buf[i] && buf[i] != ' ') {
    ++i;
  }
  --i;
  mode |= modeOTH[buf[i--] - '0'];
  mode |= modeGRP[buf[i--] - '0'];
  mode |= modeUSR[buf[i--] - '0'];
  if (i > -1) {
    mode |= modeETC[buf[i--] - '0'];
  }
  switch (typeflag) {
  case TYPEFLAG_REG:
  case TYPEFLAG_REGALT:
    return mode | S_IFREG;
  case TYPEFLAG_LNK:
    return mode | S_IFLNK;
  case TYPEFLAG_DIR:
    return mode | S_IFDIR;
  }
  return mode;
}

int canPrint(char *lookFor, char **fileList, int numSize) {
  if (numSize == 0 || fileList == NULL) {
    return 1;
  }
  int i = 0;
  /* check if file is included in list of files to Print */
  for (; i < numSize; i++) {
    if (strncmp(lookFor, fileList[i], strlen(fileList[i])) == 0) {
      return 1;
    }
  }
  return 0;
}

int checkHeader(char *header, char sFlag) {
  int i, checksum = 0;
  char cksum[SIZE_CHKSUM];
  char magic[SIZE_MAGIC];
  int foundUSTAR = 0;
  char *strtoulPtr;
  for (i = 0; i < SIZE_CHKSUM; ++i) {
    cksum[i] = header[i + OFFSET_CHKSUM];
    header[i + OFFSET_CHKSUM] = ' ';
  }
  int headerSum = strtoul(cksum, &strtoulPtr, OCTAL);
  for (i = 0; i < BUFF_SIZE; ++i) {
    checksum += header[i];
  }

  for (i = 0; i < SIZE_MAGIC; ++i) {
    magic[i] = header[i + OFFSET_MAGIC];
  }

  if (sFlag &&
      !(0 == strncmp(magic, "ustar", 5) && magic[SIZE_MAGIC - 1] == '\0')) {
    return -1;
  }
  if (0 == strncmp(magic, "ustar", 5)) {
    foundUSTAR = 1;
  } else if (0 == strncmp(&magic[1], "ustar", 5)) {
    foundUSTAR = 1;
  }

  if (!foundUSTAR) {
    return -1;
  }

  if (headerSum != checksum) {
    return -1;
  }

  /* if S flag avaliable */
  if (sFlag &&
      !(header[OFFSET_VERSION] == '0' && header[OFFSET_VERSION + 1] == '0')) {
    return -1;
  }
  return 0;
}

void readBlock(int fd, char *buf, char *tarFileName) {
  int r = read(fd, buf, BUFF_SIZE);
  if (512 != r) {
    printf("Bad tar file\n");
    exit(EXIT_FAILURE);
  } else if (-1 == r) {
    perrWith(tarFileName);
  }
}
