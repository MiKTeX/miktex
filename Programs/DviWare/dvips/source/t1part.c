/*
 *   T1part.c version 1.59 beta  Copyright (C)1994, 1996
 *   by Sergey Lesenko
 *   lesenko@mx.ihep.su
 *
 *   It is distributed with no warranty of any kind.
 *   You may modify and use this program. It can be included
 *   in any distribution, commercial or otherwise, so long as
 *   copyright notice be preserved on all copies.
 */
#include "dvips.h"
#include "t1part.h"
#ifndef SEEK_SET
#define SEEK_SET (0)
#endif

#ifdef BORLANDC
void huge *UniGetMem(size)
ub4 size;
{
    void huge *tmp;
    if ((tmp =(void huge*) farcalloc(1L, size)) == NULL)
    {
        fprintf(stderr,"Error allocating far memory\n");
        exit(1);
    }
    return tmp;
}
#else
#define  UniGetMem  getmem
#endif

#ifdef WIN32
/* CHAR is typedef'd by <windows.h> -- popineau@esemetz.ese-metz.fr.  */
#define CHAR CHARACTER
#endif

typedef struct Char
{
    unsigned char *name;
    int length;
    int num;
    int choose;
    struct Char *NextChar;
}
CHAR;

typedef struct String
{
    unsigned char *name;
    int num;
    struct String *NextStr;
}
STRING;

static STRING *FirstStr;
static STRING *RevStr;

static CHAR *FirstChar;
static CHAR *FirstCharA;
CHAR *FirstCharB;
static CHAR *FirstCharW;

int CharCount;
int GridCount;
int ind_ref;

static unsigned char CDeCrypt(unsigned char, unsigned int *);
static void CorrectGrid(void);
static void OutHEX(FILE *);
static int Afm(void);
static int LoadVector(int, CHAR *);
static int ChooseVect(CHAR *);
static void ErrorOfScan(int err);
static void NameOfProgram(void);

typedef struct
{
    int num[4];
    int select;
}
def_ref;

def_ref refer[10];

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif


#define PFA              1
#define PFB              2


#define NOTHING          0
#define FLG_ASCII        1
#define FLG_BINARY       2
#define FLG_EOF          3
#define FLG_ZERO_LINE    4

#define FLG_OUT_STR    (-1)
#define CHAR_NOT_DEF   (-1)
#define SEAC_NOT_END   (-2)

#define FLG_LOAD_BASE   (1)

#define STANDARD_ENC    (1)
#define SPECIAL_ENC     (2)
#define AFM_ENC         (4)


#define FLG_REENCODE    (4)


#define BYTES_PER_LINE  32

#ifndef KPATHSEA_TYPES_H
#define TRUE             1
#define FALSE            0
#endif /* not KPATHSEA_TYPES_H */

#define LENIV            0
#define SUBRS            1
#define CHARSTRINGS      2

#define ERR_FIRST_NUM  (-1)
#define ERR_SECOND_NUM (-2)
#define ERR_FIRST_TOK  (-3)
#define ERR_SECOND_TOK (-4)
#define ERR_STACK      (-5)
#define ERR_NUM_CHAR   (-6)
#define ERR_NAME_CHAR  (-7)

#define SUBR_STR         1
#define CHAR_STR        -1
#define CHAR_SEAC       -2
#define SKIP_ON_DUP      3

#define C1           52845
#define C2           22719
#define EDR          55665
#define EER          55665
#define CDR           4330

#define MAX_ESCAPE      33

#define HSTEM            1
#define VSTEM            3
#define VMOVETO          4
#define CHARS_RLINETO    5
#define HLINETO          6
#define VLINETO          7
#define RRCURVETO        8
#define CHARS_CLOSEPATH  9
#define CALLSUBR        10
#define RETURN          11
#define ESCAPE          12
#define HSBW            13
#define ENDCHAR         14
#define CHARS_RMOVETO   21
#define HMOVETO         22
#define VHCURVETO       30
#define HVCURVETO       31

#define DOTSECTION       0
#define VSTEM3           1
#define HSTEM3           2
#define SEAC             6
#define SBW              7
#define CHARS_DIV       12
#define CALLOTHERSUBR   16
#define POP             17
#define SETCURRENTPOINT 33

typetemp  BORLAND_HUGE   *temp;
typetemp  BORLAND_HUGE   *begin_of_scan;
typetemp  BORLAND_HUGE   *end_of_scan;
unsigned char *line;
unsigned char *tmpline;
unsigned char token[64];
unsigned char notdef[]="/.notdef";
unsigned char grid[256];
unsigned char tmpgrid[256];
unsigned char psfontfile[500]; /* really should dynamically allocate */
unsigned char psvectfile[500];
unsigned char basevect[]="ad.enc";
unsigned char version[] = "v1.59 beta (c)1994, 1996";
unsigned char tmp_token[64];
/* static int j = 0; */
static int stack[128];

ub1 buf[BUFSIZ];

int loadbase = 0;
static int encode;
static int reencode;

int find_encod;
int lastpart=0;
int keep_flg=0;
int keep_num=0;

int  number;
int  offset;
long value;
int  lenIV = 4;

static int grow=0;
static int level;
static int flg_seac=0;

typedef struct
{
    const char *command;
    int code;
}
tablecommand;

tablecommand TableCommand[] =
{
    {"callsubr", CALLSUBR}, {"callothersubr", CALLOTHERSUBR},
    {"pop", POP }, {"seac", SEAC}, {""}
};

typedef struct
{
    const char *extension;
    int num;
}
typefonts;


typefonts TypeFonts[] =
{
    {".pfa", PFA}, {".pfb", PFB},
    {".PFA", PFA}, {".PFB", PFB}, {""}
};

static char Dup[]="dup";

typedef struct
{
    typetemp  BORLAND_HUGE  *begin;
    int   length;
    int   type;
    int   offset;
    int   oldnum;
    int   newnum;
}
def_key;

def_key keyword[6];

int FirstKey, current;

int subrs, char_str;


typedef struct
{
    const char *name;
    int num;
}
type_key;


type_key Key[] =
{
    {"/LenIV", LENIV}, {"/Subrs", SUBRS}, {"/CharStrings", CHARSTRINGS},  {""}
};


struct def_label
{
    typetemp  BORLAND_HUGE   *begin;
    unsigned char skip;
    int length;
    short select;
    int num;
};

struct def_label label[NUM_LABEL];



static int
DefTypeFont(unsigned char *name)
{
    int i;

    for(i=0;*TypeFonts[i].extension;i++)
    {
        if(strstr((char *)name,TypeFonts[i].extension)!=0)
            return(TypeFonts[i].num);
    }
    return -1;
}

static int
GetZeroLine(unsigned char *str)
{
    int token_type=0;
    if(*str!='0')
    {
        return (token_type=0);
    }
    while(*str=='0')
    {
        str++;
    }

    if(*str=='\n' || *str=='\r')
        token_type= -1;
    else
        token_type=0;
    return(token_type);
}

/* We get token type and its content for ASCII code */

static int
GetWord(unsigned char *mem)
{
    int  token_type=0;
    register unsigned char *tmp;
    tmp=mem;
    *tmp= *line;
    while((*line!='\0')&&(*line!='%'))
    {
        if(*line=='-')
        {
            *tmp++= *line++;
        }

        if(isdigit(*line))
        {
            while(isdigit(*line))
            {
                *tmp++= *line++;
            }
            *tmp = '\0';
            return 5;
        }

        if(*line=='/')
        {
            *tmp++= *line++; token_type=1;
        }
        if(*line=='.')
        {
            *tmp++= *line++;

            if(token_type==1)
            {
                if(*line==' ')
                {
                    *tmp = '\0';
                    return(token_type=token_type+2);
                }
            }
        }
        if(isalpha(*line))
        {
            *tmp++ = *line++;
            while(!isspace(*line) && *line != '/')
                *tmp++= *line++;
            *tmp = '\0';
            return(token_type=token_type+2);
        }

        token_type=0;
        tmp=mem;
        line++;
    }
    return(token_type= -1);
}

/* We get token type and its content for BINARY code */

static int
GetToken(void)
{
    register unsigned char *tmp;
    int token_type=0;

    tmp=token;

    *tmp= *temp;
    while(temp<end_of_scan)
    {
        if(*temp=='/')
        {
            *tmp++= *temp++; token_type=1;
        }

        if(*temp=='.')
        {
            *tmp++= *temp++;
            token_type++;
        }

        if(isalpha(*temp))
        {
            while(isalnum(*temp) || *temp == '.')
                *tmp++= *temp++;

            *tmp = '\0';

            return(token_type=token_type+2);
        }
        token_type=0;
        tmp=token;
        temp++;
    }
    return(token_type= -1);
}

static int
GetNum(void)
{
    unsigned char *tmp;
    tmp=token;
    *tmp= *temp;

    while(temp<end_of_scan)
    {
        if(isdigit(*temp))
        {
            while(isdigit(*temp))
                *tmp++= *temp++;
            *tmp = '\0';
            return(atoi((char *)token));
        }
        temp++;
    }

    return -1;
}

/* We pass token without definition its type, it's for speed */

static int
PassToken(void)
{
    while(temp < end_of_scan)
    {
        if(!isspace(*temp))
        {
            while(!isspace(*temp))
                temp++;
            return 1;
        }
        temp++;
    }
    return -1;
}

/*                                                */
/*  Simple pass off without charstring decrypting */
/*                                                */

static int
PassString(unsigned char flg)
{
    int len_str;

    len_str=GetNum();
    if(len_str<0)
    {
        return ERR_SECOND_NUM;
    }

    if(PassToken()<0)
        return  ERR_FIRST_TOK;


    if(flg==1)
    {
        label[number].length = len_str;
        label[number].skip = temp - label[number].begin;
    }

    temp= temp+1+len_str;

    if(PassToken()<0)
        return ERR_SECOND_TOK;
    return 1;
}

void *
getmem(unsigned size)
{
    void *tmp;
    if ((tmp = calloc(1, size)) == NULL)
    {
        fprintf(stderr,"Error allocating memory\n");
        exit(1);
    }
    return tmp;
}

static CHAR *
AddChar(CHAR *TmpChar, unsigned char *CharName, int num)
{
    int length;

    CHAR *ThisChar = (CHAR*) getmem(sizeof(CHAR));
    length         = strlen((char *) CharName);
    ThisChar->name = (unsigned char *) getmem(length+1);
    strcpy((char *) ThisChar->name, (char *) CharName);
    ThisChar->length= length;
    ThisChar->num=num;
    ThisChar->NextChar = TmpChar;
    TmpChar = ThisChar;
    return TmpChar;
}


static void
AddStr(unsigned char *name, int num)
{
    int length;

    STRING *ThisStr = (STRING *) getmem(sizeof(STRING));
    length         = strlen((char *) name);
    ThisStr->name = (unsigned char *) getmem(length+1);
    strcpy((char *) ThisStr->name, (char *) name);
    ThisStr->num=num;

    ThisStr->NextStr = FirstStr;
    FirstStr = ThisStr;
}


/* We prepare own encoding vector for output */

static void
RevChar(CHAR *TmpChar)
{
    int i;
    CHAR *ThisChar = TmpChar;

    while (ThisChar != NULL)
    {
            for(i=keyword[char_str].offset-1; i < number; i++)
            {
                 if(ThisChar->num == label[i].num)
                 {
                    if (label[i].select==FLG_BINARY)
                    {
                        CHAR *Rev_Char     = (CHAR *) getmem(sizeof(CHAR));
                        Rev_Char->name     = ThisChar->name;
                        Rev_Char->num      = ThisChar->num;

                        Rev_Char->NextChar = FirstCharW;
                        FirstCharW         = Rev_Char;
                        break;
                    }
                }
            }
        ThisChar = ThisChar->NextChar;
    }
}

/* And here we produce own resulting encoding vector for partial font */

static void
OutChar(CHAR *TmpChar, FILE *fout)
{

    CHAR *ThisChar = TmpChar;

    while (ThisChar != NULL)
    {
        CHAR *tm = ThisChar;
        if (ThisChar->num >= 0)
#ifdef SHIFTLOWCHARS
           fprintf(fout, "dup %d %s put\n",T1Char(ThisChar->num),ThisChar->name);
#else
           fprintf(fout, "dup %d %s put\n",ThisChar->num,ThisChar->name);
#endif
        ThisChar = ThisChar->NextChar;
        free(tm);
    }

    FirstCharW = NULL;
}




/* We prepare strings list for output */

static void
Reeverse(STRING *TmpStr)
{

    int tmp;

    if(encode==AFM_ENC)
        tmp = -2;
    else
        tmp=0;


    while (TmpStr != NULL)
    {

        if(TmpStr->num < tmp)
        {
            STRING *ThisStr   = (STRING *) getmem(sizeof(STRING));
            ThisStr->name     = TmpStr->name;

            ThisStr->NextStr = RevStr;
            RevStr         = ThisStr;
        }
        TmpStr = TmpStr->NextStr;
    }
}


/* And here we post strings to out */

static void
OutStr(STRING *TmpStr, FILE *fout)
{
    STRING *ThisStr = TmpStr;
    if(encode==AFM_ENC)
        fprintf(fout, "readonly def\n");
    while (ThisStr != NULL)
    {
        STRING *tm = ThisStr;
        fprintf(fout, "%s",ThisStr->name);
        ThisStr = ThisStr->NextStr;
        free(tm);
    }
    RevStr = NULL;
}






static void
PrintChar(CHAR *TmpChar)
{
    CHAR *ThisChar = TmpChar;
    while (ThisChar != NULL)
    {
        if(ThisChar->choose==1)
        {
            fprintf_str(stderr, " Debug: Char %d '%s'\n",
                    ThisChar->num,ThisChar->name);
        }
        ThisChar = ThisChar->NextChar;
    }

}


static int
ClearB(void)
{
    CHAR *ThisChar = FirstCharB;
    while (ThisChar != NULL)
    {
        ThisChar->choose=0;
        ThisChar = ThisChar->NextChar;
    }
    return 1;
}

/* We mark chars in encoding vector thanks same names from
reencoding vector */

static int
ChooseChar(unsigned char *name, CHAR *TmpChar)
{
    int length;
    CHAR *ThisChar = TmpChar;
    length=strlen((char *) name);
    while (ThisChar != NULL)
    {
        CHAR *NextChar = ThisChar->NextChar;
        if(ThisChar->length==length)
        {
            if (strcmp((char *) name, (char *) ThisChar->name) == 0)
            {
                ThisChar->choose=1;
                return  1;
            }
        }
/*
 *   If we couldn't find it, NextChar is NULL here.  We fake up an entry
 *   (assuming it really does exist in the font).  Fix is from Melissa
 *   O'Neill; bugs in application of fix due to me.
 */
        if (NextChar == 0) {
           NextChar = (CHAR *) getmem(sizeof(CHAR));
           NextChar->name = (unsigned char *) getmem(length + 1);
           strcpy((char *) NextChar->name, (char *) name);
           NextChar->length = length;
           NextChar->num = -1;
           NextChar->NextChar = 0;
           NextChar->choose = 1;
           ThisChar->NextChar = NextChar;
        }
        ThisChar = NextChar;
    }
/*
 *   Can't ever get here.
 */
    return -1;
}

/* We find index in label array for char, wich is required
for compose char, if it uses SEAC command */

static int
FindSeac(int num)
{
    int i;

    for(i=keyword[char_str].offset-1; i < number; i++)
    {
        if(label[i].num==num)
        {
            return i;
        }
    }
    return -1;
}


static void ClearCW(CHAR *);

static int
FindCharW(unsigned char *name, int length)
{
    CHAR *ThisChar = FirstCharW;

    int find = 0;
    int keep_char = 0;
    int tmp = 0;
    int tmp_num = 0;

    int ddd = 0;

#ifdef DEBUG
        if(dd(D_VIEW_VECTOR))
        {
            ddd = 1;
        }
#endif

    while (ThisChar != NULL)
    {
        /* since ClearCW may free ThisChar, follow the pointer now. */
        CHAR *NextChar = ThisChar->NextChar;

        if(ThisChar->length==length)
        {
            if (strcmp((char *) name, (char *) ThisChar->name) == 0)
            {
                if(ThisChar->choose==1)
                {
                    if(find !=2)
                    {
                        find = 2;
                        tmp=FLG_BINARY;
                        label[number].num=ThisChar->num;
                    }
                }
                else
                {
                    keep_char = 0;
                    if(find != 2)
                    {
                        find = 1;
                        tmp_num = ThisChar->num;
                    }
                }


                if(keep_char == 0)
                {
                    ClearCW(ThisChar);
                }

                if((find == 2)&&(ddd == 1))
                    keep_char = 1;
            }
        }
        ThisChar = NextChar;
    }

    if(find == 1)
    {
        tmp=NOTHING;
        label[number].num=tmp_num;
    }
    return tmp;
}


static void
ClearCW(CHAR *ThisChar)
{

        if (ThisChar == FirstCharW)
            FirstCharW = ThisChar->NextChar;
        else
        {
            CHAR *tm = FirstCharW;
            while (tm != NULL)
            {
                if (ThisChar == tm->NextChar)
                {
                    tm->NextChar = ThisChar->NextChar;
                    break;
                }
                tm = tm->NextChar;
            }
        }
        free(ThisChar);
}


/* We build temporary 'work' encoding vector only for searching
needed chars */

static int
WorkVect(CHAR *TmpChar)
{
    while (TmpChar != NULL) {
        {
            CHAR *ThisChar     = (CHAR *) getmem(sizeof(CHAR));
            ThisChar->name     = TmpChar->name;
            ThisChar->length   = TmpChar->length;
            ThisChar->num      = TmpChar->num;
            ThisChar->choose   = TmpChar->choose;

            ThisChar->NextChar = FirstCharW;
            FirstCharW         = ThisChar;
        }
        TmpChar = TmpChar->NextChar;
    }
    return 0;
}


static void
UnDefineCharsW(void)
{
    CHAR *ThisChar = FirstCharW;
    while (ThisChar != NULL)
    {
        CHAR *tm = ThisChar;
        ThisChar = ThisChar->NextChar;
        free(tm);
    }
    FirstCharW = NULL;
    CharCount = 0;
}

CHAR *
UnDefineChars(CHAR *TmpChar)
{
    CHAR *ThisChar = TmpChar;
    while (ThisChar != NULL)
    {
        CHAR *tm = ThisChar;
        free(ThisChar->name);
        ThisChar = ThisChar->NextChar;
        free(tm);
    }
    TmpChar = NULL;
    CharCount = 0;
    return TmpChar;
}



static void
UnDefineStr(void)
{
    STRING *ThisStr = FirstStr;
    while (ThisStr != NULL)
    {
        STRING *tm = ThisStr;
        free(ThisStr->name);
        ThisStr = ThisStr->NextStr;
        free(tm);
    }
    FirstStr = NULL;
}



/*                                                    */
/* We mark subroutines without charstring decrypting  */
/*                                                    */

static void
ScanSubrs(int i)
{
    int err_num;
    int word_type = 0;
    int len_dup;
    int num_err=0;
    int test=0;

    len_dup = strlen(Dup);

    for(; number <  keyword[i].oldnum + keyword[i].offset;)
    {
        if((word_type=GetToken())>0)
        {
            if(word_type==2)
            {
                if(!strcmp((char *) token,Dup))
                {
                    if(test==0)
                        test=1;
                    label[number].begin = temp-len_dup;

                    err_num=GetNum();
                    if(err_num<0)
                      ;
                    else
                    {
                        if(err_num<4)
                        {
                            label[number].select=FLG_BINARY;
                                keyword[i].newnum++;

                        }
                    }

                    label[number].num=err_num;

                    num_err=PassString(1);

                    if(num_err<0)
                    {
                        ErrorOfScan(num_err);
                        fprintf(stderr,"in %d Subr string", number - 1);
                        exit(1);
                    }
                    if(test>1)
                        PassToken();

                    number++;
                }
                else
                {
                    if(test>=1)
                        test++;
                }
            }
        }
        else
        {
            ErrorOfScan(0);
            fprintf(stderr,
            "Token 'def' not found in %d Subr string ", number - 1);
            exit(1);
        }
    }
}

static void
ViewReturnCall(int num_err, int top, int *pstack,
               int j, int depth)
{
    int k,m;

#ifdef DEBUG
    if((dd(D_CALL_SUBR))&&(num_err>0))
    {
        if(grow==1)
        {
            grow=0;
            fprintf(stderr, "\n              Top: ");
        }
        else
            fprintf(stderr,   "             Back: ");
    }

#endif

    if(num_err<0)
    {
        if(grow==1)
        {
            grow=0;
            fprintf(stderr, "\n            ERROR: ");
            ErrorOfScan(num_err);
        }
        else
            fprintf(stderr,   "             Back: ");
    }

    fprintf(stderr, " %d Subr \n", top);

    fprintf(stderr," %dth level> STACK: ", level);

    for(m=0; m < j; m++, pstack++)
    {
        if(depth>(j-(m+1)))
        {
            for(k=0;
                TableCommand[k].command;
            k++)
            {
                if(TableCommand[k].code==*pstack)
                {
                    fprintf_str(stderr," %s",
                    TableCommand[k].command);
                    k=0;
                    break;
                }
            }
            if(k!=0)
                fprintf(stderr," (%d)", *pstack);
        }
        else
            fprintf(stderr, " %d", *pstack);
    }
    fprintf(stderr, " \n");
}

/*                                               */
/* We decrypt charstring  with recursive descent */
/*                                               */

static int
DeCodeStr(int num, int numseac)
{
    unsigned int loccr;
    unsigned char byte;
    static int j;
    int i;
    unsigned char jj,k;
    int tmpnum;
    int depth = 0;
    int num_err = 0;
    int len_str;
    typetemp  BORLAND_HUGE   *loc;
    typetemp  BORLAND_HUGE   *end_str;
    int pstack[64];
    int last_subr;

    if(num  > CHAR_STR)
    {
        last_subr=keyword[subrs].offset+keyword[subrs].oldnum;

        for(tmpnum=keyword[subrs].offset; tmpnum<last_subr; tmpnum++)
        {
              if(label[tmpnum].num==num)
                break;
        }

        if(tmpnum==last_subr)
        {
            fprintf(stderr, " Error: %d Subr not found \n", num);
            exit(1);
        }
        if(label[tmpnum].select==FLG_BINARY)
        {
            if(j==0)
                return 1;
        }
        else
        {
            label[tmpnum].select=FLG_BINARY;
            if(num+1 > keyword[subrs].newnum )    /* max num of subr */
                keyword[subrs].newnum = num+1;
        }
        loc = label[tmpnum].begin + label[tmpnum].skip;
        len_str=label[tmpnum].length;
    }
    else
    {
        j=0;

        if(num == CHAR_SEAC)
        {
            if(label[numseac].select!=FLG_BINARY)
            {
                label[numseac].select=FLG_BINARY;
                keyword[char_str].newnum++;
                temp = label[numseac].begin;
            }
            else return 1;
        }
        len_str=GetNum();

        if(len_str < 0)
        {
            return ERR_SECOND_NUM;
        }
        num_err = PassToken();
        if(num_err < 0)
        {
            return ERR_FIRST_TOK;
        }
        loc=temp;
    }
    loc++;

    end_str=loc+len_str;

    loccr = CDR;

    for (i = 0; i < lenIV; i++,loc++)
    {
        byte = CDeCrypt(*loc, &loccr);
    }
    for (; loc < end_str;)
    {
        byte = CDeCrypt(*loc++, &loccr);
        if (byte == RETURN)
        {
            j=0;
            break;
        }
        else if (byte == ESCAPE)
        {
            byte = CDeCrypt(*loc++, &loccr);
            if (byte > MAX_ESCAPE)
                fprintf_str(stderr,
            "Error: not_defined_e%d in %s", byte, psfontfile);
            else
            {
                switch(byte)
                {
                    case  DOTSECTION      : j=0; break;
                    case  VSTEM3          : j=0; break;
                    case  HSTEM3          : j=0; break;
                    case  SEAC            :
                        stack[j++]=byte;
                        grow=1;
                        level++;
                        jj=j;
                        for(k=0;k<jj;k++)
                            pstack[k]=stack[k];
                        num_err=FindSeac(pstack[jj-3]);
                        if(num_err<0)
                        {
                            flg_seac=1;
                            CharCount++;
                            keyword[char_str].newnum--;
                            if(flg_seac > -3)
                                label[number].select=SEAC_NOT_END;
                            grow=0;
                            level=0;
                            j=0;
                            break;
                        }

                        num_err=DeCodeStr(CHAR_SEAC, num_err);
                        level--;

#ifdef DEBUG
                        if((num_err<0)||(dd(D_CALL_SUBR)))
#else
                        if(num_err<0)
#endif
                        ViewReturnCall
                        (num_err, pstack[jj-3],pstack,jj,1);
                        grow=1;
                        level++;
                        num_err=FindSeac(pstack[jj-2]);
                        if(num_err<0)
                        {
                            flg_seac=1;
                            CharCount++;
                            keyword[char_str].newnum--;
                            keyword[char_str].newnum--;
                            if(flg_seac > -3)
                                label[number].select=SEAC_NOT_END;
                            grow=0;
                            level=0;
                            j=0;
                            break;
                        }
                        num_err=DeCodeStr(CHAR_SEAC, num_err);
                        level--;
#ifdef DEBUG
                        if((num_err<0)||(dd(D_CALL_SUBR)))
#else
                        if(num_err<0)
#endif

                        ViewReturnCall
                        (num_err, pstack[jj-2],pstack,jj,1);

                        if(num_err<0)
                            return ERR_STACK;
                        j=0; break;
                    case  SBW             : j=0; break;
                    case  CHARS_DIV       : j=0; break;
                    case  CALLOTHERSUBR   : stack[j++]=byte;
                        depth=depth+2;
                        break;
                    case  POP             : stack[j++]=byte;
                        depth=depth+2;
                        break;
                    case  SETCURRENTPOINT : j=0; break;
                }
            }
            continue;
        }
        else if (byte < 32)
        {
            switch(byte)
            {
                case  HSTEM           : j=0; break;
                case  VSTEM           : j=0; break;
                case  VMOVETO         : j=0; break;
                case  CHARS_RLINETO   : j=0; break;
                case  HLINETO         : j=0; break;
                case  VLINETO         : j=0; break;
                case  RRCURVETO       : j=0; break;
                case  CHARS_CLOSEPATH : j=0; break;
                case  CALLSUBR        : stack[j++]=byte;
                    depth=depth+2;
                    level++;
                    grow=1;
                    jj=j;
                    j=j-depth;
                    for(k=0;k<jj;k++)
                        pstack[k]=stack[k];

                    num_err = DeCodeStr(stack[j],0);

                    level--;
#ifdef DEBUG
                    if((num_err<0)||(dd(D_CALL_SUBR)))
#else
                    if(num_err<0)
#endif
                    ViewReturnCall
                    (num_err, pstack[jj-depth], pstack,jj,depth/2);

                    if(num_err<0)
                        return ERR_STACK;
                    else
                    {
                        depth=0;
                        break;
                    }
                case  RETURN          : j=0; break;
                case  ESCAPE          : break;
                case  HSBW            : j=0; break;
                case  ENDCHAR         : j=0; break;
                case  CHARS_RMOVETO   : j=0; break;
                case  HMOVETO         : j=0; break;
                case  VHCURVETO       : j=0; break;
                case  HVCURVETO       : j=0; break;
            }
        }
        if (byte >= 32)
        {
            if (byte <= 246)
            {
                value= byte  - 139;
                stack[j++]=value;
            }
            else if ((byte >= 247) && (byte <= 250))
            {
                value= (byte  - 247) * 256 + CDeCrypt(*loc++, &loccr) + 108;
                stack[j++]=value;
            }
            else if ((byte >= 251) && (byte <= 254))
            {
                value= -(byte  - 251) * 256 - CDeCrypt(*loc++, &loccr) - 108;
                stack[j++]=value;
            }
            else if (byte == 255)
            {
                value = CDeCrypt(*loc++, &loccr);
                value <<= 8;
                value += CDeCrypt(*loc++, &loccr);
                value <<= 8;
                value += CDeCrypt(*loc++, &loccr);
                value <<= 8;
                value += CDeCrypt(*loc++, &loccr);
                stack[j++]=value;
            }
        }
    }
    if(num  == CHAR_STR)
    {
        temp=loc;
        num_err = PassToken();
        if(num_err<0)
        {
            return ERR_SECOND_TOK;
        }
    }
    return 1;
}

/*                                        */
/*  We mark only necessary charstring     */
/*                                        */


static void
ScanChars(int i)
{

    int word_type=0;
    int found;
    int str_len;
    int max_num;
    int counter;
    int num_err = 0;
    typetemp  BORLAND_HUGE   *tmptemp;


    CharCount++;
    counter=number;
    max_num = keyword[i].offset+keyword[i].oldnum;

    while( number < max_num )
    {
        if((word_type=GetToken())>0)
        {
            if(word_type>=3)
            {
                strcpy((char *) tmp_token, (char *) token);
                str_len = strlen((char *) token);


                if(CharCount!=0)
                {

                    num_err=FindCharW(token, str_len);

                    if(num_err==FLG_BINARY)
                    {
                        CharCount--;
                        found=num_err;
                        keyword[i].newnum++;
                    }
                    else
                    {
#ifdef DEBUG

                        if(dd(D_VIEW_VECTOR)&&(num_err==-1))
                        {
                            fprintf_str(stderr,
                         " Debug: Char '%s' not used in WorkVector\n", token);

                        }
#endif
                        if(word_type>3)
                        {
                            if(strstr((char *) token, (char *) notdef)!=NULL)
                            {
                                CharCount--;
                                label[number].num = -2;
                                found=FLG_BINARY;
                                keyword[i].newnum++;
                            }
                            else
                                found=NOTHING;
                        }
                        else
                            found=NOTHING;
                    }
                }
                else
                {
                    found=NOTHING;
                }

                label[number].begin = temp-str_len;
                label[number].select = found;

                switch(found)
                {
                    case FLG_BINARY:
                        tmptemp=temp;
                        for(subrs=FirstKey; subrs<char_str; subrs++)
                        {
                            level=0;
                            if(subrs!=FirstKey)
                            {
                                temp=tmptemp;
                            }

                            num_err=DeCodeStr(CHAR_STR,0);

                        }
#ifdef DEBUG
                        if(dd(D_CALL_SUBR))
                        {
                            if(num_err>0)
                                fprintf_str(stderr,
                            " Debug for Char '%s'\n", tmp_token);
                        }
#endif
                        break;
                    case NOTHING:

                    num_err=PassString(0);
                    break;
                }

                if(num_err<0)
                {
                    ErrorOfScan(num_err);
                    fprintf_str(stderr,"in Char string of '%s'", tmp_token);
                    exit(1);
                }
                number++;
            }
        }
        else
        {
            fprintf_str(stderr,
           "\n File <%s> ended before all chars have been found.", psfontfile);

            fprintf(stderr,
            "\n We scan %d Chars from %d",
            counter - (2 + keyword[subrs].oldnum),
            keyword[i].oldnum);

            if(tmp_token!=NULL)
            {
                fprintf_str(stderr, "\n Last seen token was '%s'\n", tmp_token);
            }
            exit(1);
        }
    }

    if(flg_seac!=0)
    {
        tmptemp=temp;
        flg_seac--;
        for(; counter<max_num; counter++)
        {
            if((int) label[counter].select==SEAC_NOT_END)
            {
                for(subrs=FirstKey; subrs<char_str; subrs++)
                {
                    level=0;
                    temp=label[counter].begin;
                    num_err=DeCodeStr(CHAR_STR,0);
                }
                if(num_err<0)
                {
                    fprintf(stderr," Warning: %d char not choose during SEAC\n",
                    label[counter].num);
                }
                else
                {
                    CharCount--;
                    label[counter].select=FLG_BINARY;
                    if(CharCount==0)
                        break;
                }
            }
        }

        temp=tmptemp;
    }

    if(CharCount!=0)
    {
        fprintf(stderr," WARNING: Not all chars found.");
        PrintChar(FirstCharW);

    }
}

static void
LastLook(void)
{
    label[number].begin = temp;
    label[number].select = FLG_BINARY;
    number++;
}

static int
FindKeyWord(int First_Key, int lastkey)
{
    int word_type=0;
    int i;
    int tmp_num=0;

    for(;;)
    {
        if((word_type=GetToken())>0)
        {
            if(word_type==3)
            {
                for(i=First_Key; i<=lastkey; i++)
                {
                    if(!strcmp((char *) token, Key[i].name))
                    {
                        tmp_num = GetNum();
                        if(tmp_num<0)
                        {
                            fprintf_str(stderr,
                            "\n ERROR: Number not found for '%s' in <%s>",
                            Key[i].name, psfontfile);
                            exit(1);
                        }
                        keyword[current].oldnum = tmp_num;
                        keyword[current].length=strlen((char *) token);
                        keyword[current].begin=temp - keyword[current].length;
                        return i;
                    }
                }
            }
        }
        else
        {
            fprintf_str(stderr,
            "\n ERROR: In <%s> keyword not found:", psfontfile);

            for(i=First_Key; i<=lastkey; i++)
                fprintf_str(stderr,"\n %dth > '%s' ",i,Key[i].name);
            exit(1);
        }
    }
}

/* To increase scan speed we use dynamic range of keywords */

static int
ScanBinary(void)
{
    int i;
    int firstnum, lastnum;
    firstnum= LENIV;
    lastnum=SUBRS;

    number=0;
    label[number].begin = begin_of_scan;
    temp = label[number].begin;
    label[number].select = FLG_BINARY;
    offset= ++number;

    for (current=0, FirstKey=current;; current++)
    {
        i=FindKeyWord(firstnum,lastnum);
        switch(i)
        {
            case  LENIV:
                FirstKey++;
                firstnum=SUBRS;
                lastnum=SUBRS;
                lenIV=keyword[current].oldnum;
                keyword[current].type=Key[0].num;
                break;
            case  SUBRS:
                firstnum=SUBRS;
                lastnum= CHARSTRINGS;
                keyword[current].offset=number;
                keyword[current].newnum=0;
                keyword[current].type=Key[1].num;
                ScanSubrs(current);
                LastLook();
                break;
            case  CHARSTRINGS:
                char_str=current;
                keyword[current].offset=number;
                keyword[current].newnum=0;
                keyword[current].type=Key[2].num;
                ScanChars(current);
                LastLook();
#ifdef DEBUG
                if(dd(D_CALL_SUBR))
                {
                    for(i=0;i<=2;i++)
                    {
                        if(keyword[i].oldnum!=0)
                            fprintf_str(stderr, " Result for <%s>:  %s  %d (instead %d) \n",
                            psfontfile, Key[keyword[i].type].name,keyword[i].newnum, keyword[i].oldnum);
                    }

                }
#endif
                return 1;
        }
    }
}

static unsigned char *
itoasp(int n, unsigned char *s, int len)
{
    static int i, j;

    j++;
    if(n/10)
        itoasp(n/10,s,len);
    else
        i=0;
    s[i++]=abs(n)%10+'0';
    j--;
    if(j==0)
    {
        for(; i<len;)
            s[i++]=' ';
        s[i]='\0';
        return s;
    }
    return NULL;
}

static void
SubstNum(void)
{
    int i, j;

    for(i=FirstKey;i<=char_str;i++)
    {
        itoasp(keyword[i].newnum,token,keyword[i].length);
        temp=keyword[i].begin;
        for(j=0;token[j];++j,++temp)
        {
            *temp=token[j];
        }
        temp=keyword[i].begin;
    }
}

static ub4
little4(ub1 *buff)
{
    return (ub4) buff[0] +
    ((ub4) buff[1] << 8) +
    ((ub4) buff[2] << 16) +
    ((ub4) buff[3] << 24);
}

unsigned short int  c1 = C1, c2 = C2;
unsigned short int edr;


static unsigned char
CDeCrypt(unsigned char cipher, unsigned int *lcdr)
{
    register unsigned char plain;

    plain = (cipher ^ (*lcdr >> 8));
    *lcdr = (cipher + *lcdr) * c1 + c2;
    return plain;
}

unsigned short int eer;

/* We find end of own vector with non StandardEncoding,


*/


static int
EndOfEncoding(int err_num)
{

    int j;
    int i = 0;
    int flg_get_word=0;


    static const char *RefKey[] =
    {
       "readonly",
       "getinterval",
       "exch",
       "def",
       ""
    };

    for(;;)
    {
        if(flg_get_word==0)
            flg_get_word = 1;
        else
        {
            err_num=GetWord(token);

        }

        if(err_num <= 0)
            return -1;

        if(err_num==5)
            refer[ind_ref].num[i++]=atoi((char *) token);
        else
        {
            for(j=0; *RefKey[j]; j++)
            {
                 if(strcmp((char *) token, RefKey[j]) ==0)
                        break;
            }
            switch(j)
            {
                case 0:
                case 3:
                    find_encod=1;
                    keep_num = -2;
                    if(ind_ref!=0)
                    {
                        CorrectGrid();
                    }
                    return 1;

               case 1:
                    break;

               case 2:
                    if(i==1)
                    {
                        refer[ind_ref].num[1] = 1;
                        refer[ind_ref].num[2] = refer[ind_ref].num[0];
                        GetWord(token);
                        refer[ind_ref].num[0]= atoi((char *) token);
                    }
                    i=0;
                    refer[ind_ref].select=1;
                    ind_ref++;
                    break;
                default:
                    break;
            }
        }
    }

}

/* We rebuild grid for case
   "dup dup 161 10 getinterval 0 exch putinterval
    dup dup 173 23 getinterval 10 exch putinterval
    dup dup 127 exch 196 get put readonly def"
in non StandardEncoding */


static void
CorrectGrid(void)
{
    int i, j, k, imax;


    for(j=0; j<=ind_ref; j++){
        if(refer[j].select==1){
            imax= refer[j].num[1] + refer[j].num[2];

            for(k=0, i=refer[j].num[2]; i< imax; k++,i++){
                if(grid[i]==1){
                    grid[i]=0;
                    grid[k+refer[j].num[0]]=1;
                }
            }
        }
    }
}
 /* We build vector for non StandardEncoding */

static int
CharEncoding(void)
{
    int err_token=0;
    int num=0;
    int seen = 0;

    while (1) {
    err_token=GetWord(token);

    if(err_token==2)
    {
        if(strcmp((char *) token, Dup) ==0)
        {
            err_token=GetWord(token);
            if(err_token<0)
                return ERR_NUM_CHAR;

            if(err_token!=2)       /* define "dup word" */
            {
                num=atoi((char *) token);

                err_token=GetWord(token);
                if(err_token<0)
                {
                    return ERR_NAME_CHAR;
                }
                FirstChar=AddChar(FirstChar,token, num);
                keep_num=num;
                keep_flg=1;
                seen++;
                err_token = GetWord(token);
            }
        } else {

           if(keep_flg==1)
           {
               keep_num=FLG_OUT_STR;

               if(EndOfEncoding(err_token)<0)
               {
                   return -1;
               }
           }
        }
    } else
       return seen;
    }
}



static void
FindEncoding(void)
{
    int num_err=0;
    int tmpnum;

    line=tmpline;

    if(encode==0)
    {

        while((num_err=GetWord(token))>=0)
        {
            if(num_err==3)
            {
                if (strcmp((char *) token,"/Encoding") == 0)
                {

                    tmpnum=GetWord(token);

                    if(tmpnum==5)
                    {
                        encode=SPECIAL_ENC;
                    }

                    else
                    {
                        find_encod=1;
                        encode=STANDARD_ENC;
                    }
                    return;
                }
            }
        }
    }

    else
    {
        num_err= CharEncoding();
        if(num_err<0)
        {
            ErrorOfScan(num_err);
            fprintf_str(stderr,
            "\n ERROR in encoding vector in <%s>",  psfontfile);
            exit(1);
        }
    }
}

/* Before parse in BINARY portion of font we should mark needed chars,
reencode them if there is reencoding vector for this case and
build work vector */

static void
CheckChoosing(void)
{

    CHAR *TmpChar;
    int err_num, i;

    if(encode==STANDARD_ENC)
    {
        TmpChar = FirstCharB;
    }
    else
    {
        if(encode==SPECIAL_ENC)
        {
            TmpChar = FirstChar;
        }
        else
        {
            fprintf_str(stderr,
            "WARNING: '/Encoding' not found in <%s>\n", psfontfile);
            exit(1);
        }
    }

    if(reencode==FLG_REENCODE)
        err_num=LoadVector(reencode, TmpChar);
    else
        err_num=ChooseVect(TmpChar);

    if(err_num<0)
    {
            Afm();
            encode=AFM_ENC;

            TmpChar = FirstCharA;

            for(i=0;i<=255;i++)
                grid[i]=tmpgrid[i];


    if(reencode==FLG_REENCODE)
        err_num=LoadVector(reencode, TmpChar);
    else
        err_num=ChooseVect(TmpChar);

        if(err_num<0)
        {
            fprintf(stderr,
            "\n Warning: after loading AFM file \n");

            fprintf_str(stderr,
            " only %d chars found instead %d for <%s>\n",
            CharCount, GridCount, psfontfile);
        }

   }
    WorkVect(TmpChar);

#ifdef DEBUG

    if(dd(D_VIEW_VECTOR))
    {
        fprintf(stderr, "\n");
        if(encode==1)
            fprintf(stderr, " Encoding: standard \n");
        else
            fprintf(stderr, " Encoding: not standard \n");

        if(reencode==FLG_REENCODE)
            fprintf_str(stderr, " with reencode vector <%s>\n", psvectfile);

        PrintChar(FirstCharW);
    }
#endif

}

/*
 *   As we write the output file, we search for /UniqueID, and if we find
 *   such, we munge it.  We need to do this because the font is only partial,
 *   and if we send out a valid UniqueID, this well may mess up future jobs
 *   that use this font, but characters that we do not include.  We munge
 *   the string from /UniqueID to /UniqueXX just to make it more clear
 *   what we've done.
 */

static char *
KillUnique(char *s)
{
   char *r = strstr(s, "/UniqueID");
   if (r) {
      r[7] = 'X';
      r[8] = 'X';
   }
   return s;
}

static void
OutASCII(FILE *fout, ub1 *buff, ub4 len)
{
    ub4 i;

    for (i = 0; i < len; i++)
    {
        if ((*buff == 10)||(*buff == 13))
        {
            buff++;
            *line++='\n';
            *line='\0';

            if((find_encod==0)&&(lastpart==0))
            {
                FindEncoding();
            }

            line=(unsigned char *) KillUnique((char *) tmpline);

            if(keep_flg==0)
                fprintf(fout,"%s", line);
            else
            {
                if(keep_num<0)
                {
                    AddStr(line,keep_num);
                    if(keep_num==-2)
                        keep_num = -3;
                }

            }
        }
        else
        {
            *line++ = *buff++;
        }
    }
}

/* It's eexec decription for PFB format */

static void
BinEDeCrypt(ub1 *buff, ub4 len)
{
    ub4 i;

    for (i = 0; i < len; i++, temp++, buff++)
    {
        *temp  = (*buff ^ (edr >> 8));
        edr = (*buff + edr) * c1 + c2;
    }
}

/* And  it's eexec decription for PFA format */


static void
HexEDeCrypt(unsigned char *mem)
{
    int ch1, ch2, cipher;


    for(;*mem!='\n' && *mem!='\r'; temp++)
    {
        ch1= *mem++;
        ch2= *mem++;

        if ('A' <= ch1 && ch1 <= 'F')
            ch1 -= 'A' - 10;
        else if ('a' <= ch1 && ch1 <= 'f')
            ch1 -= 'a' - 10;
        else
            ch1 -= '0';
        ch1<<=4;

        if ('A' <= ch2 && ch2 <= 'F')
            ch2 -= 'A' - 10;
        else if ('a' <= ch2 && ch2 <= 'f')
            ch2 -= 'a' - 10;
        else
            ch2 -= '0';

        cipher = ch1 + ch2;

        *temp = (cipher ^ (edr >> 8));
        edr = (cipher + edr) * c1 + c2;

    }
}

static int
PartialPFA(FILE *fin, FILE *fout)
{
    ub1  type;
    ub4 memory, addmemory, length, add_of_len;
    unsigned char *mem = tmpline;
    int check_vect=0;

    tmpline=buf;
    edr  = EDR;
    type = FLG_ASCII;
    memory = BASE_MEM;
    addmemory= ADD_MEM;
    length=0;
    temp=(typetemp *) UniGetMem(memory);
    begin_of_scan=temp;

    for(;;)
    {
        if(fgets((char *)buf,BUFSIZ,fin)==NULL)
            break;
        switch (type)
        {
            case FLG_ASCII:
                if(strstr((char *)buf,"currentfile eexec") != NULL)
                {
                    type=FLG_BINARY;
                }

                if((find_encod==0)&&(lastpart==0))
                {
                    FindEncoding();
                }

                if(keep_flg==0)
                    fprintf(fout,"%s", KillUnique((char *)buf));
                else
                {
                    AddStr(buf,keep_num);
                }
                break;

            case FLG_BINARY:
                if(check_vect==0)
                {
                    tmpline=mem;
                    CheckChoosing();
                    check_vect=1;
                }

                if(GetZeroLine(buf)<0)
                {
                    type = FLG_ZERO_LINE;
                    end_of_scan=temp;
                    ScanBinary();
                    SubstNum();
                    if(keep_flg==1)
                    {
                        keep_flg=0;
                        lastpart=1;
                        if(encode!=1)
                        {
                            UnDefineCharsW();
                            if(encode==4)
                                RevChar(FirstCharA);
                             else
                                 RevChar(FirstChar);

                            OutChar(FirstCharW, fout);
                        }
                        Reeverse(FirstStr);
                        OutStr(RevStr, fout);
                    }

                    OutHEX(fout);
                    UniFree(begin_of_scan);
                    fprintf(fout, "%s", KillUnique((char*) buf));
                    break;
                }

                add_of_len=strlen((char *) buf)/2;
                length=length + add_of_len;

                if(length>memory)
                {
                    memory = memory + addmemory;
/* Using "memory = length;" retains minimum */
/* of memory  but it will be more slowly    */
                    begin_of_scan = (typetemp*) UniRealloc(begin_of_scan, memory);
                    temp = begin_of_scan + length - add_of_len;
                }
                HexEDeCrypt(buf);
                break;
            case FLG_ZERO_LINE:
                fprintf(fout, "%s", buf);
                break;
        }
    }
    if(type == FLG_ZERO_LINE)
        return TRUE;
    else return FALSE;
}

#define FIRST_ASCII     1
#define FIRST_BINARY    2
#define NEXT_BINARY     3
#define NEXT_ASCII      4

static int
PartialPFB(FILE *fin, FILE *fout)
{
    ub1  type;
    ub4  t_length, length, nread;
    int  nbytes, rc;
    int  check_vect = 0;
    int  sub_type = FIRST_ASCII;

    line=tmpline;
    edr  = EDR;

/* To find summary length of multiple binary parts we prescan font file */

    t_length = 0L;

    for (;;)
    {
        if ((rc = fread((char *) buf, 1, 6, fin)) < 2)
        {
            return FALSE;
        }

        if (buf[0] != 128)
            return FALSE;

        type = buf[1];

        if (type == FLG_EOF)
        {
            break;
        }

        if (rc != 6)
            return FALSE;

        length = little4(&buf[2]);

        if(type==FLG_BINARY)
        {
            t_length = (ub4)(length + t_length);
        }
        fseek(fin, ftell(fin) + length, SEEK_SET);
   }

/* Here we start real parsing */

    sub_type = FIRST_BINARY;

    fseek(fin, 0L, SEEK_SET);

    for (;;)
    {
        if ((rc = fread((char *) buf, 1, 6, fin)) < 2)
        {
            return FALSE;
        }
        if (buf[0] != 128)
            return FALSE;

        type = buf[1];

        if (type == FLG_EOF)
        {
            return TRUE;
        }

        if (rc != 6)
            return FALSE;

        length = little4(&buf[2]);

        if(type==FLG_BINARY)
        {
            if(sub_type == FIRST_BINARY)
            {
                sub_type = NEXT_BINARY;
                temp=(typetemp*) UniGetMem(t_length);
                begin_of_scan=temp;
            }
        }
        else
        {
            if( sub_type == NEXT_BINARY)
            {
                sub_type = NEXT_ASCII;
                end_of_scan=temp;
                ScanBinary();
                SubstNum();
                if(keep_flg==1)
                {
                    keep_flg=0;
                    lastpart=1;
                    if(encode!=1)
                    {
                        UnDefineCharsW();
                        if(encode==4)
                            RevChar(FirstCharA);
                        else
                            RevChar(FirstChar);

                        OutChar(FirstCharW, fout);
                    }

                    Reeverse(FirstStr);
                    OutStr(RevStr, fout);
                }
            OutHEX(fout);
            UniFree(begin_of_scan);
        }
        }

        for (nread = 0; nread < length; nread += rc)
        {
            nbytes = min(BUFSIZ, length - nread);

            if ((rc = fread((char *) buf, 1, nbytes, fin)) == 0)
            {
                return FALSE;
            }
            switch (type)
            {
                case FLG_ASCII:
                    OutASCII(fout, buf, (ub4) rc);
                    break;
                case FLG_BINARY:
                    if(check_vect==0)
                    {
                        CheckChoosing();
                        check_vect=1;
                    }
                    BinEDeCrypt(buf, (ub4) rc);
                    break;
                default:
                    return FALSE;
            }
        }
    }
}

static void
OutHEX(FILE *fout)
{
    int i=0;
    int num;
    static const char *hexstr = "0123456789abcdef";
    int bin;

    line=tmpline;
    eer  = EER;
    label[number].begin  =  end_of_scan;
    label[number].select = NOTHING;
    number++;

    for(num=0; num < number; num++)
    {
        switch(label[num].select)
        {
            case NOTHING:
                break;
            case FLG_BINARY:
                label[num].select=NOTHING;
                for(temp=label[num].begin; temp<label[num+1].begin; temp++,i++)
                {
                    bin = (*temp ^ (eer >> 8));    /* Eexec encryption */
                    eer = ((bin + eer) * c1 + c2);

                    *line++= hexstr[(bin&0xf0)>>4];
                    *line++= hexstr[bin&0xf];

                    if (!((i + 1) % BYTES_PER_LINE))
                    {
                        *line++='\0';
                        line =tmpline;
                        fprintf(fout, "%s\n",line);
                    }
                }
                break;
        }
    }
    if (i % BYTES_PER_LINE)
    {
        *line++='\0';
        line =tmpline;
        fprintf(fout, "%s\n",line);
    }
}

/* We parse AFM file only if we've received errors after
parsing of own vector */

static int
Afm(void)
{
    unsigned char afmfile[100];
    FILE  *fafm;
    int err_num=0;
    int i,j,k,num=0;
    unsigned char name[40];

    static const char *AfmKey[] =
    {
        "StartCharMetrics",
        "EndCharMetrics",
            ""
    };

    static const char *InfoKey[] =
    {
        "C",
        "N",
        ""
    };

    for(i=0; psfontfile[i]; i++)
    {
        if(psfontfile[i] == '.')
            break;
        else
            afmfile[i]=psfontfile[i];
    }

    afmfile[i]='\0';
    strcat((char *) afmfile,".afm");
    fprintf_str(stderr, "<%s>", afmfile);

    if ((fafm = psfopen((char *) afmfile, "r")) == NULL)
    {
        NameOfProgram();
        perror((char *) afmfile);
        return -1;
    }

    for(j=0;;)
    {
        line = tmpline;

        if(fgets((char *) line,BUFSIZ,fafm)==NULL)
            break;

        if(strstr((char *) line, AfmKey[j])!=NULL)
        {
            if(j==0)
            {
                j++;
                continue;
            }
            else
            {
                fclose(fafm);
                return 1;
            }
        }

        if(j==1)
        {
            for(k=0; err_num>=0; )
            {
                err_num=GetWord(token);
                if(err_num==2)
                {
                    if(strcmp((char *) token,InfoKey[k])==0)
                    {
                        if(k==0)
                        {
                            err_num=GetWord(token);
                            num=atoi((char *) token);
                            k=1;
                            continue;
                        }
                        else
                        {
                            err_num=GetWord(token);
                            name[0]='/';
                            name[1]='\0';
                            strcat((char *) name, (char *) token);
                            if(num>=0)
                                FirstCharA=AddChar(FirstCharA, name, num);
                            break;
                        }
                    }
                }

            }
        }
    }
    return -2;
}

int
FontPart(FILE *fout, unsigned char *fontfile,
	 unsigned char *vectfile)
{
    FILE  *fin=0;
    int   num;
    int   rc;
    int i;

    ind_ref=0;
    reencode=0;
    encode=0;
    lastpart=0;
    keep_flg=0;
    flg_seac=0;
    strcpy((char *) psfontfile, (char *) fontfile);
    find_encod=0;
    CharCount=0;

    if(loadbase != 1)
    {
        for(i=offset; i < NUM_LABEL; i++)
              label[i].num=CHAR_NOT_DEF;


        strcpy((char *) psvectfile, (char *) basevect);

#ifdef DEBUG
        if(dd(D_VIEW_VECTOR))
           fprintf_str(stderr, " Base vector <%s>.", basevect);
#endif

        if(LoadVector(1, FirstCharB)==1)
        {
            loadbase = FLG_LOAD_BASE;

        }
        else
            exit(1);
    }

    if(vectfile)
    {
        reencode=FLG_REENCODE;
        strcpy((char *) psvectfile, (char *) vectfile);
    }

    for(num=0;num<NUM_LABEL;num++)
        label[num].select = NOTHING;

    switch(DefTypeFont(fontfile))
    {
        case PFA:
            if ((fin = psfopen((char *) fontfile, "r"))==NULL)
            {
                NameOfProgram();
                perror((char *) fontfile);
                return -1;
            }
            rc = PartialPFA(fin,fout);
            if (rc == FALSE)
            {
                NameOfProgram();
                (void) fprintf_str(stderr,
                "Error: %s is not a valid PFA file\n", fontfile);
                return -1;
            }

            break;
        case PFB:
            if ((fin = psfopen((char *) fontfile, OPEN_READ_BINARY))==NULL)
            {
                NameOfProgram();
                perror((char *) fontfile);
                return -1;
            }
            rc = PartialPFB(fin,fout);
            if (rc==FALSE)
            {
                NameOfProgram();
                (void) fprintf_str(stderr,
                "Error: %s is not a valid PFB file\n", fontfile);
                return -1;
            }
            break;
        case -1:
            NameOfProgram();
            fprintf_str(stderr,
            "Error: %s has neither PFA nor PFB extension", fontfile);
            return -1;
    }

    UnDefineCharsW();

    if(encode==AFM_ENC)
        FirstCharA=UnDefineChars(FirstCharA);

    if(encode!=1)
    {
        UnDefineStr();
    }

    FirstChar=UnDefineChars(FirstChar);

    fclose(fin);

    for(i=0; i < number; i++)
        label[i].num=CHAR_NOT_DEF;

    ClearB();
    return 1;
}



static int
LoadVector(int num, CHAR *TmpChar)
{

    FILE  *fvect;
    int i = 0;
    int j = 0;
    int end_vect=0;
    int index_grid = 0;

    CharCount = 0;

    if ((fvect = psfopen((char *) psvectfile, "r")) == NULL)
    {
        NameOfProgram();
        perror((char *) psvectfile);
        return -1;
    }

    for(;;)
    {
        line = tmpline;

        if((fgets((char*)line,BUFSIZ,fvect)==NULL)||(end_vect!=0))
            break;

        for(;;)
        {
            j=GetWord(token);
            if(j==3)
            {
                if(i==0)
                {
                    i++;
                    continue;
                }

                if(num==4)
                {
                    if(grid[index_grid]==1)
                    {
                        if(ChooseChar(token, TmpChar)> 0)
                            CharCount++;
                        else
                        {
                           fprintf_str(stderr,
               "Error: '%s' not found in reencoding vector <%s> for <%s>\n",
                             token,psvectfile, psfontfile);
                        }
                    }
                    index_grid++;
                }
                else
                {
                    if(num==1)                    /* Build base vector */
                    {
                        FirstCharB=AddChar(FirstCharB,token, CharCount);
                        CharCount++;
                    }
                }
                continue;
            }
            if(j== -1)
                break;
            if(j==2)
            {
                i=0;
                end_vect = 1;
                break;
            }
        }
    }

    if(j==2)
    {
        if((index_grid!=256)&&(CharCount!=256))
        {
            fclose(fvect);
            fprintf_str(stderr,"Error during Load Vector in <%s>  \n",
            psvectfile);
            fprintf(stderr,
                    "Found %d chars instead 256\n", max(index_grid,CharCount));
            return -3;
        }

        if(CharCount>0)
        {
            fclose(fvect);
            return 1;
        }
        else
        {
            fclose(fvect);
            fprintf_str(stderr,
                     "\n Warning: Vector from <%s> for <%s> doesn't load\n",
            psvectfile, psfontfile);
            return -1;
        }
    }
    else
    {
        fprintf_str(stderr,"\n Error: ending token 'def' not found in <%s> \n",
        psvectfile);
        return -2;
    }
}

static int
ChooseVect(CHAR *tmpChar)
{
    CHAR *ThisChar = tmpChar;

    CharCount=0;
    while (ThisChar != NULL)
    {
        ThisChar->choose= grid[ThisChar->num];
        if(grid[ThisChar->num]==1)
        {
            CharCount++;
        }
        ThisChar = ThisChar->NextChar;
    }

    if(CharCount<GridCount)
        return -1;
    else
        return 1;

}

static void
ErrorOfScan(int err)
{
    switch(err)
    {

        case 0: break;

        case ERR_FIRST_NUM:
            fprintf(stderr, " First number not found ");
            break;
        case ERR_SECOND_NUM:
            fprintf(stderr, " Second number not found ");
            break;

        case ERR_FIRST_TOK:
            fprintf(stderr, " First token not found ");
            break;

        case ERR_SECOND_TOK:
            fprintf(stderr, " Second token not found ");
            break;

        case ERR_STACK:
            fprintf(stderr, " End of stack ");
            break;

        case ERR_NUM_CHAR:
            fprintf(stderr, " Number of char not found ");
            break;

        case ERR_NAME_CHAR:
            fprintf(stderr, " Name of char not found ");
            break;
    }
}

static void
NameOfProgram(void)
{
#ifdef DVIPS
    fprintf(stderr,"This is DVIPS, t1part module \n");
#else
    fprintf(stderr,"This is t1part, %s by Sergey Lesenko\n", version);
#endif
}


