/*1:*/

/*4:*/

#if defined(MIKTEX)
#include <miktex/ExitThrows> 
#endif
#include <ctype.h>  
#include <kpathsea/simpletypes.h>  
#include <stddef.h>  
#include <stdint.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  

#ifndef HAVE_GETTEXT
#define HAVE_GETTEXT 0
#endif

#if HAVE_GETTEXT
#include <libintl.h> 
#else
#define gettext(a) a
#endif

/*:4*/

#define banner "This is CTANGLE, Version 4.7" \
 \

#define _(s) gettext(s)  \

#define and_and 04
#define lt_lt 020
#define gt_gt 021
#define plus_plus 013
#define minus_minus 01
#define minus_gt 031
#define non_eq 032
#define lt_eq 034
#define gt_eq 035
#define eq_eq 036
#define or_or 037
#define dot_dot_dot 016
#define colon_colon 06
#define period_ast 026
#define minus_gt_ast 027 \

#define compress(c) if(loc++<=limit) return c \

#define xisalpha(c) (isalpha((int) (c) ) &&((eight_bits) (c) <0200) ) 
#define xisdigit(c) (isdigit((int) (c) ) &&((eight_bits) (c) <0200) ) 
#define xisspace(c) (isspace((int) (c) ) &&((eight_bits) (c) <0200) ) 
#define xislower(c) (islower((int) (c) ) &&((eight_bits) (c) <0200) ) 
#define xisupper(c) (isupper((int) (c) ) &&((eight_bits) (c) <0200) ) 
#define xisxdigit(c) (isxdigit((int) (c) ) &&((eight_bits) (c) <0200) ) 
#define isxalpha(c) ((c) =='_'||(c) =='$')  \

#define ishigh(c) ((eight_bits) (c) > 0177)  \
 \

#define max_include_depth 10 \

#define max_file_name_length 1024
#define cur_file file[include_depth]
#define cur_file_name file_name[include_depth]
#define cur_line line[include_depth]
#define web_file file[0]
#define web_file_name file_name[0] \

#define length(c) (size_t) ((c+1) ->byte_start-(c) ->byte_start) 
#define print_id(c) term_write((c) ->byte_start,length(c) ) 
#define llink link
#define rlink dummy.Rlink
#define root name_dir->rlink \
 \

#define spotless 0
#define harmless_message 1
#define error_message 2
#define fatal_message 3
#define mark_harmless if(history==spotless) history= harmless_message
#define mark_error history= error_message
#define confusion(s) fatal(_("! This can't happen: ") ,s)  \
 \

#define show_banner flags['b']
#define show_progress flags['p']
#define show_happiness flags['h']
#define show_stats flags['s']
#define make_xrefs flags['x']
#define check_for_change flags['c'] \

#define update_terminal fflush(stdout) 
#define new_line putchar('\n') 
#define term_write(a,b) fflush(stdout) ,fwrite(a,sizeof(char) ,b,stdout)  \

#define buf_size 1000
#define longest_name 10000 \

#define long_buf_size (buf_size+longest_name) 
#define max_bytes 1000000 \

#define max_names 10239 \

#define max_sections 4000 \

#define max_texts 10239
#define max_toks 1000000
#define equiv equiv_or_xref \

#define macro 0
#define section_flag max_texts \

#define string 02
#define constant 03
#define join 0177
#define output_defs_flag (2*024000-1)  \

#define stack_size 50
#define cur_end cur_state.end_field
#define cur_byte cur_state.byte_field
#define cur_name cur_state.name_field
#define cur_repl cur_state.repl_field
#define cur_section cur_state.section_field \

#define section_number 0201
#define identifier 0202 \

#define normal 0
#define num_or_id 1
#define post_slash 2
#define unbreakable 3
#define verbatim 4 \

#define max_files 256
#define C_printf(c,a) fprintf(C_file,c,a) 
#define C_putc(c) putc((int) (c) ,C_file)  \

#define translit_length 10 \

#define ignore 00
#define ord 0302
#define control_text 0303
#define translit_code 0304
#define output_defs_code 0305
#define format_code 0306
#define definition 0307
#define begin_C 0310
#define section_name 0311
#define new_section 0312 \

#define app_repl(c) { \
if(tok_ptr==tok_mem_end) overflow(_("token") ) ; \
else*(tok_ptr++) = (eight_bits) c; \
} \

#define store_id(a) a= id_lookup(id_first,id_loc,'\0') -name_dir; \
app_repl((a/0400) +0200) ; \
app_repl(a%0400)  \

#define keep_digit_separators flags['k'] \

#define max_banner 50 \



/*3:*/

typedef uint8_t eight_bits;
typedef uint16_t sixteen_bits;
typedef enum{
ctangle,cweave,ctwill
}cweb;
extern cweb program;
extern int phase;

/*:3*//*5:*/

extern char section_text[];
extern char*section_text_end;
extern char*id_first;
extern char*id_loc;

/*:5*//*6:*/

extern char buffer[];
extern char*buffer_end;
extern char*loc;
extern char*limit;

/*:6*//*7:*/

extern int include_depth;
extern FILE*file[];
extern FILE*change_file;
extern char file_name[][max_file_name_length];

extern char change_file_name[];
extern char check_file_name[];
extern int line[];
extern int change_line;
extern int change_depth;
extern boolean input_has_ended;
extern boolean changing;
extern boolean web_file_open;

/*:7*//*9:*/

extern sixteen_bits section_count;
extern boolean changed_section[];
extern boolean change_pending;
extern boolean print_where;

/*:9*//*10:*/

typedef struct name_info{
char*byte_start;
struct name_info*link;
union{
struct name_info*Rlink;

char Ilk;
}dummy;
void*equiv_or_xref;
}name_info;
typedef name_info*name_pointer;
typedef name_pointer*hash_pointer;
extern char byte_mem[];
extern char*byte_mem_end;
extern char*byte_ptr;
extern name_info name_dir[];
extern name_pointer name_dir_end;
extern name_pointer name_ptr;
extern name_pointer hash[];
extern hash_pointer hash_end;
extern hash_pointer h;

/*:10*//*12:*/

extern int history;

/*:12*//*14:*/

extern int argc;
extern char**argv;
extern char C_file_name[];
extern char tex_file_name[];
extern char idx_file_name[];
extern char scn_file_name[];
extern boolean flags[];
extern const char*use_language;

/*:14*//*15:*/

extern FILE*C_file;
extern FILE*tex_file;
extern FILE*idx_file;
extern FILE*scn_file;
extern FILE*active_file;
extern FILE*check_file;

/*:15*//*116:*/

extern char cb_banner[];

/*:116*/

/*19:*/

typedef struct{
eight_bits*tok_start;
sixteen_bits text_link;
}text;
typedef text*text_pointer;

/*:19*//*31:*/

typedef struct{
eight_bits*end_field;
eight_bits*byte_field;
name_pointer name_field;
text_pointer repl_field;
sixteen_bits section_field;
}output_state;
typedef output_state*stack_pointer;

/*:31*/

/*20:*/

static text text_info[max_texts];
static text_pointer text_info_end= text_info+max_texts-1;
static text_pointer text_ptr;
static eight_bits tok_mem[max_toks];
static eight_bits*tok_mem_end= tok_mem+max_toks-1;
static eight_bits*tok_ptr;

/*:20*//*26:*/

static text_pointer last_unnamed;

/*:26*//*32:*/

static output_state cur_state;

static output_state stack[stack_size+1];
static stack_pointer stack_end= stack+stack_size;
static stack_pointer stack_ptr;

/*:32*//*37:*/

static int cur_val;

/*:37*//*42:*/

static eight_bits out_state;
static boolean protect;

/*:42*//*45:*/

static name_pointer output_files[max_files];
static name_pointer*cur_out_file,*end_output_files,*an_output_file;
static char cur_section_name_char;
static char output_file_name[longest_name+1];

/*:45*//*52:*/

static boolean output_defs_seen= false;

/*:52*//*57:*/

static char translit[128][translit_length];

/*:57*//*62:*/

static eight_bits ccode[256];

/*:62*//*66:*/

static boolean comment_continues= false;

/*:66*//*68:*/

static name_pointer cur_section_name;
static boolean no_where;

/*:68*//*82:*/

static text_pointer cur_text;
static eight_bits next_control;

/*:82*/

/*8:*/

extern boolean get_line(void);
extern void check_complete(void);
extern void reset_input(void);

/*:8*//*11:*/

extern boolean names_match(name_pointer,const char*,size_t,eight_bits);
extern name_pointer id_lookup(const char*,const char*,eight_bits);

extern name_pointer section_lookup(char*,char*,boolean);
extern void init_node(name_pointer);
extern void init_p(name_pointer,eight_bits);
extern void print_prefix_name(name_pointer);
extern void print_section_name(name_pointer);
extern void sprint_section_name(char*,name_pointer);

/*:11*//*13:*/

extern int wrap_up(void);
extern void err_print(const char*);
extern void fatal(const char*,const char*);
extern void overflow(const char*);

/*:13*//*16:*/

extern void common_init(void);
extern void print_stats(void);
extern void cb_show_banner(void);

/*:16*//*30:*/
static void store_two_bytes(sixteen_bits);

/*:30*//*35:*/

static void push_level(name_pointer);
static void pop_level(boolean);

/*:35*//*39:*/
static void get_output(void);

/*:39*//*44:*/
static void flush_buffer(void);

/*:44*//*49:*/
static void phase_two(void);

/*:49*//*53:*/

static void output_defs(void);
static void out_char(eight_bits);

/*:53*//*65:*/

static eight_bits skip_ahead(void);
static boolean skip_comment(boolean);

/*:65*//*70:*/
static eight_bits get_next(void);

/*:70*//*84:*/
static void scan_repl(eight_bits);

/*:84*//*91:*/
static void scan_section(void);

/*:91*//*99:*/
static void phase_one(void);

/*:99*//*101:*/
static void skip_limbo(void);

/*:101*/


/*:1*//*2:*/

int main(
int ac,
char**av)
{
argc= ac;argv= av;
program= ctangle;
/*21:*/

text_info->tok_start= tok_ptr= tok_mem;
text_ptr= text_info+1;text_ptr->tok_start= tok_mem;


/*:21*//*23:*/

init_node(name_dir);

/*:23*//*27:*/
last_unnamed= text_info;text_info->text_link= macro;

/*:27*//*46:*/

cur_out_file= end_output_files= output_files+max_files;

/*:46*//*58:*/

{
int i;
for(i= 0;i<128;i++)sprintf(translit[i],"X%02X",(unsigned int)(128+i));
}

/*:58*//*63:*/
{
int c;
for(c= 0;c<256;c++)ccode[c]= ignore;
}
ccode[' ']= ccode['\t']= ccode['\n']= ccode['\v']= ccode['\r']= ccode['\f']
= ccode['*']= new_section;
ccode['@']= (eight_bits)'@';ccode['=']= string;
ccode['d']= ccode['D']= definition;
ccode['f']= ccode['F']= ccode['s']= ccode['S']= format_code;
ccode['c']= ccode['C']= ccode['p']= ccode['P']= begin_C;
ccode['^']= ccode[':']= ccode['.']= ccode['t']= ccode['T']= 
ccode['q']= ccode['Q']= control_text;
ccode['h']= ccode['H']= output_defs_code;
ccode['l']= ccode['L']= translit_code;
ccode['&']= join;
ccode['<']= ccode['(']= section_name;
ccode['\'']= ord;

/*:63*//*78:*/
section_text[0]= ' ';

/*:78*//*117:*/

strncpy(cb_banner,banner,max_banner-1);

/*:117*/

common_init();
if(show_banner)cb_show_banner();
phase_one();
phase_two();
return wrap_up();
}

/*:2*//*24:*/

boolean names_match(
name_pointer p,
const char*first,
size_t l,
eight_bits t)
{(void)t;
return length(p)==l&&strncmp(first,p->byte_start,l)==0;
}

/*:24*//*25:*/

void
init_node(
name_pointer node)
{
node->equiv= (void*)text_info;
}
void
init_p(name_pointer p,eight_bits t){(void)p;(void)t;}

/*:25*//*29:*/

static void
store_two_bytes(
sixteen_bits x)
{
if(tok_ptr+2> tok_mem_end)overflow(_("token"));
*tok_ptr++= x>>8;
*tok_ptr++= x&0377;
}

/*:29*//*34:*/

static void
push_level(
name_pointer p)
{
if(stack_ptr==stack_end)overflow(_("stack"));
*stack_ptr= cur_state;
stack_ptr++;
if(p!=NULL){
cur_name= p;cur_repl= (text_pointer)p->equiv;
cur_byte= cur_repl->tok_start;cur_end= (cur_repl+1)->tok_start;
cur_section= 0;
}
}

/*:34*//*36:*/

static void
pop_level(
boolean flag)
{
if(flag&&cur_repl->text_link<section_flag){
cur_repl= cur_repl->text_link+text_info;
cur_byte= cur_repl->tok_start;cur_end= (cur_repl+1)->tok_start;
return;
}
stack_ptr--;
if(stack_ptr> stack)cur_state= *stack_ptr;
}

/*:36*//*38:*/

static void
get_output(void)
{
sixteen_bits a;
restart:if(stack_ptr==stack)return;
if(cur_byte==cur_end){
cur_val= -((int)cur_section);
pop_level(true);
if(cur_val==0)goto restart;
out_char(section_number);return;
}
a= *cur_byte++;
if(out_state==verbatim&&a!=string&&a!=constant&&a!='\n')
C_putc(a);
else if(a<0200)out_char(a);
else{
a= (a-0200)*0400+*cur_byte++;
switch(a/024000){
case 0:cur_val= (int)a;out_char(identifier);break;
case 1:if(a==output_defs_flag)output_defs();
else/*40:*/

{
a-= 024000;
if((a+name_dir)->equiv!=(void*)text_info)push_level(a+name_dir);
else if(a!=0){
fputs(_("\n! Not present: <"),stdout);
print_section_name(a+name_dir);err_print(">");

}
goto restart;
}

/*:40*/

break;
default:cur_val= (int)a-050000;
if(cur_val> 0)cur_section= (sixteen_bits)cur_val;
out_char(section_number);
}
}
}

/*:38*//*43:*/

static void
flush_buffer(void)
{
C_putc('\n');
if(cur_line%100==0&&show_progress){
putchar('.');
if(cur_line%500==0)printf("%d",cur_line);
update_terminal;
}
cur_line++;
}

/*:43*//*48:*/

static void
phase_two(void){
phase= 2;
web_file_open= false;
cur_line= 1;
/*33:*/

stack_ptr= stack+1;cur_name= name_dir;cur_repl= text_info->text_link+text_info;
cur_byte= cur_repl->tok_start;cur_end= (cur_repl+1)->tok_start;cur_section= 0;

/*:33*/

/*51:*/

if(!output_defs_seen)
output_defs();

/*:51*/

if(text_info->text_link==macro&&cur_out_file==end_output_files){
fputs(_("\n! No program text was specified."),stdout);mark_harmless;

}
else{
if(cur_out_file==end_output_files){
if(show_progress){
printf(_("\nWriting the output file (%s):"),C_file_name);
update_terminal;
}
}
else{
if(show_progress){
fputs(_("\nWriting the output files:"),stdout);

printf(" (%s)",C_file_name);
update_terminal;
}
if(text_info->text_link==macro)goto writeloop;
}
while(stack_ptr> stack)get_output();
flush_buffer();
writeloop:/*50:*/

if(check_for_change){
fclose(C_file);C_file= NULL;
/*106:*/

if((C_file= fopen(C_file_name,"r"))!=NULL){
/*107:*/

boolean comparison= false;

if((check_file= fopen(check_file_name,"r"))==NULL)
fatal(_("! Cannot open output file "),check_file_name);


/*108:*/

do{
char x[BUFSIZ],y[BUFSIZ];
int x_size= fread(x,sizeof(char),BUFSIZ,C_file);
int y_size= fread(y,sizeof(char),BUFSIZ,check_file);
comparison= (x_size==y_size)&&!memcmp(x,y,x_size);
}while(comparison&&!feof(C_file)&&!feof(check_file));

/*:108*/


fclose(C_file);C_file= NULL;
fclose(check_file);check_file= NULL;

/*:107*/

/*109:*/

if(comparison)
remove(check_file_name);
else{
remove(C_file_name);
rename(check_file_name,C_file_name);
}

/*:109*/

}else
rename(check_file_name,C_file_name);

/*:106*/

}
for(an_output_file= end_output_files;an_output_file> cur_out_file;){
an_output_file--;
sprint_section_name(output_file_name,*an_output_file);
if(check_for_change)/*105:*/
{
if((C_file= fopen(output_file_name,"a"))==NULL)
fatal(_("! Cannot open output file "),output_file_name);

else fclose(C_file);
if((C_file= fopen(check_file_name,"wb"))==NULL)
fatal(_("! Cannot open output file "),check_file_name);
}

/*:105*/

else{
fclose(C_file);
if((C_file= fopen(output_file_name,"wb"))==NULL)
fatal(_("! Cannot open output file "),output_file_name);

}
if(show_progress){printf("\n(%s)",output_file_name);update_terminal;}
cur_line= 1;
stack_ptr= stack+1;
cur_name= *an_output_file;
cur_repl= (text_pointer)cur_name->equiv;
cur_byte= cur_repl->tok_start;
cur_end= (cur_repl+1)->tok_start;
while(stack_ptr> stack)get_output();
flush_buffer();
if(check_for_change){
fclose(C_file);C_file= NULL;
/*110:*/

if(0==strcmp("/dev/stdout",output_file_name))
/*112:*/
{
/*115:*/

char in_buf[BUFSIZ+1];
int in_size;
boolean comparison= true;
if((check_file= fopen(check_file_name,"r"))==NULL)
fatal(_("! Cannot open output file "),check_file_name);


/*:115*/

do{
in_size= fread(in_buf,sizeof(char),BUFSIZ,check_file);
in_buf[in_size]= '\0';
fprintf(stdout,"%s",in_buf);
}while(!feof(check_file));
fclose(check_file);check_file= NULL;
/*111:*/

if(comparison)
remove(check_file_name);
else{
remove(output_file_name);
rename(check_file_name,output_file_name);
}

/*:111*/

}

/*:112*/

else if(0==strcmp("/dev/stderr",output_file_name))
/*113:*/
{
/*115:*/

char in_buf[BUFSIZ+1];
int in_size;
boolean comparison= true;
if((check_file= fopen(check_file_name,"r"))==NULL)
fatal(_("! Cannot open output file "),check_file_name);


/*:115*/

do{
in_size= fread(in_buf,sizeof(char),BUFSIZ,check_file);
in_buf[in_size]= '\0';
fprintf(stderr,"%s",in_buf);
}while(!feof(check_file));
fclose(check_file);check_file= NULL;
/*111:*/

if(comparison)
remove(check_file_name);
else{
remove(output_file_name);
rename(check_file_name,output_file_name);
}

/*:111*/

}

/*:113*/

else if(0==strcmp("/dev/null",output_file_name))
/*114:*/
{
boolean comparison= true;
/*111:*/

if(comparison)
remove(check_file_name);
else{
remove(output_file_name);
rename(check_file_name,output_file_name);
}

/*:111*/

}

/*:114*/

else{
if((C_file= fopen(output_file_name,"r"))!=NULL){
/*107:*/

boolean comparison= false;

if((check_file= fopen(check_file_name,"r"))==NULL)
fatal(_("! Cannot open output file "),check_file_name);


/*108:*/

do{
char x[BUFSIZ],y[BUFSIZ];
int x_size= fread(x,sizeof(char),BUFSIZ,C_file);
int y_size= fread(y,sizeof(char),BUFSIZ,check_file);
comparison= (x_size==y_size)&&!memcmp(x,y,x_size);
}while(comparison&&!feof(C_file)&&!feof(check_file));

/*:108*/


fclose(C_file);C_file= NULL;
fclose(check_file);check_file= NULL;

/*:107*/

/*111:*/

if(comparison)
remove(check_file_name);
else{
remove(output_file_name);
rename(check_file_name,output_file_name);
}

/*:111*/

}else
rename(check_file_name,output_file_name);
}

/*:110*/

}
}
if(check_for_change)
strcpy(check_file_name,"");

/*:50*/

if(show_happiness){
if(show_progress)new_line;
fputs(_("Done."),stdout);
}
}
}

/*:48*//*54:*/

static void
output_defs(void)
{
sixteen_bits a;
push_level(NULL);
for(cur_text= text_info+1;cur_text<text_ptr;cur_text++)
if(cur_text->text_link==macro){
cur_byte= cur_text->tok_start;
cur_end= (cur_text+1)->tok_start;
C_printf("%s","#define ");
out_state= normal;
protect= true;
while(cur_byte<cur_end){
a= *cur_byte++;
if(cur_byte==cur_end&&a=='\n')break;
if(out_state==verbatim&&a!=string&&a!=constant&&a!='\n')
C_putc(a);

else if(a<0200)out_char(a);
else{
a= (a-0200)*0400+*cur_byte++;
if(a<024000){
cur_val= (int)a;out_char(identifier);
}
else if(a<050000)confusion(_("macro defs have strange char"));
else{
cur_val= (int)a-050000;cur_section= (sixteen_bits)cur_val;
out_char(section_number);
}

}
}
protect= false;
flush_buffer();
}
pop_level(false);
}

/*:54*//*55:*/

static void
out_char(
eight_bits cur_char)
{
char*j,*k;
restart:
switch(cur_char){
case'\n':if(protect&&out_state!=verbatim)C_putc(' ');
if(protect||out_state==verbatim)C_putc('\\');
flush_buffer();if(out_state!=verbatim)out_state= normal;break;
/*59:*/

case identifier:
if(out_state==num_or_id)C_putc(' ');
for(j= (cur_val+name_dir)->byte_start,k= (cur_val+name_dir+1)->byte_start;
j<k;j++)
if((eight_bits)(*j)<0200)C_putc(*j);

else C_printf("%s",translit[(eight_bits)(*j)-0200]);
out_state= num_or_id;break;

/*:59*/

/*60:*/

case section_number:
if(cur_val> 0)C_printf("/*%d:*/",cur_val);
else if(cur_val<0)C_printf("/*:%d*/",-cur_val);
else if(protect){
cur_byte+= 4;
cur_char= (eight_bits)'\n';
goto restart;
}else{
sixteen_bits a;
a= *cur_byte++*0400;
a+= *cur_byte++;
C_printf("\n#line %d \"",(int)a);

cur_val= (int)(*cur_byte++-0200)*0400;
cur_val+= *cur_byte++;
for(j= (cur_val+name_dir)->byte_start,k= (cur_val+name_dir+1)->byte_start;
j<k;j++){
if(*j=='\\'||*j=='"')C_putc('\\');
C_putc(*j);
}
C_putc('"');C_putc('\n');
}
break;

/*:60*/

/*56:*/

case plus_plus:C_putc('+');C_putc('+');out_state= normal;break;
case minus_minus:C_putc('-');C_putc('-');out_state= normal;break;
case minus_gt:C_putc('-');C_putc('>');out_state= normal;break;
case gt_gt:C_putc('>');C_putc('>');out_state= normal;break;
case eq_eq:C_putc('=');C_putc('=');out_state= normal;break;
case lt_lt:C_putc('<');C_putc('<');out_state= normal;break;
case gt_eq:C_putc('>');C_putc('=');out_state= normal;break;
case lt_eq:C_putc('<');C_putc('=');out_state= normal;break;
case non_eq:C_putc('!');C_putc('=');out_state= normal;break;
case and_and:C_putc('&');C_putc('&');out_state= normal;break;
case or_or:C_putc('|');C_putc('|');out_state= normal;break;
case dot_dot_dot:C_putc('.');C_putc('.');C_putc('.');out_state= normal;
break;
case colon_colon:C_putc(':');C_putc(':');out_state= normal;break;
case period_ast:C_putc('.');C_putc('*');out_state= normal;break;
case minus_gt_ast:C_putc('-');C_putc('>');C_putc('*');out_state= normal;
break;

/*:56*/

case'=':case'>':C_putc(cur_char);C_putc(' ');
out_state= normal;break;
case join:out_state= unbreakable;break;
case constant:if(out_state==verbatim){
out_state= num_or_id;break;
}
if(out_state==num_or_id)C_putc(' ');out_state= verbatim;break;
case string:if(out_state==verbatim)out_state= normal;
else out_state= verbatim;break;
case'/':C_putc('/');out_state= post_slash;break;
case'*':if(out_state==post_slash)C_putc(' ');

default:C_putc(cur_char);out_state= normal;break;
}
}

/*:55*//*64:*/

static eight_bits
skip_ahead(void)
{
eight_bits c;
while(true){
if(loc> limit&&(get_line()==false))return new_section;
*(limit+1)= '@';
while(*loc!='@')loc++;
if(loc<=limit){
loc++;c= ccode[(eight_bits)*loc];loc++;
if(c!=ignore||*(loc-1)=='>')return c;
}
}
}

/*:64*//*67:*/

static boolean skip_comment(
boolean is_long_comment)
{
char c;
while(true){
if(loc> limit){
if(is_long_comment){
if(get_line())return comment_continues= true;
else{
err_print(_("! Input ended in mid-comment"));

return comment_continues= false;
}
}
else return comment_continues= false;
}
c= *(loc++);
if(is_long_comment&&c=='*'&&*loc=='/'){
loc++;return comment_continues= false;
}
if(c=='@'){
if(ccode[(eight_bits)*loc]==new_section){
err_print(_("! Section name ended in mid-comment"));loc--;

return comment_continues= false;
}
else loc++;
}
}
}

/*:67*//*69:*/

static eight_bits
get_next(void)
{
static boolean preprocessing= false;
eight_bits c;
while(true){
if(loc> limit){
if(preprocessing&&*(limit-1)!='\\')preprocessing= false;
if(get_line()==false)return new_section;
else if(print_where&&!no_where){
print_where= false;
/*85:*/

{
eight_bits a;
store_two_bytes(0150000);
if(changing&&include_depth==change_depth){
id_first= change_file_name;
store_two_bytes((sixteen_bits)change_line);
}else{
id_first= cur_file_name;
store_two_bytes((sixteen_bits)cur_line);
}
id_loc= id_first+strlen(id_first);
store_id(a);
}

/*:85*/

}
else return(eight_bits)'\n';
}
c= (eight_bits)*loc;
if(comment_continues||(c=='/'&&(*(loc+1)=='*'||*(loc+1)=='/'))){
if(skip_comment(comment_continues||*(loc+1)=='*'))return'\n';

else continue;
}
loc++;
if(xisdigit(c)||c=='.')/*73:*/
{
boolean hex_flag= false;
id_first= loc-1;
if(*id_first=='.'&&!xisdigit(*loc))goto mistake;
if(*id_first=='0'){
if(*loc=='x'||*loc=='X'){
hex_flag= true;
loc++;while(xisxdigit(*loc)||*loc=='\'')loc++;
}
else if(*loc=='b'||*loc=='B'){
loc++;while(*loc=='0'||*loc=='1'||*loc=='\'')loc++;goto found;
}
}
while(xisdigit(*loc)||*loc=='\'')loc++;
if(*loc=='.'){
loc++;
while((hex_flag&&xisxdigit(*loc))||xisdigit(*loc)||*loc=='\'')loc++;
}
if(*loc=='e'||*loc=='E'){
if(*++loc=='+'||*loc=='-')loc++;
while(xisdigit(*loc)||*loc=='\'')loc++;
}
else if(hex_flag&&(*loc=='p'||*loc=='P')){
if(*++loc=='+'||*loc=='-')loc++;
while(xisxdigit(*loc)||*loc=='\'')loc++;
}
found:while(*loc=='u'||*loc=='U'||*loc=='l'||*loc=='L'
||*loc=='f'||*loc=='F')loc++;
id_loc= loc;
return constant;
}

/*:73*/

else if(c=='\''||c=='"'
||((c=='L'||c=='u'||c=='U')&&(*loc=='\''||*loc=='"'))
||((c=='u'&&*loc=='8')&&(*(loc+1)=='\''||*(loc+1)=='"')))
/*74:*/
{
char delim= (char)c;
id_first= section_text+1;
id_loc= section_text;*++id_loc= delim;
if(delim=='L'||delim=='u'||delim=='U'){
if(delim=='u'&&*loc=='8')*++id_loc= *loc++;
delim= *loc++;*++id_loc= delim;
}
while(true){
if(loc>=limit){
if(*(limit-1)!='\\'){
err_print(_("! String didn't end"));loc= limit;break;

}
if(get_line()==false){
err_print(_("! Input ended in middle of string"));loc= buffer;break;

}
else if(++id_loc<=section_text_end)*id_loc= '\n';

}
if((c= (eight_bits)*loc++)==delim){
if(++id_loc<=section_text_end)*id_loc= (char)c;
break;
}
if(c=='\\'){
if(loc>=limit)continue;
if(++id_loc<=section_text_end)*id_loc= '\\';
c= (eight_bits)*loc++;
}
if(++id_loc<=section_text_end)*id_loc= (char)c;
}
if(id_loc>=section_text_end){
fputs(_("\n! String too long: "),stdout);

term_write(section_text+1,25);
err_print("...");
}
id_loc++;
return string;
}

/*:74*/

else if(isalpha((int)c)||isxalpha(c)||ishigh(c))
/*72:*/
{
id_first= --loc;
do
++loc;
while(isalpha((int)*loc)||isdigit((int)*loc)
||isxalpha(*loc)||ishigh(*loc));
id_loc= loc;return identifier;
}

/*:72*/

else if(c=='@')/*75:*/

switch(c= ccode[(eight_bits)*loc++]){
case ignore:continue;
case translit_code:err_print(_("! Use @l in limbo only"));continue;

case control_text:while((c= skip_ahead())=='@');

if(*(loc-1)!='>')
err_print(_("! Double @ should be used in control text"));

continue;
case section_name:
cur_section_name_char= *(loc-1);
/*77:*/
{
char*k= section_text;
/*79:*/

while(true){
if(loc> limit&&get_line()==false){
err_print(_("! Input ended in section name"));

loc= buffer+1;break;
}
c= (eight_bits)*loc;
/*80:*/

if(c=='@'){
c= (eight_bits)*(loc+1);
if(c=='>'){
loc+= 2;break;
}
if(ccode[(eight_bits)c]==new_section){
err_print(_("! Section name didn't end"));break;

}
if(ccode[(eight_bits)c]==section_name){
err_print(_("! Nesting of section names not allowed"));break;

}
*(++k)= '@';loc++;
}

/*:80*/

loc++;if(k<section_text_end)k++;
if(xisspace(c)){
c= (eight_bits)' ';if(*(k-1)==' ')k--;
}
*k= (char)c;
}
if(k>=section_text_end){
fputs(_("\n! Section name too long: "),stdout);

term_write(section_text+1,25);
printf("...");mark_harmless;
}
if(*k==' '&&k> section_text)k--;

/*:79*/

if(k-section_text> 3&&strncmp(k-2,"...",3)==0)
cur_section_name= section_lookup(section_text+1,k-3,true);

else cur_section_name= section_lookup(section_text+1,k,false);

if(cur_section_name_char=='(')
/*47:*/

{
for(an_output_file= cur_out_file;
an_output_file<end_output_files;an_output_file++)
if(*an_output_file==cur_section_name)break;
if(an_output_file==end_output_files){
if(cur_out_file> output_files)
*--cur_out_file= cur_section_name;
else overflow(_("output files"));
}
}

/*:47*/

return section_name;
}

/*:77*/

case string:/*81:*/

id_first= loc++;*(limit+1)= '@';*(limit+2)= '>';
while(*loc!='@'||*(loc+1)!='>')loc++;
if(loc>=limit)err_print(_("! Verbatim string didn't end"));

id_loc= loc;loc+= 2;
return string;

/*:81*/

case ord:/*76:*/

id_first= loc;
if(*loc=='\\')
if(*++loc=='\'')loc++;
while(*loc!='\''){
if(*loc=='@'){
if(*(loc+1)!='@')
err_print(_("! Double @ should be used in ASCII constant"));

else loc++;
}
loc++;
if(loc> limit){
err_print(_("! String didn't end"));loc= limit-1;break;

}
}
loc++;
return ord;

/*:76*/

default:return c;
}

/*:75*/

else if(xisspace(c)){
if(!preprocessing||loc> limit)continue;

else return(eight_bits)' ';

}
else if(c=='#'&&loc==buffer+1)preprocessing= true;
mistake:/*71:*/

switch(c){
case'+':if(*loc=='+')compress(plus_plus);break;
case'-':if(*loc=='-'){compress(minus_minus);}
else if(*loc=='>'){
if(*(loc+1)=='*'){loc++;compress(minus_gt_ast);}
else compress(minus_gt);
}break;
case'.':if(*loc=='*'){compress(period_ast);}
else if(*loc=='.'&&*(loc+1)=='.'){
loc++;compress(dot_dot_dot);
}break;
case':':if(*loc==':')compress(colon_colon);break;
case'=':if(*loc=='=')compress(eq_eq);break;
case'>':if(*loc=='='){compress(gt_eq);}
else if(*loc=='>')compress(gt_gt);break;
case'<':if(*loc=='='){compress(lt_eq);}
else if(*loc=='<')compress(lt_lt);break;
case'&':if(*loc=='&')compress(and_and);break;
case'|':if(*loc=='|')compress(or_or);break;
case'!':if(*loc=='=')compress(non_eq);break;
}

/*:71*/

return c;
}
}

/*:69*//*83:*/

static void
scan_repl(
eight_bits t)
{
sixteen_bits a;
if(t==section_name)/*85:*/

{
eight_bits a;
store_two_bytes(0150000);
if(changing&&include_depth==change_depth){
id_first= change_file_name;
store_two_bytes((sixteen_bits)change_line);
}else{
id_first= cur_file_name;
store_two_bytes((sixteen_bits)cur_line);
}
id_loc= id_first+strlen(id_first);
store_id(a);
}

/*:85*/

while(true)switch(a= get_next()){
/*86:*/

case identifier:store_id(a);
if(*buffer=='#'&&(
(id_loc-id_first==5&&strncmp("endif",id_first,5)==0)||
(id_loc-id_first==4&&strncmp("else",id_first,4)==0)||
(id_loc-id_first==4&&strncmp("elif",id_first,4)==0)))

print_where= true;
break;
case section_name:if(t!=section_name)goto done;
else{
/*87:*/
{
char*try_loc= loc;
while(*try_loc==' '&&try_loc<limit)try_loc++;
if(*try_loc=='+'&&try_loc<limit)try_loc++;
while(*try_loc==' '&&try_loc<limit)try_loc++;
if(*try_loc=='=')err_print(_("! Missing `@ ' before a named section"));



}

/*:87*/

a= cur_section_name-name_dir;
app_repl((a/0400)+0250);
app_repl(a%0400);
/*85:*/

{
eight_bits a;
store_two_bytes(0150000);
if(changing&&include_depth==change_depth){
id_first= change_file_name;
store_two_bytes((sixteen_bits)change_line);
}else{
id_first= cur_file_name;
store_two_bytes((sixteen_bits)cur_line);
}
id_loc= id_first+strlen(id_first);
store_id(a);
}

/*:85*/

}
break;
case output_defs_code:if(t!=section_name)err_print(_("! Misplaced @h"));

else{
output_defs_seen= true;
a= output_defs_flag;
app_repl((a/0400)+0200);
app_repl(a%0400);
/*85:*/

{
eight_bits a;
store_two_bytes(0150000);
if(changing&&include_depth==change_depth){
id_first= change_file_name;
store_two_bytes((sixteen_bits)change_line);
}else{
id_first= cur_file_name;
store_two_bytes((sixteen_bits)cur_line);
}
id_loc= id_first+strlen(id_first);
store_id(a);
}

/*:85*/

}
break;
case constant:case string:
/*88:*/

app_repl(a);
while(id_first<id_loc){
if(*id_first=='@'){
if(*(id_first+1)=='@')id_first++;
else err_print(_("! Double @ should be used in string"));

}
else if(a==constant&&*id_first=='\''&&!keep_digit_separators)
id_first++;
app_repl(*id_first++);
}
app_repl(a);

/*:88*/

break;
case ord:
/*89:*/
{
int c= (int)((eight_bits)*id_first);
if(c=='\\'){
c= (int)((eight_bits)*++id_first);
if(c>='0'&&c<='7'){
c-= '0';
if(*(id_first+1)>='0'&&*(id_first+1)<='7'){
c= 8*c+*(++id_first)-'0';
if(*(id_first+1)>='0'&&*(id_first+1)<='7'&&c<32)
c= 8*c+*(++id_first)-'0';
}
}
else switch(c){
case't':c= '\t';break;
case'n':c= '\n';break;
case'b':c= '\b';break;
case'f':c= '\f';break;
case'v':c= '\v';break;
case'r':c= '\r';break;
case'a':c= '\7';break;
case'?':c= '?';break;
case'x':
if(xisdigit(*(id_first+1)))c= (int)(*(++id_first)-'0');
else if(xisxdigit(*(id_first+1))){
++id_first;
c= toupper((int)*id_first)-'A'+10;
}
if(xisdigit(*(id_first+1)))c= 16*c+(int)(*(++id_first)-'0');
else if(xisxdigit(*(id_first+1))){
++id_first;
c= 16*c+toupper((int)*id_first)-(int)'A'+10;
}
break;
case'\\':c= '\\';break;
case'\'':c= '\'';break;
case'\"':c= '\"';break;
default:err_print(_("! Unrecognized escape sequence"));

}
}

app_repl(constant);
if(c>=100)app_repl((int)'0'+c/100);
if(c>=10)app_repl((int)'0'+(c/10)%10);
app_repl((int)'0'+c%10);
app_repl(constant);
}

/*:89*/

break;
case definition:case format_code:case begin_C:if(t!=section_name)goto done;
else{
err_print(_("! @d, @f and @c are ignored in C text"));continue;

}
case new_section:goto done;

/*:86*/

case')':app_repl(a);
if(t==macro)app_repl(' ');
break;
default:app_repl(a);
}
done:next_control= (eight_bits)a;
if(text_ptr> text_info_end)overflow(_("text"));
cur_text= text_ptr;(++text_ptr)->tok_start= tok_ptr;
}

/*:83*//*90:*/

static void
scan_section(void)
{
name_pointer p;
text_pointer q;
sixteen_bits a;
section_count++;no_where= true;
if(*(loc-1)=='*'&&show_progress){
printf("*%d",(int)section_count);update_terminal;
}
next_control= ignore;
while(true){
/*92:*/

while(next_control<definition)

if((next_control= skip_ahead())==section_name){
loc-= 2;next_control= get_next();
}

/*:92*/

if(next_control==definition){
/*93:*/

while((next_control= get_next())=='\n');
if(next_control!=identifier){
err_print(_("! Definition flushed, must start with identifier"));

continue;
}
store_id(a);
if(*loc!='('){
app_repl(string);app_repl(' ');app_repl(string);
}
scan_repl(macro);
cur_text->text_link= macro;

/*:93*/

continue;
}
if(next_control==begin_C){
p= name_dir;break;
}
if(next_control==section_name){
p= cur_section_name;
/*94:*/

while((next_control= get_next())=='+');
if(next_control!='='&&next_control!=eq_eq)
continue;

/*:94*/

break;
}
return;
}
no_where= print_where= false;
/*95:*/

/*96:*/

store_two_bytes((sixteen_bits)(0150000+section_count));


/*:96*/

scan_repl(section_name);
/*97:*/

if(p==name_dir||p==NULL){
last_unnamed->text_link= cur_text-text_info;last_unnamed= cur_text;
}
else if(p->equiv==(void*)text_info)p->equiv= (void*)cur_text;

else{
q= (text_pointer)p->equiv;
while(q->text_link<section_flag)
q= q->text_link+text_info;
q->text_link= cur_text-text_info;
}
cur_text->text_link= section_flag;


/*:97*/


/*:95*/

}

/*:90*//*98:*/

static void
phase_one(void){
phase= 1;
section_count= 0;
reset_input();
skip_limbo();
while(!input_has_ended)scan_section();
check_complete();
}

/*:98*//*100:*/

static void
skip_limbo(void)
{
while(true){
if(loc> limit&&get_line()==false)return;
*(limit+1)= '@';
while(*loc!='@')loc++;
if(loc++<=limit){
char c= *loc++;
switch(ccode[(eight_bits)c]){
case new_section:return;
case translit_code:/*102:*/

while(xisspace(*loc)&&loc<limit)loc++;
loc+= 3;
if(loc> limit||!xisxdigit(*(loc-3))||!xisxdigit(*(loc-2))
||(*(loc-3)>='0'&&*(loc-3)<='7')||!xisspace(*(loc-1)))
err_print(_("! Improper hex number following @l"));

else{
unsigned int i;
char*beg;
sscanf(loc-3,"%x",&i);
while(xisspace(*loc)&&loc<limit)loc++;
beg= loc;
while(loc<limit&&(xisalpha(*loc)||xisdigit(*loc)||*loc=='_'))loc++;
if(loc-beg>=translit_length)
err_print(_("! Replacement string in @l too long"));

else{
strncpy(translit[i-0200],beg,(size_t)(loc-beg));
translit[i-0200][loc-beg]= '\0';
}
}

/*:102*/
break;
case format_code:case'@':break;
case control_text:if(c=='q'||c=='Q'){
while((c= (char)skip_ahead())=='@');
if(*(loc-1)!='>')
err_print(_("! Double @ should be used in control text"));

break;
}
default:err_print(_("! Double @ should be used in limbo"));

}
}
}
}

/*:100*//*103:*/

void
print_stats(void){
puts(_("\nMemory usage statistics:"));
printf(_("%td names (out of %ld)\n"),
(ptrdiff_t)(name_ptr-name_dir),(long)max_names);
printf(_("%td replacement texts (out of %ld)\n"),
(ptrdiff_t)(text_ptr-text_info),(long)max_texts);
printf(_("%td bytes (out of %ld)\n"),
(ptrdiff_t)(byte_ptr-byte_mem),(long)max_bytes);
printf(_("%td tokens (out of %ld)\n"),
(ptrdiff_t)(tok_ptr-tok_mem),(long)max_toks);
}

/*:103*/
