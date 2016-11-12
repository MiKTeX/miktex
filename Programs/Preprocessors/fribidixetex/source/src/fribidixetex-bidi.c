#include "fribidixetex-defines.h"
#include "fribidixetex-bidi.h"
#include "fribidixetex-util.h"
#include "fribidixetex-dict.h"
#include "fribidixetex-ignore.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fribidixetex-io.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif 

#ifndef max
#define max(x,y) ((x) > (y) ? (x) : (y))
#endif 


/***********************/
/* Global Data */

typedef struct COMMAND {
	char *name;
	struct COMMAND *next;
} 
	bidi_cmd_t;

bidi_cmd_t *bidi_command_list;

enum { MODE_BIDIOFF, MODE_BIDION, MODE_BIDILTR };


static int bidi_mode;
static FriBidiLevel bidi_embed[MAX_LINE_SIZE];
static FriBidiChar	translation_buffer[MAX_LINE_SIZE];

/* only ASCII charrecters mirroring are
 * supported - the coded below 128 according to
 * http://www.unicode.org/Public/4.1.0/ucd/BidiMirroring.txt */

/* TODO: implement full mirroring list according to unicode 
 * standard */
static const char *bidi_mirror_list[][2] = 
{
	{"(",  ")"},
	{")",  "("},
	{"<",  ">"},
	{">",  "<"},
	{"[",  "]"},
	{"]",  "["},
	{"\\{","\\}"},
	{"\\}","\\{"},
	{NULL,NULL}
};

static const char *bidi_hack_list[][2] = 
{
	{"---","{\\fribidixetexemdash}"},
	{"--","{\\fribidixetexendash}"},
	{NULL,NULL}
};

/********/
/* TAGS */
/********/

#define TAG_BIDI_ON			"%BIDION"
#define TAG_BIDI_OFF		"%BIDIOFF"
#define TAG_BIDI_NEW_TAG	"%BIDITAG"
#define TAG_BIDI_LTR		"%BIDILTR"
#define TAG_BIDI_DIC_TAG	"%BIDIDICTAG"
#define TAG_BIDI_DIC_ENV	"%BIDIDICENV"

#define TAG_RTL			"\\fribidixetexRLE{"
#define TAG_LTR			"\\fribidixetexLRE{"
#define TAG_CLOSE		"}"

#define TAG_LATIN_NUM		"\\fribidixetexlatinnumbers{"
#define TAG_NONLATIN_NUM		"\\fribidixetexnonlatinnumbers{"

/***********************/

/* Compares begining of string U and C 
 * for example "Hello!" == "Hel" */
static int bidi_strieq_u_a(const FriBidiChar *u,const char *c)
{
	while(*u && *c) {
		if(*u!=*c)
			return 0;
		u++; c++;
	}
	if(*u ==0 && *c!=0) {
		return 0;
	}
	return 1;
}

static int bidi_strlen(FriBidiChar *in)
{
	int len;
	for(len=0;*in;len++) in++;
	return len;
}

/* Safe functions for adding charrecters to buffer 
 * if the line is too long the program just exits */

static void bidi_add_char_u(FriBidiChar *out,int *new_len,
							int limit,FriBidiChar ch)
{
	if(*new_len+2<limit){
		out[*new_len]=ch;
		++*new_len;
		out[*new_len]=0;
	}
	else {
		fprintf(stderr,"The line is too long: line %d\n",io_line_number);
		exit(1);
	}
}
static void bidi_add_str_c(FriBidiChar *out,int *new_len,
							int limit,const char *str)
{
	while(*str){
		bidi_add_char_u(out,new_len,limit,*str);
		str++;
	}
}

/* Function looks if the first charrecter or sequence 
 * in "in" is mirrored charrecter and returns its mirrot
 * in this case. If it is not mirrored then returns NULL */
static const char *bidi_one_mirror(FriBidiChar *in,int *size,
								const char *list[][2])
{
	int pos=0;
	while(list[pos][0]) {
		if(bidi_strieq_u_a(in,list[pos][0])) {
			*size=strlen(list[pos][0]);
			return list[pos][1];
		}
		pos++;
	}
	return NULL;
}

static const char *bidi_mirror(FriBidiChar *in,int *size,
								int replace_minus)
{
	const char *ptr;
	if((ptr=bidi_one_mirror(in,size,bidi_mirror_list))!=NULL) {
		return ptr;
	}
	if(replace_minus) {
		return bidi_one_mirror(in,size,bidi_hack_list);
	}
	return NULL;
}


/* Returns the minimal embedding level for required direction */
int bidi_basic_level(int is_rtl)
{
	if(is_rtl)
		return 1;
	return 0;
}

void bidi_add_command(const char *name)
{
	bidi_cmd_t *new_cmd;
	char *new_text;
	
	new_cmd=(bidi_cmd_t*)utl_malloc(sizeof(bidi_cmd_t));
	new_text=(char*)utl_malloc(strlen(name)+1);
	
	new_cmd->next=bidi_command_list;
	new_cmd->name=new_text;
	strcpy(new_text,name);
	bidi_command_list = new_cmd;
}

int bidi_is_cmd_char(FriBidiChar ch)
{
	if( ('a'<= ch && ch <='z') || ('A' <=ch && ch <= 'Z') )
		return 1;
	return 0;
}

/* Verirfies wether then text of length "len" is
 * in command list */
int bidi_in_cmd_list(FriBidiChar *text,int len)
{
	int i;
	bidi_cmd_t *p;
	for(p=bidi_command_list;p;p=p->next) {
		for(i=0;i<len;i++) {
			if(text[i]!=p->name[i]) 
				break;
		}
		if(i==len && p->name[len]==0){
			return 1;
		}
	}
	return 0;
}


/* Find special charrecters */
int bidi_is_latex_special_char(FriBidiChar *text)
{
	if(text[0]!='\\')
		return FALSE;
	/* Special charrecters according to lshort.pdf
	 *	# $ % ^ & _ { } ~ \, "{}" should be mirrored
	 * thus not included to the list */
	switch (text[1]) {
		case '#' :
		case '$' :
		case '%' :
		case '^' :
		case '&' :
		case '_' :
		case '\\':
			return TRUE;
		default :
			return FALSE;
	}
	
}


/*Verifies wether the next string is command 
 * ie: "\\[a-zA-Z]+" or "\\[a-zA-Z]+\*" */
int bidi_is_command(FriBidiChar *text,int *command_length)
{
	int len;
	
	if(bidi_is_latex_special_char(text)) {
		/* Charrecters like \\ or \$ that should be treated
		 * as `commands' */
		*command_length=2;
		return TRUE;
	}
	
	if(*text != '\\' || !bidi_is_cmd_char(text[1])) {
		return FALSE;
	}
	len=1;
	while(bidi_is_cmd_char(text[len])) {
		len++;
	}
	if(text[len] == '*') {
		len++;
	}
	*command_length = len;
	return TRUE;
}

/* This is implementation of state machine with stack 
 * that distinguishs between text and commands */

/* STACK VALUES */
enum { 
	EMPTY,
	SQ_BRACKET ,SQ_BRACKET_IGN,
	BRACKET, BRACKET_IGN,
	CMD_BRACKET, CMD_BRACKET_IGN
};
/* STATES */
enum { ST_NO, ST_NORM, ST_IGN };

/* Used for ignore commands */
int bidi_is_ignore(int top)
{
	return	top == SQ_BRACKET_IGN
			|| top== BRACKET_IGN
			|| top == CMD_BRACKET_IGN;
}

int bidi_state_on_left_br(int top,int *after_command_state)
{
	int ign_addon;
	int push,state = *after_command_state;
	if(bidi_is_ignore(top) || state == ST_IGN) {
		ign_addon = 1;
	}
	else {
		ign_addon = 0;
	}
	
	if(state) {
		push = CMD_BRACKET;
	}
	else{
		push = BRACKET;
	}
	
	*after_command_state = ST_NO;
	return push + ign_addon;
}

int bidi_state_on_left_sq_br(int top,int *after_command_state)
{
	int push;
	if(bidi_is_ignore(top) || *after_command_state == ST_IGN) {
		push = SQ_BRACKET_IGN;
	}
	else {
		push = SQ_BRACKET;
	}
	*after_command_state = ST_NO;
	return push;
}

void bidi_state_on_right_br(int top,int *after_command_state)
{
	if(top == CMD_BRACKET) {
		*after_command_state = ST_NORM;
	}
	else if(top == BRACKET || top == BRACKET_IGN) {
		*after_command_state = ST_NO;
	}
	else {/*top == CMD_BRACKET_IGN*/
		*after_command_state = ST_IGN;
	}
}

void bidi_state_on_right_sq_br(int top,int *after_command_state)
{
	if(top == SQ_BRACKET_IGN) {
		*after_command_state = ST_IGN;
	}
	else { /* top == SQ_BRACKET */
		*after_command_state = ST_NORM;
	}
}
/* Using marks "$$" */
int bidi_calc_equation_inline(FriBidiChar *in)
{
	int len=1;
	while(in[len] && in[len]!='$') {
		if(in[len]=='\\' && (in[len+1]=='$' || in[len+1]=='\\')) {
			len+=2;
		}
		else {
			len++;
		}
	}
	if(in[len]=='$')
		len++;
	return len;		
}

/* using \[ and \] marks */
int bidi_calc_equation_display(FriBidiChar *in)
{
	int len=2;
	while(in[len]){
		if(in[len]=='\\' && in[len+1]=='\\')
			len+=2;
		else if(in[len]=='\\' && in[len+1]==']')
			return len+2;
		else
			len++;
	}
	return len;
}

/* Support of equations */
int bidi_calc_equation(FriBidiChar *in)
{
	if(*in=='$')
		return bidi_calc_equation_inline(in);
	else
		return bidi_calc_equation_display(in);
}

/* This function parses the text "in" in marks places that
 * should be ignored by fribidi in "is_command" as true */
void bidi_mark_commands(FriBidiChar *in,int len,char *is_command,int is_rtl)
{

	char *parthness_stack;
	int stack_size=0;
	int cmd_len,top;
	int after_command_state=ST_NO;
	int mark,pos,symbol,i,push;

	/* Assumption - depth of stack can not be bigger then text length */
	parthness_stack = utl_malloc(len);
	
	pos=0;
	
	while(pos<len) {
		
		top = stack_size == 0 ? EMPTY : parthness_stack[stack_size-1];
		symbol=in[pos];
#ifdef DEBUG_STATE_MACHINE		
		printf("pos=%d sybol=%c state=%d top=%d\n",
			pos,(symbol < 127 ? symbol : '#'),after_command_state,top);
#endif 
		if(bidi_is_command(in+pos,&cmd_len)) {
			for(i=0;i<cmd_len;i++) {
				is_command[i+pos]=TRUE;
			}
			if(bidi_in_cmd_list(in+pos+1,cmd_len-1)) {
				after_command_state = ST_IGN;
			}
			else {
				after_command_state = ST_NORM;
			}
			pos+=cmd_len;
			continue;
		}
		else if(symbol=='$' || (symbol=='\\' && in[pos+1]=='[')) {
			cmd_len=bidi_calc_equation(in+pos);
			
			for(i=0;i<cmd_len;i++) {
				is_command[i+pos]=TRUE;
			}
			pos+=cmd_len;
			continue;
		}
		else if( symbol == '{' ) {
			push = bidi_state_on_left_br(top,&after_command_state);
			parthness_stack[stack_size++] = push;
			mark=TRUE;
		}
		else if(symbol == '[' && after_command_state) {
			push = bidi_state_on_left_sq_br(top,&after_command_state);
			parthness_stack[stack_size++] = push;
			mark=TRUE;
		}
		else if(symbol == ']' && (top == SQ_BRACKET || top == SQ_BRACKET_IGN)){
			bidi_state_on_right_sq_br(top,&after_command_state);
			stack_size--;
			mark=TRUE;
		}
		else if(symbol == '}' && (BRACKET <= top && top <= CMD_BRACKET_IGN)) {
			bidi_state_on_right_br(top,&after_command_state);
			stack_size--;
			mark=TRUE;
		}
		else {
			mark = bidi_is_ignore(top);
			after_command_state = ST_NO;
		}
		is_command[pos++]=mark;
	}
	
	utl_free(parthness_stack);
}

#if ( FRIBIDI_MAJOR_VERSION * 100 + FRIBIDI_MINOR_VERSION ) > 10 // 0.10
static void get_bidi_levels(FriBidiChar *in,int length,int is_rtl,FriBidiLevel *embed)
{
	FriBidiCharType *types = utl_malloc(sizeof(FriBidiCharType)*length);
	FriBidiParType direction = is_rtl ? FRIBIDI_PAR_RTL : FRIBIDI_PAR_LTR;

	fribidi_get_bidi_types(in,length,types);
	fribidi_get_par_embedding_levels(types,length,&direction,embed);

	utl_free(types);
}

#else // old fribidi
static void get_bidi_levels(FriBidiChar *in,int length,int is_rtl,FriBidiLevel *embed)
{
	FriBidiCharType direction;
	if(is_rtl)
		direction = FRIBIDI_TYPE_RTL;
	else
		direction = FRIBIDI_TYPE_LTR;

	fribidi_log2vis_get_embedding_levels(in,length,&direction,embed);
}
#endif

/* This function marks embedding levels at for text "in",
 * it ignores different tags */
void bidi_tag_tolerant_fribidi_l2v(	FriBidiChar *in,int len,
									int is_rtl,
									FriBidiLevel *embed,
									char *is_command)
{
	int in_pos,out_pos,cmd_len,i;
	FriBidiChar *in_tmp;
	FriBidiLevel *embed_tmp,fill_level;
	
	in_tmp=(FriBidiChar*)utl_malloc(sizeof(FriBidiChar)*(len+1));
	embed_tmp=(FriBidiLevel*)utl_malloc(sizeof(FriBidiLevel)*len);
	
	/**********************************************
	 * This is main parser that marks commands    *
	 * across the text i.e. marks non text        *
	 **********************************************/
	bidi_mark_commands(in,len,is_command,is_rtl);
	
	/**********************************************/
	/* Copy all the data without tags for fribidi */
	/**********************************************/
	in_pos=0;
	out_pos=0;

	while(in_pos<len) {
		if(is_command[in_pos]){
			in_pos++;
			continue;
		}
		/* Copy to buffer */
		in_tmp[out_pos]=in[in_pos];
		out_pos++;
		in_pos++;
	}
	
	/***************
	 * RUN FRIBIDI *
	 ***************/
	
	/* Note - you must take the new size for firibidi */
	get_bidi_levels(in_tmp,out_pos,is_rtl,embed_tmp);

	/****************************************************
	 * Return the tags and fill missing embedding level *
	 ****************************************************/
	in_pos=0;
	out_pos=0;

	while(in_pos<len) {
		if(is_command[in_pos]){
			/* Find the length of the part that 
			 * has a command/tag */
			for(cmd_len=0;in_pos+cmd_len<len;cmd_len++) {
				if(!is_command[cmd_len+in_pos])
					break;
			}
			
			if(in_pos == 0 || in_pos + cmd_len == len){
				/* When we on start/end assume basic direction */
				fill_level = bidi_basic_level(is_rtl);
			}
			else {
				/* Fill with minimum on both sides */
				fill_level = min(embed_tmp[out_pos-1],embed_tmp[out_pos]);
			}
			
			for(i=0;i<cmd_len;i++){
				embed[in_pos]=fill_level;
				in_pos++;
			}
			continue;
		}
		/* Retrieve embedding */
		embed[in_pos]=embed_tmp[out_pos];
		out_pos++;
		in_pos++;
	}

	/* Not forget to free */
	utl_free(embed_tmp);
	utl_free(in_tmp);
}

int bidi_only_latin_number(FriBidiLevel *levels,FriBidiChar *string)
{
	FriBidiChar ch;
	while(*string && (*levels & 1)==0 ) {
		ch=*string;
		if(ch>127) { /* other foreign language */
			return 0;
		}
		if(('a'<=ch && ch<='z') || ('A'<=ch && ch<='Z')) {
			/* Find latin characters */
			return 0;
		}
		string++;
		levels++;
	}
	return 1;
}

int bidi_only_nonlatin_number(FriBidiLevel *levels,FriBidiChar *string)
{
	FriBidiChar ch;
	while(*string && (*levels & 1)==0 ) {
		ch=*string;
		if('0'<=ch && ch<='9') { /* Find latin numbers */
			return 0;
		}
		if(('a'<=ch && ch<='z') || ('A'<=ch && ch<='Z')) {
			/* Find latin characters */
			return 0;
		}
		string++;
		levels++;
	}
	return 1;
}

/* Mark Unicode  LRM & RLM charrecters */
int bidi_is_directional_mark(FriBidiChar c)
{
	if(c==0x200F || c==0x200E) {
		return 1;
	}
	return 0;
}

/* The function that parses line and adds required \R \L tags */
void bidi_add_tags(FriBidiChar *in,FriBidiChar *out,int limit,
					int is_rtl,int replace_minus,int no_mirroring)
{
	int len,new_len,level,new_level,brakets;
	int i,size;
	int is_number_env=0;
	
	const char *tag;
	char *is_command;
	
	
	len=bidi_strlen(in);
	
	is_command=(char*)utl_malloc(len);
	
	bidi_tag_tolerant_fribidi_l2v(in,len,is_rtl,bidi_embed,is_command);

	level=bidi_basic_level(is_rtl);
	
	new_len=0;
	out[0]=0;
	brakets=0;
	for(i=0,new_len=0;i<len;i++){
		new_level=bidi_embed[i];

		if(new_level>level) {
			/* LTR Direction according to odd/even value of level */
			is_number_env=FALSE;
			if((new_level & 1) == 0) {
				if(bidi_only_latin_number(bidi_embed+i,in+i)){
					tag=TAG_LATIN_NUM;
					is_number_env=TRUE;
				}
				else if(bidi_only_nonlatin_number(bidi_embed+i,in+i)){
					tag=TAG_NONLATIN_NUM;
					is_number_env=TRUE;
				}
				else {
					tag=TAG_LTR;
				}
			}
			else {
				tag=TAG_RTL;
			}
			brakets++;
			bidi_add_str_c(out,&new_len,limit,tag);
		}
		else if(new_level<level) {
			bidi_add_str_c(out,&new_len,limit,TAG_CLOSE);
			brakets--;
		}
		
		if(
			(new_level & 1)!=0 
			&& !is_command[i]
			&& !no_mirroring
			&& (tag=bidi_mirror(in+i,&size,replace_minus))!=NULL)
		{
			/* Replace charrecter with its mirror only in case
			 * we are in RTL direction */
		 
			/* Note this can be a sequence like "\{" */
			bidi_add_str_c(out,&new_len,limit,tag);
			i+=size-1;
		}
		else if(
			(new_level & 1)!=1
			&& is_number_env
			&& replace_minus
			&& !is_command[i]
			&& (tag=bidi_one_mirror(in+i,&size,bidi_hack_list))!=NULL)
		{
			/* Replace "--/---" with a tag "\tex{en|em}dash" only in LTR
			 * direction only if this is nubmers environment */
			
			bidi_add_str_c(out,&new_len,limit,tag);
			i+=size-1;
		}
		else if(bidi_is_directional_mark(in[i])){
			/* Do nothing -- erase marks that are 
			 * not actual charrecters and thus not presented in fonts */
		}
		else {
			bidi_add_char_u(out,&new_len,limit,in[i]);
		}
		level=new_level;
	}
	/* Fill all missed brakets */
	while(brakets){
		bidi_add_str_c(out,&new_len,limit,TAG_CLOSE);
		brakets--;
	}
	utl_free(is_command);
}

/*************************/
/* T R A N S L A T I O N */
/*************************/

static void bidi_error(char *text)
{
	fprintf(stderr,"ERROR in line %d: `%s'\n",io_line_number,text);
	exit(1);
}


static int is_hebrew_tag(FriBidiChar buffer[MAX_COMMAND_LEN],FriBidiChar **ptr)
{
	FriBidiChar *tmp_ptr = *ptr;
	int size=0;
	if(!dict_is_hebrew_letter(*tmp_ptr)) {
		return FALSE;
	}
	while(dict_is_hebrew_letter(*tmp_ptr)) {
		if(size<MAX_COMMAND_LEN-1) {
			buffer[size]=*tmp_ptr;
			tmp_ptr++;
			size++;
		}
		else {
			bidi_error("Hebrew tag is too long");
			return FALSE;
		}
	}
	buffer[size]=0;
	*ptr=tmp_ptr;
	return TRUE;
	
}

static int is_ascii_tag(char buffer[MAX_COMMAND_LEN],FriBidiChar **ptr)
{
	FriBidiChar *tmp_ptr = *ptr;
	int size=0;
	if(!bidi_is_cmd_char(*tmp_ptr)) {
		return FALSE;
	}
	while(bidi_is_cmd_char(*tmp_ptr)) {
		if(size<MAX_COMMAND_LEN-1) {
			buffer[size]=*tmp_ptr;
			tmp_ptr++;
			size++;
		}
		else {
			/* No warning need - no limit */
			return FALSE;
		}
	}
	buffer[size]=0;
	*ptr=tmp_ptr;
	return TRUE;
	
}


static char const *is_tag_begin_or_end(FriBidiChar **ptr)
{
	char ascii_tag[MAX_COMMAND_LEN];
	char *trans=NULL;
	FriBidiChar *tmp_ptr=*ptr;
	FriBidiChar unicode_tag[MAX_COMMAND_LEN];
	
	if(is_ascii_tag(ascii_tag,&tmp_ptr) || 
		  (
			is_hebrew_tag(unicode_tag,&tmp_ptr) 
			&& (trans=dict_translate_tag(unicode_tag))
		  )
	  )
	{
		if(trans) strcpy(ascii_tag,trans);
		
		if(strcmp(ascii_tag,"begin")==0){
			*ptr=tmp_ptr;
			return "begin";
		}
		else if(strcmp(ascii_tag,"end")==0){
			*ptr=tmp_ptr;
			return "end";
		}
		else {
			return NULL;
		}
	}
	else {
		return NULL;
	}
}

#define is_open_bracket(x)  ( **(x) == '{' ? ++*(x) : FALSE )
#define is_close_bracket(x) ( **(x) == '}' ? ++*(x) : FALSE )

static const char *is_heb_word(FriBidiChar **ptr,trans_func_t tras_func)
{
	FriBidiChar *tmp_ptr=*ptr;
	FriBidiChar unicode_tag[MAX_COMMAND_LEN];
	char *trans;
	if(is_hebrew_tag(unicode_tag,&tmp_ptr)) {
		if((trans=tras_func(unicode_tag))!=NULL) {
			*ptr=tmp_ptr;
			return trans;
		}
		else {
			bidi_error("Unknown hebrew tag - not translated");
			return NULL;
		}
	}
	else {
		return NULL;
	}
}

#define is_heb_environment(x)	is_heb_word((x),dict_translate_env)
#define is_heb_tag(x)			is_heb_word((x),dict_translate_tag)

static char *is_hebrew_command(FriBidiChar *text,int *length)
{
	char const *tag_txt,*env_txt;
	
	static char buffer[MAX_COMMAND_LEN*2+3];
	/* The command can consist at most of name of tag,
	 * environment and breakets */
	
	FriBidiChar *end_ptr;
	if(*text!='\\') {
		return NULL;
	}
	
	end_ptr=text+1;
	if((tag_txt = is_tag_begin_or_end(&end_ptr))!=NULL
		&& is_open_bracket(&end_ptr)
		&& (env_txt=is_heb_environment(&end_ptr))!=NULL
		&& is_close_bracket(&end_ptr))
	{
		*length=end_ptr-text;
		strcpy(buffer,"\\");
		strcat(buffer,tag_txt);
		strcat(buffer,"{");
		strcat(buffer,env_txt);
		strcat(buffer,"}");
		return buffer;
	}
	
	end_ptr=text+1;
	if((tag_txt=is_heb_tag(&end_ptr))!=NULL){
		*length=end_ptr-text;
		strcpy(buffer,"\\");
		strcat(buffer,tag_txt);
		return buffer;
	}
	return NULL;
}

void bidi_translate_tags(FriBidiChar *in,FriBidiChar *out,int limit)
{
	int ptr;
	int new_length,cmd_len;
	char *tr_command;
	ptr=0;
	new_length=0;
	
	out[0]=0;
	
	while(in[ptr]) {
		/* In case there is \\ before hebrew word */
		if(in[ptr]=='\\' && in[ptr+1]=='\\'){
			ptr+=2;
			bidi_add_str_c(out,&new_length,limit,"\\\\");
			continue;
		}
		else if((tr_command = is_hebrew_command(in+ptr,&cmd_len))!=NULL) {
			ptr+=cmd_len;
			bidi_add_str_c(out,&new_length,limit,tr_command);
		}
		else {
			bidi_add_char_u(out,&new_length,limit,in[ptr]);
			ptr++;
		}
	}
	
}

void bidi_grammar_skip_blank(FriBidiChar **ptr)
{
	FriBidiChar ch;
	while((ch=**ptr)==' ' || ch=='\t' || ch=='\r' || ch=='\n')
		++*ptr;
}

/*
 * format is following:
 * "lh" - latin word, hebrew word,
 * "l" - one latin word
 * parameters are (FriBidiChar *) for h and (char *) for l
 */
int bidi_grammar(FriBidiChar *in,char *tagname,char *format,
					FriBidiChar hebrew[MAX_COMMAND_LEN],
					char ascii[MAX_COMMAND_LEN])
{
	if(!bidi_strieq_u_a(in,tagname)) {
		return 0;
	}
	
	in+=strlen(tagname);
	
	bidi_grammar_skip_blank(&in);

	while(*format) {
		if(*format=='h'){
			if(!is_hebrew_tag(hebrew,&in)){
				bidi_error("Hebrew word expected");
			}
		}
		else if(*format == 'l') {
			if(!is_ascii_tag(ascii,&in)){
				bidi_error("Latin word expected");
			}
		}
		else {
			fprintf(stderr,"Internall error\n");
			exit(1);
		}
		format++;
		bidi_grammar_skip_blank(&in);
	}
	if(*in!=0) {
		bidi_error("Unexpected charrecter");
	}
	return 1;
}

void bidi_parse_fribidixetex_command(FriBidiChar *in)
{
	FriBidiChar unicode[MAX_COMMAND_LEN];
	char ascii[MAX_COMMAND_LEN];
	
	if(bidi_grammar(in,TAG_BIDI_ON,"",NULL,NULL)) {
		bidi_mode = MODE_BIDION;
	}
	else if(bidi_grammar(in,TAG_BIDI_OFF,"",NULL,NULL)) {
		bidi_mode = MODE_BIDIOFF;
	}
	else if(bidi_grammar(in,TAG_BIDI_LTR,"",NULL,NULL)) {
		bidi_mode = MODE_BIDILTR;
	}
	else if(bidi_grammar(in,TAG_BIDI_NEW_TAG,"l",NULL,ascii)) {
		bidi_add_command(ascii);
	}
	else if(bidi_grammar(in,TAG_BIDI_DIC_TAG,"hl",unicode,ascii)) {
		dict_add_tans(unicode,ascii,DICT_TAG);
	}
	else if(bidi_grammar(in,TAG_BIDI_DIC_ENV,"hl",unicode,ascii)) {
		dict_add_tans(unicode,ascii,DICT_ENV);
	}
	else {
		bidi_error("Unknown fribidixetex command");
	}
}

/*********************
 * M A I N   L O O P *
 *********************/

/* Main line processing function */
int bidi_process(FriBidiChar *in,FriBidiChar *out,
						int replace_minus,int tranlate_only,int no_mirroring)
{
	int i,is_rtl;
	
	if(bidi_strieq_u_a(in,"%BIDI")) {
		bidi_parse_fribidixetex_command(in);
		return 0;
	}
	
	if(bidi_mode != MODE_BIDIOFF) {
		is_rtl = (bidi_mode == MODE_BIDION);
		if(tranlate_only) {
			/* In case of translation only put directly to output buffer */
			bidi_translate_tags(in,out,MAX_LINE_SIZE);
		}
		else { /* Normal processing */
			/* Translate hebrew tags to ascii tags */
			bidi_translate_tags(in,translation_buffer,MAX_LINE_SIZE);
			
			/* Apply BiDi algorithm */
			bidi_add_tags(translation_buffer,out,MAX_LINE_SIZE,
					is_rtl ,replace_minus,no_mirroring);
		}
	}
	else {
		for(i=0;in[i];i++){
			out[i]=in[i];
		}
		out[i]=0;
	}
	return 1;
}

void bidi_finish(void)
{
	bidi_cmd_t *tmp;
	while(bidi_command_list) {
		tmp=bidi_command_list;
		bidi_command_list=bidi_command_list->next;
		utl_free(tmp->name);
		utl_free(tmp);
	}
	if(bidi_mode != MODE_BIDIOFF) {
		fprintf(stderr,"Warning: No %%BIDIOFF Tag at the end of the file\n");
	}
}

void bidi_init(FILE *f_out)
{
	int i;

	bidi_mode = MODE_BIDIOFF;
	
	for(i=0;ignore_tags_list[i][0];i++) {
		bidi_add_command(ignore_tags_list[i]);
	}

}
