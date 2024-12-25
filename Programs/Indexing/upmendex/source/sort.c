#include "mendex.h"
#include "qsort.h"

#include "exkana.h"
#include "exvar.h"
#include "exhanzi.h"

#define RULEBUFSIZE  29652+STYBUFSIZE
/*
	length of collation rule in ICU 76.1

	icu_locale          length
	ja                    6410
	ja@collation=unihan     61
	ko                   12577
	ko@collation=unihan     51
	ko@collation=search    782
	zh  (pinin)          27369
	zh@collation=unihan     82
	zh@collation=stroke  29651
	zh@collation=zhuyin  28880
*/

int sym,nmbr,ltn,kana,hngl,hnz,cyr,grk,dvng,thai,arab,hbrw;

static int wcomp(const void *p, const void *q);
static int pcomp(const void *p, const void *q);
static int ordering(UChar *c);
static int get_charset_juncture(UChar *str);
static int unescape(const unsigned char *src, UChar *dist);

/*   init ICU collator   */
void init_icu_collator()
{
	UErrorCode status;
	UParseError parse_error;
	UChar rules[RULEBUFSIZE] = {'\0'};
	int i;
	int32_t len;

	status = U_ZERO_ERROR;
	if (strlen(icu_rules)>0) {
		if (strcmp(icu_locale,"root")!=0) {
			icu_collator = ucol_open(icu_locale, &status);
			if (U_FAILURE(status)) {
				verb_printf(efp, "\n[ICU] Collator creation failed.: %s\n", u_errorName(status));
				exit(254);
			}
			len = ucol_getRulesEx(icu_collator, UCOL_TAILORING_ONLY, rules, RULEBUFSIZE);
			if (u_strlen(rules)<len) {
				verb_printf(efp, "\n[ICU] Failed to extract collation rules by locale (%s). Need buffer size %d.\n",
					icu_locale, len);
				exit(254);
			}
			len = RULEBUFSIZE - u_strlen(rules);
			if (len<STYBUFSIZE) {
				warn_printf(efp, "\nWarning: [ICU] Remaining buffer size (%d) is small for custom collation rules with rules by locale (%s).\n",
					len, icu_locale);
			}
			ucol_close(icu_collator);
		}
		unescape((unsigned char *)icu_rules, rules);
		status = U_ZERO_ERROR;
		icu_collator = ucol_openRules(rules, -1, UCOL_OFF, UCOL_TERTIARY, &parse_error, &status);
	} else
		icu_collator = ucol_open(icu_locale, &status);
	if (U_FAILURE(status)) {
		verb_printf(efp, "\n[ICU] Collator creation failed.: %s\n", u_errorName(status));
		exit(254);
	}
	if (status == U_USING_DEFAULT_WARNING) {
		warn_printf(efp, "\nWarning: [ICU] U_USING_DEFAULT_WARNING for locale %s\n",
			    icu_locale);
	}
	if (status == U_USING_FALLBACK_WARNING) {
		warn_printf(efp, "\nWarning: [ICU] U_USING_FALLBACK_WARNING for locale %s\n",
			    icu_locale);
	}
	for (i=0;i<UCOL_ATTRIBUTE_COUNT;i++) {
		if (icu_attributes[i]!=UCOL_DEFAULT) {
			status = U_ZERO_ERROR;
			ucol_setAttribute(icu_collator, i, icu_attributes[i], &status);
		}
		if (U_FAILURE(status)) {
			warn_printf(efp, "\nWarning: [ICU] Failed to set attribute (%d): %s\n",
				    i, u_errorName(status));
		}
	}
}

/*   sort index   */
void wsort(struct index *ind, int num)
{
	int i,order;

	for (order=1,i=0;;i++) {
		switch (character_order[i]) {
		case '\0':
			goto BREAK;
			break;

		case 'S':
			sym=order++;
			break;

		case 'N':
			nmbr=order++;
			break;

		case 'L':
			ltn=order++;
			break;

		case 'J':
			kana=order++;
			break;

		case 'K':
			hngl=order++;
			break;

		case 'H':
			hnz=order++;
			break;

		case 'C':
			cyr=order++;
			break;

		case 'G':
			grk=order++;
			break;

		case 'D':
			dvng=order++;
			break;

		case 'T':
			thai=order++;
			break;

		case 'a':
			arab=order++;
			break;

		case 'h':
			hbrw=order++;
			break;

		case '@':
			sym=nmbr=ltn=kana=hngl=hnz=cyr=grk=dvng=thai=arab=hbrw=order++;
			break;

		default:
			verb_printf(efp,"\nWarning: Illegal input for character_order (%c).",character_order[i]);
			break;
		}
	}

BREAK:

	if (sym==0) sym=order++;
	if (nmbr==0) nmbr=order++;
	if (ltn==0) ltn=order++;
	if (kana==0) kana=order++;
	if (hngl==0) hngl=order++;
	if (hnz==0) hnz=order++;
	if (cyr==0) cyr=order++;
	if (grk==0) grk=order++;
	if (dvng==0) dvng=order++;
	if (thai==0) thai=order++;
	if (arab==0) arab=order++;
	if (hbrw==0) hbrw=order++;

	qsort(ind,num,sizeof(struct index),wcomp);
}

/*   compare for sorting index   */
static int wcomp(const void *p, const void *q)
{
	int i, j, len1, len2, cmp;
	const struct index *index1 = p, *index2 = q;
	UChar ch1, ch2;
	UChar *str1, *str2;
	UCollationResult col_result;

	scount++;

	for (j=0;j<MAXDEPTH;j++) {

/*   check level   */
		if (((*index1).words==j)&&((*index2).words!=j)) return -1;
		else if (((*index1).words!=j)&&((*index2).words==j)) return 1;

		for(i=0;;i+=len1) {

			str1=&((*index1).dic[j][i]);
			str2=&((*index2).dic[j][i]);
			ch1=(*index1).dic[j][i];
			ch2=(*index2).dic[j][i];

/*   even   */
			if ((ch1==L'\0')&&(ch2==L'\0')) break;

/*   index1 is shorter   */
			if (ch1==L'\0') return -1;

/*   index2 is shorter   */
			if (ch2==L'\0') return 1;

/*   priority   */
			if ((priority!=0)&&(i>0)) {
				if ((is_jpn_kana(str1))&&(!is_jpn_kana(str2)))
					return -1;

				if ((is_jpn_kana(str2))&&(!is_jpn_kana(str1)))
					return 1;
			}

/*   compare group   */
			if (ordering(str1)<ordering(str2))
				return -1;

			if (ordering(str1)>ordering(str2))
				return 1;

/*   simple compare   */
			if (priority==0) len1=len2=-1;
			else {
				len1=get_charset_juncture(str1);
				len2=get_charset_juncture(str2);
			}
			col_result = ucol_strcoll(icu_collator, str1, len1, str2, len2);
			if (col_result == UCOL_LESS) return -1;
			else if (col_result == UCOL_GREATER) return 1;

			if (priority==0) break;
		}

/*   compare index   */
		str1=&((*index1).idx[j][0]);
		str2=&((*index2).idx[j][0]);
		col_result = ucol_strcoll(icu_collator, str1, -1, str2, -1);
		if (col_result == UCOL_LESS) return -1;
		else if (col_result == UCOL_GREATER) return 1;
		cmp=u_strcmp(str1,str2);
		if (cmp<0) return -1;
		else if (cmp>0) return 1;
	}
	return 0;
}

/*   sort page   */
void pagesort(struct index *ind, int num)
{
	int i,j;
	struct page *buff;

	for (i=0;i<num;i++) {
		if (ind[i].num==0) continue;

		buff=xmalloc(sizeof(struct page)*(ind[i].num+1));
		for (j=0;j<=ind[i].num;j++) {
			buff[j]=ind[i].p[j];
		}
		qsort(buff,ind[i].num+1,sizeof(struct page),pcomp);
		for (j=0;j<=ind[i].num;j++) {
			ind[i].p[j]=buff[j];
		}
		free(buff);
	}
}

/*   compare for sorting page   */
static int pcomp(const void *p, const void *q)
{
	int i,j,cc=0,num1,num2;
	char buff[16],*p0,*p1;
	const struct page *page1 = p, *page2 = q;

	scount++;

	for (i=0;i<PAGE_COMPOSIT_DEPTH;i++) {
		if ((page1->attr[i]<0)&&(page2->attr[i]<0)) return 0;
		else if ((page1->attr[i]<0)&&(page2->attr[i]>=0)) return -1;
		else if ((page2->attr[i]<0)&&(page1->attr[i]>=0)) return 1;

		if (page1->attr[i]>page2->attr[i]) return 1;
		if (page1->attr[i]<page2->attr[i]) return -1;

		p0=&page1->page[cc];
		p1=strstr(p0, page_compositor);
		j=p1 ? p1-p0 : strlen(p0);
		strncpy(buff,p0,j);
		buff[j]='\0';
		num1=pnumconv(buff,page1->attr[i]);

		p0=&page2->page[cc];
		p1=strstr(p0, page_compositor);
		j=p1 ? p1-p0 : strlen(p0);
		strncpy(buff,p0,j);
		buff[j]='\0';
		num2=pnumconv(buff,page2->attr[i]);

		if (num1>num2) return 1;
		else if (num1<num2) return -1;

		if (page1->enc[0]=='(' || page2->enc[0]==')') return -1;
		if (page1->enc[0]==')' || page2->enc[0]=='(') return 1;

		if (p1) cc+=j+strlen(page_compositor);
		else return 0;
	}

	return 0;
}

static int ordering(UChar *c)
{
	if      (*c<0x20)                return sym;  /* control */
	else if (*c<0x7F) {
		if      (is_latin(c))    return ltn;
		else if (is_numeric(c))  return nmbr;
		else                     return sym;
	}
	else if (*c<0xA0)                return sym;  /* control */
	else {
		if      (is_latin(c))    return ltn;
		else if (is_jpn_kana(c)) return kana;
		else if (is_kor_hngl(c)) return hngl;
		else if (is_hanzi(c))    return hnz;
		else if (is_cyrillic(c)) return cyr;
		else if (is_greek(c))    return grk;
		else if (is_numeric(c))  return nmbr;
		else if (is_devanagari(c)) return dvng;
		else if (is_thai(c))     return thai;
		else if (is_arabic(c))   return arab;
		else if (is_hebrew(c))   return hbrw;
		else                     return sym;
	}
}

int charset(UChar *c)
{
	if      (*c<0x20)                return CH_UNKNOWN;  /* control */
	else if (*c<0x7F) {
		if      (is_latin(c))    return CH_LATIN;
		else if (is_numeric(c))  return CH_NUMERIC;
		else                     return CH_SYMBOL;
	}
	else if (*c<0xA0)                return CH_UNKNOWN;  /* control */
	else {
		if      (is_latin(c))    return CH_LATIN;
		else if (is_jpn_kana(c)) return CH_KANA;
		else if (is_kor_hngl(c)) return CH_HANGUL;
		else if (is_hanzi(c))    return CH_HANZI;
		else if (is_cyrillic(c)) return CH_CYRILLIC;
		else if (is_greek(c))    return CH_GREEK;
		else if (is_numeric(c))  return CH_NUMERIC;
		else if (is_devanagari(c)) return CH_DEVANAGARI;
		else if (is_thai(c))     return CH_THAI;
		else if (is_arabic(c))   return CH_ARABIC;
		else if (is_hebrew(c))   return CH_HEBREW;
		else                     return CH_SYMBOL;
	}
}

static int get_charset_juncture(UChar *str)
{
	int k, l, len, chset0, chset_k, chset_l;

	chset0=CH_UNKNOWN;
	for(k=0;;k++) {
		if (str[k]==L'\0') {
			len=k;
			return len;
		}
		if (k==0) continue;
		if (k>0 && is_surrogate_pair(&str[k-1])) continue;
		if (k>1 && is_surrogate_pair(&str[k-2])) l = k-2;
		else l = k-1;
		chset_l=charset(&str[l]);
		chset_k=charset(&str[k]);
		if (chset0==CH_UNKNOWN && is_any_script(chset_l)) {
			chset0=chset_l;
		}
		if (chset0!=CH_UNKNOWN && is_any_script(chset_k)) {
			if (chset0!=chset_k) {
				len=k;
				return len;
			}
		}
	}
}

static int unescape(const unsigned char *src, UChar *dest)
{
	int i,j,k,ret;
	char tmp[STYBUFSIZE];
	UErrorCode status;

	for (i=j=0;i<STYBUFSIZE;i++) {
		if (src[i]=='\0') {
			return i;
		}
		else if (src[i]< 0x80 && (src[i+1]>=0x80 || src[i+1]=='\0')) {
			strncpy(tmp,(char *)&src[j],i-j+1);
			tmp[i-j+1]='\0';
			k=u_strlen(dest);
			ret=u_unescape(tmp, &dest[k], RULEBUFSIZE-k);
			if (ret==0) {
				verb_printf(efp, "\n[ICU] Escape sequence in input seems malformed.\n");
				exit(254);
			}
			j=i+1;
		}
		else if (src[i]>=0x80 && (src[i+1]< 0x80 || src[i+1]=='\0')) {
			strncpy(tmp,(char *)&src[j],i-j+1);
			tmp[i-j+1]='\0';
			k=u_strlen(dest);
			status=U_ZERO_ERROR;
			u_strFromUTF8(&dest[k], RULEBUFSIZE-k, NULL, tmp, -1, &status);
			if (U_FAILURE(status)) {
				verb_printf(efp, "\n[ICU] Input string seems malformed.: %s\n", u_errorName(status));
				exit(254);
			}
			j=i+1;
		}
	}
	return -1;
}

int is_latin(UChar *c)
{
	if (( *c>=L'A' && *c<=L'Z' )||( *c>=L'a' && *c<=L'z' )) return 1;
	else if ( *c==0x00AA || *c==0x00BA ) return 1; /* Latin-1 Supplement */
	else if ( *c>=0x00C0 && *c<=0x00D6 ) return 1;
	else if ( *c>=0x00D8 && *c<=0x00F6 ) return 1;
	else if ( *c>=0x00F8 && *c<=0x00FF ) return 1;
	else if ( *c>=0x0100 && *c<=0x024F ) return 1; /* Latin Extended-A,B */
	else if ( *c>=0x0250 && *c<=0x02AF ) return 1; /* IPA Extensions */
	else if ( *c>=0x2C60 && *c<=0x2C7F ) return 1; /* Latin Extended-C */
	else if ( *c>=0xA720 && *c<=0xA7FF ) return 1; /* Latin Extended-D */
	else if ( *c>=0xAB30 && *c<=0xAB6F ) return 1; /* Latin Extended-E */
	else if ( *c>=0x1E00 && *c<=0x1EFF ) return 1; /* Latin Extended Additional */
	else if ( *c>=0xFB00 && *c<=0xFB06 ) return 1; /* Latin ligatures */
	else if ( *c>=0xFF21 && *c<=0xFF3A ) return 1; /* Fullwidth Latin Capital Letter */
	else if ( *c>=0xFF41 && *c<=0xFF5A ) return 1; /* Fullwidth Latin Small Letter */
		/* Property of followings is "Common, So (other symbol)", but seem to be treated as Latin by ICU collator */
	else if ( *c>=0x24B6                           /* CIRCLED LATIN CAPITAL LETTER */
	                     && *c<=0x24E9 ) return 1; /* CIRCLED LATIN SMALL LETTER */

	if (is_surrogate_pair(c)) {
		UChar32 c32;
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
		if      ( c32>=0x10780  &&  c32<=0x107BF ) return 2; /* Latin Extended-F */
		else if ( c32>=0x1DF00  &&  c32<=0x1DFFF ) return 2; /* Latin Extended-G */
	}
	return 0;
}

int is_numeric(UChar *c)
{
	UChar32 c32;

	if ( *c>=L'0' && *c<=L'9' ) return 1;
	else if ( *c>=0xFF10 && *c<=0xFF19 ) return 1; /* Fullwidth Digit */
		/* followings do not seem to be treated as numbers by ICU collator though charType is U_OTHER_NUMBER */
	else if ( *c>=0x3192 && *c<=0x3195 ) return 0; /* IDEOGRAPHIC ANNOTATION ONE MARK..IDEOGRAPHIC ANNOTATION FOUR MARK */
	else if ( *c>=0x3220 && *c<=0x3229 ) return 0; /* PARENTHESIZED IDEOGRAPH ONE..PARENTHESIZED IDEOGRAPH TEN */
	else if ( *c>=0x3280 && *c<=0x3289 ) return 0; /* CIRCLED IDEOGRAPH ONE..CIRCLED IDEOGRAPH TEN */
	else if ( *c>=0xA830 && *c<=0xA835 ) return 0; /* NORTH INDIC FRACTION ONE QUARTER..NORTH INDIC FRACTION THREE SIXTEENTHS */

	if (is_surrogate_pair(c))
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
	else c32=*c;

	switch (u_charType(c32)) {
	case U_DECIMAL_DIGIT_NUMBER:
		return 1;
	case U_OTHER_NUMBER:
		return 2;
	default:
		return 0;
	}
}

int is_jpn_kana(UChar *c)
{
	if      ( *c==0x30A0 )               return 0; /* KATAKANA-HIRAGANA DOUBLE HYPHEN */
	else if ( *c==0x30FB )               return 0; /* KATAKANA MIDDLE DOT */
	else if ( *c>=0x3040 && *c<=0x30FF ) return 1; /* Hiragana, Katakana */
	else if ( *c>=0x31F0 && *c<=0x31FF ) return 1; /* Katakana Phonetic Extensions */
	else if ( *c>=0x32D0 && *c<=0x32FE ) return 1; /* Circled Katakana */
	else if ( *c>=0xFF66 && *c<=0xFF9F ) return 1; /* Halfwidth Katakana */
	else if ( *c>=0x3300 && *c<=0x3357 ) return 1; /* Squared Katakana words */

	if (is_surrogate_pair(c)) {
		UChar32 c32;
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
		if ( c32>=0x1B130  &&  c32<=0x1B16F ) return 2; /* Small Kana Extensions */
		else if ( c32==0x1B000 )              return 2; /* KATAKANA LETTER ARCHAIC E */
		else if ( c32>=0x1B11F                          /* HIRAGANA LETTER ARCHAIC WU */
		                   &&  c32<=0x1B122 ) return 2; /* KATAKANA LETTER ARCHAIC WU */
		else if ( c32==0x1F200 )              return 2; /* SQUARE HIRAGANA HOKA */
		else if ( c32==0x1B001 ) {
		/* check whether U+1B001 is HIRAGANA LETTER ARCHAIC YE or not.
		                  It may be HENTAIGANA LETTER E-1              */
			if (kana_ye_mode==0) {
				UCollationResult order;
				UCollationStrength strgth;
				UChar strX[4],strZ[4];
				strgth = ucol_getStrength(icu_collator);
				ucol_setStrength(icu_collator, UCOL_PRIMARY);
				strX[0] = 0xD82C; strX[1] = 0xDC01; strX[2] = L'\0'; /* U+1B001 */
				strZ[0] = 0xD82C; strZ[1] = 0xDD21; strZ[2] = L'\0'; /* U+1B121 */
				order = ucol_strcoll(icu_collator, strZ, -1, strX, -1);
				kana_ye_mode = (order==UCOL_EQUAL) ? 2 : 1;
				ucol_setStrength(icu_collator, strgth);
			}
			if (kana_ye_mode==2) return 2;
		}
	}
	return 0;
		/* ICU 71.1 does not seem to support
		   most of "Kana Supplement" and "Kana Extended-A" yet. (2022/09/11) */
}

int is_kor_hngl(UChar *c)
{
	if      ( *c>=0xAC00 && *c<=0xD7AF ) return 1; /* Hangul Syllables */
	else if ( *c>=0x1100 && *c<=0x11FF ) return 1; /* Hangul Jamo */
	else if ( *c>=0xA960 && *c<=0xA97F ) return 1; /* Hangul Jamo Extended-A */
	else if ( *c>=0xD7B0 && *c<=0xD7FF ) return 1; /* Hangul Jamo Extended-B */
	else if ( *c>=0x3130 && *c<=0x318F ) return 1; /* Hangul Compatibility Jamo */
	else if ( *c>=0xFFA0 && *c<=0xFFDC ) return 1; /* Hangul Halfwidth Jamo */
	else if ( *c>=0x3200 && *c<=0x321E ) return 1; /* Enclosed CJK Letters and Months */
	else if ( *c>=0x3260 && *c<=0x327E ) return 1; /* Enclosed CJK Letters and Months */
	else return 0;
}

int is_hanzi(UChar *c)
{
	if      ( *c>=0x2E80                           /* CJK Radicals Supplement */
	                     && *c<=0x2FDF ) return 1; /* Kangxi Radicals */
	else if ( *c>=0x31C0 && *c<=0x31EF ) return 1; /* CJK Strokes */
	else if ( *c>=0x3300                           /* CJK Compatibility */
	                     && *c<=0x4DBF ) return 1; /* CJK Unified Ideographs Extension A */
	else if ( *c>=0x4E00 && *c<=0x9FFF ) return 1; /* CJK Unified Ideographs */
	else if ( *c>=0xF900 && *c<=0xFAFF ) return 1; /* CJK Compatibility Ideographs */

	if (is_surrogate_pair(c)) {
		UChar32 c32;
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
		if ( c32>=0x20000  &&         /* CJK Unified Ideographs Extension B,C,D,E,F,I */
		                              /* CJK Compatibility Ideographs Supplement */
		     c32<=0x323AF ) return 2; /* CJK Unified Ideographs Extension G,H */
	}
	if (*c==0xFDD0) { /* Noncharacter */
		if (hanzi_mode==HANZI_PINYIN &&
		    *(c+1)>=L'A'   && *(c+1)<=L'Z'  ) return -1; /* Pinyin Index */
		if (hanzi_mode==HANZI_ZHUYIN &&
		    *(c+1)>=0x3105 && *(c+1)<=0x3129) return -1; /* Zhuyin Index */
	}
	return 0;
}

int is_zhuyin(UChar *c)
{
	if      ( *c>=0x3100 && *c<=0x312F ) return 1; /* Bopomofo */
	else if ( *c>=0x31A0 && *c<=0x31BF ) return 1; /* Bopomofo Extended */
	else return 0;
}

int is_cyrillic(UChar *c)
{
	if      ( *c==0x0482 )               return 0; /* Cyrillic Thousands Sign */
	else if ( *c>=0x0400                           /* Cyrillic */
	                     && *c<=0x052F ) return 1; /* Cyrillic Supplement */
	else if ( *c>=0x1C80 && *c<=0x1C8F ) return 1; /* Cyrillic Extended-C */
	else if ( *c>=0x2DE0 && *c<=0x2DFF ) return 1; /* Cyrillic Extended-A */
	else if ( *c>=0xA640 && *c<=0xA69F ) return 1; /* Cyrillic Extended-B */

	if (is_surrogate_pair(c)) {
		UChar32 c32;
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
		if ( c32>=0x1E030  &&  c32<=0x1E08F ) return 2; /* Cyrillic Extended-D */
	}
	return 0;
}

int is_greek(UChar *c)
{
	if      ( *c==0x03F6 )               return 0; /* Greek Reversed Lunate Epsilon Symbol */
	else if ( *c>=0x0370 && *c<=0x03FF ) return 1; /* Greek */
	else if ( *c>=0x1F00 && *c<=0x1FFF ) return 1; /* Greek Extended */
	else return 0;
}

int is_devanagari(UChar *c)
{
	if      ( *c>=0x0964                           /* Generic punctuation for scripts of India */
	                     && *c<=0x096F ) return 0; /* Devanagari Digit */
	else if ( *c>=0x0900 && *c<=0x097F ) return 1; /* Devanagari */
	else if ( *c>=0xA8E0 && *c<=0xA8FF ) return 1; /* Devanagari Extended */

	if (is_surrogate_pair(c)) {
		UChar32 c32;
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
		if ( c32>=0x11B00  &&  c32<=0x11B5F ) return 2; /* Devanagari Extended-A */
	}
	return 0;
}

int is_thai(UChar *c)
{
	if      ( *c==0x0E3F )               return 0; /* Thai Currency Symbol Baht */
	else if ( *c>=0x0E50 && *c<=0x0E59 ) return 0; /* Thai Digit */
	else if ( *c>=0x0E00 && *c<=0x0E7F ) return 1; /* Thai */
	else return 0;
}

int is_arabic(UChar *c)
{
	if      ( *c>=0x0600                           /* ARABIC NUMBER SIGN..ARABIC SIGN SAMVAT */
	                                               /* ARABIC NUMBER MARK ABOVE */
	                     && *c<=0x0608 ) return 0; /* ARABIC-INDIC CUBE ROOT..ARABIC RAY */
	else if ( *c==0x060B )               return 0; /* AFGHANI SIGN */
	else if ( *c==0x060C )               return 0; /* ARABIC COMMA */
	else if ( *c>=0x060E && *c<=0x060F ) return 0; /* ARABIC POETIC VERSE SIGN..ARABIC SIGN MISRA */
	else if ( *c>=0x0660 && *c<=0x0669 ) return 0; /* ARABIC-INDIC DIGIT ZERO..ARABIC-INDIC DIGIT NINE */
	else if ( *c==0x061B )               return 0; /* ARABIC SEMICOLON */
	else if ( *c==0x061C )               return 0; /* ARABIC LETTER MARK */
	else if ( *c==0x061F )               return 0; /* ARABIC QUESTION MARK */
	else if ( *c==0x0640 )               return 0; /* ARABIC TATWEEL */
	else if ( *c==0x06DD )               return 0; /* ARABIC END OF AYAH */
	else if ( *c==0x06DE )               return 0; /* ARABIC START OF RUB EL HIZB */
	else if ( *c==0x06E9 )               return 0; /* ARABIC PLACE OF SAJDAH */
	else if ( *c>=0x06F0 && *c<=0x06F9 ) return 0; /* EXTENDED ARABIC-INDIC DIGIT ZERO..EXTENDED ARABIC-INDIC DIGIT NINE */
	else if ( *c>=0x06FD && *c<=0x06FE ) return 0; /* ARABIC SIGN SINDHI AMPERSAND..ARABIC SIGN SINDHI POSTPOSITION MEN */
	else if ( *c==0x08E2 )               return 0; /* ARABIC DISPUTED END OF AYAH */
	else if ( *c>=0x0890 && *c<=0x0891 ) return 0; /* ARABIC POUND MARK ABOVE..ARABIC PIASTRE MARK ABOVE */
	else if ( *c>=0xFD40 && *c<=0xFD4F ) return 0; /* ARABIC LIGATURE RAHIMAHU ALLAAH..ARABIC LIGATURE RAHIMAHUM ALLAAH */
	else if ( *c==0xFDCF )               return 0; /* ARABIC LIGATURE SALAAMUHU ALAYNAA */
	else if ( *c==0xFDFC )               return 0; /* RIAL SIGH */
	else if ( *c>=0xFDFD && *c<=0xFDFF ) return 0; /* ARABIC LIGATURE BISMILLAH AR-RAHMAN AR-RAHEEM..ARABIC LIGATURE AZZA WA JALL */

	else if ( *c>=0x0600 && *c<=0x06FF ) return 1; /* Arabic */
	else if ( *c>=0x0750 && *c<=0x077F ) return 1; /* Arabic Supplement */
	else if ( *c>=0x0870                           /* Arabic Extended-B */
	                     && *c<=0x08FF ) return 1; /* Arabic Extended-A */
	else if ( *c>=0xFB50 && *c<=0xFDFF ) return 1; /* Arabic Presentation Forms-A */
	else if ( *c>=0xFE70 && *c<=0xFEFF ) return 1; /* Arabic Presentation Forms-B */

	if (is_surrogate_pair(c)) {
		UChar32 c32;
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
		if ( c32>=0x10EC0  &&  c32<=0x10EFF ) return 2; /* Arabic Extended-C */
	}
	return 0;
}

int is_hebrew(UChar *c)
{
	if      ( *c==0xFB29 )               return 0; /* Hebrew Letter Alternative Plus Sign */
	else if ( *c>=0x0590 && *c<=0x05FF ) return 1; /* Hebrew */
	else if ( *c>=0xFB1D && *c<=0xFB4F ) return 1; /* Hebrew presentation forms */
	else return 0;
}

int is_type_mark_or_punct(UChar *c)
{
	UChar32 c32;

	if (is_surrogate_pair(c))
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
	else c32=*c;

	switch (u_charType(c32)) {
	case U_MODIFIER_LETTER:
	case U_DASH_PUNCTUATION: case U_START_PUNCTUATION: case U_END_PUNCTUATION:
	case U_CONNECTOR_PUNCTUATION: case U_OTHER_PUNCTUATION:
	case U_INITIAL_PUNCTUATION: case U_FINAL_PUNCTUATION:
	case U_NON_SPACING_MARK: case U_ENCLOSING_MARK: case U_COMBINING_SPACING_MARK:
	case U_FORMAT_CHAR:
		return 1;
	default:
		return 0;
	}
}

int is_type_symbol(UChar *c)
{
	UChar32 c32;

	if (is_surrogate_pair(c))
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
	else c32=*c;

	switch (u_charType(c32)) {
	case U_MODIFIER_SYMBOL:
		return 1;
	case U_MATH_SYMBOL: case U_CURRENCY_SYMBOL:
	case U_OTHER_SYMBOL:
		return 2;
	default:
		return 0;
	}
}

int chkcontinue(struct page *p, int num)
{
	int i,j,cc=0,num1,num2,k1,k2;
	char buff1[16],buff2[16],*p0,*p1;

	for (i=0;i<PAGE_COMPOSIT_DEPTH;i++) {
		if ((p[num].attr[i]<0)&&(p[num+1].attr[i]<0)) return 1;
		else if (p[num].attr[i]!=p[num+1].attr[i]) return 0;

		p0=&p[num].page[cc];
		p1=strstr(p0, page_compositor);
		if (p1) {
			j=p1-p0;
			k1=j;
		} else {
			j=strlen(p0);
			k1=0;
		}
		strncpy(buff1,p0,j);
		buff1[j]='\0';
		num1=pnumconv(buff1,p[num].attr[i]);

		p0=&p[num+1].page[cc];
		p1=strstr(p0, page_compositor);
		if (p1) {
			j=p1-p0;
			k2=j;
		} else {
			j=strlen(p0);
			k2=0;
		}
		strncpy(buff2,p0,j);
		buff2[j]='\0';
		num2=pnumconv(buff2,p[num+1].attr[i]);

		if (k1>0 || k2>0) {
			if (k1!=k2) return 0;
			if (strcmp(buff1,buff2)) return 0;
			cc+=k1+strlen(page_compositor);
			continue;
		}

		if (num1==num2 || num1+1==num2) return 1;
		else return 0;
	}

	return 1;
}

int ss_comp(UChar *s1, UChar *s2)
{
	UCollationResult ret;

/*   compare group   */
	if (ordering(s1)<ordering(s2))
		return -1;
	if (ordering(s1)>ordering(s2))
		return 1;

/*   simple compare   */
	ret = ucol_strcoll(icu_collator, s1, -1, s2, -1);
	if (ret == UCOL_LESS) return -1;
	else if (ret == UCOL_GREATER) return 1;
	return 0;
}
