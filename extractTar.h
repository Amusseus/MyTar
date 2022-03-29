/* extracts from tarFile given the listFiles to extract,
 * extracts all file if listFiles is NULL */
void extractTar(int TarFile, char vFlag, char sFlag, char **listFiles,
                int numListFiles, char *tarfilename);
