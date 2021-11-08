#include "mendex.h"

#include <kpathsea/tex-file.h>

#include "exvar.h"

#include "kp.h"

FILE *fp;
static char icu_attr_str[STYBUFSIZE];

static void convline(char *buff1, int start, char *buff2);
static int scompare(char *buff1, const char *buff2);
static int getparam(char *buff, const char *paraname, char *param);
static int getparachar(char *buff, const char *paraname, char *param);
static size_t sstrlen(const char *buff);
static int sstrcmp(const char *s1, const char *s2);
static int sstrncmp(const char *s1, const char *s2, size_t len);
static int escape_mode=0;

static char *
bfgets (char *buf, int size, FILE *fp)
{
	char *p, *q;
	p = fgets (buf, size, fp);
	for (q = buf; *q; q++) {
		if(*q == '\r') {
			*q++ = '\n';
			*q = '\0';
			break;
		}
	}
	return p;
}
#define fgets bfgets

/*   read style file   */
void styread(const char *filename)
{
	int i,cc;
	char buff[4096],tmp[4096];

	filename = KP_find_file(&kp_ist,filename);
	if(kpse_in_name_ok(filename))
		fp=fopen(filename,"rb");
	else
		fp=NULL;
	if (fp==NULL) {
		fprintf(stderr,"%s does not exist.\n",filename);
		exit(255);
	}
	verb_printf(efp,"Scanning style file %s.",filename);

	for (i=0;;i++) {
		if (fgets(buff,4095,fp)==NULL) break;
		if (getparam(buff,"keyword",keyword)) continue;
		if (getparachar(buff,"arg_open",&arg_open)) continue;
		if (getparachar(buff,"arg_close",&arg_close)) continue;
		if (getparachar(buff,"range_open",&range_open)) continue;
		if (getparachar(buff,"range_close",&range_close)) continue;
		if (getparachar(buff,"level",&level)) continue;
		if (getparachar(buff,"actual",&actual)) continue;
		if (getparachar(buff,"encap",&encap)) continue;
		if (getparachar(buff,"quote",&quote)) continue;
		if (getparachar(buff,"escape",&escape)) continue;
		if (getparam(buff,"preamble",preamble)) continue;
		if (getparam(buff,"postamble",postamble)) continue;
		if (getparam(buff,"setpage_prefix",setpage_prefix)) continue;
		if (getparam(buff,"setpage_suffix",setpage_suffix)) continue;
		if (getparam(buff,"group_skip",group_skip)) continue;
		if (getparam(buff,"lethead_prefix",lethead_prefix)) continue;
		if (getparam(buff,"heading_prefix",lethead_prefix)) continue;
		if (getparam(buff,"lethead_suffix",lethead_suffix)) continue;
		if (getparam(buff,"heading_suffix",lethead_suffix)) continue;
		if (getparam(buff,"symhead_positive",symhead_positive)) continue;
		if (getparam(buff,"symhead_negative",symhead_negative)) continue;
		if (getparam(buff,"numhead_positive",numhead_positive)) continue;
		if (getparam(buff,"numhead_negative",numhead_negative)) continue;
		if ( (cc=scompare(buff,"lethead_flag")) != -1 ||
		     (cc=scompare(buff,"heading_flag")) != -1 ||
		     (cc=scompare(buff,"headings_flag")) != -1 ) {
			lethead_flag=atoi(&buff[cc]);
			continue;
		}
		cc=scompare(buff,"priority");
		if (cc!= -1) {
			priority=atoi(&buff[cc]);
			continue;
		}
		if (getparam(buff,"item_0",item_0)) continue;
		if (getparam(buff,"item_1",item_1)) continue;
		if (getparam(buff,"item_2",item_2)) continue;
		if (getparam(buff,"item_01",item_01)) continue;
		if (getparam(buff,"item_x1",item_x1)) continue;
		if (getparam(buff,"item_12",item_12)) continue;
		if (getparam(buff,"item_x2",item_x2)) continue;
		if (getparam(buff,"delim_0",delim_0)) continue;
		if (getparam(buff,"delim_1",delim_1)) continue;
		if (getparam(buff,"delim_2",delim_2)) continue;
		if (getparam(buff,"delim_n",delim_n)) continue;
		if (getparam(buff,"delim_r",delim_r)) continue;
		if (getparam(buff,"delim_t",delim_t)) continue;
		if (getparam(buff,"suffix_2p",suffix_2p)) continue;
		if (getparam(buff,"suffix_3p",suffix_3p)) continue;
		if (getparam(buff,"suffix_mp",suffix_mp)) continue;
		if (getparam(buff,"encap_prefix",encap_prefix)) continue;
		if (getparam(buff,"encap_infix",encap_infix)) continue;
		if (getparam(buff,"encap_suffix",encap_suffix)) continue;
		cc=scompare(buff,"line_max");
		if (cc!= -1) {
			line_max=atoi(&buff[cc]);
			continue;
		}
		if (getparam(buff,"indent_space",indent_space)) continue;
		cc=scompare(buff,"indent_length");
		if (cc!= -1) {
			indent_length=atoi(&buff[cc]);
			continue;
		}
		if (getparam(buff,"symbol",symhead)) continue;
		cc=scompare(buff,"symbol_flag");
		if (cc!= -1) {
			symbol_flag=atoi(&buff[cc]);
			continue;
		}
		cc=scompare(buff,"letter_head");
		if (cc!= -1) {
			letter_head=atoi(&buff[cc]);
			continue;
		}
		if (getparam(buff,"kana_head",tmp)) {
			multibyte_to_widechar(kana_head,STYBUFSIZE,tmp);
			continue;
		}
		if (getparam(buff,"hangul_head",tmp) || getparam(buff,"tumunja",tmp)) {
			multibyte_to_widechar(hangul_head,STYBUFSIZE,tmp);
			continue;
		}
		if (getparam(buff,"hanzi_head",tmp)) {
			multibyte_to_widechar(hanzi_head,STYBUFSIZE,tmp);
			continue;
		}
		if (getparam(buff,"thai_head",tmp)) {
			multibyte_to_widechar(thai_head,STYBUFSIZE,tmp);
			continue;
		}
		if (getparam(buff,"devanagari_head",tmp)) {
			multibyte_to_widechar(devanagari_head,STYBUFSIZE,tmp);
			continue;
		}
		if (getparam(buff,"page_compositor",page_compositor)) continue;
		if (getparam(buff,"page_precedence",page_precedence)) continue;
		if (getparam(buff,"character_order",character_order)) continue;
		if (getparam(buff,"icu_locale",     icu_locale     )) continue;
		cc=scompare(buff,"icu_rules");
		if (cc!= -1) {
			escape_mode=1;
			getparam(buff,"icu_rules",icu_rules);
			escape_mode=0;
			continue;
		}
		if (getparam(buff,"icu_attributes", icu_attr_str   )) continue;

		cc=strcspn(buff," \t\r\n");
		if (cc>0) buff[cc]='\0';
		if (buff[0]=='%' || buff[0]=='\n') continue;
		if (strlen(buff)>0) {
			verb_printf(efp,"\nWarning: Unknown specifier (%s).", buff);
		}
	}
	fclose(fp);

	verb_printf(efp,"...done.\n");
}

/*   analize string parameter of style file   */
static void convline(char *buff1, int start, char *buff2)
{
	int i,j,cc;

	for (i=start,j=cc=0;;i++) {
		if (j==STYBUFSIZE-1) {
			buff2[j]='\0';
			break;
		}
		if (buff1[i]=='\"') {
			if (cc==0) {
				cc=1;
			}
			else {
				buff2[j]='\0';
				break;
			}
		}
		else if ((buff1[i]=='\n')||(buff1[i]=='\0')) {

			if (cc==1) {
				buff2[j++]='\n';
			}
			if (fgets(buff1,4095,fp)==NULL) {
				buff2[j]='\0';
				break;
			}
			i= -1;
		}
		else if (cc==1) {
			if ((unsigned char)buff1[i]>0x80) {
				int len = multibytelen((unsigned char)buff1[i]);
				if (len<0) {
					verb_printf(efp,"\nWarning: Illegal input of lead byte 0x%x in UTF-8.", (unsigned char)buff1[i]);
					continue;
				}
				else if (j+len>STYBUFSIZE-1) {
					buff2[j]='\0';
					break;
				}
				while(len--) {
					buff2[j++]=buff1[i++];
				}
				i--;
				continue;
			}
			else if (buff1[i]=='\\' && !escape_mode) {
				i++;
				if (buff1[i]=='\\') buff2[j]='\\';
				else if (buff1[i]=='n') buff2[j]='\n';
				else if (buff1[i]=='t') buff2[j]='\t';
				else if (buff1[i]=='r') buff2[j]='\r';
				else if (buff1[i]=='\"') buff2[j]='\"';
			}
			else buff2[j]=buff1[i];
			j++;
		}
	}
}

/*   compare strings   */
static int scompare(char *buff1, const char *buff2)
{
	int i;

	if (sstrlen(buff1) == sstrlen(buff2)) {
		if (sstrcmp(buff1,buff2) == 0) {
			if (fgets(buff1,4095,fp)==NULL) return -1;
			return 0;
		}
		return -1;
	}
	for (i=0;i<sstrlen(buff1)-sstrlen(buff2);i++) {
		if (buff1[i]==' ' || buff1[i]=='\t') {
			continue;
		}
		else if ((sstrncmp(&buff1[i],buff2,sstrlen(buff2))==0)
			&&((buff1[i+strlen(buff2)]==' ')||(buff1[i+strlen(buff2)]=='\t')||(buff1[i+strlen(buff2)]=='\n'))) {
			return i+strlen(buff2);
		}
		else return -1;
	}
	return -1;
}

/*   get string of style patameter   */
static int getparam(char *buff, const char *paraname, char *param)
{
	int cc;

	cc=scompare(buff,paraname);
	if (cc!=-1) {
		convline(buff,cc,param);
		return 1;
	}
	return 0;
}

/*   get character of style parameter   */
static int getparachar(char *buff, const char *paraname, char *param)
{
	int j,cc;

	cc=scompare(buff,paraname);
	if (cc!=-1) {
		for (j=cc;j<strlen(buff);j++) {
			if (buff[j]=='\'') {
				(*param)=buff[j+1];
				break;
			}
			else if (buff[j]=='\n') {
				if (fgets(buff,4095,fp)==NULL) {
					break;
				}
				else j= -1;
			}
		}
		return 1;
	}
	return 0;
}

static size_t sstrlen(const char *s)
{
	if (s == NULL) return 0;
	return strlen(s);
}

static int sstrcmp(const char *s1, const char *s2)
{
	if (s1 == NULL) return -1;
	if (s2 == NULL) return 1;
	return strcmp(s1, s2);
}

static int sstrncmp(const char *s1, const char *s2, size_t len)
{
	if (s1 == NULL) return -1;
	if (s2 == NULL) return 1;
	return strncmp(s1, s2, len);
}

void set_icu_attributes(void)
{
	int i,attr;
	char *pos, *head, *tmp;

	for (i=0;i<UCOL_ATTRIBUTE_COUNT;i++) icu_attributes[i]=UCOL_DEFAULT;
	tmp=icu_attr_str;
	head=strtok(tmp, " \t\r\n");
	while (head!=NULL) {
	    pos=head;
	    if (strncmp(pos,"alternate:",10)==0) {
		pos+=10;  attr=UCOL_ALTERNATE_HANDLING;
		if      (strcmp(pos,"shifted")==0)       icu_attributes[attr]=UCOL_SHIFTED;
		else if (strcmp(pos,"non-ignorable")==0) icu_attributes[attr]=UCOL_NON_IGNORABLE;
		else	goto illegal;
	    } else
	    if (strncmp(pos,"strength:",9)==0) {
		pos+=9;   attr=UCOL_STRENGTH;
		if      (strcmp(pos,"primary")==0)       icu_attributes[attr]=UCOL_PRIMARY;
		else if (strcmp(pos,"secondary")==0)     icu_attributes[attr]=UCOL_SECONDARY;
		else if (strcmp(pos,"tertiary")==0)      icu_attributes[attr]=UCOL_TERTIARY;
		else if (strcmp(pos,"quaternary")==0)    icu_attributes[attr]=UCOL_QUATERNARY;
		else if (strcmp(pos,"identical")==0)     icu_attributes[attr]=UCOL_IDENTICAL;
		else	goto illegal;
	    } else
	      if (strncmp(pos,"french-collation:",17)==0) {
		pos+=17;  attr=UCOL_FRENCH_COLLATION;
		if      (strcmp(pos,"on")==0)            icu_attributes[attr]=UCOL_ON;
		else if (strcmp(pos,"off")==0)           icu_attributes[attr]=UCOL_OFF;
		else	goto illegal;
	    } else
	    if (strncmp(pos,"case-first:",11)==0) {
		pos+=11;  attr=UCOL_CASE_FIRST;
		if      (strcmp(pos,"off")==0)           icu_attributes[attr]=UCOL_OFF;
		else if (strcmp(pos,"upper-first")==0)   icu_attributes[attr]=UCOL_UPPER_FIRST;
		else if (strcmp(pos,"lower-first")==0)   icu_attributes[attr]=UCOL_LOWER_FIRST;
		else	goto illegal;
	    } else
	    if (strncmp(pos,"case-level:",11)==0) {
		pos+=11;  attr=UCOL_CASE_LEVEL;
		if      (strcmp(pos,"on")==0)            icu_attributes[attr]=UCOL_ON;
		else if (strcmp(pos,"off")==0)           icu_attributes[attr]=UCOL_OFF;
		else	goto illegal;
	    } else
	    if (strncmp(pos,"normalization-mode:",19)==0) {
		pos+=19;  attr=UCOL_NORMALIZATION_MODE;
		if      (strcmp(pos,"on")==0)            icu_attributes[attr]=UCOL_ON;
		else if (strcmp(pos,"off")==0)           icu_attributes[attr]=UCOL_OFF;
		else	goto illegal;
	    } else
	    if (strncmp(pos,"numeric-ordering:",17)==0) {
		pos+=17;  attr=UCOL_NUMERIC_COLLATION;
		if      (strcmp(pos,"on")==0)            icu_attributes[attr]=UCOL_ON;
		else if (strcmp(pos,"off")==0)           icu_attributes[attr]=UCOL_OFF;
		else	goto illegal;
	    } else {
	illegal:
		verb_printf(efp,"\nWarning: Illegal input for icu_attributes (%s).",head);
	    }
	    head=strtok(NULL, " \t\r\n");
	}
}
