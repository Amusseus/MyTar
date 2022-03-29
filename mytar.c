#include "createTar.h"
#include "extractTar.h"
#include "readTar.h"
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
#include <sys/types.h>
#include <unistd.h>

/* functions for mytar */

/* traverses given path */
void traversePath(int fd, char *path, int loc, int isDIR, char verbose);

/* usage mytar [ctxvS]f tarfile [ path [ ... ] ] */
/* options c,t, and x are mutually exclusive */
/* f is required in this implementation of mytar */
int main(int argc, char *argv[]) {
  int i;                    /* looping var */
  char mainOpt = 0;         /* either c, t, or x */
  char foundMainOpt = 0;    /* found main opt */
  char vFlag = 0;           /* 1 if included, 0 if not */
  char SFlag = 0;           /* 1 if included, 0 if not */
  char foundUnknownOpt = 0; /* encountered unknown variable */
  char foundF = 0;          /* found the f option */
  int travOpt;              /* used to traverse cmd line options */
  char *archiveName;        /* holds name of tar file */
  int tarFile;              /* file descriptor of the tarfile */
  char **fileList = NULL;   /* list of all paths given */
  int fileListNum = 0;      /* number of all paths given */

  /* failure messages */
  char noOpt[] = "you must specify atleast one of the 'ctx' options.";
  char weirdOpt[] = "unrecognized option";
  char noMainOPt[] = "you must choose one of the 'ctx' options.";
  char tooManyMainOpt[] = "you may only choose one of the 'ctx' options.";
  char noArchiveName[] = "option 'f' requires an archive name";
  /* path file */
  /* program name */
  char *prgName = argv[0];

  /* parsing the commnand line */
  if (argc > 1) {
    /* size of of options string, must be inputted after ./mytar */
    travOpt = strlen(argv[1]);
    /* parse through all options */
    for (i = 0; i < travOpt; i++) {
      switch (argv[1][i]) {
      case 'c':
        mainOpt = 'c';
        foundMainOpt++;
        break;
      case 't':
        mainOpt = 't';
        foundMainOpt++;
        break;
      case 'x':
        mainOpt = 'x';
        foundMainOpt++;
        break;
      case 'v':
        vFlag = 1;
        break;
      case 'S':
        SFlag = 1;
        break;
      case 'f':
        foundF = 1;
        /* get tarfile Name */
        if (argc > 2) {
          archiveName = argv[2];
        }
        /* print error if no file found */
        else {
          printErrorStr(prgName, noArchiveName);
          foundUnknownOpt = 1;
        }
        break;
        /* error message if unknown option encountered */
      default:
        /* print unknown option encountered */
        /* prints -> /filepath: statement 'char encountered'. */
        fprintf(stderr, "%s: %s '%c'.\n", prgName, weirdOpt, argv[1][i]);
        foundUnknownOpt = 1;
        break;
      }
    }
    /* checking for error in input */

    /* if no main options found */
    if (!foundMainOpt) {
      printErrorStr(prgName, noMainOPt);
      failWithUsage(prgName);
    }
    /* more than one main option found */
    else if (foundMainOpt > 1) {
      printErrorStr(prgName, tooManyMainOpt);
      failWithUsage(prgName);
    } else if (foundUnknownOpt || !foundF) {
      failWithUsage(prgName);
    }
  }
  /* no argument given besides ./mytar */
  else {
    printErrorStr(prgName, noOpt);
    failWithUsage(prgName);
  }

  /* obtaining all paths from command line input  */
  if (argc - 3) {
    fileList = (char **)malloc(sizeof(char *) * (argc - 3));
    for (i = 3; i < argc; i++) {
      fileList[i - 3] = argv[i];
      fileListNum++;
    }
  }

  /* Pick between three mytar Functions */
  switch (mainOpt) {

  /* create tar file */
  case 'c':
    /* create tarFile to write to */
    tarFile = open(archiveName, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
    /* traverse every path given and add to tarFile */
    for (i = 3; i < argc; i++) {
      char *path = argv[i];
      struct stat buf; /* holds stat info */
      /* exit with failure if lstat fails */
      if (lstat(path, &buf) == 0) {
        traversePath(tarFile, path, strlen(path), S_ISDIR(buf.st_mode), vFlag);
      } else {
        perror(path);
      }
    }
    /* write EOA for tar file */
    writeEmptyBlock(tarFile);
    writeEmptyBlock(tarFile);
    break;

  /* read table of content of tar file */
  case 't':
    /* open tarfile to read, fail if can't be opened */
    if (-1 == (tarFile = open(archiveName, O_RDONLY))) {
      perrWith(archiveName);
    }
    /* print table of contents, print all if fileListNum = NULL */
    printTableContent(tarFile, vFlag, SFlag, fileList, fileListNum,
                      archiveName);
    break;

  /* extract from table of content */
  case 'x':
    /* open tarFile to read, fail if cannot be opened */
    if (-1 == (tarFile = open(archiveName, O_RDONLY))) {
      perrWith(archiveName);
    }
    /* extract all files, unless files have been specified in fileListNum */
    extractTar(tarFile, vFlag, SFlag, fileList, fileListNum, archiveName);
    break;
  }

  return 0;
}

/* recursively traverses all the way through given path */
void traversePath(int fd, char *path, int loc, int isDIR, char verbose) {
  struct stat buf; /* holds stat info */
  /* exit with failure if lstat fails */
  if (lstat(path, &buf) != 0) {
    perror(path);
    return;
  }
  writeFileToTar(fd, path, &buf, verbose);
  /* if specified path is DIR */
  if (S_ISDIR(buf.st_mode)) {
    /* Open DIR */
    DIR *d = opendir(path);
    /* if DIR cannot be opened */
    if (d == NULL) {
      perror(path);
      return;
    }
    struct dirent *rd;
    int i, len;
    /* read all the contents of DIR */
    while ((rd = readdir(d))) {
      /* lstat fails */
      if (lstat(path, &buf) != 0) {
        perror(path);
        return;
      }
      /* ignore the current and parent dir when reading */
      if (strcmp(rd->d_name, ".") == 0 || strcmp(rd->d_name, "..") == 0) {
        continue;
      }

      len = strlen(rd->d_name);
      path[loc] = '/'; /* add / to end of current path */
      /* insert file/Dir name into path */
      for (i = 0; i < len; ++i) {
        path[loc + i + 1] = rd->d_name[i];
      }
      /* terminate with \0 char */
      path[loc + len + 1] = '\0';
      /* traverse through DIR */
      traversePath(fd, path, loc + len + 1, isDIR, verbose);
      path[loc] = '\0';
    }
  }
  /* if not a DIR */
  else {
  }
}
