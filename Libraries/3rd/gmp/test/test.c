#include <gmp.h>
#include <string.h>

int
main (int argc, char **argv)
{
  int num = 100 * __GNU_MP_VERSION + 10 * __GNU_MP_VERSION_MINOR + __GNU_MP_VERSION_PATCHLEVEL;
  return (num >= 600 && strlen(gmp_version) > 0) ? 0 : 1;
}
