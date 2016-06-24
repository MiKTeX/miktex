#include <mpfr.h>
#include <string.h>

int
main (int argc, char **argv)
{
  mpfr_t a;
  mpfr_t b;
  mpfr_t c;
  char * lpsz;
  mpfr_exp_t exp = 0;

  int num = 100 * MPFR_VERSION_MAJOR + 10 * MPFR_VERSION_MINOR + MPFR_VERSION_PATCHLEVEL;
  if (!(num >= 312 && strlen(mpfr_get_version()) > 0))
  {
    return 1;
  }
  mpfr_inits2(32, a, b, c, NULL);
  mpfr_set_str(a, "3.1415926535897932384626433832795028841971693993751058209749445923078164062862", 10, MPFR_RNDN);
  mpfr_set_si(b, 12345678);
  mpfr_mul(c, a, b, MPFR_RNDN);
  lpsz = mpfr_get_str(NULL, &exp, 10, 13, c, MPFR_RNDN);
  if (strcmp(lpsz, "3878509131250") != 0)
  {
    return 1;
  }
  return 0;
}
