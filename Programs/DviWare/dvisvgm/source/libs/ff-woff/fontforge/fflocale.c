#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fflocale.h"

void switch_to_c_locale(ff_locale_t * tmplocale_p, ff_locale_t * oldlocale_p) {
#ifdef HAVE_USELOCALE
  *tmplocale_p = newlocale(LC_NUMERIC_MASK, "C", NULL);
  if (*tmplocale_p == NULL)
    fprintf(stderr, "Failed to create temporary locale.\n");
  else if ((*oldlocale_p = uselocale(*tmplocale_p)) == NULL) {
    fprintf(stderr, "Failed to change locale.\n");
    freelocale(*tmplocale_p); *tmplocale_p = NULL;
  }
#else
  *tmplocale_p = setlocale(LC_NUMERIC, "C");
  if (*tmplocale_p)
	  *oldlocale_p = strdup(*tmplocale_p);
  else {
    *oldlocale_p = NULL;
    fprintf(stderr, "Failed to change locale.\n");
  }
#endif
}

void switch_to_old_locale(ff_locale_t * tmplocale_p, ff_locale_t * oldlocale_p) {
#ifdef HAVE_USELOCALE
  if (*oldlocale_p)
    uselocale(*oldlocale_p);
  else
    uselocale(LC_GLOBAL_LOCALE);
  *oldlocale_p = NULL; // This ends the lifecycle of the temporary old locale storage.
  if (*tmplocale_p) {
    freelocale(*tmplocale_p);
    *tmplocale_p = NULL;
  }
#else
  if (*oldlocale_p) {
    setlocale(LC_NUMERIC, *oldlocale_p);
    free(*oldlocale_p);
    *oldlocale_p = NULL;
  }
#endif
}
