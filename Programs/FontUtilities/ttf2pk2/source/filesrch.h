/*
 *   filesrch.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000, 2002 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef FILESRCH_H
#define FILESRCH_H

#include "ttf2tfm.h"

/*
 *   Arguments are the full program name and two identification strings
 *   (for the program and the environment).
 */

void TeX_search_init(char *exec_name,
                     const char *program_identifier,
                     const char *env_identifier);

/*
 *   The next function returns a version string.
 */

const char *TeX_search_version(void);

/*
 *   The following functions take a file name (either relative or absolute),
 *   probably append a default extension, and return the complete path to the
 *   file.
 */

char *TeX_search_tfm(char **name);
char *TeX_search_encoding_file(char **name);
char *TeX_search_replacement_file(char **name);
char *TeX_search_sfd_file(char **name);
char *TeX_search_map_file(char **name);
char *TeX_search_config_file(const char **name);
char *TeX_search_ttf_file(char **name);

void get_tfm_fullname(Font *fnt);
void handle_extension(char **stringp,
                      const char *extension);

#endif /* FILESRCH_H */


/* end */
