#include "mendex.h"

#include <kpathsea/tex-file.h>
#include <kpathsea/variable.h>

#include "qsort.h"

#include "exkana.h"
#include "exvar.h"

#include "kp.h"

#define BUFFERLEN 4096
#define BUFFERLEN1 256
#define BUFFERLEN2 (BUFFERLEN1*3)
#define BUFFERLEN3 (BUFFERLEN2+50)

struct dictionary{
UChar* dic[2];
};

static struct dictionary *dictable,*envdic;
static int dlines=0,elines=0;

static int dicvalread(const char *filename, struct dictionary *dicval, int line);

UChar * u_xstrdup (const UChar *string)
{
	return u_strcpy(xmalloc((u_strlen(string)+1)*sizeof(UChar)), string);
}

/*   initialize kana table   */
void initkanatable(void)
{
	int i;

	akasatana=u_xstrdup(AKASATANA);
	for (i=0;;i++) {
		if (akasatana[i]==0) break;
		akasatana[i]+=KATATOP-HIRATOP; /* hiragana -> katakana */
	}

	aiueo=u_xstrdup(AIUEO);
	for (i=0;;i++) {
		if (aiueo[i]==0) break;
		aiueo[i]+=KATATOP-HIRATOP; /* hiragana -> katakana */
	}

	u_strcpy(atama,akasatana);
}

/*   get dictionary   */
int dicread(const char *filename)
{
	int i,ecount=0;
	const char *envfile;
	char buff[BUFFERLEN];
	FILE *fp;

	if (filename!=NULL) {
		filename = KP_find_file(&kp_dict,filename);
		if(kpse_in_name_ok(filename))
			fp=fopen(filename,"rb");
		else
			fp = NULL;
		if (fp==NULL) {
			warn_printf(efp,"Warning: Couldn't find dictionary file %s.\n",filename);
			goto ENV;
		}
		verb_printf(efp,"Scanning dictionary file %s.",filename);

		for (i=0;;i++) {
			if (fgets(buff,BUFFERLEN-1,fp)==NULL) break;
			if ((buff[0]=='\r')||(buff[0]=='\n')||(buff[0]=='\0')) i--;
		}
		fclose(fp);

		dictable=xmalloc(sizeof(struct dictionary)*i);

		dlines=dicvalread(filename,dictable,i);

		verb_printf(efp,"...done.\n");
	}

ENV:
	envfile=kpse_var_value("INDEXDEFAULTDICTIONARY");
	if ((envfile!=NULL)&&(strlen(envfile)!=0)) {
		envfile = KP_find_file(&kp_dict,envfile);
		if(kpse_in_name_ok(envfile))
			fp=fopen(envfile,"rb");
		else
			fp = NULL;
		if (fp==NULL) {
			warn_printf(efp,"Warning: Couldn't find environment dictionary file %s.\n",envfile);
			return ecount;
		}
		verb_printf(efp,"Scanning environment dictionary file %s.",envfile);

		for (i=0;;i++) {
			if (fgets(buff,255,fp)==NULL) break;
			if ((buff[0]=='\r')||(buff[0]=='\n')||(buff[0]=='\0')) i--;
		}
		fclose(fp);

		envdic=xmalloc(sizeof(struct dictionary)*i);

		elines=dicvalread(envfile,envdic,i);

		verb_printf(efp,"...done.\n");
	}

	return 0; /* FIXME: is this right? */	
}

static int dcomp(const void *bf1, const void *bf2);

/*   read dictionary file   */
static int dicvalread(const char *filename, struct dictionary *dicval, int line)
{
	int i,j,k;
	char buff[BUFFERLEN1],buff2[BUFFERLEN1];
	UChar ubuff[BUFFERLEN1],ubuff2[BUFFERLEN1];
	FILE *fp;

	if(kpse_in_name_ok(filename))
		fp=fopen(filename,"rb");
	else {
		fprintf(stderr, "upmendex: %s is forbidden to open for reading.\n",filename);
		exit(255);
	}
	for (i=0;i<line;i++) {
		if (fgets(buff,255,fp)==NULL) break;
		if ((buff[0]=='\r')||(buff[0]=='\n')||(buff[0]=='\0')) {
			i--;
			continue;
		}
		for (j=0;((buff[j]==' ')||(buff[j]=='\t'));j++);
		for (k=0;((buff[j]!='\r')&&(buff[j]!='\n')&&(buff[j]!=' ')&&(buff[j]!='\t'));j++,k++) {
			buff2[k]=buff[j];
		}
		buff2[k]='\0';
		if (strlen(buff2)==0) {
			i--;
			continue;
		}
		multibyte_to_widechar(ubuff,BUFFERLEN1,buff2);
		dicval[i].dic[0]=u_xstrdup(ubuff);
		for (;((buff[j]==' ')||(buff[j]=='\t'));j++);
		for (k=0;((buff[j]!='\r')&&(buff[j]!='\n')&&(buff[j]!=' ')&&(buff[j]!='\t'));j++,k++) {
			buff2[k]=buff[j];
		}
		buff2[k]='\0';
		if (strlen(buff2)==0) {
			free(dicval[i].dic[0]);
			i--;
			continue;
		}
		multibyte_to_widechar(ubuff,BUFFERLEN1,buff2);
		convert(ubuff,ubuff2);
		dicval[i].dic[1]=u_xstrdup(ubuff2);
	}

	fclose(fp);

	qsort(dicval,i,sizeof(struct dictionary),dcomp);
	return(i);
}

/*   comp-function of dictionary sorting   */
static int dcomp(const void *bf1, const void *bf2)
{
	const struct dictionary *buff1 = (const struct dictionary *) bf1;
	const struct dictionary *buff2 = (const struct dictionary *) bf2;
	int i;

	for (i=0;i<BUFFERLEN1;i++) {
		if (((*buff1).dic[0][i]==L'\0')&&((*buff2).dic[0][i]==L'\0')) return 0;
		else if (((*buff1).dic[0][i]==L'\0')&&((*buff2).dic[0][i]!=L'\0')) return 1;
		else if (((*buff1).dic[0][i]!=L'\0')&&((*buff2).dic[0][i]==L'\0')) return -1;
		else if ((*buff1).dic[0][i]<(*buff2).dic[0][i]) return 1;
		else if ((*buff1).dic[0][i]>(*buff2).dic[0][i]) return -1;
	}
	return 0;
}

/*   convert to capital-hiragana character   */
int convert(UChar *buff1, UChar *buff2)
{
	int i=0,j=0,k;
	char errbuff[BUFFERLEN2],errbuff2[BUFFERLEN3];
	int wclen;
	UChar buff3[3];

	while(1) {
		if (buff1[i]==L'\0') {
			buff2[j]=L'\0';
			break;
		}
		else {
			wclen = is_surrogate_pair(&buff1[i]) ? 2 : 1;
			               buff3[0]    =buff1[i];
			if (wclen==2){ buff3[1]    =buff1[i+1]; }
			               buff3[wclen]=L'\0';

			if ( lorder==1 &&( buff1[i]==' ' || buff1[i]=='\t' || buff3[0]==0x00A0
				|| buff3[0]==0x202F || buff3[0]==0x2060 || buff3[0]==0xFEFF )) {
				i++;
			}

			else if (buff1[i]<0x20 && buff1[i]!='\t') { /* ignore control characters */
				i++;
			}

			else if (buff1[i]<0x7F) {
				buff2[j]=buff1[i];
				i++;
				j++;
			}

			else if (buff1[i]<0xA0) { /* ignore control characters */
				i++;
			}

			else if (is_latin(buff3)||is_cyrillic(buff3)||is_greek(buff3)
				 ||is_jpn_kana(buff3)||is_kor_hngl(buff3)||is_zhuyin(buff3)
				 ||is_numeric(buff3)==1||is_type_symbol(buff3)==1
				 ||is_devanagari(buff3)||is_thai(buff3)||is_arabic(buff3)||is_hebrew(buff3)
					||is_type_mark_or_punct(buff3)) {
				buff2[j]=buff3[0];
				if (wclen==2) buff2[j+1]=buff3[1];
				i+=wclen;
				j+=wclen;
			}

			else {
				for (k=0;k<dlines;k++) {
/*   dictionary table   */
					if (u_strncmp(dictable[k].dic[0],&buff1[i],u_strlen(dictable[k].dic[0]))==0) {
						u_strncpy(&buff2[j],dictable[k].dic[1],u_strlen(dictable[k].dic[1]));
						i+=u_strlen(dictable[k].dic[0]);
						j+=u_strlen(dictable[k].dic[1]);
						break;
					}
				}
				if ((k==dlines)&&(elines!=0)) {
/*   environment dictionary table   */
					for (k=0;k<elines;k++) {
						if (u_strncmp(envdic[k].dic[0],&buff1[i],u_strlen(envdic[k].dic[0]))==0) {
							u_strncpy(&buff2[j],envdic[k].dic[1],u_strlen(envdic[k].dic[1]));
							i+=u_strlen(dictable[k].dic[0]);
							j+=u_strlen(envdic[k].dic[1]);
							break;
						}
					}
				}
				if (((k==dlines)&&(elines==0))||((k==elines)&&(elines!=0))) {
					if (is_hanzi(buff3) || is_numeric(buff3) || is_type_symbol(buff3) || force==1) {
/*   forced convert   */
						buff2[j]=buff3[0];
						if (wclen==2) buff2[j+1]=buff3[1];
						i+=wclen;
						j+=wclen;
					}
					else {
						widechar_to_multibyte(errbuff,BUFFERLEN2,&buff1[i]);
						snprintf(errbuff2,BUFFERLEN3,"\nError: %s is no entry in dictionary file ",errbuff);
						fputs(errbuff2,efp);
						if (efp!=stderr) fputs(errbuff2,stderr);
						return -1;
					}
				}
			}
		}
	}
	return 0;
}

int pnumconv(char *page, int attr)
{
	int i,cc=0;

	switch (page_precedence[attr]) {
	case 'a':
		cc=page[0]-'a'+1;
		break;

	case 'A':
		cc=page[0]-'A'+1;
		break;

	case 'n':
		cc=atoi(page);
		break;

	case 'r':
	case 'R':
		for (i=0;i<strlen(page);i++) {
			switch (page[i]) {
			case 'i':
			case 'I':
				if (i==0) cc=1;
				else cc++;
				break;

			case 'v':
			case 'V':
				if (i==0) cc=5;
				else {
					switch (page[i-1]) {
					case 'i':
					case 'I':
						cc+=3;
						break;

					case 'x':
					case 'X':
					case 'l':
					case 'L':
					case 'c':
					case 'C':
					case 'd':
					case 'D':
					case 'm':
					case 'M':
						cc+=5;
						break;

					default:
						break;
					}
				}
				break;

			case 'x':
			case 'X':
				if (i==0) cc=10;
				else {
					switch (page[i-1]) {
					case 'i':
					case 'I':
						cc+=8;
						break;

					case 'x':
					case 'X':
					case 'l':
					case 'L':
					case 'c':
					case 'C':
					case 'd':
					case 'D':
					case 'm':
					case 'M':
						cc+=10;
						break;

					default:
						break;
					}
				}
				break;

			case 'l':
			case 'L':
				if (i==0) cc=50;
				else {
					switch (page[i-1]) {
					case 'x':
					case 'X':
						cc+=30;
						break;

					case 'c':
					case 'C':
					case 'd':
					case 'D':
					case 'm':
					case 'M':
						cc+=50;
						break;

					default:
						break;
					}
				}
				break;

			case 'c':
			case 'C':
				if (i==0) cc=100;
					switch (page[i-1]) {
					case 'x':
					case 'X':
						cc+=80;
						break;

					case 'c':
					case 'C':
					case 'd':
					case 'D':
					case 'm':
					case 'M':
						cc+=100;
						break;

					default:
						break;
					}
				break;

			case 'd':
			case 'D':
				if (i==0) cc=500;
				else {
					switch (page[i-1]) {
					case 'c':
					case 'C':
						cc+=300;
						break;

					case 'm':
					case 'M':
						cc+=500;
						break;

					default:
						break;
					}
				}
				break;

			case 'm':
			case 'M':
				if (i==0) cc=1000;
					switch (page[i-1]) {
					case 'c':
					case 'C':
						cc+=800;
						break;

					case 'm':
					case 'M':
						cc+=1000;
						break;

					default:
						break;
					}
				break;

			default:
				break;
			}
		}
		break;
	default:
		break;
	}
	return cc;
}
