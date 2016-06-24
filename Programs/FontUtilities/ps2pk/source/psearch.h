/* FILE:    psearch.h
 * PURPOSE: PATH search module
 * VERSION: 1.0 (Oct. 1995)
 * AUTHOR:  Piet Tutelaers
 * (see README for license)
 */

#include <sys/types.h>
#include <sys/stat.h>

/*
 * This function initiates a path by using a built-in <deFault> and
 * a PATH environment variable that can be set at run time by the user.
 * A typical usage is
 *    userenv = path(".:/someplace", getenv("USERENV"))
 *
 * If USERENV is not set getenv("USERENV") returns NULL and the returned
 * path equals the value of the first argument. Otherwise the returned
 * path is the value of second argument in which empty path elements are
 * replaced by <deFault>. For example when USERENV is set to "~/myplace:"
 * the resulting path will be "~myplace:.:/someplace".
 */
char *path(char *deFault, char *env);

/*
 * This function substitutes the <patterns> encountered in <str>
 * with their corresponding <value>. Recognized patterns are %b, %d,
 * %f, %F and %m. They will be replaced with their corresponding integer
 * (%b, %d) or string values (%f, %F and %m). These values can be used as
 * placeholders for basic resolution (%b), resolution (%d), font name
 * (%f or %F) or mode definition (%m). The %F is a special case for
 * representing the fontname leftadjusted in a string of eight characters.
 */
void substitute(char *str, char *patterns, ...);

#define NONFATAL 0
#undef FATAL
#define FATAL    1
/*
 * Look for a <file> in <path>. Return absolute filename if found.
 * If not found return NULL (if <terminate> is NONFATAL) or terminate
 * program with appropriate message (if <terminate> is FATAL).
 * On UNIX ~ or ~someone in paths are evaluated to the proper HOME
 * directories.
 * When the path does contain recursion characters ("//" on UNIX and
 * "!!" on MSDOS) directories are searched recursively.
 */
char *search_file(char *path, char *file, int terminate);

/* set global paramaters for PK font search */
void init_pksearch(int bdpi, char *mode, int margin, char *tolerance);

/*
 * Look for a PK <font> of resolution <dpi> in <path>. To avoid floating
 * around lots of different font sizes TeX prefers to work with whole
 * and half magsteps. So instead of using a 330 dpi font for a 11 point
 * version of a 300 dpi font we would prefer the 329 dpi size because
 * this size matches magstep 0.5 of 300 (329 = int(300 * 1.2 ** 0.5).
 * The process of truncating real font sizes to magsteps can be
 * controlled by the global parameter <tolerance> which may be an
 * expression of the actual value of <dpi> itself. The value dvips(1)
 * uses for <tolerance> is 1 + <dpi> / 500 allowing more tolerance in
 * bigger sizes. Due to round-off different DVI drivers choose different
 * <dpi> sizes. With <margin> we can define what font sizes are looked
 * at whenever size <dpi> is wanted. With <margin> set to zero only the
 * exact size is looked at, while a value of one would allow for a
 * round-off of one. Both <margin> and <tolerance> can be set before a
 * search starts otherwise they get their default values (zero) with
 * init_pksearch().
 *
 * When <path> does contain placeholders like %b, %d, %f, %p and %m
 * they will be substituted by their corresponding values _bdpi, <dpi>,
 * <font>, "pk" and _mode respectivally. This allows to define paths 
 * for a whole range of printers. Occurrences off ~ or * ~user in <path>
 * will be substituted by the corresponding HOME directories on UNIX.
 * 
 * The result of search_pkfile() is a pointer to the name of <font> when
 * one is found otherwise NULL.
 */
char *search_pkfile(char *path, char *font, int dpi);

/*
 * evaluate <expr> after substituting %d with <dpi>. Return the
 * integer result or abort with message when invalid expression.
 */
int evaluate(char *expr, int dpi);

/*
 * Round <dpi> to a whole or half magstep of <bdpi> if the distance of
 * <dpi> is within a certain <tolerance> from it. The value of <bdpi> and
 * <tolerance> should be set with init_pksearch().
 */
int nearesthalf(int dpi);

extern int tilde;

/*
 * During scanning of paths we often need to replace ~user with user's
 * HOME directory and ~ with HOME directory. Subtilde() copies the actual
 * value of `~' or `~user' from <path> into <fn>. The pointers <fn> and
 * <path> are `updated'.
 */
void subtilde(char **fn, char **path);

/* Is <name> an absolute filename? */
int absname(char *name);

/*
 * Look for font libraries in <path> (ex.: c:\emtex\texfonts;fli_0;fli_1).
 * Run <font()> function for each .fli file found in <path>. Continue
 * if this function returns 0, stop when if this function return a 1.
 * In the last case the name of the FLI file is returned.
 */
char *search_flipath(char *flipath, int (*font)(char *, int));

