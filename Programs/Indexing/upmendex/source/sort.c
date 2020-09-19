#include "mendex.h"
#include "qsort.h"

#include "exkana.h"
#include "exvar.h"

int sym,nmbr,ltn,kana,hngl,hnz,cyr,grk;

static int wcomp(const void *p, const void *q);
static int pcomp(const void *p, const void *q);
static int ordering(UChar *c);
static int get_charset_juncture(UChar *str);
static int unescape(const unsigned char *src, UChar *dist);

/*   sort index   */
void wsort(struct index *ind, int num)
{
	int i,order;
	UErrorCode status;
	UChar rules[STYBUFSIZE];

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

		default:
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

	status = U_ZERO_ERROR;
	if (strlen(icu_rules)>0) {
		unescape((unsigned char *)icu_rules, rules);
		icu_collator = ucol_openRules(rules, -1, UCOL_OFF, UCOL_TERTIARY, NULL, &status);
	} else
		icu_collator = ucol_open(icu_locale, &status);
	if (U_FAILURE(status)) {
		verb_printf(efp, "\n[ICU] Collator creation failed.: %s\n", u_errorName(status));
		exit(254);
	}
	if (status == U_USING_DEFAULT_WARNING) {
		warn_printf(efp, "\nWarning, [ICU] U_USING_DEFAULT_WARNING for locale %s\n",
			    icu_locale);
	}
	if (status == U_USING_FALLBACK_WARNING) {
		warn_printf(efp, "\nWarning, [ICU] U_USING_FALLBACK_WARNING for locale %s\n",
			    icu_locale);
	}
	for (i=0;i<UCOL_ATTRIBUTE_COUNT;i++) {
		if (icu_attributes[i]!=UCOL_DEFAULT) {
			status = U_ZERO_ERROR;
			ucol_setAttribute(icu_collator, i, icu_attributes[i], &status);
		}
		if (U_FAILURE(status)) {
			warn_printf(efp, "\nWarning, [ICU] Failed to set attribute (%d): %s\n",
				    i, u_errorName(status));
		}
	}
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

	for (j=0;j<3;j++) {

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
	char buff[16];
	const struct page *page1 = p, *page2 = q;

	scount++;

	for (i=0;i<3;i++) {
		if ((page1->attr[i]<0)&&(page2->attr[i]<0)) return 0;
		else if ((page1->attr[i]<0)&&(page2->attr[i]>=0)) return -1;
		else if ((page2->attr[i]<0)&&(page1->attr[i]>=0)) return 1;

		if (page1->attr[i]>page2->attr[i]) return 1;
		if (page1->attr[i]<page2->attr[i]) return -1;

		for (j=cc;j<strlen(page1->page);j++) {
			if (strncmp(&page1->page[j],page_compositor,strlen(page_compositor))==0) break;
		}
		strncpy(buff,&page1->page[cc],j-cc);
		buff[j-cc]='\0';
		num1=pnumconv(buff,page1->attr[i]);

		for (j=cc;j<strlen(page2->page);j++) {
			if (strncmp(&page2->page[j],page_compositor,strlen(page_compositor))==0) break;
		}
		strncpy(buff,&page2->page[cc],j-cc);
		buff[j-cc]='\0';
		num2=pnumconv(buff,page2->attr[i]);

		if (num1>num2) return 1;
		else if (num1<num2) return -1;

		if (page1->enc[0]=='(' || page2->enc[0]==')') return -1;
		if (page1->enc[0]==')' || page2->enc[0]=='(') return 1;

		cc=j+strlen(page_compositor);
	}

	return 0;
}

static int ordering(UChar *c)
{
	if (*c<0x80) {
		if (is_latin(c)) return ltn;
		else if (is_numeric(c)) return nmbr;
		else return sym;
	}
	else {
		if (is_latin(c)) return ltn;
		else if (is_jpn_kana(c)) return kana;
		else if (is_kor_hngl(c)) return hngl;
		else if (is_hanzi(c))    return hnz;
		else if (is_cyrillic(c)) return cyr;
		else if (is_greek(c))    return grk;
		else return sym;
	}
}

int charset(UChar *c)
{
	if (*c==0x00) return CH_UNKNOWN;
	else if (*c<0x80) {
		if (is_latin(c)) return CH_LATIN;
		else if (is_numeric(c)) return CH_NUMERIC;
		else return CH_SYMBOL;
	}
	else {
		if (is_latin(c)) return CH_LATIN;
		else if (is_jpn_kana(c)) return CH_KANA;
		else if (is_kor_hngl(c)) return CH_HANGUL;
		else if (is_hanzi(c))    return CH_HANZI;
		else if (is_cyrillic(c)) return CH_CYRILLIC;
		else if (is_greek(c))    return CH_GREEK;
		else return CH_SYMBOL;
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
		if (chset0==CH_UNKNOWN && chset_l!=CH_SYMBOL && chset_l!=CH_NUMERIC) {
			chset0=chset_l;
		}
		if (chset_k!=CH_SYMBOL && chset_k!=CH_NUMERIC) {
			if (chset0!=chset_k) {
				len=k;
				return len;
			}
		}
	}
}

static int unescape(const unsigned char *src, UChar *dist)
{
	int i,j,k;
	char tmp[STYBUFSIZE];

	for (i=j=0;i<STYBUFSIZE;i++) {
		if (src[i]=='\0') {
			return i;
		}
		else if (src[i]< 0x80 && (src[i+1]>=0x80 || src[i+1]=='\0')) {
			strncpy(tmp,(char *)&src[j],i-j+1);
			tmp[i-j+1]='\0';
			k=u_strlen(dist);
			u_unescape(tmp, &dist[k], STYBUFSIZE-k);
			j=i+1;
		}
		else if (src[i]>=0x80 && (src[i+1]< 0x80 || src[i+1]=='\0')) {
			strncpy(tmp,(char *)&src[j],i-j+1);
			tmp[i-j+1]='\0';
			k=u_strlen(dist);
			multibyte_to_widechar(&dist[k], STYBUFSIZE-k, tmp);
			j=i+1;
		}
	}
	return -1;
}

int is_alphanumeric(UChar *c)
{
	if (((*c>=L'A')&&(*c<=L'Z'))||((*c>=L'a')&&(*c<=L'z'))||((*c>=L'0')&&(*c<=L'9')))
		return 1;
	else return 0;
}

int is_latin(UChar *c)
{
	if (((*c>=L'A')&&(*c<=L'Z'))||((*c>=L'a')&&(*c<=L'z'))) return 1;
	else if ((*c>=0x00C0)&&(*c<=0x00D6)) return 1; /* Latin-1 Supplement */
	else if ((*c>=0x00D8)&&(*c<=0x00F6)) return 1;
	else if ((*c>=0x00F8)&&(*c<=0x00FF)) return 1;
	else if ((*c>=0x0100)&&(*c<=0x024F)) return 1; /* Latin Extended-A,B */
	else if ((*c>=0x0250)&&(*c<=0x02AF)) return 1; /* IPA Extensions */
	else if ((*c>=0x2C60)&&(*c<=0x2C7F)) return 1; /* Latin Extended-C */
	else if ((*c>=0xA720)&&(*c<=0xA7FF)) return 1; /* Latin Extended-D */
	else if ((*c>=0xAB30)&&(*c<=0xAB6F)) return 1; /* Latin Extended-E */
	else if ((*c>=0x1E00)&&(*c<=0x1EFF)) return 1; /* Latin Extended Additional */
	else if ((*c>=0xFB00)&&(*c<=0xFB06)) return 1; /* Latin ligatures */
	else return 0;
}

int is_numeric(UChar *c)
{
	if ((*c>=L'0')&&(*c<=L'9')) return 1;
	else return 0;
}

int is_jpn_kana(UChar *c)
{
	UChar32 c32;

	if      ((*c>=0x3040)&&(*c<=0x30FF)) return 1; /* Hiragana, Katakana */
	else if ((*c>=0x31F0)&&(*c<=0x31FF)) return 1; /* Katakana Phonetic Extensions */

	if (is_surrogate_pair(c)) {
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
		if ((c32>=0x1B130) && (c32<=0x1B16F)) return 2; /* Small Kana Extensions */
	}
	return 0;
		/* ICU 65 does not seem to support
		   "Kana Supplement" and "Kana Extended-A" yet. (2020/02/16) */
}

int is_kor_hngl(UChar *c)
{
	if      ((*c>=0xAC00)&&(*c<=0xD7AF)) return 1; /* Hangul Syllables */
	else if ((*c>=0x1100)&&(*c<=0x11FF)) return 1; /* Hangul Jamo */
	else if ((*c>=0xA960)&&(*c<=0xA97F)) return 1; /* Hangul Jamo Extended-A */
	else if ((*c>=0xD7B0)&&(*c<=0xD7FF)) return 1; /* Hangul Jamo Extended-B */
	else if ((*c>=0x3130)&&(*c<=0x318F)) return 1; /* Hangul Compatibility Jamo */
	else if ((*c>=0xFFA0)&&(*c<=0xFFDC)) return 1; /* Hangul Halfwidth Jamo */
	else if ((*c>=0x3200)&&(*c<=0x321E)) return 1; /* Enclosed CJK Letters and Months */
	else if ((*c>=0x3260)&&(*c<=0x327E)) return 1; /* Enclosed CJK Letters and Months */
	else return 0;
}

int is_hanzi(UChar *c)
{
	UChar32 c32;

	if      ((*c>=0x2E80)&&(*c<=0x2EFF)) return 1; /* CJK Radicals Supplement */
	else if ((*c>=0x2F00)&&(*c<=0x2FDF)) return 1; /* Kangxi Radicals */
	else if ((*c>=0x31C0)&&(*c<=0x31EF)) return 1; /* CJK Strokes */
	else if ((*c>=0x3300)&&(*c<=0x33FF)) return 1; /* CJK Compatibility */
	else if ((*c>=0x3400)&&(*c<=0x4DBF)) return 1; /* CJK Unified Ideographs Extension A */
	else if ((*c>=0x4E00)&&(*c<=0x9FFF)) return 1; /* CJK Unified Ideographs */
	else if ((*c>=0xF900)&&(*c<=0xFAFF)) return 1; /* CJK Compatibility Ideographs */

	if (is_surrogate_pair(c)) {
		c32=U16_GET_SUPPLEMENTARY(*c,*(c+1));
		if ((c32>=0x20000) &&         /* CJK Unified Ideographs Extension B,C,D,E,F */
		                              /* CJK Compatibility Ideographs Supplement */
		    (c32<=0x3134F)) return 2; /* CJK Unified Ideographs Extension G */
	}
	return 0;
}

int is_zhuyin(UChar *c)
{
	if      ((*c>=0x3100)&&(*c<=0x312F)) return 1; /* Bopomofo */
	else if ((*c>=0x31A0)&&(*c<=0x31BF)) return 1; /* Bopomofo Extended */
	else return 0;
}

int is_cyrillic(UChar *c)
{
	if      ((*c>=0x0400)&&(*c<=0x052F)) return 1; /* Cyrillic, Cyrillic Supplement */
	else if ((*c>=0x1C80)&&(*c<=0x1C8F)) return 1; /* Cyrillic Extended-C */
	else if ((*c>=0x2DE0)&&(*c<=0x2DFF)) return 1; /* Cyrillic Extended-A */
	else if ((*c>=0xA640)&&(*c<=0xA69F)) return 1; /* Cyrillic Extended-B */
	else return 0;
}

int is_greek(UChar *c)
{
	if      ((*c>=0x0370)&&(*c<=0x03FF)) return 1; /* Greek */
	else if ((*c>=0x1F00)&&(*c<=0x1FFF)) return 1; /* Greek Extended */
	else return 0;
}

int is_comb_diacritical_mark(UChar *c)
{
	if      ((*c>=0x02B0)&&(*c<=0x02FF)) return 1; /* Spacing Modifier Letters */
	else if ((*c>=0x0300)&&(*c<=0x036F)) return 1; /* Combining Diacritical Marks */
	else if ((*c>=0x1DC0)&&(*c<=0x1DFF)) return 1; /* Combining Diacritical Marks Supplement */
	else if ((*c>=0x1AB0)&&(*c<=0x1AFF)) return 1; /* Combining Diacritical Marks Extended */
	else if ((*c>=0x3099)&&(*c<=0x309A)) return 1; /* Combining Kana Voiced Sound Marks */
	else return 0;
}

int chkcontinue(struct page *p, int num)
{
	int i,j,cc=0,num1,num2;
	char buff[16];

	for (i=0;i<3;i++) {
		if ((p[num].attr[i]<0)&&(p[num+1].attr[i]<0)) return 1;
		else if (p[num].attr[i]!=p[num+1].attr[i]) return 0;

		for (j=cc;j<strlen(p[num].page);j++) {
			if (strncmp(&p[num].page[j],page_compositor,strlen(page_compositor))==0) break;
		}
		strncpy(buff,&p[num].page[cc],j);
		buff[j]='\0';
		num1=pnumconv(buff,p[num].attr[i]);

		for (j=cc;j<strlen(p[num+1].page);j++) {
			if (strncmp(&p[num+1].page[j],page_compositor,strlen(page_compositor))==0) break;
		}
		strncpy(buff,&p[num+1].page[cc],j);
		buff[j]='\0';
		num2=pnumconv(buff,p[num+1].attr[i]);

		if (num1==num2 || num1+1==num2) {
			if (i==2) return 1;
			if ((p[num].attr[i+1]<0)&&(p[num+1].attr[i+1]<0)) return 1;
			else return 0;
		}
		else if (num1!=num2) return 0;

		cc=j+strlen(page_compositor);
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
