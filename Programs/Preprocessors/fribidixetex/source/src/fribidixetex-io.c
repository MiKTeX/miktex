#include <fribidi/fribidi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fribidixetex-io.h"

static char text_buffer[MAX_LINE_SIZE];
int io_line_number;

void io_init(void)
{
	io_line_number=0;
}

int io_read_line(FriBidiChar *text,int encoding,FILE *f)
{
	int len_char,len_uni;
	if(fgets(text_buffer,MAX_LINE_SIZE-1,f)==NULL) {
		return 0;
	}
	
	len_char=strlen(text_buffer);
	
	if(len_char==MAX_LINE_SIZE - 2) {
		fprintf(stderr,"Warning: line %d is too long\n",io_line_number+1);
	}
	if(len_char>0 && text_buffer[len_char-1]=='\n') {
		io_line_number++;
		text_buffer[len_char-1]=0;
		len_char--;
	}
	switch(encoding) {
		case ENC_ISO_8859_8:
			len_uni=fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_ISO8859_8,text_buffer,len_char,text);
			text[len_uni]=0;
			break;
		case ENC_CP1255:
			len_uni=fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_CP1255,text_buffer,len_char,text);
#if ( FRIBIDI_MAJOR_VERSION * 100 + FRIBIDI_MINOR_VERSION ) <= 10 // 0.10
			{
				int i;
				for(i=0;i<len_uni;i++) {
					if(text[i]==253 || text[i]==254)
						text[i] += (0x200E - 253);
				}
			}
#endif
			text[len_uni]=0;
			break;
		case ENC_UTF_8:
			if(len_char >= 3 && memcmp(text_buffer,"\xEF\xBB\xBF",3) == 0) {
				// remove BOM, support Win32 Notepad
				len_uni=fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8,text_buffer+3,len_char-3,text);
			}
			else {
				len_uni=fribidi_charset_to_unicode(FRIBIDI_CHAR_SET_UTF8,text_buffer,len_char,text);
			}
			text[len_uni]=0;
			break;
		default:
			fprintf(stderr,"Internal error - wrong encoding\n");
			exit(1);
	}
	return 1;
}

void io_write_line(FriBidiChar *text,int encoding,FILE *f)
{
	int len,char_len,tmp_len;
	/* Find legth of buffer */
	for(len=0;text[len] && len<MAX_LINE_SIZE-1;len++) /* NOTHING */;
		
	if(len>=MAX_LINE_SIZE-1) {
		fprintf(stderr,"Internall error - unterminated line\n");
		exit(1);
	}
	
	if(encoding == ENC_ISO_8859_8) {
		char_len=fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_ISO8859_8,text,len,text_buffer);
		text_buffer[char_len]=0;
		fprintf(f,"%s\n",text_buffer);
	}
	else if(encoding == ENC_CP1255) {
		char_len=fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_CP1255,text,len,text_buffer);
		text_buffer[char_len]=0;
		fprintf(f,"%s\n",text_buffer);
	}
	else if(encoding == ENC_UTF_8) {
		while(len) {
			/* Because maybe |unicode| < |utf8| 
			 * in order to prevetn overflow */
			if(len>MAX_LINE_SIZE/8){
				tmp_len=MAX_LINE_SIZE/8;
			}
			else {
				tmp_len = len;
			}
			char_len=fribidi_unicode_to_charset(FRIBIDI_CHAR_SET_UTF8,text,tmp_len,text_buffer);
			text_buffer[char_len]=0;
			fprintf(f,"%s",text_buffer);
			len  -= tmp_len;
			text += tmp_len;
		}
		fprintf(f,"\n");
	}
	else {
		fprintf(stderr,"Internal error - wrong encoding\n");
		exit(1);
	}
}
