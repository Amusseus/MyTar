#include "createTar.h"
#include "readTar.h"
#include "util.h"
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define getDevMajor(x) ((int32_t)(((u_int32_t)(x) >> 24) & 0xff))
#define getDevMinor(x) ((int32_t)((x)&0xffffff))

void copyBuf(char *buf, int *cursor, char *src, int srcSize) {
  copyStrWithOffset(buf, *cursor, src, srcSize);
  *cursor = *cursor + srcSize;
}

void writeHeader(int fd, Header *header) {
  char buf[BLOCK_SIZE];
  int i = 0;
  copyBuf(buf, &i, header->name, SIZE_NAME);
  copyBuf(buf, &i, header->mode, SIZE_MODE);
  copyBuf(buf, &i, header->uid, SIZE_UID);
  copyBuf(buf, &i, header->gid, SIZE_GID);
  copyBuf(buf, &i, header->size, SIZE_SIZE);
  copyBuf(buf, &i, header->mtime, SIZE_MTIME);
  copyBuf(buf, &i, header->chksum, SIZE_CHKSUM);
  copyBuf(buf, &i, header->typeflag, SIZE_TYPEFLAG);
  copyBuf(buf, &i, header->linkname, SIZE_LINKNAME);
  copyBuf(buf, &i, header->magic, SIZE_MAGIC);
  copyBuf(buf, &i, header->version, SIZE_VERSION);
  copyBuf(buf, &i, header->uname, SIZE_UNAME);
  copyBuf(buf, &i, header->gname, SIZE_GNAME);
  copyBuf(buf, &i, header->devmajor, SIZE_DEVMAJOR);
  copyBuf(buf, &i, header->devminor, SIZE_DEVMINOR);
  copyBuf(buf, &i, header->prefix, SIZE_PREFIX);
  int checksum = 0;
  for (i = 0; i < BLOCK_SIZE; ++i) {
    checksum += buf[i];
  }
  getOctalStringLeftAligned(checksum, SIZE_CHKSUM - 1, header->chksum);
  i = SIZE_NAME + SIZE_MODE + SIZE_UID + SIZE_GID + SIZE_SIZE + SIZE_MTIME;
  copyBuf(buf, &i, header->chksum, SIZE_CHKSUM);
  write(fd, buf, BLOCK_SIZE);
}

int getMode(mode_t mode) {
  int result = 0;
  result += (mode & S_ISUID) ? S_ISUID : 0;
  result += (mode & S_ISGID) ? S_ISGID : 0;
  result += (mode & S_ISVTX) ? S_ISVTX : 0;
  result += (mode & S_IRUSR) ? S_IRUSR : 0;
  result += (mode & S_IWUSR) ? S_IWUSR : 0;
  result += (mode & S_IXUSR) ? S_IXUSR : 0;
  result += (mode & S_IRGRP) ? S_IRGRP : 0;
  result += (mode & S_IWGRP) ? S_IWGRP : 0;
  result += (mode & S_IXGRP) ? S_IXGRP : 0;
  result += (mode & S_IROTH) ? S_IROTH : 0;
  result += (mode & S_IWOTH) ? S_IWOTH : 0;
  result += (mode & S_IXOTH) ? S_IXOTH : 0;
  return result;
}

/* writes the files header data */
void writeFileToTar(int fd, char *path, struct stat *buf, char verbose) {
  int i, l = strlen(path), j;
  int isDir = S_ISDIR(buf->st_mode);
  int isReg = 0, isLink = 0;
  if (isDir) {
    path[l] = '/';
    path[++l] = '\0';
  }
  Header *header = calloc(1, sizeof(Header));
  int splitLoc = -1;
  if (l > 256) {
    printf("Path too long. Skipping...\n");
    return;
  }
  /*
   * if splitLoc is not -1, path[splitLoc] should be a slash
   * name should be [splitLoc + 1, l)
   * prefix should be [0, splitLoc)
   */
  if (l > 100) {
    for (i = l - 1; i >= 0; --i) {
      if (path[i] == '/') {
        splitLoc = i;
      }
      if (l - i > 100) {
        if (splitLoc < 0) {
          printf("Name too long. Skipping...\n");
          return;
        }
        break;
      }
    }
  }
  j = 0;
  for (i = splitLoc + 1; i < l; ++i) {
    header->name[j++] = path[i];
  }
  for (i = 0; i < splitLoc; ++i) {
    header->prefix[i] = path[i];
  }
  getOctalStringRightAligned(getMode(buf->st_mode), SIZE_MODE - 1,
                             header->mode);
  getOctalStringLeftAligned(buf->st_uid, SIZE_UID - 1, header->uid);
  getOctalStringLeftAligned(buf->st_gid, SIZE_GID - 1, header->gid);
  getOctalStringRightAligned((isDir || isLink) ? 0 : buf->st_size,
                             SIZE_SIZE - 1, header->size);
  getOctalStringLeftAligned(buf->st_mtime, SIZE_MTIME - 1, header->mtime);
  for (i = 0; i < SIZE_CHKSUM; ++i) {
    header->chksum[i] = ' ';
  }
  if (isDir) {
    header->typeflag[0] = TYPEFLAG_DIR;
  } else if ((isReg = S_ISREG(buf->st_mode))) {
    header->typeflag[0] = TYPEFLAG_REG;
  } else if ((isLink = S_ISLNK(buf->st_mode))) {
    header->typeflag[0] = TYPEFLAG_LNK;
  }
  readlink(path, header->linkname, SIZE_LINKNAME);
  copyStr(header->magic, SIZE_MAGIC, "ustar");
  copyStr(header->version, SIZE_VERSION, "00");
  struct passwd *pwd = getpwuid(buf->st_uid);
  copyStr(header->uname, SIZE_UNAME, pwd->pw_name);
  struct group *grp = getgrgid(buf->st_gid);
  copyStr(header->gname, SIZE_GNAME, grp->gr_name);
  getOctalStringLeftAligned(getDevMajor(buf->st_dev), SIZE_DEVMAJOR,
                            header->devmajor);
  getOctalStringLeftAligned(getDevMinor(buf->st_dev), SIZE_DEVMINOR,
                            header->devminor);
  if (verbose) {
    char mode[MODE_BUFSIZE];
    struct tm *tk = localtime(&buf->st_mtime);
    getStrMode(buf->st_mode, mode);
    printf("%s %s/%s %lld %04d-%02d-%02d %02d:%02d %s\n", mode, header->uname,
           header->gname, (long long int)buf->st_size, tk->tm_year + 1900,
           tk->tm_mon + 1, tk->tm_mday, tk->tm_hour, tk->tm_min, path);
  }
  writeHeader(fd, header);
  if (isReg) {
    int blockCount = 1, num;
    char blockBuf[BLOCK_SIZE];
    int fdin;
    if (-1 == (fdin = open(path, O_RDONLY))) {
      perrWith(path);
    }
    while ((num = read(fdin, blockBuf, BLOCK_SIZE))) {
      for (i = num; i < BLOCK_SIZE; ++i) {
        blockBuf[i] = 0;
      }
      write(fd, blockBuf, BLOCK_SIZE);
      ++blockCount;
    }
  } else {
  }
}
