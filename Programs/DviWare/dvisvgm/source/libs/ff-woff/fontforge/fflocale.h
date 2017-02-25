#ifndef FFLOCALE_H
#define FFLOCALE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <locale.h>
#ifdef HAVE_XLOCALE_H
#include <xlocale.h>
#endif

#ifdef HAVE_USELOCALE
typedef locale_t ff_locale_t;
#else
typedef char* ff_locale_t;
#endif // HAVE_USELOCALE

void switch_to_c_locale(ff_locale_t *tmplocale_p, ff_locale_t *oldlocale_p);
void switch_to_old_locale(ff_locale_t *tmplocale_p, ff_locale_t *oldlocale_p);

#endif /* FFLOCALE_H */

