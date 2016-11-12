/*********************************/
/* Copyright Vafa Khalighi 2016 */
/* Copyright Khaled Hosny 2015 */
/* Copyright Artyom Tonkikh 2007 */
/* License GPL                   */
/*********************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fribidi/fribidi.h>

#include "fribidixetex-defines.h"
#include "fribidixetex-io.h"
#include "fribidixetex-bidi.h"

void help(void)
{
	fprintf(stderr,
			"usage: fribidixetex [ parameters ] [ inputfilename ]\n"
			"       -o file_name.tex             - output file name\n"
			"       -e utf8 | iso8859-8 | cp1255 - encoding\n"
			"       -t utf8 | iso8859-8 | cp1255 - output encoding\n"
			"       -d       transalte only - not apply bidi\n"
			"                this is usefull for latex2html that has native\n"
			"                bidirectional support\n"
			"       -m       replace '--'  &   '---'\n"
			"                by   '\\fribidixetexLRE{--} & \\fribidixetexLRE{'---'}\n"
			"       -n       no mirroring - do not mirror parethesis\n"
			"                for engines that do that natively (like XeTeX)\n"
	);
	exit(1);
}


/* Read cmd line parameters */
void read_parameters(int argc,char **argv,
					char **fname_in,char **fname_out,
					int *encoding,int *out_encoding,
					int *replace_minus,int *transalte_only,
					int *no_mirroring)
{
	int i;
	int cnt1=0,cnt2=0,cnt3=0,cnt4=0;
	int *ptr;

	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"-h")==0) {
			help();
		}
		else if(strcmp(argv[i],"-d")==0) {
			*transalte_only = 1;
		}
		else if(strcmp(argv[i],"-m")==0) {
			*replace_minus = 1;
		}
		else if(strcmp(argv[i],"-n")==0) {
			*no_mirroring = 1;
		}
		else if(strcmp(argv[i],"-o")==0) {
			i++;
			if(i>=argc){
				help();
			}
			*fname_out = argv[i];
			cnt1++;
		}
		else if(strcmp(argv[i],"-e")==0 || strcmp(argv[i],"-t")==0) {
			if(i+1>=argc){
				help();
			}

			if(argv[i][1]=='e'){
				ptr=encoding;
				cnt2++;
			}
			else{
				ptr=out_encoding;
				cnt4++;
			}
			i++;
			if(strcmp(argv[i],"utf8")==0) {
				*ptr=ENC_UTF_8;
			}
			else if(strcmp(argv[i],"cp1255")==0) {
				*ptr=ENC_CP1255;
			}
			else if(strcmp(argv[i],"iso8859-8")==0) {
				*ptr=ENC_ISO_8859_8;
			}
			else {
				help();
			}
		}
		else {
			*fname_in=argv[i];
			cnt3++;
		}
	}
	if(cnt1>1 || cnt2>1 || cnt3>1 || cnt4>1){
		help();
	}
}

/* Global buffers */

static FriBidiChar text_line_in[MAX_LINE_SIZE];
static FriBidiChar text_line_out[MAX_LINE_SIZE];

/****************************
 ******** M A I N ***********
 ****************************/
int main(int argc,char **argv)
{
	char *fname_in=NULL,*fname_out=NULL;
	int encoding=ENC_DEFAULT,out_encoding = -1;
	int replace_minus = 0;
	int transalte_only = 0;
	int no_mirroring = 0;

	FILE *f_in,*f_out;

	/******************
	 * Inicialization *
	 ******************/

	read_parameters(argc,argv,&fname_in,&fname_out,
			&encoding,&out_encoding,
			&replace_minus,&transalte_only,
			&no_mirroring);
	if(out_encoding == -1) {
		out_encoding = encoding;
	}

	if(!fname_in) {
		f_in = stdin;
	}
	else {
		if(!(f_in=fopen(fname_in,"r"))) {
			fprintf(stderr,"Failed to open %s for reading\n",fname_in);
			exit(1);
		}
	}

	if(!fname_out) {
		f_out = stdout;
	}
	else {
		if(!(f_out=fopen(fname_out,"w"))) {
			fprintf(stderr,"Failed to open %s for writing\n",fname_out);
			exit(1);
		}
	}


	/*************
	 * Main loop *
	 *************/

	io_init();

	bidi_init(f_out);

	while(io_read_line(text_line_in,encoding,f_in)) {

		if(bidi_process(text_line_in,text_line_out,
							replace_minus,transalte_only,no_mirroring))
		{
			/*If there is something to print */
			io_write_line(text_line_out,out_encoding,f_out);
		}

	}

	/**********
	 * Finish *
	 **********/

	if(f_out!=stdout) fclose(f_out);
	if(f_in!=stdin) fclose(f_in);

	bidi_finish();

	return 0;
}
