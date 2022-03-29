#include <sys/stat.h>
#define BUFF_SIZE 512
#define OCTAL 8
#define VPERMISSSION 10
#define START_OF_YEAR 1900
#define START_OF_MONTH 1

/* size of data */
#define MODE_BUFSIZE 11
#define TYPEFLAG_DIR '5'
#define TYPEFLAG_REG '0'
#define TYPEFLAG_REGALT '\0'
#define TYPEFLAG_LNK '2'
#define MPATH_MAX 256
#define BLOCK_SIZE 512
#define SIZE_NAME 100
#define SIZE_MODE 8
#define SIZE_UID 8
#define SIZE_GID 8
#define SIZE_SIZE 12
#define SIZE_MTIME 12
#define SIZE_CHKSUM 8
#define SIZE_TYPEFLAG 1
#define SIZE_LINKNAME 100
#define SIZE_MAGIC 6
#define SIZE_VERSION 2
#define SIZE_UNAME 32
#define SIZE_GNAME 32
#define SIZE_DEVMAJOR 8
#define SIZE_DEVMINOR 8
#define SIZE_PREFIX 155

/* offset of data in header */
#define OFFSET_NAME 0
#define OFFSET_MODE 100
#define OFFSET_UID 108
#define OFFSET_GID 116
#define OFFSET_SIZE 124
#define OFFSET_MTIME 136
#define OFFSET_CHKSUM 148
#define OFFSET_TYPEFLAG 156
#define OFFSET_LINKNAME 157
#define OFFSET_MAGIC 257
#define OFFSET_VERSION 263
#define OFFSET_UNAME 265
#define OFFSET_GNAME 297
#define OFFSET_DEVMAJOR 329
#define OFFSET_DEVMINOR 337
#define OFFSET_PREFIX 345

/* header data struct */
typedef struct {
  char name[SIZE_NAME + 1];
  char mode[SIZE_MODE];
  char uid[SIZE_UID];
  char gid[SIZE_GID];
  char size[SIZE_SIZE];
  char mtime[SIZE_MTIME];
  char chksum[SIZE_CHKSUM];
  char typeflag[SIZE_TYPEFLAG];
  char linkname[SIZE_LINKNAME + 1];
  char magic[SIZE_MAGIC];
  char version[SIZE_VERSION];
  char uname[SIZE_UNAME + 1];
  char gname[SIZE_GNAME + 1];
  char devmajor[SIZE_DEVMAJOR];
  char devminor[SIZE_DEVMINOR];
  char prefix[SIZE_PREFIX + 1];
} Header;

/* header functions */

/* reads contents of Buff into header struct */
void readHeaderIntoStruct(Header *h, char *readBuf);

/* Error functions */

/* prints specified str into stderr */
void printErrorStr(char *filepath, char *strErr);
/* fails with Usage error */
void failWithUsage(char *filepath);
/*  fails, and prints fileName in stderr */
void perrWith(char *strErr);

/* formating functions */
void getOctalStringRightAligned(int x, int bufLen, char *buf);
void getOctalStringLeftAligned(int x, int bufLen, char *buf);
void copyStr(char *dest, int destSize, char *src);
void copyStrWithOffset(char *dest, int destOffset, char *src, int srcSize);
mode_t getModeFromStr(char *buf, char typeflag);
void getStrMode(mode_t mode, char *buf);

/* tar utility functions */

/* checks if a file is within the printList */
int canPrint(char *lookFor, char **fileList, int numSize);

/* writes an empty block */
void writeEmptyBlock(int fd);

int checkHeader(char *header, char sFlag);

void readBlock(int fd, char *buf, char *tarFileName);
