#include <sys/stat.h>
#include <sys/types.h>
/* functions for creating a tar file */

void writeFileToTar(int fd, char *path, struct stat *buf, char verbose);
