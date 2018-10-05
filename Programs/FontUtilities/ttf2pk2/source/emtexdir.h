/* emtexdir.h -- written by Eberhard Mattes, donated to the public domain */

#if !defined (_EMTEXDIR_H)
#define _EMTEXDIR_H

#if defined (__cplusplus)
extern "C" {
#endif


/* Flags for emtex_dir_setup */

#define EDS_ONESUBDIR 0x0001
#define EDS_ALLSUBDIR 0x0002
#define EDS_BANG      0x0004

/* Flags for emtex_dir_find */

#define EDF_CWD       0x0001
#define EDF_FSLASH    0x0002
#define EDF_TRACE     0x8000

/* Methods for emtex_dir_trunc */

#define EDT_5_PLUS_3  0         /* 5+3.3 */
#define EDT_8         1         /* 8.3 */

/* Return values for emtex_dir_trunc */

#define EDT_UNCHANGED 0         /* Path name not changed */
#define EDT_CHANGED   1         /* Path name truncated */
#define EDT_TOOLONG   2         /* Path name too long */
#define EDT_INVALID   3         /* Path name invalid */

struct emtex_dir
{
  char **list;
  int used;
  int alloc;
};

extern void (*emtex_dir_find_callback)(const char *name, int ok);

extern int setup_list(struct emtex_dir *dst, char *path,
                      const char *list, unsigned flags);

int emtex_dir_setup (struct emtex_dir *ed, const char *env, const char *dir,
    unsigned flags);

int emtex_dir_find (char *path, size_t path_size, const struct emtex_dir *ed,
    const char *fname, unsigned flags);

int emtex_dir_trunc (char *dst, size_t dst_size, const char *src,
    unsigned flags, int method);

#if defined (__cplusplus)
}
#endif

#endif /* !defined (_EMTEXDIR_H) */
