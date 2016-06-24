/*
 *  ChkTeX, finds typographic errors in (La)TeX files.
 *  Copyright (C) 1995-96 Jens T. Berger Thielemann
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  Contact the author at:
 *              Jens Berger
 *              Spektrumvn. 4
 *              N-0666 Oslo
 *              Norway
 *              E-mail: <jensthi@ifi.uio.no>
 *
 *
 */


#include "ChkTeX.h"
#ifdef KPATHSEA
#include <kpathsea/getopt.h>
#else
#include <getopt.h>
#endif
#include "OpSys.h"
#include "Utility.h"
#include "FindErrs.h"
#include "Resource.h"
#include <string.h>

#undef MSG
#define MSG(num, type, inuse, ctxt, text) {(enum ErrNum)num, type, inuse, ctxt, text},

struct ErrMsg PrgMsgs[pmMaxFault + 1] = {
    PRGMSGS {(enum ErrNum)pmMaxFault, etErr, TRUE, 0, INTERNFAULT}
};

struct Stack CharStack = {
    0L
}, InputStack =

{
0L}, EnvStack =

{
0L};

/************************************************************************/

const char BrOrder[NUMBRACKETS + 1] = "()[]{}";

unsigned long Brackets[NUMBRACKETS];

/************************************************************************/


/*
 * Have to do things this way, to ease some checking throughout the
 * program.
 */


NEWBUF(TmpBuffer, BUFSIZ);
NEWBUF(ReadBuffer, BUFSIZ);

static const char *Banner =
    "ChkTeX v" PACKAGE_VERSION " - Copyright 1995-96 Jens T. Berger Thielemann.\n"
#ifdef __OS2__
    "OS/2 port generated with emx compiler, by Wolfgang Fritsch, <fritsch@hmi.de>\n"
#elif defined(__MSDOS__)
    "MS-DOS port by Bj\\o rn Ove Thue, <bjort@ifi.uio.no>\n"
#endif
#if HAVE_PCRE
    "Compiled with PCRE regex support."
#else
#if HAVE_POSIX_ERE
    "Compiled with POSIX extended regex support."
#else
    "Compiled with no regex support."
#endif
#endif
    "\n";

static const char *BigBanner =
    "ChkTeX comes with ABSOLUTELY NO WARRANTY; details on this and\n"
    "distribution conditions in the GNU General Public License file.\n"
    "Type \"ChkTeX -h\" for help, \"ChkTeX -i\" for distribution info.\n"
    "Author: Jens Berger, Spektrumvn. 4, N-0666 Oslo, Norway.\n"
    "E-mail: <jensthi@ifi.uio.no>\n"
    "Press " STDIN_BREAK " to terminate stdin input.\n";

static const char *Distrib =
    "\n"
    "This program is free software; you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation; either version 2 of the License, or\n"
    "(at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program; if not, write to the Free Software\n"
    "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n";

static const char *OnText = "On";

static const char *OffText = "Off";

static const char *HowHelp =
    "-h or --help gives usage information. See also ChkTeX.{ps,dvi}.\n";

static const char *HelpText =
    "\n"
    "\n"
    "                         Usage of ChkTeX v" PACKAGE_VERSION "\n"
    "                         ~~~~~~~~~~~~~~~~~~~~~~\n"
    "\n"
    "                               Template\n"
    "                               ~~~~~~~~\n"
    "chktex [-hiqrW] [-v[0-...]] [-l <rcfile>] [-[wemn] <[1-42]|all>]\n"
    "       [-d[0-...]] [-p <name>] [-o <outfile>] [-[btxgI][0|1]]\n"
    "       file1 file2 ...\n"
    "\n"
    "----------------------------------------------------------------------\n"
    "                       Description of options:\n"
    "                       ~~~~~~~~~~~~~~~~~~~~~~~\n"
    "Misc. options\n"
    "~~~~~~~~~~~~~\n"
    "    -h  --help      : This text.\n"
    "    -i  --license   : Show distribution information\n"
    "    -l  --localrc   : Read local .chktexrc formatted file.\n"
    "    -d  --debug     : Debug information. Give it a number.\n"
    "    -r  --reset     : Reset settings to default.\n"
    "\n"
    "Muting warning messages:\n"
    "~~~~~~~~~~~~~~~~~~~~~~~~\n"
    "    -w  --warnon    : Makes msg # given a warning and turns it on.\n"
    "    -e  --erroron   : Makes msg # given an error and turns it on.\n"
    "    -m  --msgon     : Makes msg # given a message and turns it on.\n"
    "    -n  --nowarn    : Mutes msg # given.\n"
    "    -L  --nolinesupp: Disables per-line suppressions.\n"
    "\n"
    "Output control flags:\n"
    "~~~~~~~~~~~~~~~~~~~~~\n"
    "    -v  --verbosity : How errors are displayed.\n"
    "                      Default 1, 0=Less, 2=Fancy, 3=lacheck.\n"
    "    -V  --pipeverb  : How errors are displayed when stdout != tty.\n"
    "                      Defaults to the same as -v.\n"
    "    -s  --splitchar : String used to split fields when doing -v0\n"
    "    -o  --output    : Redirect error report to a file.\n"
    "    -q  --quiet     : Shuts up about version information.\n"
    "    -p  --pseudoname: Input file-name when reporting.\n"
    "    -f  --format    : Format to use for output\n"
    "\n"
    "Boolean switches (1 -> enables / 0 -> disables):\n"
    "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
    "    -b  --backup    : Backup output file.\n"
    "    -x  --wipeverb  : Ignore contents of `\\verb' commands.\n"
    "    -g  --globalrc  : Read global .chktexrc file.\n"
    "    -I  --inputfiles: Execute \\input statements.\n"
    "    -H  --headererr : Show errors found in front of \\begin{document}\n"
    "\n"
    "Miscellaneous switches:\n"
    "~~~~~~~~~~~~~~~~~~~~~~~\n"
    "    -W  --version   : Version information\n"
    "\n"
    "----------------------------------------------------------------------\n"
    "If no LaTeX files are specified on the command line, we will read from\n"
    "stdin.   For explanation of warning/error messages, please consult the\n"
    "main documentation ChkTeX.dvi, ChkTeX.ps or ChkTeX.pdf.\n";




/*
 * Options we will set.
 *
 */

enum Quote Quote;

char VerbNormal[] = "%k %n in %f line %l: %m\n" "%r%s%t\n" "%u\n";

#define DEF(type, name, value)  type name = value
OPTION_DEFAULTS;
STATE_VARS;
#undef DEF
FILE *OutputFile = NULL;

char *PrgName;

int StdInTTY, StdOutTTY;

/*
 * End of config params.
 */

static int ParseArgs(int argc, char **argv);
static void ShowIntStatus(void);
static int OpenOut(void);
static int ShiftArg(char **Argument);


/*
 * Duplicates all arguments, and appends an asterix to each of them.
 */

static void AddStars(struct WordList *wl)
{
    unsigned long Count, CmdLen;
    char *Data;

    FORWL(Count, *wl)
    {
        Data = wl->Stack.Data[Count];
        CmdLen = strlen(Data);
        if (Data[CmdLen - 1] != '*')
        {
            strcpy(TmpBuffer, Data);
            strcat(TmpBuffer, "*");
            InsertWord(TmpBuffer, wl);
        }
    }
}

/*
 * Sets up all the lists.
 *
 */

static void SetupLists(void)
{
    unsigned long i;

    AddStars(&VerbEnvir);
    AddStars(&MathEnvir);

    MakeLower(&UserWarnCase);

    ListRep(&WipeArg, ':', 0);
    ListRep(&NoCharNext, ':', 0);

#define ThisItem ((char *) AbbrevCase.Stack.Data[i])

    FORWL(i, AbbrevCase)
    {
        if (isalpha((unsigned char)ThisItem[0]))
        {
            ThisItem[0] = toupper((unsigned char)ThisItem[0]);
            InsertWord(ThisItem, &Abbrev);
            ThisItem[0] = tolower((unsigned char)ThisItem[0]);
        }
        InsertWord(ThisItem, &Abbrev);
    }
}

#define NOCOMMON(a,b) NoCommon(&a,#a,&b,#b)

/*
 * Checks that two lists don't have any common element.
 */

static void
NoCommon(struct WordList *a, const char *aName,
         struct WordList *b, const char *bName)
{
    unsigned long i;

    FORWL(i, *a)
    {
        if (HasWord((char *) a->Stack.Data[i], b))
            PrintPrgErr(pmNoCommon, a->Stack.Data[i], aName, bName);
    }
}

/*
 * Expands the tabs in a string to regular intervals sized
 * TSize.
 */

static void ExpandTabs(char *From, char *To, long TSize, long MaxDiff)
{
    char *Next;
    char *Orig;
    unsigned long Diff;
    static short HasExpandedTooLong = 0;

    Next = From;
    Orig = To;

    while ((Next = strchr(From, '\t')))
    {
        if ((Diff = Next - From))
        {
            strncpy(To, From, Diff);
            To += Diff;
            Diff = TSize - ((To - Orig) % TSize);
        }
        else
            Diff = TSize;

        /* Make sure we don't expand this buffer out of the memory we
         * have allocated for it. */
        if ( Diff > MaxDiff+1 )
        {
            Diff = MaxDiff+1;
            if ( !HasExpandedTooLong )
            {
                PrintPrgErr(pmTabExpands, BUFSIZ);
            }
            HasExpandedTooLong = 1;
        }
        MaxDiff -= (Diff-1);

        memset(To, ' ', Diff);
        To += Diff;

        From = ++Next;
    }
    strcpy(To, From);
}

#if defined(MIKTEX)
#endif
int main(int argc, char **argv)
{
    int retval = EXIT_FAILURE, ret, CurArg;
    unsigned long Count;
    int StdInUse = FALSE;
    long Tab = 8;

#ifdef __LOCALIZED

    InitStrings();
#endif

    OutputFile = stdout;
#ifdef KPATHSEA
    kpse_set_program_name(argv[0], "chktex");
    PrgName = kpse_program_name;
#ifdef WIN32
    setmode(fileno(stdout), _O_BINARY);
#endif
#else
    PrgName = argv[0];
#endif

#undef KEY
#undef LCASE
#undef LIST
#undef LNEMPTY
#define KEY(a, def)
#define LCASE(a)
#define LIST(a)
#define LNEMPTY(a) InsertWord("", &a);

    RESOURCE_INFO

    while (SetupVars())
        ReadRC(ConfigFile);

    if (CmdLine.Stack.Used)
    {
        ParseArgs(CmdLine.Stack.Used, (char **) CmdLine.Stack.Data);
        CmdLine.Stack.Used = 1L;
    }

    if ((CurArg = ParseArgs((unsigned long) argc, argv)))
    {
        retval = EXIT_SUCCESS;
        if (CmdLine.Stack.Used)
        {
            ParseArgs(CmdLine.Stack.Used, (char **) CmdLine.Stack.Data);
            CmdLine.Stack.Used = 1L;
        }

        if (!Quiet || LicenseOnly)
            fprintf(stderr, "%s", Banner);

        if (CurArg == argc)
            UsingStdIn = TRUE;

#if defined(HAVE_FILENO) && defined(HAVE_ISATTY)

        StdInTTY = isatty(fileno(stdin));
        StdOutTTY = isatty(fileno(stdout));
#else

        StdInTTY = StdOutTTY = TRUE;
#endif

        SetupTerm();

        if ((UsingStdIn && StdInTTY && !Quiet) || LicenseOnly)
        {
            fprintf(stderr, "%s", BigBanner);
        }

        if (!StdOutTTY && PipeOutputFormat)
            OutputFormat = PipeOutputFormat;

        if (LicenseOnly)
        {
            fprintf(stderr, "%s", Distrib);
        }
        else
        {
            SetupLists();
            if (QuoteStyle)
            {
                if (!strcasecmp(QuoteStyle, "LOGICAL"))
                    Quote = quLogic;
                else if (!strcasecmp(QuoteStyle, "TRADITIONAL"))
                    Quote = quTrad;
                else
                {
                    PrintPrgErr(pmQuoteStyle, QuoteStyle);
                    Quote = quTrad;
                }
            }

            if (DebugLevel)
                ShowIntStatus();

            NOCOMMON(Italic, NonItalic);
            NOCOMMON(Italic, ItalCmd);
            NOCOMMON(LowDots, CenterDots);

            if (TabSize && isdigit((unsigned char)*TabSize))
                Tab = strtol(TabSize, NULL, 10);

            if (OpenOut())
            {
                for (;;)
                {
                    for (Count = 0; Count < NUMBRACKETS; Count++)
                        Brackets[Count] = 0L;

#define DEF(type, name, value) name = value
                    STATE_VARS;
#undef DEF
                    if (UsingStdIn)
                    {
                        if (StdInUse)
                            break;
                        else
                        {
                            StdInUse = TRUE;
                            PushFile("stdin", stdin, &InputStack);
                        }
                    }
                    else
                    {
                        if (CurArg <= argc)
                        {
                            const char *filename = NULL;
                            if (CurArg < argc)
                                filename = argv[CurArg++];

                            if (!PushFileName(filename, &InputStack))
                                break;
                        }
                    }

                    if (StkTop(&InputStack) && OutputFile)
                    {
                        while (!ferror(OutputFile)
                               && StkTop(&InputStack)
                               && !ferror(CurStkFile(&InputStack))
                               && FGetsStk(ReadBuffer, BUFSIZ - 1,
                                           &InputStack))
                        {

                            /* Make all spaces ordinary spaces */

                            strrep(ReadBuffer, '\n', ' ');
                            strrep(ReadBuffer, '\r', ' ');
                            ExpandTabs(ReadBuffer, TmpBuffer, Tab, BUFSIZ - 1 - strlen(ReadBuffer) );
                            strcpy(ReadBuffer, TmpBuffer);

                            strcat(ReadBuffer, " ");
                            ret = FindErr(ReadBuffer, CurStkLine(&InputStack));
                            if ( ret != EXIT_SUCCESS ) {
                                retval = ret;
                            }
                        }

                        PrintStatus(CurStkLine(&InputStack));
                    }
                }
            }
        }
    }
    return retval;
}

/*
 * Opens the output file handle & possibly renames
 */

static int OpenOut(void)
{
    int Success = TRUE;

    if (*OutputName)
    {
        if (BackupOut && fexists(OutputName))
        {
            strcpy(TmpBuffer, OutputName);
            AddAppendix(TmpBuffer, BAKAPPENDIX);

            if (fexists(TmpBuffer))
                remove(TmpBuffer);

            if (!rename(OutputName, TmpBuffer))
                PrintPrgErr(pmRename, OutputName, TmpBuffer);
            else
            {
                PrintPrgErr(pmRenameErr, OutputName, TmpBuffer);
                Success = FALSE;
            }

        }

        if (Success)
        {
#ifdef KPATHSEA
            if (!(OutputFile = fopen(OutputName, "wb")))
#else
            if (!(OutputFile = fopen(OutputName, "w")))
#endif
            {
                PrintPrgErr(pmOutOpen);
                Success = FALSE;
            }
        }
    }
    else
        OutputFile = stdout;

    return (Success);
}

#ifndef STRIP_DEBUG
static void ShowWL(const char *Name, const struct WordList *wl)
{
    unsigned long i, j, percent;

    fprintf(stderr, "Name: %12s", Name);

    if (DebugLevel & FLG_DbgListInfo)
    {
        fprintf(stderr, ", MaxLen: %3ld, Entries: %3ld, ",
                wl->MaxLen, wl->Stack.Used);

        if (wl->Hash.Index && wl->Stack.Used)
        {
            for (i = j = 0; i < HASH_SIZE; i++)
            {
                if (wl->Hash.Index[i])
                    j++;
            }
            percent = (j * 10000) / wl->Stack.Used;


            fprintf(stderr, "Hash usage: %3ld.%02ld%%",
                    percent / 100, percent % 100);
        }
        else
            fprintf(stderr, "No hash table.");
    }

    fputc('\n', stderr);

    if (DebugLevel & FLG_DbgListCont)
    {
        FORWL(i, *wl) fprintf(stderr, "\t%s\n", (char *) wl->Stack.Data[i]);
    }
}
#endif

#define BOOLDISP(var)           ((var)? OnText : OffText)
#define SHOWSTAT(text, arg)     fprintf(stderr, "\t" text ": %s\n", arg)
#define BOOLSTAT(name, var)     SHOWSTAT(name, BOOLDISP(var))
#define SHOWSTR(text, arg)      fprintf(stderr, "%s:\n\t%s\n", text, arg);


/*
 * Prints some of the internal flags; mainly for debugging purposes
 */

static void ShowIntStatus(void)
{
#ifndef STRIP_DEBUG
    unsigned long Cnt;
    const char *String;
    const char *iuStr = "";

    if (DebugLevel & FLG_DbgMsgs)
    {
        fprintf(stderr, "There are %d warnings/error messages available:\n",
                emMaxFault - emMinFault - 1);

        for (Cnt = emMinFault + 1; Cnt < emMaxFault; Cnt++)
        {
            switch (LaTeXMsgs[Cnt].Type)
            {
            case etWarn:
                String = "Warning";
                break;
            case etErr:
                String = "Error";
                break;
            case etMsg:
                String = "Message";
                break;
            default:
                String = "";
                break;
            }

            switch (LaTeXMsgs[Cnt].InUse)
            {
            case iuOK:
                iuStr = "In use";
                break;
            case iuNotUser:
                iuStr = "User muted";
                break;
            case iuNotSys:
                iuStr = "System muted";
                break;
            }

            fprintf(stderr, "Number: %2ld, Type: %s, Status: %s\n"
                    "\tText: %s\n\n",
                    Cnt, String, iuStr, LaTeXMsgs[Cnt].Message);
        }
    }

#undef KEY
#undef LCASE
#undef LNEMPTY
#undef LIST

#define LNEMPTY    LIST
#define LIST(a)    ShowWL(#a, &a);
#define LCASE(a)   LIST(a); LIST(a ## Case);
#define KEY(a,def) SHOWSTR(#a, a);


    if (DebugLevel & (FLG_DbgListInfo | FLG_DbgListCont))
    {
        RESOURCE_INFO
    }

    if (DebugLevel & FLG_DbgOtherInfo)
    {
        SHOWSTR("Outputformat", OutputFormat);

        fprintf(stderr, "Current flags include:\n");

        BOOLSTAT("Read global resource", GlobalRC);
        BOOLSTAT("Wipe verbose commands", WipeVerb);
        BOOLSTAT("Backup outfile", BackupOut);
        BOOLSTAT("Quiet mode", Quiet);
        BOOLSTAT("Show license", LicenseOnly);
        BOOLSTAT("Use stdin", UsingStdIn);
        BOOLSTAT("\\input files", InputFiles);
        BOOLSTAT("Output header errors", HeadErrOut);
        BOOLSTAT("No line suppressions", NoLineSupp);
    }
#endif
}

/*
 * Resets all stacks.
 *
 */

#undef KEY
#undef LCASE
#undef LNEMPTY
#undef LIST

#define LNEMPTY     LIST
#define LIST(a)     ClearWord(&a);
#define LCASE(a)    LIST(a); LIST(a ## Case);
#define KEY(a,def)  a = def;

static void ResetStacks(void)
{
    RESOURCE_INFO
}

/*
 * Resets all flags (not wordlists) to their default values. Sets
 * Outputfile to stdout.
 *
 */

static void ResetSettings(void)
{

#define DEF(type, name, value)  name = value
    OPTION_DEFAULTS;
#undef DEF
    if (OutputFile != stdout)
    {
        fclose(OutputFile);
        OutputFile = stdout;
    }
}

/*
 * Reads a numerical argument from the argument. Supports concatenation
 * of arguments (main purpose)
 */

static int ParseNumArg(long *Dest,      /* Where to put the value */
                       long Default,    /* Value to put in if no in argue */
                       char **Argument) /* optarg or similar */
{
    if (Argument && *Argument && isdigit((unsigned char)**Argument))
        *Dest = strtol(*Argument, Argument, 10);
    else
        *Dest = Default;

    return (ShiftArg(Argument));
}

/*
 * Same as above; however, will toggle the boolean if user doesn't
 * supply value
 */

static int ParseBoolArg(int *Dest,      /* Boolean value */
                        char **Argument)        /* optarg or similar */
{
    long D = *Dest ? 1L : 0L;
    int Retval;

    Retval = ParseNumArg(&D, *Dest ? 0L : 1L, Argument);

    *Dest = D ? TRUE : FALSE;

    return (Retval);
}

/*
 * Returns the first character in the string passed, updates the
 * string pointer, if the string is non-empty.
 *
 * 0 if the string is empty.
 */

static int ShiftArg(char **Argument)    /* optarg or similar */
{
    if (Argument && *Argument && **Argument)
        return (*((char *) (*Argument)++));
    else
        return 0;
}

/*
 * Parses an argv similar array.
 */

static int ParseArgs(int argc, char **argv)
{
    /* Needed for option parsing. */

    static const struct option long_options[] = {
        {"help", no_argument, 0L, 'h'},
        {"localrc", required_argument, 0L, 'l'},
        {"output", required_argument, 0L, 'o'},
        {"warnon", required_argument, 0L, 'w'},
        {"erroron", required_argument, 0L, 'e'},
        {"msgon", required_argument, 0L, 'm'},
        {"nowarn", required_argument, 0L, 'n'},
        {"nolinesupp", no_argument, 0L, 'L'},
        {"verbosity", optional_argument, 0L, 'v'},
        {"pipeverb", optional_argument, 0L, 'V'},
        {"debug", required_argument, 0L, 'd'},
        {"reset", no_argument, 0L, 'r'},
        {"quiet", no_argument, 0L, 'q'},
        {"license", no_argument, 0L, 'i'},
        {"splitchar", required_argument, 0L, 's'},
        {"format", required_argument, 0L, 'f'},
        {"pseudoname", required_argument, 0L, 'p'},

        {"inputfiles", optional_argument, 0L, 'I'},
        {"backup", optional_argument, 0L, 'b'},
        {"globalrc", optional_argument, 0L, 'g'},
        {"wipeverb", optional_argument, 0L, 'x'},
        {"tictoc", optional_argument, 0L, 't'},
        {"headererr", optional_argument, 0L, 'H'},
        {"version", no_argument, 0L, 'W'},

        {0L, 0L, 0L, 0L}
    };

    int option_index = 0L, c, i, nextc, ErrType;

    int Retval = FALSE, InUse;
    int Success, Foo;
    long Err, Verb = 1, PipeVerb = 1;

    enum
    {
        aeNoErr = 0,
        aeArg,                  /* missing/bad required argument */
        aeOpt,                  /* unknown option returned */
        aeHelp,                 /* just a call for help */
        aeMem                   /* no memory */
    } ArgErr = aeNoErr;

    optind = 0;

    while (!ArgErr &&
           ((c = getopt_long((int) argc, argv,
                             "b::d:e:f:g::hH::I::il:m:n:Lo:p:qrs:t::v::V::w:Wx::",
                             long_options, &option_index)) != EOF))
    {
        while (c)
        {
            nextc = 0;
            switch (c)
            {
            case 's':
                if (!(Delimit = strdup(optarg)))
                {
                    PrintPrgErr(pmStrDupErr);
                    ArgErr = aeMem;
                }

                break;
            case 'p':
                if (!(PseudoInName = strdup(optarg)))
                {
                    PrintPrgErr(pmStrDupErr);
                    ArgErr = aeMem;
                }

                break;

            case 'd':
#ifdef STRIP_DEBUG

                PrintPrgErr(pmNoDebugFlag);
#else

                nextc = ParseNumArg(&DebugLevel, 0xffff, &optarg);
#endif

                break;
            case 'i':
                LicenseOnly = TRUE;

                nextc = ShiftArg(&optarg);
                break;
            case 'L':
                NoLineSupp = TRUE;

                nextc = ShiftArg(&optarg);
                break;
            case 'q':
                Quiet = TRUE;

                nextc = ShiftArg(&optarg);
                break;

            LOOP(warntype,
            case 'w': ErrType = etWarn; InUse = iuOK; LAST(warntype);
            case 'e': ErrType = etErr; InUse = iuOK; LAST(warntype);
            case 'm': ErrType = etMsg; InUse = iuOK; LAST(warntype);
            case 'n': ErrType = etMsg; InUse = iuNotUser; LAST(warntype);
                )
                if (isdigit((unsigned char)*optarg))
                {
                    nextc = ParseNumArg(&Err, -1, &optarg);
                    if (betw(emMinFault, Err, emMaxFault))
                    {
                        LaTeXMsgs[Err].Type = ErrType;
                        LaTeXMsgs[Err].InUse = InUse;
                    }
                    else
                    {
                        ArgErr = aeOpt;
                        PrintPrgErr(pmWarnNumErr);
                    }
                }
                else if (!strcasecmp(optarg, "all"))
                {
                    for (i = emMinFault + 1; i < emMaxFault; i++)
                    {
                        LaTeXMsgs[i].Type = ErrType;
                        LaTeXMsgs[i].InUse = InUse;
                    }
                }
                else
                {
                    ArgErr = aeOpt;
                    PrintPrgErr(pmWarnNumErr);
                }

                break;

            case 'g':
                nextc = ParseBoolArg(&GlobalRC, &optarg);
                if (!GlobalRC)
                {
                    ResetStacks();
                }
                break;

            case 'r':
                ResetSettings();
                nextc = ShiftArg(&optarg);
                break;

            case 'l':
                if (optarg)
                    ReadRC(optarg);
                break;

            case 'f':
                if (!(OutputFormat = strdup(optarg)))
                {
                    PrintPrgErr(pmStrDupErr);
                    ArgErr = aeMem;
                }
                break;

            case 'v':
                nextc = ParseNumArg(&Verb, 2, &optarg);

                if (Verb < (long) OutFormat.Stack.Used)
                    OutputFormat = strdup(OutFormat.Stack.Data[Verb]);
                else
                {
                    PrintPrgErr(pmVerbLevErr);
                    ArgErr = aeArg;
                }
                break;
            case 'V':
                nextc = ParseNumArg(&PipeVerb, 1, &optarg);

                if (PipeVerb < (long) OutFormat.Stack.Used)
                    PipeOutputFormat = strdup(OutFormat.Stack.Data[PipeVerb]);
                else
                {
                    PrintPrgErr(pmVerbLevErr);
                    ArgErr = aeArg;
                }
                break;

            case 'o':
                if (optarg)
                {
                    if (*OutputName)
                    {
                        PrintPrgErr(pmOutTwice);
                        ArgErr = aeOpt;
                    }
                    else
                    {
                        if (!(OutputName = strdup(optarg)))
                        {
                            PrintPrgErr(pmStrDupErr);
                            ArgErr = aeMem;
                        }
                    }
                }

                break;

            case 't':
                nextc = ParseBoolArg(&Foo, &optarg);
                break;
            case 'x':
                nextc = ParseBoolArg(&WipeVerb, &optarg);
                break;
            case 'b':
                nextc = ParseBoolArg(&BackupOut, &optarg);
                break;
            case 'I':
                nextc = ParseBoolArg(&InputFiles, &optarg);
                break;
            case 'H':
                nextc = ParseBoolArg(&HeadErrOut, &optarg);
                break;
            case 'W':
                printf("%s", Banner);
                exit(EXIT_SUCCESS);
            case '?':
            default:
                fputs(Banner, stderr);
                fputs(HowHelp, stderr);
                exit(EXIT_FAILURE);
                break;
            case 'h':
                ArgErr = aeHelp;
                break;
            }
            c = nextc;
        }
    }

    if ((argc > optind) && !strcmp(argv[optind], "?"))
        ArgErr = aeHelp;

    if (ArgErr)
    {
        fputs(Banner, stderr);
        fputs(BigBanner, stderr);
        fputs(HelpText, stderr);
        Success = FALSE;
    }
    else
        Success = TRUE;

    if (Success)
        Retval = optind;

    return (Retval);
}

/*
 * Outputs a program error.
 */


void PrintPrgErr(enum PrgErrNum Error, ...)
{
    const char *Type;
    va_list MsgArgs;

    if (betw(pmMinFault, Error, pmMaxFault))
    {
        switch (PrgMsgs[Error].Type)
        {
        case etWarn:
            Type = "WARNING";
            break;
        case etMsg:
            Type = "NOTE";
            break;
        default:
        case etErr:
            Type = "ERROR";
            break;
        }
        fprintf(stderr, "%s: %s -- ", PrgName, Type);

        va_start(MsgArgs, Error);
        vfprintf(stderr, PrgMsgs[Error].Message, MsgArgs);
        va_end(MsgArgs);
        fputc('\n', stderr);

        if (PrgMsgs[Error].Type == etErr)
            exit(EXIT_FAILURE);
    }
}

void ErrPrintf(const char *fmt, ...)
{
    va_list MsgArgs;

    va_start(MsgArgs, fmt);
    vfprintf(stderr, fmt, MsgArgs);
    va_end(MsgArgs);
}
