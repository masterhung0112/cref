struct path {
	bool isDir: 1;
};

int getPathInfo(char *path, path *info);
char *getRelPath(char *path, char *path1);
char *getAbspath(char *path);
char joinPath(char *path, char *path1);
bool isPathRel(char *path);
