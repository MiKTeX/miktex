
/* t4ht.c (2012-07-25-19:28), generated from tex4ht-t4ht.tex
   Copyright (C) 2009-2012 TeX Users Group
   Copyright (C) 1998-2009 Eitan M. Gurari

%
% This work may be distributed and/or modified under the
% conditions of the LaTeX Project Public License, either
% version 1.3c of this license or (at your option) any
% later version. The latest version of this license is in
%   http://www.latex-project.org/lppl.txt
% and version 1.3c or later is part of all distributions
% of LaTeX version 2005/12/01 or later.
%
% This work has the LPPL maintenance status "maintained".
%
% The Current Maintainer of this work
% is the TeX4ht Project <http://tug.org/tex4ht>.
%
% If you modify this program, changing the
% version identification would be appreciated.

*/

/* **********************************************
    Compiler options                            *
    (uncommented | command line)                *
------------------------------------------------*
       Classic C (CC)             default
#define ANSI                      ansi-c, c++
#define DOS_C
#define HAVE_STRING_H             <string.h>
#define HAVE_DIRENT_H             <dirent.h>
#define HAVE_SYS_NDIR_H           <sys/ndir.h>
#define HAVE_SYS_DIR_H            <sys/dir.h>
#define HAVE_NDIR_H               <ndir.h>
#define HAVE_IO_H                 <io.h>
#define HAVE_UNISTD_H             <unistd.h>
#define WIN32
#define KPATHSEA
#define SYSTEM_FUNCTION_OK
#define CDECL                    ..........
#define BCC32                    bordland c++

*************************************************
    Tex4ht variables                            *
    (uncommented | command line)                *
----------------------------------------------- */

#ifndef ENVFILE

#endif


/* ******************************************** */


#ifdef BCC32
#define WIN32
#define ANSI
#define HAVE_IO_H
#define HAVE_STRING_H
#define PLATFORM "ms-win32"
#endif



#ifdef BCC
#define DOS_C
#define ANSI
#define HAVE_IO_H
#define PLATFORM "ms-dos"
#endif



#ifdef __DJGPP__
#define DOS_WIN32
#define ANSI
#endif


#ifdef DOS_C
#define DOS
#endif
#ifdef DOS
#define DOS_WIN32
#ifndef HAVE_STRING_H
#define HAVE_STRING_H
#endif
#endif
#ifdef WIN32
#define DOS_WIN32
#endif

#ifdef DOS_WIN32
#define STRUCT_DIRENT
#endif



#ifdef KPATHSEA
#ifdef WIN32
#define KWIN32
#endif
#endif


#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif

#ifdef KPATHSEA
#include <kpathsea/config.h>
#endif
#include <stdio.h>   
#include <stdlib.h>  


#ifdef HAVE_STRING_H
#include <string.h>
#endif


#ifdef HAVE_DIRENT_H

#include <dirent.h>


#else
#ifndef STRUCT_DIRENT
#define STRUCT_DIRECT
#endif

#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif


#endif


#include <signal.h>




#ifndef __DJGPP__

#ifndef F_OK
#ifdef DOS_WIN32
#define  F_OK 0               
#endif
#ifndef KPATHSEA
#ifndef DOS_WIN32
#define HAVE_UNISTD_H
#endif
#endif
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifndef KPATHSEA
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif


#endif


#ifdef KPATHSEA
#include <kpathsea/c-errno.h>
#include <kpathsea/c-ctype.h>
#include <kpathsea/c-fopen.h>
#include <kpathsea/c-pathmx.h>
#include <kpathsea/proginit.h>
#include <kpathsea/tex-file.h>
#include <kpathsea/tex-make.h>
#include <kpathsea/variable.h>
#include <signal.h>
#if !defined(_AMIGA) && !defined(WIN32)
#include <sys/time.h>
#endif
#include <fcntl.h>
#include <setjmp.h>
#endif 

#ifdef __DJGPP__

#ifndef F_OK
#ifdef DOS_WIN32
#define  F_OK 0               
#endif
#ifndef KPATHSEA
#ifndef DOS_WIN32
#define HAVE_UNISTD_H
#endif
#endif
#endif
#ifdef HAVE_IO_H
#include <io.h>
#endif
#ifndef KPATHSEA
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif


#endif






#ifdef DOS
#define PROTOTYP
#endif
#ifdef ANSI
#define PROTOTYP
#endif
#ifdef KWIN32
#define PROTOTYP
#endif


#ifdef KPATHSEA
#ifdef WIN32
#undef CDECL
#define CDECL                     __cdecl
#else
#define CDECL
#endif
#endif


#if defined(DOS_WIN32) || defined(__MSDOS__)
#define READ_BIN_FLAGS "rb"
#define READ_TEXT_FLAGS "r"
#define WRITE_BIN_FLAGS "wb"
#ifdef WIN32
#define WRITE_TEXT_FLAGS "wb"
#else
#define WRITE_TEXT_FLAGS "w"
#endif
#else
#define READ_BIN_FLAGS "r"
#define READ_TEXT_FLAGS "r"
#define WRITE_BIN_FLAGS "w"
#define WRITE_TEXT_FLAGS "w"
#endif



#define Q_CHAR char
#define U_CHAR char
#define C_CHAR char
#define Q_NULL (Q_CHAR *) 0
#define U_NULL (U_CHAR *) 0
#define C_NULL (C_CHAR *) 0


#define IGNORED void


#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif


#ifdef PROTOTYP
#define MYVOID void
#define ARG_I(x) x
#define ARG_II(x,y) x,y
#define ARG_III(x,y,z) x,y,z
#define ARG_IV(x,y,z,w) x,y,z,w
#define ARG_V(x,y,z,w,u) x,y,z,w,u
#else
#define MYVOID
#define ARG_I(x)
#define ARG_II(x,y)
#define ARG_III(x,y,z)
#define ARG_IV(x,y,z,w)
#define ARG_V(x,y,z,w,u)
#endif


#if defined(DOS_WIN32) || defined(__DJGPP__)
#define dir_path_slash(str) (is_forward_slash(str)? '/' : '\\')
#else
#define dir_path_slash(str)  '/'
#endif


struct empty_pic_struct{
  long int n;
  struct empty_pic_struct *next;
};


#ifndef HTFDIR
#define HTFDIR      ""
#endif


struct files_rec{
  FILE* file;
  char *name, *group, op;
  Q_CHAR file_mode[5];
  int loc, label;
  struct files_rec  *from_rec, *right, *left, *down, *up;
};
#define No_op     0
#define From_op   1
#define Until_op  2
#define Skip_op   3
#define Cont_op   4
#define Addr_op   5
#define Set_op    6


struct env_c_rec{
  Q_CHAR *option;
  struct env_c_rec  *next;
};


struct sys_call_rec{
  Q_CHAR * filter;
  struct sys_call_rec *next;
};


struct htf_struct{
  Q_CHAR *key,  *body, *media;
  struct htf_struct *next;
};


struct script_struct{
  Q_CHAR *command;
  struct script_struct *next;
};


#define NULL_SCRIPT (struct script_struct *) 0


#define LG_EXT ".lg"


#define bad_arg            err_arg(0)
#define bad_mem             err_i(1)


#define eq_str(x,y) (!strcmp(x,y))


#define m_alloc(typ,n) (typ *) malloc_chk((int) ((n) * sizeof(typ)))


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef BOOL
#define BOOL int
#endif


#define eq_str(x,y) (!strcmp(x,y))



static int eoln_ch;


static BOOL check_tex4ht_c_err = FALSE;


static Q_CHAR *ch_mod = Q_NULL;
static Q_CHAR *debug = Q_NULL;
static Q_CHAR *Xfield = Q_NULL;
static Q_CHAR *Dotfield = Q_NULL;
static Q_CHAR *dir = Q_NULL;
static Q_CHAR *lg_name  = Q_NULL, tmp_name[255], job_name[255];
static Q_CHAR *nopict = Q_NULL;
static Q_CHAR *bitmaps_no_dm = Q_NULL;
static BOOL newchmod = FALSE;
static Q_CHAR *noreuse = Q_NULL;


U_CHAR *HOME_DIR;


static struct env_c_rec *envChoice
       = (struct env_c_rec*) 0;


#if ! defined(MIKTEX)
static BOOL system_yes;
#endif
static struct sys_call_rec *system_calls = (struct sys_call_rec *) 0;


static struct htf_struct *htf_rec = (struct htf_struct *) 0;


static BOOL status;


static char *Font_css_base = NULL,
            *Font_css_mag = NULL;
static int base_font_size = 10;


static struct script_struct
    * dvigif_script = NULL_SCRIPT,
    * dvigif_glyp_script = NULL_SCRIPT,
    * move_script = NULL_SCRIPT,
    * empty_fig_script = NULL_SCRIPT,
    * copy_script = NULL_SCRIPT,
    * 
file_script

 = NULL_SCRIPT,
    * 
ext_script

 = NULL_SCRIPT,
    * chmod_script = NULL_SCRIPT;


static FILE* lg_file;
static long  begin_lg_file;


static const C_CHAR *warn_err_mssg[]={ 

"\n--------------------------------------------------------------------\n"
"t4ht [-f<dir char>]filename ...\n"
"  -b     ignore -d -m -M for bitmaps\n"
"  -c...  choose named segment in env file\n"
"  -d...  directory for output files       (default:  current)\n"
"  -e...  location of tex4ht.env\n"
"  -i     debugging info\n"
"  -g     ignore errors in system calls\n"
"  -m...  chmod ... of new output files (reused bitmaps excluded)\n"
"  -p     don't convert pictures           (default:  convert)\n"
"  -r     replace bitmaps of all glyphs    (default:  reuse old ones)\n"
"  -M...  chmod ... of all output files\n"
"  -Q     quit, if tex4ht.c had problems\n"
"  -S...  permission for system calls: *-always, filter\n"
"  -X...  content for field %%3 in X scripts\n"
"  -....  content for field %%2 in . scripts\n\n"
"Example: \n"
"   t4ht name -d/WWW/temp/ -etex4ht-32.env -m644\n"
"--------------------------------------------------------------------\n"

,                            
"Insufficient memory\n",                                

"Illegal storage address\n", 
"Floating-point\n",          
"Interrupt with Cntr-C\n",   


"Can't find/open file `%s'\n",                         

 "" };


static Q_CHAR command[255];
static int system_return;


static BOOL system_yes;


static BOOL always_call_sys = FALSE;


static Q_CHAR* match[10];
static int   max_match[10];



#if defined(DOS_WIN32) || defined(__DJGPP__)
   static BOOL is_forward_slash( ARG_I(Q_CHAR *) );
#endif


static char *  abs_addr( ARG_II( U_CHAR *, U_CHAR *) );


static void execute_script(
  ARG_V(struct script_struct*,const Q_CHAR *,const Q_CHAR *,const Q_CHAR *,const Q_CHAR *) );


static struct script_struct  * filterGifScript(
        ARG_II( struct script_struct  *, Q_CHAR *) );


void free_script(
        ARG_I( struct script_struct  *) );


static BOOL strpre( ARG_II(char *,char *) );


static struct script_struct * add_script( ARG_I(struct script_struct *) );


static FILE* f_open( ARG_II(const char*,const char*) );


static FILE* f_home_open( ARG_II(const char*,const char*) );


static FILE* open_file( ARG_II(const C_CHAR *, const C_CHAR *) );


static void err_i( ARG_I(int) );


static void err_arg( ARG_I(int) );


static void warn_i_str( ARG_II(int,const Q_CHAR *) );


static void

#ifdef CDECL
CDECL
#endif


sig_err(ARG_I(int));


#ifdef KWIN32
static BOOL sigint_handler(ARG_I(DWORD));
#endif


static void call_sys(ARG_I(Q_CHAR *));


static void strct( ARG_II(C_CHAR *, const C_CHAR *) );


static long int get_long_int( ARG_I(Q_CHAR *) );


static void* malloc_chk(ARG_I(int));


static void* r_alloc(ARG_II(void *, size_t));


static BOOL scan_until_end_str( ARG_IV(const C_CHAR *, int, BOOL, FILE *) );


static BOOL scan_until_str( ARG_IV(const C_CHAR *, int, BOOL, FILE *) );


static BOOL scan_str( ARG_III(const C_CHAR *, BOOL, FILE *) );



#if defined(DOS_WIN32) || defined(__DJGPP__)

static BOOL is_forward_slash
#ifdef ANSI
#define SEP ,
(
                                    Q_CHAR * str
)
#undef SEP
#else
#define SEP ;
(str)
                                    Q_CHAR * str
;
#undef SEP
#endif
{
   while( *str ){  if( *(str++) == '/' ) { return TRUE; } }
   return FALSE;
}
#endif



static char *  abs_addr
#ifdef ANSI
#define SEP ,
(
                                                 U_CHAR  *dir SEP 
                                                 U_CHAR  *base

)
#undef SEP
#else
#define SEP ;
( dir, base)
                                                 U_CHAR  *dir SEP 
                                                 U_CHAR  *base

;
#undef SEP
#endif
{                                         U_CHAR  *p;
   p = m_alloc(char, (int) strlen((char *)  dir ) +
                     (base? (int) strlen((char *)  base ):0) +
                     (int) strlen((char *)  HOME_DIR  ));
   *p = '\0';
   if( (*(dir+1) == '~') && base ){
     if( *base == '~' ){
       (IGNORED) strct(p, HOME_DIR);
       (IGNORED) strct(p, base+1);
     } else {
       (IGNORED) strct(p, base);
     }
     (IGNORED) strct(p, dir+2);
   } else {
     (IGNORED) strct(p, HOME_DIR);
     (IGNORED) strct(p, dir+1);
   }
   return p;
}



static void execute_script
#ifdef ANSI
#define SEP ,
(
                    struct script_struct* script SEP 
                    const Q_CHAR * match_1 SEP 
                    const Q_CHAR * match_2 SEP 
                    const Q_CHAR * match_3 SEP 
                    const Q_CHAR * match_4

)
#undef SEP
#else
#define SEP ;
(script,match_1,match_2,match_3,match_4)
                    struct script_struct* script SEP 
                    const Q_CHAR * match_1 SEP 
                    const Q_CHAR * match_2 SEP 
                    const Q_CHAR * match_3 SEP 
                    const Q_CHAR * match_4

;
#undef SEP
#endif
{                               struct script_struct* temp;
                                 Q_CHAR *p, *q;
                                 const Q_CHAR *t;
  
#ifdef KPATHSEA
const char * texmf = kpse_var_value( "SELFAUTOPARENT" );
#endif


   temp = script;  system_return = 0;
   while( temp ){
      
p = temp->command;
q = command;
while( *p != '\0' ){
  *q = *(p++);
  if( (*q == '%') && (*p == '%')
     && (    (*(p+1) == '%')
          || (*(p+1) == '~')
          || ( (*(p+1) >= '0') && (*(p+1) < '5') ) )
  ){  p++;
    switch( *(p++) ){
      case '%':{  q++; break; }
      case '0':{   t = job_name;
            while( *t != '\0' ){ *(q++) = *(t++); }  break; }
      case '1':{  t = match_1;
            while( *t != '\0' ){ *(q++) = *(t++); }  break; }
      case '2':{  t = match_2;
            while( *t != '\0' ){ *(q++) = *(t++); }  break; }
      case '3':{  t = match_3;
            while( *t != '\0' ){ *(q++) = *(t++); }  break; }
      case '4':{  t = match_4;
            while( *t != '\0' ){ *(q++) = *(t++); }  break; }
      case '~':{ 
#ifdef KPATHSEA
   if( texmf ){
      t = texmf;
      while( *t != '\0' ){ *(q++) = *(t++); }
   } else {
      *(q++) = '~';
   }
#else
   *(q++) = '~';
#endif

          break; }
      default: {  }
  }} else { q++; }
}
*q = '\0';


      if( (command[0] != '\0') && !system_return ){
         
(IGNORED) call_sys(command);

 }
      temp = temp->next;
}  }



static struct script_struct * filterGifScript
#ifdef ANSI
#define SEP ,
(
                                   struct script_struct *script SEP 
                                                   Q_CHAR *file
)
#undef SEP
#else
#define SEP ;
( script, file)
                                   struct script_struct *script SEP 
                                                   Q_CHAR *file
;
#undef SEP
#endif
{
                   struct script_struct *filtered_script, *scr,
                                                   *old_script,
                                                   *new_script;
                                         Q_CHAR *command, *ext;
   filtered_script = NULL_SCRIPT;
   if( script == NULL_SCRIPT )
       return filtered_script;
   old_script = script;
   command = old_script->command;
   if( *command != '.' ){
      
new_script = NULL_SCRIPT;
while( old_script != NULL_SCRIPT ){
   command = old_script->command;
   if( *command == '.' ){ return filtered_script; }
   scr = (struct script_struct *)
            m_alloc(struct script_struct, (int) 1);
   scr->command = old_script->command;
   scr->next = NULL_SCRIPT;
   if( new_script == NULL_SCRIPT ){
     filtered_script = new_script = scr;
   } else {
     new_script = new_script->next = scr;
   }
   old_script = old_script->next;
}
return filtered_script;


   } else {
      

ext = file;
while( *ext != '\0' ){ ext++; }
while( (*ext != '.') && (ext != file) ){ ext--; }


while( TRUE ){
  if( old_script == NULL_SCRIPT ){
     return NULL_SCRIPT;
  }
  command = old_script->command;
  if(*command != '.') {
    old_script = old_script->next;
    continue;
  }
  old_script = old_script->next;
  if( (*(command+1) == '\0') || eq_str(ext,command) ){
    break;
  }
}

new_script = NULL_SCRIPT;
while( old_script != NULL_SCRIPT ){
   command = old_script->command;
   if( *command == '.' ){ return filtered_script; }
   scr = (struct script_struct *)
            m_alloc(struct script_struct, (int) 1);
   scr->command = old_script->command;
   scr->next = NULL_SCRIPT;
   if( new_script == NULL_SCRIPT ){
     filtered_script = new_script = scr;
   } else {
     new_script = new_script->next = scr;
   }
   old_script = old_script->next;
}
return filtered_script;




   }
}



void free_script
#ifdef ANSI
#define SEP ,
(
                                   struct script_struct *script
)
#undef SEP
#else
#define SEP ;
(script)
                                   struct script_struct *script
;
#undef SEP
#endif
{
                                   struct script_struct *temp;
  while( script != NULL_SCRIPT ){
    temp = script;
    script = script->next;
    free( temp );
  }
}



static BOOL strpre
#ifdef ANSI
#define SEP ,
(
                    char * s1 SEP 
                    char * s2
)
#undef SEP
#else
#define SEP ;
(s1,s2)
                    char * s1 SEP 
                    char * s2
;
#undef SEP
#endif
{                   int i;
  for( i=0; i < (int) strlen((char *) s1); i++){
    if( *(s1+i) != *(s2+i) ){ return FALSE; }
  }
  return TRUE;
}



static struct script_struct * add_script
#ifdef ANSI
#define SEP ,
(
                           struct script_struct * script
)
#undef SEP
#else
#define SEP ;
(script)
                           struct script_struct * script
;
#undef SEP
#endif
{
   struct script_struct* temp, * q;
   temp = (struct script_struct *)
                      m_alloc(struct script_struct, (int) 1);
   temp->command = match[1];
   if( debug ){
      (IGNORED) printf(".......   %s\n", temp->command); }
   temp->next = NULL_SCRIPT;
   match[1] = (Q_CHAR *) malloc(70);
   max_match[1] = 70;
   if( script ){
      q = script;
      while( q->next ){ q = q->next; }
      q->next = temp;
   } else {
      script = temp;
   }
   return script;
}


#if defined(__MSDOS__)


static char *get_env_dir
#ifdef ANSI
#define SEP ,
(
      Q_CHAR *progname

)
#undef SEP
#else
#define SEP ;
(progname)
      Q_CHAR *progname

;
#undef SEP
#endif
{    int  i;
      Q_CHAR *p;
  if(! progname || ! *progname)  return NULL;  
  i = (int) strlen((char *) progname);
  while( (progname[--i] != (int) dir_path_slash(progname) )
        && i > 0) ;                               
  if(i == 0)  return NULL;                        
  p = (Q_CHAR *) malloc(i+12);
  if(p == NULL)  return NULL;     
  strncpy(p, progname, i+1);                         
  (IGNORED) strcpy((char *) &p[i+1],
                   "tex4ht.env");                    
  return p;
}


#endif



static FILE* f_open
#ifdef ANSI
#define SEP ,
(
                          const char*  name  SEP 
                          const char*  flags
)
#undef SEP
#else
#define SEP ;
( name, flags )
                          const char*  name  SEP 
                          const char*  flags
;
#undef SEP
#endif
{                        FILE* file;
  if( (file = fopen(name,flags) ) != NULL ) {
     (IGNORED)  printf("(%s)\n",name);
  }
  return file;
}



static FILE* f_home_open
#ifdef ANSI
#define SEP ,
(
                          const char*  name  SEP 
                          const char*  flags
)
#undef SEP
#else
#define SEP ;
( name, flags )
                          const char*  name  SEP 
                          const char*  flags
;
#undef SEP
#endif
{                        FILE* file;
                          U_CHAR *str;
  if( *name == '~' ){
     if( HOME_DIR ){
         str = m_alloc(char, strlen((char *) HOME_DIR)+strlen(name));
         (IGNORED) sprintf(str,"%s%s", HOME_DIR, name+1);
         file = f_open(str,flags);
         free((void *)  str);
         return file;
     } else { return NULL; }
  } else {     return f_open( name, flags );   }
}



static FILE* open_file
#ifdef ANSI
#define SEP ,
(
                         const C_CHAR *name SEP  const C_CHAR *ext
)
#undef SEP
#else
#define SEP ;
(name,ext)
                         const C_CHAR *name SEP  const C_CHAR *ext
;
#undef SEP
#endif
{                       FILE* file;
                         C_CHAR filename[255], *p;
   if( eq_str( ext,LG_EXT ) ) {
      (IGNORED) strcpy((char *) filename, (char *) job_name);
      (IGNORED) strct(filename, ext);
   } else {
      (IGNORED) strcpy((char *)  filename, name );
      p = filename;
      while( TRUE ){
        if( *p == '.' ){  break; }
        if( *p == '\0' ){ (IGNORED) strcpy((char *) p, ext); break; }
        p++;
      }
   }
   file  = fopen(filename, READ_TEXT_FLAGS);
   if( !file ) {
      (IGNORED) warn_i_str(5,filename);
   } else { (IGNORED) printf ("Entering %s\n", filename); }

   return file;
}



static void err_i
#ifdef ANSI
#define SEP ,
(      int  n

)
#undef SEP
#else
#define SEP ;
(n)      int  n

;
#undef SEP
#endif
{  (IGNORED) fprintf(stderr,"--- error --- ");
   (IGNORED) fprintf(stderr, "%s", warn_err_mssg[n]);
   exit(EXIT_FAILURE);
}



static void err_arg
#ifdef ANSI
#define SEP ,
(      int  n

)
#undef SEP
#else
#define SEP ;
(n)      int  n

;
#undef SEP
#endif
{  (IGNORED) fprintf(stderr,"--- error --- ");
   (IGNORED) fprintf(stderr, "%s", warn_err_mssg[n]);
   exit(EXIT_FAILURE);
}



static void warn_i_str
#ifdef ANSI
#define SEP ,
(
    int  n SEP 
    const Q_CHAR *str

)
#undef SEP
#else
#define SEP ;
(n,str)
    int  n SEP 
    const Q_CHAR *str

;
#undef SEP
#endif
{  (IGNORED) fprintf(stderr,"--- warning --- ");
   (IGNORED) fprintf(stderr,warn_err_mssg[n], str);
}



static void

#ifdef CDECL
CDECL
#endif


sig_err
#ifdef ANSI
#define SEP ,
(  int s
)
#undef SEP
#else
#define SEP ;
(s)  int s
;
#undef SEP
#endif
{
  (void) signal(s,SIG_IGN);  
  switch( s ){
#ifdef SIGSEGV
    case SIGSEGV: err_i(2);
#endif
    case SIGFPE : err_i(3);
#if defined(SIGINT) && !defined(WIN32)
    case SIGINT : err_i(4);
#endif
  }
  
#ifdef __DJGPP__
  if (s != SIGINT && s != SIGQUIT)
    exit(EXIT_FAILURE);
#endif


}


#ifdef KWIN32

static BOOL sigint_handler
#ifdef ANSI
#define SEP ,
(     DWORD dwCtrlType
)
#undef SEP
#else
#define SEP ;
(dwCtrlType)     DWORD dwCtrlType
;
#undef SEP
#endif
{
  err_i(32);
  return FALSE;      
}
#endif



static void call_sys
#ifdef ANSI
#define SEP ,
(  Q_CHAR * command
)
#undef SEP
#else
#define SEP ;
(command)  Q_CHAR * command
;
#undef SEP
#endif
{
   if( *command ){
      (IGNORED) printf("System call: %s\n", command);
      system_return = system_yes?  (int) system(command) : -1;
      (IGNORED) printf("%sSystem return: %d\n",
            system_return? "--- Warning --- " : "", system_return );
      if( always_call_sys ){ system_return = 0; }
   }
}



static void strct
#ifdef ANSI
#define SEP ,
(
     C_CHAR * str1 SEP 
     const C_CHAR * str2

)
#undef SEP
#else
#define SEP ;
( str1, str2 )
     C_CHAR * str1 SEP 
     const C_CHAR * str2

;
#undef SEP
#endif
{   Q_CHAR * ch;
   ch = str1 + (int) strlen((char *) str1);
   (IGNORED) strcpy((char *)  ch, str2 );
}



static long int get_long_int
#ifdef ANSI
#define SEP ,
(
                         Q_CHAR   *str
)
#undef SEP
#else
#define SEP ;
(str)
                         Q_CHAR   *str
;
#undef SEP
#endif
{                  long int    i;
                         Q_CHAR   *ch;
   ch = str;   i = 0;
   while( (*ch>= '0') && (*ch <='9') ){
     i = 10*i + *(ch++) - '0';
   }
   return i;
}



static void* malloc_chk
#ifdef ANSI
#define SEP ,
( int n
)
#undef SEP
#else
#define SEP ;
( n ) int n
;
#undef SEP
#endif
{      void* p;
   if((p = (void *) malloc( (size_t) n)) == NULL ) bad_mem;
   return p;
}



static void* r_alloc
#ifdef ANSI
#define SEP ,
(
      void   *q SEP 
      size_t  n
)
#undef SEP
#else
#define SEP ;
( q, n )
      void   *q SEP 
      size_t  n
;
#undef SEP
#endif
{    void*   p;
   if((p = (void *) realloc( q, (size_t) n)) == NULL) bad_mem;
   return p;
}



static BOOL  scan_until_end_str
#ifdef ANSI
#define SEP ,
(
                         const C_CHAR   *str SEP 
                         int    n SEP 
                         BOOL   flag SEP 
                         FILE*  file
)
#undef SEP
#else
#define SEP ;
(str,n,flag,file)
                         const C_CHAR   *str SEP 
                         int    n SEP 
                         BOOL   flag SEP 
                         FILE*  file
;
#undef SEP
#endif
{                       Q_CHAR   *p;
                         int i;
   if( !flag ) { return flag; }
   p = match[n];  i = 0;
   while( TRUE ){
     if( (i+1) >= max_match[n] ){
        max_match[n] += 10;
        p = match[n] = (Q_CHAR *)
                     r_alloc((void *) match[n], (size_t) max_match[n]);
     }
     p[i] = (char) (eoln_ch = getc(file));
     if( (eoln_ch == (int) '\n') || (eoln_ch == EOF) ){  break; }
     i++;
   }
   p[i] = '\0';
   i -= (int) strlen(str);
   if( i>= 0 ){  return eq_str(p+i,str);   }
   return FALSE;
}



static BOOL  scan_until_str
#ifdef ANSI
#define SEP ,
(
                         const C_CHAR   *str SEP 
                         int    n SEP 
                         BOOL   flag SEP 
                         FILE*  file
)
#undef SEP
#else
#define SEP ;
(str,n,flag,file)
                         const C_CHAR   *str SEP 
                         int    n SEP 
                         BOOL   flag SEP 
                         FILE*  file
;
#undef SEP
#endif
{                       Q_CHAR   *p, ch;
                         int i, j;
   if( !flag ) { return flag; }
   p = match[n];  i = 0;
   while( TRUE ){
     ch =  (char) (eoln_ch = getc(file));
     if( (eoln_ch == (int) '\n') || (eoln_ch == EOF) ){  return FALSE; }
     if( (i+1) >= max_match[n] ){
        max_match[n] += 10;
        p = match[n] = (Q_CHAR *)
                     r_alloc((void *) match[n], (size_t) max_match[n]);
     }
     p[i++] = ch;
     j =  i - (int) strlen(str);
     if( j>= 0 ){
       p[i] = '\0';
       if( eq_str(p+j,str) ) { return TRUE;  }
     }
   }
}



static BOOL  scan_str
#ifdef ANSI
#define SEP ,
(
                         const C_CHAR   *str SEP 
                         BOOL   flag SEP 
                         FILE*  file
)
#undef SEP
#else
#define SEP ;
(str,flag,file)
                         const C_CHAR   *str SEP 
                         BOOL   flag SEP 
                         FILE*  file
;
#undef SEP
#endif
{                        const Q_CHAR *p;
                         int temp_eoln_ch;
   if( !flag ) { return flag; }
   p = str;
   while( *p != '\0' ){
     if( *(p++) != (temp_eoln_ch = getc(file)) ) {
        while( (temp_eoln_ch != (int) '\n')
            && (temp_eoln_ch != EOF) ){ temp_eoln_ch = getc(file); }
        eoln_ch = temp_eoln_ch;  return FALSE;
     }
   }
   return TRUE;
}



int 
#ifdef CDECL
CDECL
#endif

 main
#ifdef ANSI
#define SEP ,
(
       int  argc SEP 
       Q_CHAR **argv
)
#undef SEP
#else
#define SEP ;
(argc, argv)
       int  argc SEP 
       Q_CHAR **argv
;
#undef SEP
#endif
{ 
struct empty_pic_struct *empty_pic;


Q_CHAR * tex4ht_env_file = (Q_CHAR *) 0;
Q_CHAR * dos_env_file =
#if defined(__MSDOS__)
  
get_env_dir(argv[0])

;
#endif
#if !defined(__MSDOS__)
  (Q_CHAR *) 0;
#endif



   
#ifdef WIN32
  /* See comments in tex4ht */
  if (argc > 2) {
    int  i, nargc;
    char **nargv, **pnargv, **pargv;

    nargv = (char **) xmalloc (2 * argc * sizeof (char *));
    pnargv = nargv;
    pargv = argv;
    *pnargv++ = xstrdup (*pargv++);
    *pnargv++ = xstrdup (*pargv++);
    nargc = 2;

    for (i=2; i < argc; i++) {
      char *p, *q, *r;
      p = q = *pargv++;
      while (*p == ' ' || *p == '\t') {
        p++;
        q++;
      }
      while (*p != ' ' && *p != '\t' && *p) {
        p++;
        if (*p == '\0') {
          *pnargv++ = xstrdup(q);
          nargc++;
        } else if (*p == ' ' || *p == '\t') {
          r = p;
          while (*p == ' ' || *p == '\t')
            p++;
          if (*p == '-' || *p == '\0') {
            *r = '\0';
            *pnargv++ = xstrdup(q);
            nargc++;
            q = p;
          }
        }
      }
    }

    nargv[nargc] = NULL;
    argv = nargv;
    argc = nargc;
  }
#endif


   

#ifdef SIGSEGV
  (void) signal(SIGSEGV,sig_err);
#endif
  (void) signal(SIGFPE,sig_err);
#ifdef KWIN32
  
SetConsoleCtrlHandler((PHANDLER_ROUTINE)sigint_handler, TRUE);


#else
#ifdef SIGINT
  (void) signal(SIGINT,sig_err);    
#endif
#endif


   
(IGNORED) printf("----------------------------\n");
#ifndef KPATHSEA
#ifdef PLATFORM
   (IGNORED) printf("t4ht.c (2012-07-25-19:28 %s)\n",PLATFORM);
#else
   (IGNORED) printf("t4ht.c (2012-07-25-19:28)\n");
#endif
#else
#ifdef PLATFORM
#  if defined(MIKTEX)
   (IGNORED) printf("t4ht.c (2012-07-25-19:28 %s MiKTeX)\n",PLATFORM);
#  else
   (IGNORED) printf("t4ht.c (2012-07-25-19:28 %s kpathsea)\n",PLATFORM);
#  endif
#else
#  if defined(MIKTEX)
   (IGNORED) printf("t4ht.c (2012-07-25-19:28 MiKTeX)\n");
#  else
   (IGNORED) printf("t4ht.c (2012-07-25-19:28 kpathsea)\n");
#  endif
#endif
#endif


{ int i, count = 0;
 for(i=0; i<argc; i++){
   Q_CHAR *p = argv[i];
   count++;
   if( (*p == '\'') || (*p == '\"') ){
     int cnt;
     int len = 0;
     for( cnt=i; cnt < argc; cnt++ ){
       len += (int) strlen((char *) argv[cnt]);
       if( *(argv[cnt] + (int) strlen((char *) argv[cnt]) -1) == *p ){
           Q_CHAR * arg = m_alloc(char, len + cnt - i + 1);
           Q_CHAR * toArg = arg;
           Q_CHAR *pp;
           i--;
           do {
             pp = argv[++i];
             while( *pp != '\0' ){
                if( *pp != *p ){ *(toArg++) = *pp; }
                pp++;
             }
             *(toArg++) = ' ';
           } while ( i != cnt );
           *(toArg-1) = '\0';
           argv[count-1] = arg;
           break;
 } } } }
 argc = count;
}


{ int i;
  for(i=0; i<argc; i++){
    (IGNORED) printf("%s%s ", (i>1)?"\n  " : "", argv[i]); }
  (IGNORED) printf("\n");
}


   
HOME_DIR = getenv("HOME");


{                   C_CHAR   *yes = NULL;
  system_yes = (
#ifdef SYSTEM_FUNCTION_OK
0
#else
system( yes ) != 0
#endif

);
}


{          int i;
  for( i=0;  i<=9; i++){
    match[i] = (Q_CHAR *) malloc(70);
    max_match[i] = 70;
  }
}



  
{      int i;
       Q_CHAR *p, *q;
  
#ifdef KPATHSEA
   kpse_set_program_name (argv[0], "tex4ht");
#endif


  
  if( argc == 1 ){ bad_arg; }
  for(i=1; i<argc; i++){
    if( *( p=argv[i] ) == '-' ){ 
if( (int) strlen((char *)  argv[i] ) == 2 ){
   if( 
   (*(p+1) != 'i')
&& (*(p+1) != 'p')
&& (*(p+1) != 'r')
&& (*(p+1) != 'b')
&& (*(p+1) != 'g')
&& (*(p+1) != 'Q')

 )
     { if( ++i == argc ) bad_arg; }
   q = argv[i];
} else q = p+2;
switch( *(p+1) ){
  case 'M':{ ch_mod = q;  newchmod = TRUE; break; }
  case 'S':{ 
{     struct sys_call_rec *q;
  q = m_alloc(struct sys_call_rec, 1);
  q->next = system_calls;
  q->filter = p + 2;
  system_calls = q;
}

 break; }
  case 'X':{ Xfield = q;  break;}
  case 'b':{ bitmaps_no_dm = q-1;  break;}
  case 'c':{ 
struct env_c_rec *temp = (struct env_c_rec*)
                   m_alloc(struct env_c_rec, (int) 1);
temp->option = q;
temp->next = envChoice;
envChoice = temp;

  break;}
  case 'd':{ dir = (*q=='~')? abs_addr(q,NULL) : q;  break; }
  case 'e':{ 
tex4ht_env_file = q;

 break; }
  case 'f':{ 
p = p + 2;
lg_name = p + (int) strlen((char *)  p );
while( *lg_name != *p ){ lg_name--; }
lg_name++;

 break; }
  case 'i':{ debug = q-1;  break;}
  case 'g':{ always_call_sys = TRUE;  break;}
  case 'm':{ ch_mod = q;  break; }
  case 'p':{ nopict = q-1;  break;}
  case 'Q':{ check_tex4ht_c_err = TRUE;  break;}
  case 'r':{ noreuse = q-1;  break;}
  case '.':{ Dotfield = q;  break;}
   default:{ bad_arg;  }
}

 }
    else
    {  lg_name = argv[i];  }
  }
  if( lg_name == NULL ){ bad_arg; }
  
{                                 Q_CHAR   *p, *q;
                                  FILE*    file;
   (IGNORED) strcpy((char *) (char *) tmp_name, (char *) (char *) lg_name);
   p = q = tmp_name + strlen((char *)  tmp_name );
   while( p != tmp_name ){
      if( *p == '.' ){
          if( eq_str( p+1,LG_EXT ) ) { *p = '\0'; }
          else {
            (IGNORED) strct( tmp_name, LG_EXT);
            file =  fopen( tmp_name, READ_TEXT_FLAGS );
            if( file ){
              (IGNORED) fclose(file); *q = '\0';
            } else {  *p = '\0'; }
          }
          break;
      }
      p--;
   }
   (IGNORED) strct(tmp_name, ".tmp");
}


  
(IGNORED) strcpy((char *)  job_name, (char *) tmp_name);
*(job_name + strlen((char *) job_name) - 4) = '\0';


}


   

{                                FILE* file;
                                 U_CHAR  env_loc[512];
   env_loc[0] = '\0';
   
{         Q_CHAR  str[512],
                                                      *TEX4HTENV;
   
if( tex4ht_env_file ){
   if( debug ){
      (IGNORED) printf("%s?\n", tex4ht_env_file);
   }
   file = f_home_open( tex4ht_env_file, READ_TEXT_FLAGS );
} else {
   file = NULL;
}
if( tex4ht_env_file ){
    (IGNORED) strcpy((char *) &env_loc[0], (char *) tex4ht_env_file);
}
if( debug && file ){
      (IGNORED) printf(".......Open: %s\n", tex4ht_env_file); }


   
if( !file ) {
   if( debug ){
      (IGNORED) printf("tex4ht.env?\n");
   }
   file = f_open("tex4ht.env", READ_TEXT_FLAGS);
   (IGNORED) strcpy((char *) &env_loc[0], "tex4ht.env");
   if( debug && file ){
      (IGNORED) printf(".......Open: ./tex4ht.env\n"); }
}


   
#ifndef DOS_WIN32
   if( !file ) {
       if( debug ){
          (IGNORED) printf(".tex4ht?\n");
       }
       file = f_open(".tex4ht", READ_TEXT_FLAGS);
       (IGNORED) strcpy((char *) &env_loc[0], ".tex4ht");
       if( debug && file ){
         (IGNORED) printf(".......Open: ./.tex4ht\n"); }
   }
#endif


   
if( !file ){
  TEX4HTENV = getenv("TEX4HTENV");
  if( TEX4HTENV ){
     if( debug ){
         (IGNORED) printf("%s?\n", TEX4HTENV);
     }
     file = f_home_open(TEX4HTENV,READ_TEXT_FLAGS);
  }
}


   
if( !file ){
  if( HOME_DIR ){ (IGNORED) sprintf(str,
#if defined(DOS_WIN32) || defined(__DJGPP__)
  is_forward_slash(HOME_DIR)?  "%s/tex4ht.env" :  "%s\\tex4ht.env"
#else
  "%s/tex4ht.env"
#endif

, HOME_DIR);
     if( debug ){
         (IGNORED) printf("%s?\n", str);
     }
     file = f_open(str,READ_TEXT_FLAGS);
     (IGNORED) strcpy((char *) &env_loc[0], (char *) str);
     if( debug && file ){
        (IGNORED) printf(".......Open: %s\n", str); }
  }
}
#ifndef DOS_WIN32
  if( !file ){
     if( HOME_DIR ){
          (IGNORED) sprintf(str,"%s/.tex4ht", HOME_DIR);
          if( debug ){
             (IGNORED) printf("%s?\n", str);
          }
          file = f_open(str,READ_TEXT_FLAGS);
          (IGNORED) strcpy((char *) &env_loc[0], (char *) str);
          if( debug && file ){
             (IGNORED) printf(".......Open: %s\n", str); }
      }
  }
#endif
#if defined(DOS_WIN32) || defined(__MSDOS__)
   if( !file ){
      if( debug ){
          (IGNORED) printf("C:/tex4ht.env?\n");
      }
      file = f_open("C:/tex4ht.env",READ_TEXT_FLAGS);
       (IGNORED) strcpy((char *) &env_loc[0], "C:/tex4ht.env");
      if( debug && file ){
        (IGNORED) printf(".......Open: C:/tex4ht.env\n"); }
   }
#endif


   
#ifdef ENVFILE
   if( !file ) {
      if( debug ){
          (IGNORED) printf("%s?\n", ENVFILE);
      }
      file = f_home_open( ENVFILE,READ_TEXT_FLAGS);
       (IGNORED) strcpy((char *) &env_loc[0], (char *) ENVFILE);
       if( debug && file ){
         (IGNORED) printf(".......Open: %s\n", ENVFILE); }
   }
#endif


   if( !file ) { 
if( dos_env_file ){
   if( debug ){
       (IGNORED) printf("%s?\n", dos_env_file);
   }
   file =  f_open( dos_env_file, READ_TEXT_FLAGS ) ;
   (IGNORED) strcpy((char *) &env_loc[0], (char *) dos_env_file);
   if( debug && file ){
      (IGNORED) printf(".......Open: %s\n", dos_env_file); }
}

 }
   

#ifdef KPATHSEA
if( !file )  {               U_CHAR * envfile;
                             char *arch, *p, str[256];
   
p = arch = (char *) kpse_var_value( "SELFAUTOLOC" );
while( *p != '\0' ){
   if( (*p ==   '/') || (*p == '\\') ){
      arch = p;
   }
   p++;
}


   envfile = (char *) 0;
   
if( arch ){
  (IGNORED) sprintf(str,"%s%ctex4ht.env", arch+1, *arch);
  if( debug ){
    (IGNORED) printf(
      "kpse_open_file (\"%s\", kpse_program_text_format)?\n", str );
  }
  envfile= kpse_find_file (str, kpse_program_text_format, 0);
}


   if ( !envfile ){ 
if( debug ){
  (IGNORED) printf(
    "kpse_open_file (\"tex4ht.env\", kpse_program_text_format)?\n");
}
envfile= kpse_find_file ("tex4ht.env", kpse_program_text_format, 0);

 }
   if ( !envfile ){ 
if( system("kpsewhich --progname=tex4ht tex4ht.env > tex4ht.tmp") == 0 ){
   
char fileaddr [256];
int loc = 0;
FILE* file =  f_open("tex4ht.tmp", READ_TEXT_FLAGS);
if( file ){
  while( (fileaddr[loc] = getc(file)) >=0  ){
    if( fileaddr[loc] == '\n' ){ fileaddr[loc] = '\0'; break; }
    loc++;
  }
  (IGNORED) fclose(file);
}


   envfile= kpse_find_file (fileaddr, kpse_program_text_format, 0);
}

 }
   if ( envfile ){
      file = kpse_open_file (envfile, kpse_program_text_format);
      (IGNORED) printf("(%s)\n",  envfile);
   }
   if( debug && file ){
      (IGNORED) printf(".......Open kpathsea %s\n", envfile);
   }
}
if( debug ){
                       U_CHAR *p, *q;
  
p = (U_CHAR *) kpse_var_value( "T4HTINPUTS" );
if( p ){
   (IGNORED) printf("T4HTINPUTS = %s\n", p);
}
q = getenv("T4HTINPUTS");
if( q ){  (IGNORED) printf(
   "Environmet var T4HTINPUTS:  %s\n", q);
}
if( !p && !q ){
   (IGNORED) printf( "Missing T4HTINPUTS for kpathsea\n" );
}


}
#endif


   if( !file ) warn_i_str( 5, 
#ifdef  DOS_WIN32
   "tex4ht.env"
#endif
#ifndef  DOS_WIN32
   "tex4ht.env | .tex4ht"
#endif

);
}


   if( file ){
      
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {                Q_CHAR ch;
  ch = (char) (eoln_ch = getc(file));
  if( eoln_ch != (int) '\n' ){
     status = scan_until_end_str("", 1, TRUE, file);
     if( status ){
       switch( ch ){
         case 'A':{ 
if( debug ){
      (IGNORED) printf(".......'A' script\n"); }
chmod_script = add_script(chmod_script);

  break;}
         case 'C':{ 
if( debug ){
      (IGNORED) printf(".......'C' script\n"); }
copy_script = add_script(copy_script);

  break;}
         case 'E':{ 
if( debug ){
      (IGNORED) printf(".......'E' script\n"); }
empty_fig_script = add_script(empty_fig_script);

  break;}
         case 'F':{ 
if( debug ){
      (IGNORED) printf(".......'F' script\n"); }
dvigif_glyp_script = add_script(dvigif_glyp_script);

   break;}
         case 'G':{ 
if( debug ){ (IGNORED) printf(".......'G' script\n"); }
dvigif_script = add_script(dvigif_script);

   break;}
         case 'M':{ 
if( debug ){
      (IGNORED) printf(".......'M' script\n"); }
move_script = add_script(move_script);

  break;}
         case 'S':{ 
{     struct sys_call_rec *q;
  q = m_alloc(struct sys_call_rec, 1);
  q->next = system_calls;
  q->filter = match[1];
              match[1] = (Q_CHAR *) malloc(70);  max_match[1] = 70;
  system_calls = q;
if( debug ){
   (IGNORED) printf(".......'S' script: '%s'\n",
                                q->filter); }
}

  break;}
         case 'X':{ 
if( debug ){
      (IGNORED) printf(".......'X' script\n"); }

file_script

 = add_script(
file_script

);

  break;}
         case '.':{ 
if( debug ){
      (IGNORED) printf(".......'.' script\n"); }

ext_script

 = add_script(
ext_script

);

  break;}
         case '<':{ 
if( *(match[1]) != '/' ){
                         U_CHAR *p;
                         BOOL env_skip;
  for( p = match[1];
      (*p != '>') && (*p != '\n') && (*p != EOF ) ;
      p++ ){}
  if( *p == '>' ){ *p = '\0'; }
  
if( envChoice == (struct env_c_rec*) 0  ){
  env_skip = !eq_str(match[1], "default" );
} else {
               struct env_c_rec *p;
  env_skip = TRUE;
  for( p=envChoice; p!=(struct env_c_rec*) 0 ; p = p->next ){
    if( eq_str(match[1], p->option ) ){ env_skip = FALSE; }
} }


  if( env_skip ){
     
                         U_CHAR cur_block[90];
if( debug ){
   (IGNORED) printf(".......   <%s>  skipping ...\n", match[1]);
}
(IGNORED) strcpy((char *)  cur_block, (char *) match[1]);
status = FALSE;
while( !status && (eoln_ch != EOF) ){
  status = scan_str("</", TRUE, file);
  status = scan_until_str(">", 1, status, file);
  status = scan_until_end_str("", 2, status, file);
  if( status ){
      *(match[1] + strlen((char *) match[1]) - 1) = '\0';
      status = eq_str(match[1], cur_block);
      if( debug ){
         (IGNORED) printf(".......   </%s>\n", match[1]);
} }   }


  } else {
     if( debug ){
        (IGNORED) printf(".......   <%s>\n", match[1]);
     }
  }
} else if( debug ){
   (IGNORED) printf(".......   <%s\n", match[1]);
}

  break;}
         default:{ }
} }  }  }


      (IGNORED) fclose(file);
}  }


lg_file = open_file(lg_name,LG_EXT);
if( lg_file ) {
   
begin_lg_file = ftell(lg_file);


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
if( check_tex4ht_c_err ){
   eoln_ch = (int) 'x';
   while( eoln_ch != EOF ) {
      status = scan_str("tex4ht.c error: ", TRUE, lg_file);
      if( status ){
          (IGNORED) fprintf(stderr,
              "--- Error --- tex4ht.c ran into problems\n"
           );
           exit(EXIT_FAILURE);
      }
      status = scan_until_end_str("", 1, status, lg_file);
}  }


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
{     
Q_CHAR *file_name, file_mode[5];
int i, start_loc, end_loc, addr = 0;
char rec_op, *ch;
static struct files_rec *to_rec, *from_rec,
   *opened_files = (struct files_rec *) 0,
   *p, *p1, *p2, *p3, *p4, *from_op;
FILE *in_file, *out_file;
BOOL write_on;


   eoln_ch = (int) 'x';
   while( eoln_ch != EOF ) {
      status = scan_str("CopyTo: ", TRUE, lg_file);
      status = scan_until_str(" ", 1, status, lg_file);
      status = scan_until_str(" ", 2, status, lg_file);
      status = scan_until_str(" ", 3, status, lg_file);
      status = scan_until_str(" ", 4, status, lg_file);
      status = scan_until_end_str("", 5, status, lg_file);
      if( status ){
         if( debug ){
            (IGNORED) printf("...CopyTo: %s%s%s%s%s...\n",
                     match[1], match[2], match[3], match[4], match[5]);
         }
         rec_op = eq_str(match[2],"From ")?  From_op :
                  ( eq_str(match[2],"Until ")? Until_op :
                    ( eq_str(match[2],"Skip ")?  Skip_op :
                      ( eq_str(match[2],"Cont ")?  Cont_op : No_op )));
         if( rec_op == No_op ){ 
ch = match[2];
if( (ch[0]=='S') && (ch[1]=='e') && (ch[2]=='t') ){
  ch += 3; rec_op = Set_op;
} else if( (ch[0]=='A') && (ch[1]=='d') && (ch[2]=='d') && (ch[3]=='r') ){
  ch += 4; rec_op = Addr_op;
}
if( rec_op != No_op ){
  addr = 0; while( (*ch>='0') && (*ch<='9') ){
    addr = addr*10 + *ch - '0'; ch++;
} }

 }
         if( rec_op == No_op ){
            (IGNORED) fprintf(stderr,"--- warning --- ");
            (IGNORED) fprintf(stderr,"CopyTo: %s%s%s%s%s?\n",
                      match[1], match[2], match[3], match[4], match[5]);
         } else {
            
file_name = match[1];
*(file_name + (int) strlen((char *) file_name) - 1) = '\0';
strcpy((char *) file_mode, WRITE_TEXT_FLAGS);
for(i=1; i<=2; i++){
  
for( p = opened_files; p != (struct files_rec*) 0;  p = p->right ){
   if( eq_str(file_name,p->name) ) { break; }
}
if( p == (struct files_rec*) 0 ){
  p = m_alloc(struct files_rec, 1);
  p->right = opened_files;   opened_files = p;
  p->down =  (struct files_rec*) 0;
  strcpy((char *) p->file_mode, (char *) file_mode);
  p->name = m_alloc(char, (int) strlen((char *) file_name) + 1);
  (IGNORED) strcpy((char *)  p->name, (char *) file_name );
  if( (p->file = fopen(file_name, file_mode)) == NULL )
    { (IGNORED) warn_i_str(5,file_name); }
}
to_rec = from_rec;  from_rec = p;


  file_name = match[5];
  strcpy((char *) file_mode, 
READ_BIN_FLAGS

);
}


            
if( to_rec->down == (struct files_rec*) 0 ){
  
to_rec->down = p = m_alloc(struct files_rec, 1);
p->up = to_rec;
p->right = p->left = p->down = (struct files_rec*) 0;
p->name = to_rec->name;
p->file = to_rec->file;
p->from_rec = from_rec;
p->loc = -1;
p->op = No_op;
to_rec = p;


} else {
  to_rec = to_rec->down;
  for( p = to_rec->right; p != (struct files_rec*) 0;  p = p->right ){
    if( to_rec->from_rec == from_rec ){ break; }
    to_rec = p;
  }
  if( to_rec->from_rec != from_rec ){
    
to_rec->right = p = m_alloc(struct files_rec, 1);
p->left = to_rec;
p->right = p->down = (struct files_rec*) 0;
p->up = to_rec->up;
p->name = to_rec->name;
p->file = to_rec->file;
p->from_rec = from_rec;
p->loc = -1;
to_rec = p;


} }


            
if( to_rec->down == (struct files_rec*) 0 ){
  
to_rec->down = p = m_alloc(struct files_rec, 1);
p->up = to_rec;
p->right = p->left = p->down = (struct files_rec*) 0;
p->name = to_rec->name;
p->file = to_rec->file;
p->from_rec = from_rec;
p->loc = -1;
p->op = No_op;
p->group = m_alloc(char, (int) strlen((char *) match[3]) + 1);
(IGNORED) strcpy((char *)  p->group, (char *) match[3] );
to_rec = p;


} else {
  to_rec = to_rec->down;
  for( p = to_rec->right; p != (struct files_rec*) 0;  p = p->right ){
    if( eq_str(to_rec->group,match[3]) ){ break; }
    to_rec = p;
  }
  if( !eq_str(to_rec->group,match[3]) ){
    
to_rec->right = p = m_alloc(struct files_rec, 1);
p->left = to_rec;
p->right = p->down = (struct files_rec*) 0;
p->up = to_rec->up;
p->name = to_rec->name;
p->file = to_rec->file;
p->from_rec = from_rec;
p->loc = -1;
p->group = m_alloc(char, (int) strlen((char *) match[3]) + 1);
(IGNORED) strcpy((char *)  p->group, (char *) match[3] );
to_rec = p;


} }


            
p = m_alloc(struct files_rec, 1);
p->down = to_rec->down;  to_rec->down = p;
p->up = to_rec;
if( p->down != (struct files_rec*) 0 ){
  (p->down)->up = p;
}
*(match[4] + (int) strlen((char *) match[4]) - 1) = '\0';
p->loc = (int) get_long_int(match[4]);
p->op  = rec_op;
p->label = addr;


            
if( rec_op == Until_op ){
  for( p = to_rec->down;
           p != (struct files_rec*) 0;  p = p->down ){
    if( p->op == From_op ){ from_op = p; break; }
  }
  if( p == (struct files_rec*) 0 ){
    
(IGNORED) fprintf(stderr,"%sMissing `CopyTo From':\n", "--- warning --- ");
for( p = to_rec->down; p != (struct files_rec*) 0;  p = p->down ){
  (IGNORED) fprintf(stderr,"   %s %s%d %s\n", to_rec->name,
          p->op == From_op ?  "From  " :
             ( p->op == Until_op ?   "Until " :
               ( p->op == Skip_op ?   "Skip  " :
                 ( p->op == Cont_op?   "Cont " :
                 ( p->op == Addr_op?   "Addr " :
                 ( p->op == Set_op?   "Set " : "No_op " ))))),
          p->loc, from_rec->name
         );
}


  } else {
    
in_file = from_rec->file;
out_file = to_rec->file;
start_loc = from_op->loc;
write_on = TRUE;
(IGNORED) fseek(in_file, (long) start_loc, 
0
);
for( p= from_op;  p != to_rec; p = p->up ){
  switch( p->op ){
    case Until_op:{ 
if( write_on ){
   end_loc = p->loc;
   for(; start_loc<end_loc; start_loc++) {
      (IGNORED) putc( getc(in_file), out_file );
}  }

 break; }
    case  Skip_op:{ 
if( write_on ){
   end_loc = p->loc;
   for(; start_loc<end_loc; start_loc++) {
      (IGNORED) putc( getc(in_file), out_file );
   }
}
write_on = FALSE;

  break; }
    case  Cont_op:{ 
end_loc = p->loc;
if( write_on ){
   for(; start_loc<end_loc; start_loc++) {
      (IGNORED) putc( getc(in_file), out_file );
   }
} else {
   start_loc = end_loc;
   (IGNORED) fseek(in_file, (long) end_loc, 
0
);
}
write_on = TRUE;

  break; }
    case   Set_op:{ 
addr = -1;
for( p1 = from_op->up; p1 != to_rec;  p1 = p1->up ){
  if( (p1->op == Addr_op) && (p1->label == p->label) ){
    addr = p1->loc; break;
} }
if( addr != -1 ){ (p->up)->loc = addr; }

   break; }
    default: {  }
} }


    
to_rec->down = from_op->down;
if( from_op->down != (struct files_rec*) 0){
    (from_op->down)->up = to_rec;
}
for( p = from_op; p != to_rec; ){
  p1 = p;  p = p->up; free((void *)  p1);
}
if( to_rec->down == (struct files_rec*) 0){
  if( to_rec->left == (struct files_rec*) 0 ){
    (to_rec->up)->down =  to_rec->right;
  } else {
    (to_rec->left)->right = to_rec->right;
  }
  if( to_rec->right != (struct files_rec*) 0 ){
    (to_rec->right)->left = to_rec->left;
  }
  p1 = to_rec; to_rec = to_rec->up;  free((void *)  p1);
}


  }
}


   }  }  }
   
for( p = opened_files; p != (struct files_rec*) 0; ){
   (IGNORED) fclose(p->file);
   free((void *)  p->name);
   
for( p1 = p->down; p1 != (struct files_rec*) 0; ){
   
for( p2 = p1->down; p2 != (struct files_rec*) 0; ){
   
for( p3 = p2->down; p3 != (struct files_rec*) 0; ){
  p4 = p3; p3 = p3->down; free((void *)  p4);
}


   p3 = p2; p2 = p2->right;  free((void *)  p3);
}


   p2 = p1; p1 = p1->right;  free((void *)  p2);
}


   p1= p;  p = p->right;    free((void *)  p1);
}


}


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
{                     struct htf_struct *last_rec, *p;
   last_rec = (struct htf_struct *) 0;
   eoln_ch = (int) 'x';
   while( eoln_ch != EOF ) {
      status = scan_str("htfcss: ", TRUE, lg_file);
      status = scan_until_end_str("", 1, status, lg_file);
      
if( status ){
            Q_CHAR *s, *t;
            int i;
   s = t = match[1];
   for( i=0; *s == ' '; s++ ) { i++; }
   if( i>0 ){
      while( *s != 0 ){
         *(t++) = *(s++);
      }
      *t = 0;
}  }


      if( status ){            Q_CHAR *key, *body, *media;
         media = body = key = match[1];
         
while( *body && (*body != ' ') ){ body++; }
if( *body == ' ' ){ media = body; *(body++) = '\0'; }

if( (int) strlen((char *) body) > 6 ){
   if(     (*(body) == '@')
       &&  (*(body+1) == 'm')
       &&  (*(body+2) == 'e')
       &&  (*(body+3) == 'd')
       &&  (*(body+4) == 'i')
       &&  (*(body+5) == 'a') )
     {
    body += 6;
    while( *body == ' ' ){ body++; }
    media = body;
    while( (*body != ' ') && (*body != '\0') ){ body++; }
    if( *body == ' ' ){ *(body++) = '\0'; }
    while( *body == ' ' ){ body++; }
}  }


if( *body ){
  if( *key ){ 
p = m_alloc(struct htf_struct, 1);
p->next =  (struct htf_struct *) 0;
p->key =   m_alloc(char, (int) strlen((char *) key) + 1);
(IGNORED) strcpy((char *)  p->key, (char *) key );
p->media =   m_alloc(char, (int) strlen((char *) media) + 1);
(IGNORED) strcpy((char *)  p->media, (char *) media );
p->body =   m_alloc(char, (int) strlen((char *) body) + 1);
(IGNORED) strcpy((char *)  p->body, (char *) body );
if( last_rec ){
   last_rec->next = p;  last_rec = p;
} else {
   htf_rec = last_rec = p;
}
if( debug ){
   (IGNORED) printf(".......%s...%s...%s\n", key, media, body);
}

 }
#if 0 /* unreachable */
  else if( last_rec ){ 
last_rec->body = (Q_CHAR *)  r_alloc((void *) last_rec->body,
      (size_t) strlen((char *) last_rec->body)
    + (size_t) strlen((char *) body)
    + 2);
(IGNORED) strct(last_rec->body,"\n");
(IGNORED) strct(last_rec->body,body);
if( debug ){
   (IGNORED) printf(".......%s\n", body);
}

 }
#endif /* 0, unreachable */
}


   }  }
}


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
   status = scan_str("Font_Css_Plus ", TRUE, lg_file);
   status = scan_until_str(" ", 1, status, lg_file);
   status = scan_until_end_str("", 2, status, lg_file);
   if( status ){                          Q_CHAR *key;
      key = match[1];
      *(key + (int) strlen((char *) key) - 1) = '\0';
      
if( (*key != '\0') && (*key != '\n') ) {
                            struct htf_struct *p;
   p = m_alloc(struct htf_struct, 1);
   p->next =  htf_rec;
   htf_rec = p;
   p->key =   m_alloc(char, (int) strlen((char *) key) + 1);
   (IGNORED) strcpy((char *)  p->key, (char *) key );
   p->body =   m_alloc(char, (int) strlen((char *) match[2]) + 1);
   (IGNORED) strcpy((char *)  p->body, (char *) match[2] );
   if( debug ){
      (IGNORED) printf(".......%s...%s\n", key, match[2]);
} }


} }


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
{                             struct empty_pic_struct *last;
   last = empty_pic = (struct empty_pic_struct *) 0;
   while( TRUE ) {
      status = scan_str("--- empty picture --- ", TRUE, lg_file);
      status = scan_until_str( ".idv[" , 1, status, lg_file);
      status = scan_until_end_str("] ---", 1, status, lg_file);
      if( status ){ 
if( last == (struct empty_pic_struct *) 0 ){
   last = empty_pic = (struct empty_pic_struct *)
                   m_alloc(struct empty_pic_struct, (int) 1);
} else {
   last = last->next = (struct empty_pic_struct *)
                   m_alloc(struct empty_pic_struct, (int) 1);
}
last->next = (struct empty_pic_struct *) 0;
last->n = get_long_int(match[1]);

 }
      if ( eoln_ch == EOF ){ break; }
   }
   
if( last == (struct empty_pic_struct *) 0 ){
   last = empty_pic = (struct empty_pic_struct *)
                   m_alloc(struct empty_pic_struct, (int) 1);
} else {
   last = last->next = (struct empty_pic_struct *)
                   m_alloc(struct empty_pic_struct, (int) 1);
}
last->next = (struct empty_pic_struct *) 0;
last->n = get_long_int(match[1]);


   last->next = (struct empty_pic_struct *) 0;
   last->n = 100000;
}


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
{                               BOOL characters, skip;
   characters = skip = FALSE;
   while( TRUE ) {
      status = scan_until_str("--- ", 1, TRUE, lg_file);
      status = scan_until_str( " ---" , 2, status, lg_file);
      if( status ) {
        if( eq_str(match[1],"--- ") ){
           if( eq_str(match[2],"needs ---") ){
                
status = scan_until_str(" ", 1, status, lg_file);
status = scan_until_str(".idv", 1, status, lg_file);
status = scan_until_str("[", 2, status, lg_file);
status = scan_until_str("] ==> ", 2, status, lg_file);
status = scan_until_str(" ", 3, status, lg_file);
   *(match[3] + (int) strlen((char *) match[3]) - 1) = '\0';
status = scan_until_end_str("---", 4, status, lg_file);
if( status ) {
               
struct script_struct
    * filtered_dvigif_script = NULL_SCRIPT;


               long int gif_i;
               Q_CHAR *p;
   gif_i = get_long_int(match[2]);
   p = match[2];
   *(p + (int) strlen((char *) p) - 6) = '\0';
   if( characters ){
      
                  Q_CHAR filename[255];
                  FILE* file;
(IGNORED) strcpy((char *) filename, "");
if( dir && !bitmaps_no_dm ){ (IGNORED) strct(filename, dir); }
(IGNORED) strct(filename, match[3]);
file  = fopen(filename, READ_TEXT_FLAGS);
if( !file || noreuse ){
   
filtered_dvigif_script = dvigif_glyp_script?
   filterGifScript(dvigif_glyp_script, match[3]):
   filterGifScript(dvigif_script, match[3]);
(void) execute_script(
    filtered_dvigif_script,match[1],match[2],match[3],job_name);
(void) free_script( filtered_dvigif_script );
if( dir && !bitmaps_no_dm && !system_return ){
  (void) execute_script(move_script,match[3],dir,".","");

if( ch_mod && !bitmaps_no_dm && !system_return ){
  (void) execute_script(chmod_script, ch_mod, dir?dir:"",match[3], "");
}


}


} else {
   (IGNORED) fclose(file);
   if( newchmod )
   { 
if( ch_mod && !bitmaps_no_dm && !system_return ){
  (void) execute_script(chmod_script, ch_mod, dir?dir:"",match[3], "");
}

 }
   (IGNORED) printf("%s already in %s\n", match[3],
                           dir? dir : "current directory" );
}


   } else { 
if( gif_i == 
empty_pic->n

 ) {
  
if( !skip ){
   (void) execute_script(empty_fig_script,
                           (dir && !bitmaps_no_dm )? dir :"", match[3],"","");
   if( ch_mod && !bitmaps_no_dm && !system_return ){
     (void) execute_script(chmod_script, ch_mod,
                           dir?dir:"",match[3], "");
   }
}
empty_pic = empty_pic->next;


} else { 
if( !nopict && !skip ){
   
filtered_dvigif_script = filterGifScript(dvigif_script, match[3]);
(void) execute_script(
  filtered_dvigif_script,match[1],match[2],match[3],job_name);
(void) free_script( filtered_dvigif_script );
if( dir && !bitmaps_no_dm && !system_return ){
  (void) execute_script(move_script,match[3],dir,".","");
}
if( ch_mod && !bitmaps_no_dm && !system_return ){
  (void) execute_script(chmod_script, ch_mod, dir?dir:"",match[3], "");
}


}

 }

 }
}


           } else if( eq_str(match[2],"characters ---") ){
              
status = scan_until_end_str("", 1, status, lg_file);
characters = eq_str(match[1],"");


           } else { 
status = scan_until_end_str("", 1, status, lg_file);

 }
        } else { 
status = scan_until_end_str(" ignore ---", 2, status, lg_file);
if( status ){
  skip =  eq_str(match[2]," ignore ---") ? TRUE :
            ( eq_str(match[2]," end ignore ---") ? FALSE : skip );
}

 }
      }
      if ( eoln_ch == EOF ){ break; }
}  }


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
{
                               Q_CHAR   css_name[255];
                               FILE   *css_file, *tmp_file;
                               BOOL   css_sty;
   
(IGNORED) strcpy((char *)  css_name, (char *) job_name);
(IGNORED) strct(css_name, ".css");
css_file = fopen(css_name, READ_TEXT_FLAGS);


   if( css_file ){
      (IGNORED) printf ("Entering %s\n", css_name);
      tmp_file = fopen(tmp_name, WRITE_TEXT_FLAGS);
      if( !tmp_file ) {
         (IGNORED) warn_i_str(5,tmp_name);
      } else { 
                   int ch;
while( (ch = getc(css_file)) != EOF ) {
  (IGNORED) putc( ch, tmp_file );
}
(IGNORED) fclose(tmp_file);
(IGNORED) fclose(css_file);

 }
      tmp_file = open_file(tmp_name, ".tmp");
      css_file = fopen(css_name, WRITE_TEXT_FLAGS);
      if( !tmp_file ) {
         (IGNORED) warn_i_str(5,tmp_name);
      } else if( !css_file ) {
         (IGNORED) warn_i_str(5,css_name);
      } else {
        
css_sty = FALSE;
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
  status = scan_until_end_str("", 1, TRUE, tmp_file);
  
{                          Q_CHAR *p, *q;
                           int  n;
  n = 0;  p = match[1];    q = match[2];
  while ( (*p != '\0') ){
    if (n == 13) { *(q-10) = '\0';  break;}
    if( *p != ' ' ){ *(q++) = *p; n++; }
    p++;
  }
  *q = '\0';
}


  if( eq_str(match[2], "/*css.sty*/") ){ css_sty = TRUE;  break; }
  (IGNORED) fprintf(css_file, "%s\n", match[1]);
}


        if( css_sty ){
           
(IGNORED) fprintf(css_file, "/* start css.sty */\n");

(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
   status = scan_str("Font_css_base: ", TRUE, lg_file);
   status = scan_until_str("Font_css_mag: ", 1, status, lg_file);
   status = scan_until_end_str("", 2, status, lg_file);
   if( status ){
      Font_css_base = match[1];
      *(Font_css_base + (int) strlen((char *) Font_css_base) - 14) = '\0';
      match[1] = (Q_CHAR *) malloc(70);  max_match[1] = 70;
      Font_css_mag = match[2];
      match[2] = (Q_CHAR *) malloc(70);  max_match[2] = 70;
} }



(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
   status = scan_str("Font_Size: ", TRUE, lg_file);
   status = scan_until_end_str("", 1, status, lg_file);
   if( status ){
      base_font_size = (int) get_long_int(match[1]);
} }



(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
   status = scan_str("Font(\"", TRUE, lg_file);
   status = scan_until_str("\",\"", 1, status, lg_file);
   status = scan_until_str("\",\"", 2, status, lg_file);
   status = scan_until_str("\",\"", 3, status, lg_file);
   status = scan_until_end_str("\")", 4, status, lg_file);
   if( status ){
                      Q_CHAR *p;
                      struct htf_struct *font_sty;
                      int second;
      p = match[1];
      *(p + (int) strlen((char *) p) - 3) = '\0';
      p = match[2];
      *(p + (int) strlen((char *) p) - 3) = '\0';
      p = match[3];
      *(p + (int) strlen((char *) p) - 3) = '\0';
      p = match[4];
      *(p + (int) strlen((char *) p) - 2) = '\0';
      
{                                                  Q_CHAR *p;
   second =   (int)
              (  (int) get_long_int(match[3])
               * (int) get_long_int(match[4])
               / base_font_size
              );
   while( second > 700 ){  second /= 10; }
   p = match[3];
   while( *p != '\0' ){
     if( (*p < '0') || (*p > '9') ){ second = 100; break; }
     p++;
   }
   
if(  (int) (  (double) get_long_int(match[2])
            / (int) get_long_int(match[4])
            + 0.5
           )
      == base_font_size
){
   second = 100;
};


}


      
font_sty = htf_rec;
while ( font_sty  ) {
  if( eq_str(font_sty->key,match[1]) ){
      if( *(font_sty->media) == '\0'  ){
         
(IGNORED) fprintf(css_file,
   (Font_css_base == NULL)? ".%s-%s" : Font_css_base,
   match[1], match[2]);
if( !eq_str(match[4],"100") ){
   (IGNORED) fprintf(css_file,
   (Font_css_mag == NULL)? "x-x-%s" : Font_css_mag,
   match[4]);
}
(IGNORED) fprintf(css_file, "{");
if( (second < 98) || (second > 102) ){
   (IGNORED) fprintf(css_file, "font-size:%d%c;", second, '%');
}
if( font_sty  ) {
   (IGNORED) fprintf(css_file, "%s", font_sty->body);
}
(IGNORED) fprintf(css_file, "}\n");


         second = 100;
      } else {
         
(IGNORED) fprintf(css_file, "@media %s{", font_sty->media);
(IGNORED) fprintf(css_file,
   (Font_css_base == NULL)? ".%s-%s" : Font_css_base,
   match[1], match[2]);
if( !eq_str(match[4],"100") ){
   (IGNORED) fprintf(css_file,
   (Font_css_mag == NULL)? "x-x-%s" : Font_css_mag,
   match[4]);
}
(IGNORED) fprintf(css_file, "{");
if( font_sty  ) {
   (IGNORED) fprintf(css_file, "%s", font_sty->body);
}
(IGNORED) fprintf(css_file, "}}\n");


      }
  }
  font_sty = font_sty->next;
}
if( second != 100 ){ 
(IGNORED) fprintf(css_file,
   (Font_css_base == NULL)? ".%s-%s" : Font_css_base,
   match[1], match[2]);
if( !eq_str(match[4],"100") ){
   (IGNORED) fprintf(css_file,
   (Font_css_mag == NULL)? "x-x-%s" : Font_css_mag,
   match[4]);
}
(IGNORED) fprintf(css_file, "{");
if( (second < 98) || (second > 102) ){
   (IGNORED) fprintf(css_file, "font-size:%d%c;", second, '%');
}
if( font_sty  ) {
   (IGNORED) fprintf(css_file, "%s", font_sty->body);
}
(IGNORED) fprintf(css_file, "}\n");

 }


}  }



(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
{            char  *font_class [256];
             int last_class;
   
last_class = 0;
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
   status = scan_str("Font_Class(", TRUE, lg_file);
   status = scan_until_str(",\"", 1, status, lg_file);
   status = scan_until_str("\"): ", 1, status, lg_file);
   status = scan_until_end_str("", 2, status, lg_file);
   if( status ){
      font_class[last_class++] = match[1];
      match[1] = (Q_CHAR *) malloc(7);  max_match[1] = 7;
}  }

(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);




   eoln_ch = (int) 'x';
   while( eoln_ch != EOF ) {
      status = scan_str("Font_Css(\"", TRUE, lg_file);
      status = scan_until_str("\"): ", 1, status, lg_file);
      status = scan_until_end_str("", 2, status, lg_file);
      if( status ){            int i;
        for(i=0; i<last_class; i++){
          if( eq_str(match[1],font_class[i]) ){
             (IGNORED) fprintf(css_file, "%s\n", match[2]);
             break;
}  }  } } }



(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
   status = scan_str("Css: ", TRUE, lg_file);
   status = scan_until_end_str("", 1, status, lg_file);
   if( status ){
      (IGNORED) fprintf(css_file, "%s\n", match[1]);
} }


(IGNORED) fprintf(css_file, "/* end css.sty */\n");


           
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
  status = scan_until_end_str("", 1, TRUE, tmp_file);
  
{                          Q_CHAR *p, *q;
                           int  n;
  n = 0;  p = match[1];    q = match[2];
  while ( (*p != '\0') ){
    if (n == 13) { *(q-10) = '\0';  break;}
    if( *p != ' ' ){ *(q++) = *p; n++; }
    p++;
  }
  *q = '\0';
}


  if( !eq_str(match[2], "/*css.sty*/") ){
     (IGNORED) fprintf(css_file, "%s\n", match[1]);
  }
}


        }
      }
      (IGNORED) fclose(tmp_file);
      (IGNORED) fclose(css_file);
}  }


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
if( 
ext_script

 ){
  eoln_ch = (int) 'x';
  while( eoln_ch != EOF ) {
     status = scan_str("File: ", TRUE, lg_file);
     status = scan_until_str(".", 1, status, lg_file);
     status = scan_until_end_str("", 2, status, lg_file);
     if( status ){                  Q_CHAR *ext;
                      struct script_struct *cur_script;
        ext = match[1];
        (IGNORED) strct( ext, match[2] );
        ext += strlen((char *) ext);
        while( *(--ext) != '.' ){ ; }
        *ext = '\0'; ext++;
        
{
            struct script_struct *cur, *add, *temp;
            Q_CHAR extPlus[256];
  cur = 
ext_script

;
  add = cur_script = NULL_SCRIPT;
  while( cur ){
     
(IGNORED) strcpy((char *) extPlus, (char *) ext);
/*
   if ( envChoice ) {
      (IGNORED) strct ( extPlus, envChoice );
   }
*/
(IGNORED) strct ( extPlus, " " );


     if( strpre(extPlus, cur->command) ){
        
temp = (struct script_struct *)
            m_alloc(struct script_struct, (int) 1);
temp->command = m_alloc(char, (int) strlen((char *) cur->command) + 1);
temp->next =  NULL_SCRIPT;
(IGNORED) strcpy((char *) temp->command,
                 (char *) cur->command + (int) strlen((char *) extPlus) );


        if( cur_script == NULL_SCRIPT ){
           cur_script = add = temp;
        } else {
           add = add->next = temp;
     }  }
     cur = cur->next;
} }


        (void) execute_script(cur_script,
                 match[1], Dotfield? Dotfield : "", "", "");
        
while(  cur_script != (struct script_struct*) 0 ){
                          struct script_struct *temp;
  temp = cur_script; cur_script = cur_script->next; free((void *) temp);
}


} }  }


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
if( 
file_script

 ){
   eoln_ch = (int) 'x';
   while( eoln_ch != EOF ) {
      status = scan_str("File: ", TRUE, lg_file);
      status = scan_until_str(".", 1, status, lg_file);
      status = scan_until_end_str("", 2, status, lg_file);
      if( status ){                         Q_CHAR *p;
         p = match[1];
         (IGNORED) strct( p, match[2] );
         p += strlen((char *) p);
         while( *(--p) != '.' ){ ; }
         *p = '\0'; p++;
         (void) execute_script(file_script, match[1],
                             p, Xfield? Xfield : "", "");
}  }  }


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {
   status = scan_str("File: ", TRUE, lg_file);
   status = scan_until_end_str("", 1, status, lg_file);
   status = status && !eq_str(match[1],tmp_name);
   if( status ){
                    FILE* file;
     file = fopen(match[1], READ_TEXT_FLAGS);
     if( file ){
        (IGNORED) fclose(file);
     } else { status = FALSE; }
   }
   if( status ){
      if( dir ){
         (void) execute_script(copy_script, match[1],
                               dir? dir :"",".","");
      }
      if( ch_mod ){
         (void) execute_script(chmod_script, ch_mod,
                               dir? dir:"",match[1], "");
      }
}  }


   
(IGNORED)  fseek(lg_file, begin_lg_file, 
0
);

  
eoln_ch = (int) 'x';
while( eoln_ch != EOF ) {              Q_CHAR *command, ch;
                                       int n;
                                       struct sys_call_rec *p;
                                       BOOL flag;
   status = scan_str("l. ", TRUE, lg_file);
   status = scan_until_str(" --- needs --- \"", 1, status, lg_file);
   status = scan_until_str("\" ---", 2, status, lg_file);
   if( status ){
      command = match[2];
      *(command + (int) strlen((char *) command) - 5) = '\0';
      
flag = FALSE;
p = system_calls;
while( p ){
  if( (n = (int) strlen((char *) p->filter)) == 1 ) {
     if( *(p->filter) == '*' ){
         flag = TRUE; break;
     }
  }
  if( strlen((char *) command) >= (unsigned int) n ) {
      ch = command[n]; command[n] = '\0';
      flag = flag || eq_str(p->filter,command);
      command[n] = ch;
  }
  p = p->next;
}


      if( flag ){
        
(IGNORED) call_sys(command);


      } else { (IGNORED) printf(
          "No permission for system call: %s\n", command); }
}  }


   (IGNORED) fclose(lg_file);
}


   return 0;
}


