/*
 *   errormsg.h
 *
 *   This file is part of the ttf2pk package.
 *
 *   Copyright 1997-1999, 2000 by
 *     Frederic Loyer <loyer@ensta.fr>
 *     Werner Lemberg <wl@gnu.org>
 */

#ifndef ERRORMSG_H
#define ERRORMSG_H

extern const char *progname;         /* the origin of the error/warning message */

void oops(const char *message,
          ...);
void boops(const char *buffer,
           size_t offset,
           const char *message,
           ...);
void warning(const char *message,
             ...);

#endif /* ERRORMSG_H */


/* end */
