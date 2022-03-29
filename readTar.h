/* functions used to read and print data about Tar */

/* prints time in yyyy-mm-dd hh:mm format given seconds since epcoch*/
void printTimeFormat(int seconds);

/* given type of file, and permissions given as string */
void printPermissions(char typeFlagm, char *permissions);

/* print file names */
int printFile(char *readBuf, char **fileList, int numSize, int vFlag);

/* print the Table of content, if files specified( numSize > 0)
 * then print those files/Dir only, else print all files */
void printTableContent(int tarFile, char vFlag, char sFlag, char **fileList,
                       int numSize, char *tarFileName);
