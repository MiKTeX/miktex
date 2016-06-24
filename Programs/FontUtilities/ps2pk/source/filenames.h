/* FILE:	filenames.h
 * PURPOSE:	functions for handling file names
 */

/*
 * Determine extension of <str>.
 */
char *extension(char *str);

/* 
 * Basename is the functional equivalent of BSD's basename utility.
 * When the suffix is NULL no suffix removal is done. When needed
 * a new string is allocated so that the original value of str is
 * not changed.
 */   
char *basename(char *str, const char *suffix);

/*
 * Newname builds a new filename by adding or replacing the extension
 * of the given <name> with the new supplied <ext>.
 */
char *newname(char *name, const char *ext);

/*
 * ps_resource returns true if name can be treated as a PS resource
 */
int ps_resource(const char *name);
