#include "mendex.h"

#include <kpathsea/tex-file.h>

#include "exkana.h"
#include "exvar.h"

#define BUFSIZE 65535

static int getestr(char *buff, char *estr);
static void chkpageattr(struct page *p);
static void copy_multibyte_char(char *buff1, char *buff2, int *i, int *j);

/*   read idx file   */
int idxread(char *filename, int start)
{
	int i,j,k,l,m,n,cc,indent,wflg,flg=0,bflg=0,nest,esc,quo,eflg=0,pacc,preject;
	char buff[BUFSIZE],wbuff[BUFSIZE],estr[256],table[BUFSIZE];
	UChar utable[BUFSIZE],*tmp1,*tmp2;
	FILE *fp;

	pacc=acc;
	preject=reject;

	if (filename==NULL) {
		fp=stdin;
#ifdef WIN32
		setmode(fileno(fp), _O_BINARY);
#endif
		verb_printf(efp, "Scanning input file stdin.");
	}
	else {
		if(kpse_in_name_ok(filename))
			fp=fopen(filename,"rb");
		else
			fp=NULL;
		if (fp==NULL) {
			sprintf(buff,"%s.idx",filename);
			if(kpse_in_name_ok(buff))
				fp=fopen(buff,"rb");
			else
				fp=NULL;
			if (fp==NULL) {
				warn_printf(efp,"Warning: Couldn't find input file %s.\n",filename);
				return 1;
			}
			else strcpy(filename,buff);
		}
		verb_printf(efp,"Scanning input file %s.",filename);
	}

	for (i=start,n=1;;i++,n++) {
		if (!(i%100))
			ind=(struct index *)xrealloc(ind,sizeof(struct index)*(i+100));
LOOP:
		ind[i].lnum=n;
		if (fgets(buff,sizeof(buff)-1,fp)==NULL) break;
		for (j=bflg=cc=0;j<strlen(buff);j++) {
			if (buff[j]!=' ' && buff[j]!='\n') cc=1;
			if (bflg==0) {
				if (strncmp(&buff[j],keyword,strlen(keyword))==0) {
					j+=strlen(keyword);
					bflg=1;
				}
			}
			if (bflg==1) {
				if (buff[j]==arg_open) {
					j++;
					break;
				}
			}
		}
		if (j==strlen(buff)) {
			if (cc) reject++;
			i--;
			continue;
		}
		indent=wflg=k=nest=esc=quo=0;

/*   analize words   */

		for (;;j++,k++) {
			if (buff[j]=='\n' || buff[j]=='\0') {
				verb_printf(efp,"\nWarning: Incomplete first argument in %s, line %d.",filename,ind[i].lnum);
				warn++;
				n++;
				goto LOOP;
			}

			if (buff[j]==quote && esc==0 && quo==0) {
				k--;
				quo=1;
				continue;
			}

			if (quo==0 && buff[j]==escape) {
				esc=1;
			}

			if (quo==0) {
				if (esc==0 && buff[j]==arg_open) {
					nest++;
					wbuff[k]=buff[j];
					continue;
				}
				else if (esc==0 && buff[j]==arg_close && nest>0) {
					nest--;
					wbuff[k]=buff[j];
					continue;
				}
				if (nest==0) {
					if (buff[j]==level) {
						esc=0;
						if (indent>=2) {
							fprintf(efp,"\nError: Extra `%c\' in %s, line %d.",level,filename,ind[i].lnum);
							if (efp!=stderr) fprintf(stderr,"\nError: Extra `%c\' in %s, line %d.",level,filename,ind[i].lnum);
							eflg++;
							reject++;
							n++;
							goto LOOP;
						}
						wbuff[k]='\0';
						multibyte_to_widechar(utable,BUFSIZE,wbuff);
						ind[i].idx[indent]=u_xstrdup(utable);
						if (!wflg) ind[i].org[indent]=NULL;
						indent++;
						wflg=0;
						k= -1;
						continue;
					}
					else if (buff[j]==actual) {
						esc=0;
						if (wflg) {
							fprintf(efp,"\nError: Extra `%c\' in %s, line %d.",actual,filename,ind[i].lnum);
							if (efp!=stderr) fprintf(stderr,"\nError: Extra `%c\' in %s, line %d.",actual,filename,ind[i].lnum);
							eflg++;
							reject++;
							n++;
							goto LOOP;
						}
						wbuff[k]='\0';
						multibyte_to_widechar(utable,BUFSIZE,wbuff);
						ind[i].org[indent]=u_xstrdup(utable);
						wflg=1;
						k= -1;
						continue;
					}
					else if ((esc==0 && buff[j]==arg_close) || buff[j]==encap) {
						esc=0;
						if (buff[j]==encap) {
							j++;
							cc=getestr(&buff[j],estr);
							if (cc<0) {
								fprintf(efp,"\nBad encap string in %s, line %d.",filename,ind[i].lnum);
								if (efp!=stderr) fprintf(stderr,"\nBad encap string in %s, line %d.",filename,ind[i].lnum);
								eflg++;
								reject++;
								n++;
								goto LOOP;
							}
							j+=cc;
						}
						else estr[0]='\0';

						wbuff[k]='\0';
						multibyte_to_widechar(utable,BUFSIZE,wbuff);
						ind[i].idx[indent]=u_xstrdup(utable);
						if (u_strlen(ind[i].idx[indent])==0) {
							if (wflg) {
								u_strcpy(ind[i].idx[indent],ind[i].org[indent]);
							}
							else if (indent>0) {
								indent--;
							}
							else {
								verb_printf(efp,"\nWarning: Illegal null field in %s, line %d.",filename,ind[i].lnum);
								warn++;
								n++;
								goto LOOP;
							}
						}
						if (!wflg) {
							ind[i].org[indent]=NULL;
						}
						break;
					}
				}
				if (bcomp==1) {
					if (buff[j]==' ' || buff[j]=='\t') {
						esc=0;
						if (k==0) {
							k--;
							continue;
						}
						else if (buff[j+1]==' ' || buff[j+1]=='\t' || buff[j+1]==encap || buff[j+1]==arg_close || buff[j+1]==actual || buff[j+1]==level) {
							k--;
							continue;
						}
						else if (buff[j]=='\t') {
							wbuff[k]=' ';
							continue;
						}
					}
				}
			}
			else quo=0;

			if (buff[j]!=escape) esc=0;
			copy_multibyte_char(buff, wbuff, &j, &k);
		}
		ind[i].words=indent+1;

/*   kana-convert   */

		for (k=0;k<ind[i].words;k++) {
			if (ind[i].org[k]==NULL) {
				cc=convert(ind[i].idx[k],utable);
				if (cc==-1) {
					fprintf(efp,"in %s, line %d.",filename,ind[i].lnum);
					if (efp!=stderr) fprintf(stderr,"in %s, line %d.",filename,ind[i].lnum);
					eflg++;
					reject++;
					n++;
					goto LOOP;
				}
				ind[i].dic[k]=u_xstrdup(utable);
			}
			else {
				cc=convert(ind[i].org[k],utable);
				if (cc==-1) {
					fprintf(efp,"in %s, line %d.",filename,ind[i].lnum);
					if (efp!=stderr) fprintf(stderr,"in %s, line %d.",filename,ind[i].lnum);
					eflg++;
					reject++;
					n++;
					goto LOOP;
				}
				ind[i].dic[k]=u_xstrdup(utable);
			}
		}
		acc++;

/*   page edit   */

		if (i==0) {
			ind[0].num=0;
			ind[0].p=xmalloc(sizeof(struct page)*16);
			for (;buff[j]!=arg_open && buff[j]!='\n' && buff[j]!='\0';j++);
			if (buff[j]=='\n' || buff[j]=='\0') {
				verb_printf(efp,"\nWarning: Missing second argument in %s, line %d.",filename,ind[i].lnum);
				acc--;
				reject++;
				warn++;
				n++;
				goto LOOP;
			}
			j++;
			for (k=nest=0;;j++,k++) {
				if (buff[j]=='\n' || buff[j]=='\0') {
					verb_printf(efp,"\nWarning: Incomplete second argument in %s, line %d.",filename,ind[i].lnum);
					acc--;
					reject++;
					warn++;
					n++;
					goto LOOP;
				}
				if (buff[j]==arg_open)
					nest++;
				else if (buff[j]==arg_close) {
					if (nest==0) {
						table[k]='\0';
						ind[0].p[0].page=xstrdup(table);
						break;
					}
					else nest--;
				}
				copy_multibyte_char(buff, table, &j, &k);
			}
			ind[0].p[0].enc=xstrdup(estr);
			chkpageattr(&ind[0].p[0]);
		}
		else {
			for (l=0;l<i;l++) {
				flg=0;
				if (ind[i].words!=ind[l].words) continue;
				for (flg=1,m=0;m<ind[i].words;m++) {
					if (u_strcmp(ind[i].idx[m],ind[l].idx[m])!=0) {
						flg=0;
						break;
					}
					if (u_strcmp(ind[i].dic[m],ind[l].dic[m])!=0) {
						if (ind[i].org[m]!=NULL) tmp1=ind[i].org[m];
						else tmp1=ind[i].idx[m];

						if (ind[l].org[m]!=NULL) tmp2=ind[l].org[m];
						else tmp2=ind[l].idx[m];

						widechar_to_multibyte(wbuff,BUFSIZE,tmp1);
						verb_printf(efp,"\nWarning: Sort key \"%s\"",wbuff);
						widechar_to_multibyte(wbuff,BUFSIZE,tmp2);
						verb_printf(efp," is different from previous key \"%s\"",wbuff);
						widechar_to_multibyte(wbuff,BUFSIZE,ind[i].idx[m]);
						verb_printf(efp," for same index \"%s\" in %s, line %d.",wbuff,filename,ind[i].lnum);
						warn++;
						flg=0;
						break;
					}
				}
				if (flg==1) break;
			}

			if (flg==1) {
				for (m=0;m<ind[i].words;m++) {
					free(ind[i].idx[m]);
					free(ind[i].dic[m]);
				}

				i--;
				for (;buff[j]!=arg_open && buff[j]!='\n' && buff[j]!='\0';j++);
				if (buff[j]=='\n' || buff[j]=='\0') {
					verb_printf(efp,"\nWarning: Missing second argument in %s, line %d.",filename,ind[i].lnum);
					acc--;
					reject++;
					warn++;
					n++;
					i++;
					goto LOOP;
				}
				j++;
				for (k=nest=0;;j++,k++) {
					if (buff[j]=='\n' || buff[j]=='\0') {
						verb_printf(efp,"\nWarning: Incomplete second argument in %s, line %d.",filename,ind[i].lnum);
						warn++;
						n++;
						i++;
						goto LOOP;
					}
					if (buff[j]==arg_open)
						nest++;
					else if (buff[j]==arg_close) {
						if (nest==0) break;
						else nest--;
					}
					copy_multibyte_char(buff, table, &j, &k);
				}

				table[k]='\0';

				for (k=0;k<=ind[l].num;k++) {
					if (strcmp(ind[l].p[k].page,table)==0) {
						if (strcmp(ind[l].p[k].enc,estr)==0) break;
					}
				}

				if (k>ind[l].num) {
					ind[l].num++;
					if (!((ind[l].num)%16)) ind[l].p=(struct page *)xrealloc(ind[l].p,sizeof(struct page)*((int)((ind[l].num)/16)+1)*16);

					ind[l].p[ind[l].num].page=xstrdup(table);

					ind[l].p[ind[l].num].enc=xstrdup(estr);
					chkpageattr(&ind[l].p[ind[l].num]);
				}
			}
			else {
				ind[i].num=0;
				ind[i].p=xmalloc(sizeof(struct page)*16);
				for (;buff[j]!=arg_open && buff[j]!='\n' && buff[j]!='\0';j++);
				if (buff[j]=='\n' || buff[j]=='\0') {
					verb_printf(efp,"\nWarning: Missing second argument in %s, line %d.",filename,ind[i].lnum);
					acc--;
					reject++;
					warn++;
					n++;
					goto LOOP;
				}
				j++;
				for (k=nest=0;;j++,k++) {
					if (buff[j]=='\n' || buff[j]=='\0') {
						verb_printf(efp,"\nWarning: Incomplete second argument in %s, line %d.",filename,ind[i].lnum);
						acc--;
						reject++;
						warn++;
						n++;
						goto LOOP;
					}
					if (buff[j]==arg_open)
						nest++;
					if (buff[j]==arg_close) {
						if (nest==0) {
							table[k]='\0';
							ind[i].p[0].page=xstrdup(table);
							break;
						}
						else nest--;
					}
					copy_multibyte_char(buff, table, &j, &k);
				}
				ind[l].p[0].enc=xstrdup(estr);
				chkpageattr(&ind[i].p[0]);
			}
		}
	}
	lines=i;
	if (filename != NULL) fclose(fp);

	verb_printf(efp,"...done (%d entries accepted, %d rejected).\n",acc-pacc, reject-preject);
	return eflg;
}

/*   pic up encap string   */
static int getestr(char *buff, char *estr)
{
	int i,nest=0;

	for (i=0;i<strlen(buff);i++) {
		if (buff[i]==encap) {
			if (i>0) {
				if ((unsigned char)buff[i-1]<0x80) {
					estr[i]=buff[i];
					i++;
				}
			}
			else {
				estr[i]=buff[i];
				i++;
			}
		}
		if (nest==0 && buff[i]==arg_close) {
			estr[i]='\0';
			return i;
		}
		if (buff[i]==arg_open) nest++;
		else if (buff[i]==arg_close) nest--;
		copy_multibyte_char(buff, estr, &i, NULL);
	}

	return -1;
}

static void copy_multibyte_char(char *buff1, char *buff2, int *i, int *j)
{
	int len;

	if ((unsigned char)buff1[*i]<0x80) {
		buff2[j ? *j : *i] = buff1[*i];
		return;
	}
	len = multibytelen((unsigned char)buff1[*i]);
	if (len<0) {
		verb_printf(efp,"\nWarning: Illegal input of lead byte 0x%x in UTF-8.",(unsigned char)buff1[*i]);
		len=1; /* copy one byte when illegal lead byte */
	}
	while(len--) {
		buff2[j ? (*j)++ : *i] = buff1[*i];
		(*i)++;
	}
	if (j!=NULL) (*j)--;
	(*i)--;
}

int multibyte_to_widechar(UChar *wcstr, int32_t size, char *mbstr)
{
	int32_t len, olen, nsub;
	UErrorCode err;

	len=strlen(mbstr)+1;
	do {
		err = U_ZERO_ERROR;
		len--;
		u_strFromUTF8WithSub(NULL, 0, &olen, mbstr, len, 0xFFFD, &nsub, &err);
	} while (olen>=size);
	err = U_ZERO_ERROR;
	u_strFromUTF8WithSub(wcstr, size, &olen, mbstr, len, 0xFFFD, &nsub, &err);
	return olen;
}

int widechar_to_multibyte(char *mbstr, int32_t size, UChar *wcstr)
{
	int32_t len, olen, nsub;
	UErrorCode err;

	len=u_strlen(wcstr)+1;
	do {
		err = U_ZERO_ERROR;
		len--;
		u_strToUTF8WithSub(NULL, 0, &olen, wcstr, len, 0xFFFD, &nsub, &err);
	} while (olen>=size);
	err = U_ZERO_ERROR;
	u_strToUTF8WithSub(mbstr, size, &olen, wcstr, len, 0xFFFD, &nsub, &err);
	return olen;
}

static void chkpageattr(struct page *p)
{
	int i,j,cc=0,cnt,pplen,pclen;
	char buff[16],*pcpos,*page0;
	static char pattr_prev[PAGE_COMPOSIT_DEPTH] = {0};

	pplen=strlen(page_precedence);
	pclen=strlen(page_compositor);
	for (i=0;i<strlen(p->page);i++) {
		page0=&p->page[i];
		if (strncmp(page_compositor,page0,pclen)==0) {
			p->attr[cc]=pattr[cc];
			cc++;
			i+=pclen-1;
			if (cc>=PAGE_COMPOSIT_DEPTH) {
				if (pclen>0)
					verb_printf(efp, "\nToo many fields of page number \"%s\".\n", p->page);
				else
					verb_printf(efp, "\nIllegular page_comositor specification.\n");
				exit(253);
			}
		}
		else {
			cnt=0;
			if (!((*page0>='0' && *page0<='9') || (*page0>='A' && *page0<='Z') || (*page0>='a' && *page0<='z'))) {
				p->attr[cc]= -1;
				if (cc<2) p->attr[++cc]= -1;
				return;
			}
			pcpos=strstr(page0,page_compositor);
			j=pcpos ? pcpos-page0 : strlen(page0);
			if (j>15) {
				verb_printf(efp, "\nToo long page number string \"%s\".\n", page0);
				exit(253);
			}
			strncpy(buff,page0,j);
			buff[j]='\0';
ATTRLOOP:
			cnt++;
			if (cnt>pplen) {
				verb_printf(efp, "\nFailed to find page type for page \"%s\" in page_precedence specification (%s).\n",
					    page0, page_precedence);
				exit(253);
			}

			switch(page_precedence[pattr[cc]]) {
			case 'r':
				if (strchr("ivxlcdm",*page0)==NULL ||
				    (strchr("lcdm",*page0) && strchr(page_precedence,'a') && strlen(buff)==1 &&
				        pattr_prev[cc]!='r')) {
					/* heuristic detection as alphabet since L=50, C=100, D=100, M=1000 are quite large */
					if (pattr[cc]<pplen-1)
						pattr[cc]++;
					else pattr[cc]=0;
					for (j=cc+1;j<PAGE_COMPOSIT_DEPTH;j++) pattr[j]=0;
					goto ATTRLOOP;
				}
				break;
			case 'R':
				if (strchr("IVXLCDM",*page0)==NULL ||
				    (strchr("LCDM",*page0) && strchr(page_precedence,'A') && strlen(buff)==1 &&
				        pattr_prev[cc]!='R')) {
					/* heuristic detection as alphabet since L=50, C=100, D=100, M=1000 are quite large */
					if (pattr[cc]<pplen-1)
						pattr[cc]++;
					else pattr[cc]=0;
					for (j=cc+1;j<PAGE_COMPOSIT_DEPTH;j++) pattr[j]=0;
					goto ATTRLOOP;
				}
				break;
			case 'n':
				if (*page0<'0' || *page0>'9') {
					if (pattr[cc]<pplen-1)
						pattr[cc]++;
					else pattr[cc]=0;
					for (j=cc+1;j<PAGE_COMPOSIT_DEPTH;j++) pattr[j]=0;
					goto ATTRLOOP;
				}
				break;
			case 'a':
				if (*page0<'a' || *page0>'z' || strlen(buff)>1 ||
				    (strchr("ivx",*page0) && strchr(page_precedence,'r') &&
				        pattr_prev[cc]!='a')) {
					/* heuristic detection as roman number since I=1, V=5, X=10 are quite small */
					if (pattr[cc]<pplen-1)
						pattr[cc]++;
					else pattr[cc]=0;
					for (j=cc+1;j<PAGE_COMPOSIT_DEPTH;j++) pattr[j]=0;
					goto ATTRLOOP;
				}
				break;
			case 'A':
				if (*page0<'A' || *page0>'Z' || strlen(buff)>1 ||
				    (strchr("IVX",*page0) && strchr(page_precedence,'R') &&
				        pattr_prev[cc]!='A')) {
					/* heuristic detection as roman number since I=1, V=5, X=10 are quite small */
					if (pattr[cc]<pplen-1)
						pattr[cc]++;
					else pattr[cc]=0;
					for (j=cc+1;j<PAGE_COMPOSIT_DEPTH;j++) pattr[j]=0;
					goto ATTRLOOP;
				}
				break;
			default:
				verb_printf(efp, "\nUnknown page type '%c' in page_precedence specification (%s).\n",
					   page_precedence[pattr[cc]], page_precedence);
				exit(253);
			}
			pattr_prev[cc] = page_precedence[pattr[cc]];
		}
	}
	p->attr[cc]=pattr[cc];
	if (cc<2) p->attr[++cc]= -1;
}
