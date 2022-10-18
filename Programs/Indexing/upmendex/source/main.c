#include "mendex.h"
#include "version.h"
#include <kpathsea/tex-file.h>
#include <kpathsea/variable.h>

#include "kana.h"
#include "hanzi.h"
#include "var.h"

#include "kp.h"

char *styfile[64],*idxfile[256],*indfile,*dicfile,*logfile;

/* default paths */
#ifndef DEFAULT_INDEXSTYLES
#define DEFAULT_INDEXSTYLES "."
#endif
#ifndef DEFAULT_INDEXDICTS
#define DEFAULT_INDEXDICTS "."
#endif
KpathseaSupportInfo kp_ist,kp_dict;

int main(int argc, char **argv)
{
	int i,j,k,cc=0,startpagenum=-1,ecount=0,chkopt=1;
	const char *envbuff;
	UVersionInfo icuVersion;
	char icu_version[U_MAX_VERSION_STRING_LENGTH] = "";

#if !defined(MIKTEX)
#ifdef WIN32
	char **av;
	int ac;
	_setmaxstdio(2048);
#endif
#endif
	kpse_set_program_name(argv[0], "upmendex");
	u_getVersion(icuVersion);
	u_versionToString(icuVersion, icu_version);

#if !defined(MIKTEX)
#ifdef WIN32
	file_system_codepage = CP_UTF8;
	is_cp932_system = 0;
	if (get_command_line_args_utf8("utf-8", &ac, &av)) {
		argv = av;
		argc = ac;
	}
#endif
#endif

	kp_ist.var_name = "INDEXSTYLE";
	kp_ist.path = DEFAULT_INDEXSTYLES; /* default path. */
	kp_ist.suffix = "ist";
	KP_entry_filetype(&kp_ist);
	kp_dict.var_name = "INDEXDICTIONARY";
	kp_dict.path = DEFAULT_INDEXDICTS; /* default path */
	kp_dict.suffix = "dict";
	KP_entry_filetype(&kp_dict);

/*   check options   */

	for (i=1,j=k=0;i<argc && j<256;i++) {
		if ((argv[i][0]=='-')&&(strlen(argv[i])>=2)&&chkopt) {
			switch (argv[i][1]) {
			case 'c':
				bcomp=1;
				break;

			case 'd':
				if ((argv[i][2]=='\0')&&(i+1<argc)) {
					dicfile=xstrdup(argv[++i]);
				}
				else {
					dicfile=xstrdup(&argv[i][2]);
				}
				break;

			case 'f':
				force=1;
				break;

			case 'g':
				gflg=1;
				break;

			case 'i':
				fsti=1;
				break;

			case 'l':
				lorder=1;
				break;

			case 'o':
				if ((argv[i][2]=='\0')&&(i+1<argc)) {
					indfile=xstrdup(argv[++i]);
				}
				else {
					indfile=xstrdup(&argv[i][2]);
				}
				break;

			case 'p':
				if ((argv[i][2]=='\0')&&(i+1<argc)) {
					i++;
					if (strcmp(argv[i],"any")==0) fpage=2;
					else if (strcmp(argv[i],"odd")==0) fpage=3;
					else if (strcmp(argv[i],"even")==0) fpage=4;
					else {
						fpage=1;
						startpagenum=atoi(argv[i]);
					}
				}
				else {
					if (strcmp(&argv[i][2],"any")==0) fpage=2;
					else if (strcmp(&argv[i][2],"odd")==0) fpage=3;
					else if (strcmp(&argv[i][2],"even")==0) fpage=4;
					else {
						fpage=1;
						startpagenum=atoi(&argv[i][2]);
					}
				}
				break;

			case 'q':
				verb=0;
				break;

			case 't':
				if ((argv[i][2]=='\0')&&(i+1<argc)) {
					logfile=xstrdup(argv[++i]);
				}
				else {
					logfile=xstrdup(&argv[i][2]);
				}
				break;

			case 'r':
				prange=0;
				break;

			case 's':
				if (k==64) {
					fprintf (stderr, "Too many style files.\n");
					exit(255);
				}
				if ((argv[i][2]=='\0')&&(i+1<argc)) {
					styfile[k]=xstrdup(argv[++i]);
				}
				else {
					styfile[k]=xstrdup(&argv[i][2]);
				}
				k++;
				break;

			case 'v':
				debug=1;
				break;

			case '-':
				if (strlen(argv[i])==2) chkopt=0;
				if (strcmp(argv[i],"--help")!=0) break;

			default:
				fprintf(stderr,"upmendex - index processor, %s (%s).\n",VERSION, TL_VERSION);
				fprintf(stderr," Copyright 2009 ASCII MEDIA WORKS, 2015-2022 TANAKA Takuji\n");
				fprintf(stderr," using ICU version %s\n",icu_version);
				fprintf(stderr,"usage:\n");
				fprintf(stderr,"%% upmendex [-ilqrcgf] [-s sty] [-d dic] [-o ind] [-t log] [-p no] [--] [idx0 idx1 ...]\n");
				fprintf(stderr,"options:\n");
				fprintf(stderr,"-i      use stdin as the input file.\n");
				fprintf(stderr,"-l      use letter ordering.\n");
				fprintf(stderr,"-q      quiet mode.\n");
				fprintf(stderr,"-r      disable implicit page formation.\n");
				fprintf(stderr,"-c      compress blanks. (ignore leading and trailing blanks.)\n");
				fprintf(stderr,"-g      make Japanese index head <%s>.\n", AKASATANAutf8);
				fprintf(stderr,"-f      force to output unknown scripts.\n");
				fprintf(stderr,"-s sty  take sty as style file.\n");
				fprintf(stderr,"-d dic  take dic as dictionary file.\n");
				fprintf(stderr,"-o ind  take ind as the output index file.\n");
				fprintf(stderr,"-t log  take log as the error log file.\n");
				fprintf(stderr,"-p no   set the starting page number of index.\n");
				fprintf(stderr,"idx...  input files.\n");
				fprintf(stderr,"\nEmail bug reports to %s.\n", BUG_ADDRESS);
				exit(0);
				break;
			}
		}
		else {
			cc=strlen(argv[i])+6;
			idxfile[j]=xmalloc(cc);
			strcpy(idxfile[j++],argv[i]);
		}
	}
	idxcount=j+fsti;

/*   check option errors   */

	if (idxcount==0) idxcount=fsti=1;

	if (styfile[0]==NULL) {
		envbuff=kpse_var_value("INDEXDEFAULTSTYLE");
		if (envbuff!=NULL) {
			styfile[0]=xstrdup(envbuff);
		}
	}

	if (!indfile &&(idxcount-fsti>0)) {
		indfile=xmalloc(strlen(idxfile[0])+6);
		for (i=strlen(idxfile[0]);i>=0;i--) {
			if (idxfile[0][i]=='.') {
				strncpy(indfile,idxfile[0],i);
				sprintf(&indfile[i],".ind");
				break;
			}
		}
		if (i==-1) sprintf(indfile,"%s.ind",idxfile[0]);
	}

	if (!logfile && (idxcount-fsti > 0)) {
		logfile=xmalloc(strlen(idxfile[0])+6);
		for (i=strlen(idxfile[0]);i>=0;i--) {
			if (idxfile[0][i]=='.') {
				strncpy(logfile,idxfile[0],i);
				sprintf(&logfile[i],".ilg");
				break;
			}
		}
		if (i==-1) sprintf(logfile,"%s.ilg",idxfile[0]);
		}
	if (logfile && kpse_out_name_ok(logfile))
		efp=fopen(logfile,"wb");
	if(efp == NULL) {
		efp=stderr;
		logfile=xstrdup("stderr");
	}

	if (strcmp(argv[0],"makeindex")==0) {
		verb_printf(efp,"This is Not `MAKEINDEX\', But `UPMENDEX\' %s [ICU %s] (%s).\n",
			    VERSION, icu_version, TL_VERSION);
	}
	else {
		verb_printf(efp,"This is upmendex %s [ICU %s] (%s).\n",
			    VERSION, icu_version, TL_VERSION);
	}

/*   init kanatable   */

	initkanatable();

/*   init hangul,devanagari,thai *_head table   */
	u_strcpy(hangul_head,GANADA);
	u_strcpy(devanagari_head,DVNG_HEAD);
	u_strcpy(thai_head,THAI_HEAD);

	for (k=0;styfile[k]!=NULL;k++) {
		styread(styfile[k]);
	}

	set_icu_attributes();

/*   read dictionary   */

	ecount+=dicread(dicfile);

	switch (letter_head) {
	case 0:
	case 1:
		if (gflg==1) {
			u_strcpy(atama,akasatana);
		}
		else {
			u_strcpy(atama,aiueo);
		}
		break;

	case 2:
		if (gflg==1) {
			u_strcpy(atama,AKASATANA);
		}
		else {
			u_strcpy(atama,AIUEO);
		}
		break;

	default:
		break;
	}
	if (u_strlen(kana_head)>0) u_strcpy(atama,kana_head);

/*   read idx file   */

	lines=0;
	ecount=0;
	ind=xmalloc(sizeof(struct index));

	for (i=0;i<idxcount-fsti;i++) {
		ecount+=idxread(idxfile[i],lines);
	}
	if (fsti==1) {
		ecount+=idxread(NULL,lines);
	}
	verb_printf(efp,"%d entries accepted, %d rejected.\n",acc,reject);

	if (ecount!=0) {
		verb_printf(efp,"%d errors, written in %s.\n",ecount,logfile);
		lines=0;
	}
	if (lines==0) {
		verb_printf(efp,"Nothing written in output file.\n");
		if (efp!=stderr) fclose(efp);
		exit(255);
	}

/*   sort index   */

	verb_printf(efp,"Sorting index.");

	scount=0;
	wsort(ind,lines);

	verb_printf(efp,"...done(%d comparisons).\n",scount);

/*   sort pages   */

	verb_printf(efp,"Sorting pages.");

	scount=0;
	pagesort(ind,lines);

	verb_printf(efp,"...done(%d comparisons).\n",scount);

/*   get last page   */

	if ((fpage>1)&&(idxcount-fsti>0)) cc=lastpage(idxfile[0]);

	switch (fpage) {
	case 2:
		startpagenum=cc+1;
		break;

	case 3:
		if ((cc+1)%2==0) startpagenum=cc+2;
		else startpagenum=cc+1;
		break;

	case 4:
		if ((cc+1)%2==1) startpagenum=cc+2;
		else startpagenum=cc+1;
		break;
		
	default:
		break;
	}

/*   write indfile   */

	verb_printf(efp,"Making index file.");

	indwrite(indfile,ind,startpagenum);

	verb_printf(efp,"...done.\n");

	if (idxcount-fsti==0) indfile=xstrdup("stdout");

	verb_printf(efp,"%d warnings, written in %s.\n",warn,logfile);
	verb_printf(efp,"Output written in %s.\n",indfile);
	if (efp!=stderr) fclose(efp);

	return 0;
}
