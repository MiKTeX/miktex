extern char start_time_str[];
extern char *makecfilename(integer s);
extern void getcreationdate(void);
extern void getfilemoddate(integer s);
extern void getfilesize(integer s);
extern void getfiledump(integer s, int offset, int length);
extern void convertStringToHexString(const char *in, char *out, int lin);
extern void getmd5sum(integer s, int file);
