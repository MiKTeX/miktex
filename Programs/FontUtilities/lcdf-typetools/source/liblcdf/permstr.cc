// -*- related-file-name: "../include/lcdf/permstr.hh" -*-

/* permstr.{cc,hh} -- permanent strings
 *
 * Copyright (c) 1998-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <lcdf/permstr.hh>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static PermString::Initializer initializer;

PermString::Doodad PermString::zero_char_doodad = {
    0, 0, { 0, 0 }
};
PermString::Doodad PermString::one_char_doodad[256];
PermString::Doodad* PermString::buckets[NHASH];

PermString::Initializer::Initializer()
{
    static int initialized = 0;
    if (!initialized) {
	PermString::static_initialize();
	initialized = 1;
    }
}

void
PermString::static_initialize()
{
    for (int i = 0; i < 256; i++) {
	one_char_doodad[i].next = 0;
	one_char_doodad[i].length = 1;
	one_char_doodad[i].data[0] = i;
	one_char_doodad[i].data[1] = 0;
    }
}


// This scatter array, and the ideas behind it, are stolen from lcc.
static int scatter[] = {        /* map characters to random values */
        2078917053, 143302914, 1027100827, 1953210302, 755253631,
        2002600785, 1405390230, 45248011, 1099951567, 433832350,
        2018585307, 438263339, 813528929, 1703199216, 618906479,
        573714703, 766270699, 275680090, 1510320440, 1583583926,
        1723401032, 1965443329, 1098183682, 1636505764, 980071615,
        1011597961, 643279273, 1315461275, 157584038, 1069844923,
        471560540, 89017443, 1213147837, 1498661368, 2042227746,
        1968401469, 1353778505, 1300134328, 2013649480, 306246424,
        1733966678, 1884751139, 744509763, 400011959, 1440466707,
        1363416242, 973726663, 59253759, 1639096332, 336563455,
        1642837685, 1215013716, 154523136, 593537720, 704035832,
        1134594751, 1605135681, 1347315106, 302572379, 1762719719,
        269676381, 774132919, 1851737163, 1482824219, 125310639,
        1746481261, 1303742040, 1479089144, 899131941, 1169907872,
        1785335569, 485614972, 907175364, 382361684, 885626931,
        200158423, 1745777927, 1859353594, 259412182, 1237390611,
        48433401, 1902249868, 304920680, 202956538, 348303940,
        1008956512, 1337551289, 1953439621, 208787970, 1640123668,
        1568675693, 478464352, 266772940, 1272929208, 1961288571,
        392083579, 871926821, 1117546963, 1871172724, 1771058762,
        139971187, 1509024645, 109190086, 1047146551, 1891386329,
        994817018, 1247304975, 1489680608, 706686964, 1506717157,
        579587572, 755120366, 1261483377, 884508252, 958076904,
        1609787317, 1893464764, 148144545, 1415743291, 2102252735,
        1788268214, 836935336, 433233439, 2055041154, 2109864544,
        247038362, 299641085, 834307717, 1364585325, 23330161,
        457882831, 1504556512, 1532354806, 567072918, 404219416,
        1276257488, 1561889936, 1651524391, 618454448, 121093252,
        1010757900, 1198042020, 876213618, 124757630, 2082550272,
        1834290522, 1734544947, 1828531389, 1982435068, 1002804590,
        1783300476, 1623219634, 1839739926, 69050267, 1530777140,
        1802120822, 316088629, 1830418225, 488944891, 1680673954,
        1853748387, 946827723, 1037746818, 1238619545, 1513900641,
        1441966234, 367393385, 928306929, 946006977, 985847834,
        1049400181, 1956764878, 36406206, 1925613800, 2081522508,
        2118956479, 1612420674, 1668583807, 1800004220, 1447372094,
        523904750, 1435821048, 923108080, 216161028, 1504871315,
        306401572, 2018281851, 1820959944, 2136819798, 359743094,
        1354150250, 1843084537, 1306570817, 244413420, 934220434,
        672987810, 1686379655, 1301613820, 1601294739, 484902984,
        139978006, 503211273, 294184214, 176384212, 281341425,
        228223074, 147857043, 1893762099, 1896806882, 1947861263,
        1193650546, 273227984, 1236198663, 2116758626, 489389012,
        593586330, 275676551, 360187215, 267062626, 265012701,
        719930310, 1621212876, 2108097238, 2026501127, 1865626297,
        894834024, 552005290, 1404522304, 48964196, 5816381,
        1889425288, 188942202, 509027654, 36125855, 365326415,
        790369079, 264348929, 513183458, 536647531, 13672163,
        313561074, 1730298077, 286900147, 1549759737, 1699573055,
        776289160, 2143346068, 1975249606, 1136476375, 262925046,
        92778659, 1856406685, 1884137923, 53392249, 1735424165,
        1602280572
};


void
PermString::initialize(const char* s, int length)
{
    const unsigned char* m = reinterpret_cast<const unsigned char*>(s);
    const unsigned char* mm;

    if (length < 0)
	length = (s ? strlen(s) : 0);

    if (length == 0) {
	_rep = zero_char_doodad.data;
	return;
    } else if (length == 1) {
	_rep = one_char_doodad[m[0]].data;
	return;
    }

    unsigned hash;
    int l;
    for (hash = 0, l = length, mm = m; l; mm++, l--)
	hash = (hash << 1) + scatter[*mm];
    hash &= (NHASH - 1);

    Doodad *buck;
    for (buck = buckets[hash]; buck; buck = buck->next)
	if (length == buck->length && memcmp(s, buck->data, length) == 0) {
	    _rep = buck->data;
	    return;
	}

    // CANNOT USE new because the structure has variable size.
    buck = (Doodad *)malloc(sizeof(Doodad) + length - 1);
    buck->next = buckets[hash];
    buckets[hash] = buck;
    buck->length = length;
    memcpy(buck->data, s, length);
    buck->data[length] = 0;

    _rep = buck->data;
}

static int pspos;
static int pscap = 64;
static char *psc = (char *)malloc(pscap);

static void
append(const char *s, int len)
{
    if (pspos + len >= pscap) {
	pscap *= 2;
	psc = (char *)realloc(psc, pscap);
    }
    memcpy(psc + pspos, s, len);
    pspos += len;
}

inline static void
extend(int len)
{
    while (pspos + len >= pscap) {
	pscap *= 2;
	psc = (char *)realloc(psc, pscap);
    }
}

PermString
vpermprintf(const char *s, va_list val)
{
    pspos = 0;
    while (1) {

	const char *pct = strchr(s, '%');
	if (!pct) {
	    if (*s)
		append(s, strlen(s));
	    break;
	}
	if (pct != s) {
	    append(s, pct - s);
	    s = pct;
	}

	int iflag = -1;
	while (1)
	    switch (*++s) {

	      case '0':
		/* zeroflag = 1; */
		break;

	      case '1': case '2': case '3': case '4': case '5':
	      case '6': case '7': case '8': case '9':
		assert(iflag == -1 /* Too many decimal flags in permprintf */);
		iflag = 0;
		while (*s >= '0' && *s <= '9') {
		    iflag = iflag * 10 + *s - '0';
		    s++;
		}
		break;

	      case '*':
		assert(iflag == -1 /* iflag given */);
		iflag = va_arg(val, int);
		break;

	      case 's': {
		  const char *x = va_arg(val, const char *);
		  if (x) {
		      if (iflag < 0)
			  append(x, strlen(x));
		      else
			  append(x, iflag);
		  }
		  goto pctdone;
	      }

	      case 'c': {
		  char c = (char)(va_arg(val, int) & 0xFF);
		  append(&c, 1);
		  goto pctdone;
	      }

	      case 'p': {
		  PermString::Capsule x = va_arg(val, PermString::Capsule);
		  PermString px;
		  if (x)
		      px = PermString::decapsule(x);
		  if (iflag < 0 || iflag > px.length())
		      append(px.c_str(), px.length());
		  else
		      append(px.c_str(), iflag);
		  goto pctdone;
	      }

	      case 'd': {
		  // FIXME FIXME rewrite for sense
		  int x = va_arg(val, int);
		  if (pspos == pscap)
		      extend(1);

		  // FIXME -2^31
		  unsigned int ux = x;
		  if (x < 0) {
		      psc[pspos++] = '-';
		      ux = -x;
		  }

		  int numdigits = 0;
		  for (unsigned digcountx = ux; digcountx > 9; digcountx /= 10)
		      numdigits++;

		  extend(numdigits + 1);
		  int digit = numdigits;
		  do {
		      psc[pspos + digit] = (ux % 10) + '0';
		      ux /= 10;
		      digit--;
		  } while (ux);
		  pspos += numdigits + 1;

		  goto pctdone;
	      }

	      case 'g': {
		  // FIXME FIXME rewrite for sense
		  double x = va_arg(val, double);
		  char buffer[1000];
		  int len;
		  sprintf(buffer, "%.10g%n", x, &len);
		  extend(len);
		  strcpy(psc + pspos, buffer);
		  pspos += len;
		  goto pctdone;
	      }

	      default:
		assert(0 /* Bad % in permprintf */);
		goto pctdone;

	    }

      pctdone:
	s++;
    }

    return PermString(psc, pspos);
}

PermString permprintf(const char *s, ...)
{
    va_list val;
    va_start(val, s);
    PermString p = vpermprintf(s, val);
    va_end(val);
    return p;
}

PermString permcat(PermString a, PermString b)
{
    if (!a || !b)
	return a ? a : b;
    unsigned al = a.length();
    unsigned bl = b.length();
    char *s = new char[al + bl];
    memcpy(s, a.c_str(), al);
    memcpy(s + al, b.c_str(), bl);
    PermString p(s, al + bl);
    delete[] s;
    return p;
}
