/*

Copyright 2009-2010 Taco Hoekwater <taco@luatex.org>

This file is part of LuaTeX.

LuaTeX is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

LuaTeX is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU General Public License along
with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

*/

#include "ptexlib.h"

#include <string.h>
#include <kpathsea/absolute.h>

/*tex

The bane of portability is the fact that different operating systems treat input
and output quite differently, perhaps because computer scientists have not given
sufficient attention to this problem. People have felt somehow that input and
output are not part of ``real'' programming. Well, it is true that some kinds of
programming are more fun than others. With existing input/output conventions
being so diverse and so messy, the only sources of joy in such parts of the code
are the rare occasions when one can find a way to make the program a little less
bad than it might have been. We have two choices, either to attack I/O now and
get it over with, or to postpone I/O until near the end. Neither prospect is very
attractive, so let's get it over with.

The basic operations we need to do are (1)~inputting and outputting of text, to
or from a file or the user's terminal; (2)~inputting and outputting of eight-bit
bytes, to or from a file; (3)~instructing the operating system to initiate
(``open'') or to terminate (``close'') input or output from a specified file;
(4)~testing whether the end of an input file has been reached.

\TeX\ needs to deal with two kinds of files. We shall use the term |alpha_file|
for a file that contains textual data, and the term |byte_file| for a file that
contains eight-bit binary information. These two types turn out to be the same on
many computers, but sometimes there is a significant distinction, so we shall be
careful to distinguish between them. Standard protocols for transferring such
files from computer to computer, via high-speed networks, are now becoming
available to more and more communities of users.

The program actually makes use also of a third kind of file, called a
|word_file|, when dumping and reloading base information for its own
initialization. We shall define a word file later; but it will be possible for us
to specify simple operations on word files before they are defined.

We finally did away with |nameoffile| and |namelength|, but the variables have to
be kept otherwise there will be link errors from |openclose.c| in the web2c
library

*/

char *nameoffile;
int namelength;

/*tex

    When input files are opened via a callback, they will also be read using
    callbacks. for that purpose, the |open_read_file_callback| returns an integer
    to uniquely identify a callback table. This id replaces the file point |f| in
    this case, because the input does not have to be a file in the traditional
    sense.

    Signalling this fact is achieved by having two arrays of integers.

*/

int *input_file_callback_id;
int read_file_callback_id[17];

/*tex

    Here we handle |-output-directory|. We assume that it is OK to look here
    first. Possibly it would be better to replace lookups in "." with lookups in
    the |output_directory| followed by "." but to do this requires much more
    invasive surgery in libkpathsea.

*/

static char *find_in_output_directory(const char *s)
{
    if (output_directory && !kpse_absolute_p(s, false)) {
        FILE *f_ptr;
        char *ftemp = concat3(output_directory, DIR_SEP_STRING, s);
        /*tex This code is used for input files only. */
        f_ptr = fopen(ftemp, "rb");
        if (f_ptr) {
            fclose(f_ptr);
            return ftemp;
        } else {
            free(ftemp);

        }
    }
    return NULL;
}

/*tex

    Find an \.{\\input} or \.{\\read} file. |n| differentiates between those
    case.

*/

int kpse_available(const char *m) {
    if (!kpse_init) {
        fprintf(stdout,"missing kpse replacement callback '%s', quitting\n",m);
        exit(1);
    }
    return 1 ;
}

char *luatex_find_read_file(const char *s, int n, int callback_index)
{
    char *ftemp = NULL;
    int callback_id = callback_defined(callback_index);
    if (callback_id > 0) {
        (void) run_callback(callback_id, "dS->R", n, s, &ftemp);
    } else if (kpse_available("find_read_file")) {
        /*tex Use kpathsea here. */
        ftemp = find_in_output_directory(s);
        if (!ftemp)
#if defined(MIKTEX)
            ftemp = kpse_find_file(s, kpse_tex_format, n == 0);
#else
            ftemp = kpse_find_file(s, kpse_tex_format, 1);
#endif
    }
    if (ftemp) {
        if (fullnameoffile)
            free(fullnameoffile);
        fullnameoffile = xstrdup(ftemp);
    }
    return ftemp;
}

/*tex Find other files types. */

char *luatex_find_file(const char *s, int callback_index)
{
    char *ftemp = NULL;
    int callback_id = callback_defined(callback_index);
    if (callback_id > 0) {
        (void) run_callback(callback_id, "S->R", s, &ftemp);
    } else if (kpse_available("find_read_file")) {
        /*tex Use kpathsea here. */
        switch (callback_index) {
            case find_enc_file_callback:
                ftemp = kpse_find_file(s, kpse_enc_format, 0);
                break;
            case find_map_file_callback:
                ftemp = kpse_find_file(s, kpse_fontmap_format, 0);
                break;
            case find_type1_file_callback:
                ftemp = kpse_find_file(s, kpse_type1_format, 0);
                break;
            case find_truetype_file_callback:
                ftemp = kpse_find_file(s, kpse_truetype_format, 0);
                break;
            case find_opentype_file_callback:
                ftemp = kpse_find_file(s, kpse_opentype_format, 0);
                if (ftemp == NULL)
                    ftemp = kpse_find_file(s, kpse_truetype_format, 0);
                break;
            case find_data_file_callback:
                ftemp = find_in_output_directory(s);
                if (!ftemp)
                    ftemp = kpse_find_file(s, kpse_tex_format, 1);
                break;
            case find_font_file_callback:
                ftemp = kpse_find_file(s, kpse_ofm_format, 1);
                if (ftemp == NULL)
                    ftemp = kpse_find_file(s, kpse_tfm_format, 1);
                break;
            case find_vf_file_callback:
                ftemp = kpse_find_file(s, kpse_ovf_format, 0);
                if (ftemp == NULL)
                    ftemp = kpse_find_file(s, kpse_vf_format, 0);
                break;
            case find_cidmap_file_callback:
                ftemp = kpse_find_file(s, kpse_cid_format, 0);
                break;
            default:
                printf("luatex_find_file(): do not know how to handle file %s of type %d\n", s, callback_index);
                break;
        }
    }
    return ftemp;
}

/*tex

    \LUATEX\ used to have private functions for these that did not use kpathsea,
    but since the file paranoia tests have to come from kpathsea anyway, that is
    no longer useful. The only downside to using luatex is that if one wants to
    disable kpathsea via the Lua startup script, it is now an absolute
    requirement that all file discovery callbacks are specified. Just using the
    find_read_file, but not setting open_read_file, for example, does not work
    any more if kpathsea is not to be used at all.

*/

#define openoutnameok(A) kpse_out_name_ok (A)
#define openinnameok(A)  kpse_in_name_ok (A)

/*tex

    Open an input file F, using the kpathsea format FILEFMT and passing
    |FOPEN_MODE| to fopen. The filename is in `fn'. We return whether or not the
    open succeeded.

*/

boolean luatex_open_input(FILE ** f_ptr, const char *fn, int filefmt, const_string fopen_mode, boolean must_exist)
{
    /*tex We haven't found anything yet. */
    string fname = NULL;
    *f_ptr = NULL;
    if (fullnameoffile)
        free(fullnameoffile);
    fullnameoffile = NULL;
    fname = kpse_find_file(fn, (kpse_file_format_type) filefmt, must_exist);
    if (fname) {
        fullnameoffile = xstrdup(fname);
        /*tex

            If we found the file in the current directory, don't leave the `./'
            at the beginning of `fn', since it looks dumb when `tex foo' says
            `(./foo.tex ... )'. On the other hand, if the user said `tex ./foo',
            and that's what we opened, then keep it -- the user specified it, so
            we shouldn't remove it.

        */
        if (fname[0] == '.' && IS_DIR_SEP(fname[1]) && (fn[0] != '.' || !IS_DIR_SEP(fn[1]))) {
            unsigned i = 0;
            while (fname[i + 2] != 0) {
                fname[i] = fname[i + 2];
                i++;
            }
            fname[i] = 0;
        }
        /*tex This fopen is not allowed to fail. */
        *f_ptr = xfopen(fname, fopen_mode);
    }
    if (*f_ptr) {
        recorder_record_input(fname);
    }
    return *f_ptr != NULL;
}

boolean luatex_open_output(FILE ** f_ptr, const char *fn, const_string fopen_mode)
{
    char *fname;
    boolean absolute = kpse_absolute_p(fn, false);
#if defined(MIKTEX)
    int isAux = !miktex_is_output_file(fn);
    const char* auxDirectory = miktex_get_aux_directory();
    if (isAux && auxDirectory != 0 && !absolute)
    {
      fname = concat3(auxDirectory, DIR_SEP_STRING, fn);
    }
    else if (output_directory != 0 && !absolute)
    {
      fname = concat3(output_directory, DIR_SEP_STRING, fn);
    }
    else
    {
      fname = xstrdup(fn);
    }
#else
    /*tex If we have an explicit output directory, use it. */
    if (output_directory && !absolute) {
        fname = concat3(output_directory, DIR_SEP_STRING, fn);
    } else {
        fname = xstrdup(fn);
    }
#endif
    /*tex Is the filename openable as given?  */
    *f_ptr = fopen(fname, fopen_mode);
    if (!*f_ptr) {
        /*tex Can't open as given. Try the envvar.  */
        string texmfoutput = kpse_var_value("TEXMFOUTPUT");
        if (texmfoutput && *texmfoutput && !absolute) {
            fname = concat3(texmfoutput, DIR_SEP_STRING, fn);
            *f_ptr = fopen(fname, fopen_mode);
        }
    }
    if (*f_ptr) {
        recorder_record_output(fname);
    }
    free(fname);
    return *f_ptr != NULL;
}

boolean lua_a_open_in(alpha_file * f, char *fn, int n)
{
    int k;
    char *fnam;
    int callback_id;
    boolean ret = true;
    boolean file_ok = true;
    if (n == 0) {
        input_file_callback_id[iindex] = 0;
    } else {
        read_file_callback_id[n] = 0;
    }
    if (*fn == '|')
        fnam = fn;
    else
        fnam = luatex_find_read_file(fn, n, find_read_file_callback);
    if (!fnam)
        return false;
    callback_id = callback_defined(open_read_file_callback);
    if (callback_id > 0) {
        k = run_and_save_callback(callback_id, "S->", fnam);
        if (k > 0) {
            ret = true;
            if (n == 0)
                input_file_callback_id[iindex] = k;
            else
                read_file_callback_id[n] = k;
        } else {
            /*tex read failed */
            file_ok = false;
        }
    } else {
        /*tex no read callback */
        if (openinnameok(fnam)) {
            ret = open_in_or_pipe(f, fnam, kpse_tex_format, FOPEN_RBIN_MODE, (n == 0 ? true : false));
        } else {
            /*tex open failed */
            file_ok = false;
        }
    }
    if (!file_ok) {
        ret = false;
    }
    return ret;
}

boolean lua_a_open_out(alpha_file * f, char *fn, int n)
{
    boolean test;
    char *fnam = NULL;
    int callback_id;
    boolean ret = false;
    callback_id = callback_defined(find_write_file_callback);
    if (callback_id > 0) {
        test = run_callback(callback_id, "dS->R", n, fn, &fnam);
        if ((test) && (fnam != NULL) && (strlen(fnam) > 0)) {
            /*tex

                There is no message here because if that is needed the macro
                package should do that in the callback code. As elsewhere,
                messaging is left to \LUA\ then.

            */
            ret = open_outfile(f, fnam, FOPEN_W_MODE);
            free(fnam);
        }
    } else {
        if (openoutnameok(fn)) {
            if (n > 0 && selector != term_only) {
                /*tex

                    This message to the log is for downward compatibility with
                    other tex's as there are scripts out there that act on this
                    message. An alternative is to let a macro package write an
                    explicit message.

                */
                fprintf(log_file,"\n\\openout%i = %s\n",n-1,fn);
             }
             ret = open_out_or_pipe(f, fn, FOPEN_W_MODE);
        }
    }
    return ret;
}

boolean lua_b_open_out(alpha_file * f, char *fn)
{
    boolean test;
    char *fnam = NULL;
    int callback_id;
    boolean ret = false;
    callback_id = callback_defined(find_output_file_callback);
    if (callback_id > 0) {
        test = run_callback(callback_id, "S->R", fn, &fnam);
        if ((test) && (fnam != NULL) && (strlen(fnam) > 0)) {
            ret = open_outfile(f, fnam, FOPEN_WBIN_MODE);
            free(fnam);
        }
    } else {
        if (openoutnameok(fn)) {
            ret = luatex_open_output(f, fn, FOPEN_WBIN_MODE);
        }
    }
    return ret;
}

void lua_a_close_in(alpha_file f, int n)
{
    int callback_id;
    if (n == 0)
        callback_id = input_file_callback_id[iindex];
    else
        callback_id = read_file_callback_id[n];
    if (callback_id > 0) {
        run_saved_callback(callback_id, "close", "->");
        destroy_saved_callback(callback_id);
        if (n == 0)
            input_file_callback_id[iindex] = 0;
        else
            read_file_callback_id[n] = 0;
    } else {
        close_file_or_pipe(f);
    }
}

void lua_a_close_out(alpha_file f)
{
    close_file_or_pipe(f);
}

/*tex

    Binary input and output are done with C's ordinary procedures, so we don't
    have to make any other special arrangements for binary~I/O. Text output is
    also easy to do with standard routines. The treatment of text input is more
    difficult, however, because of the necessary translation to |ASCII_code|
    values. \TeX's conventions should be efficient, and they should blend nicely
    with the user's operating environment.

    Input from text files is read one line at a time, using a routine called
    |lua_input_ln|. This function is defined in terms of global variables called
    |buffer|, |first|, and |last| that will be described in detail later; for
    now, it suffices for us to know that |buffer| is an array of |ASCII_code|
    values, and that |first| and |last| are indices into this array representing
    the beginning and ending of a line of text.

*/

/*tex lines of characters being read */

packed_ASCII_code *buffer;

/*tex the first unused position in |buffer| */

int first;

/*tex end of the line just input to |buffer| */

int last;

/*tex largest index used in |buffer| */

int max_buf_stack;

/*tex

    The |lua_input_ln| function brings the next line of input from the specified
    file into available positions of the buffer array and returns the value
    |true|, unless the file has already been entirely read, in which case it
    returns |false| and sets |last:=first|. In general, the |ASCII_code| numbers
    that represent the next line of the file are input into |buffer[first]|,
    |buffer[first+1]|, \dots, |buffer[last-1]|; and the global variable |last| is
    set equal to |first| plus the length of the line. Trailing blanks are removed
    from the line; thus, either |last=first| (in which case the line was entirely
    blank) or |buffer[last-1]<>" "|.

    An overflow error is given, however, if the normal actions of |lua_input_ln|
    would make |last>=buf_size|; this is done so that other parts of \TeX\ can
    safely look at the contents of |buffer[last+1]| without overstepping the
    bounds of the |buffer| array. Upon entry to |lua_input_ln|, the condition
    |first<buf_size| will always hold, so that there is always room for an
    ``empty'' line.

    The variable |max_buf_stack|, which is used to keep track of how large the
    |buf_size| parameter must be to accommodate the present job, is also kept up
    to date by |lua_input_ln|.

    If the |bypass_eoln| parameter is |true|, |lua_input_ln| will do a |get|
    before looking at the first character of the line; this skips over an |eoln|
    that was in |f^|. The procedure does not do a |get| when it reaches the end
    of the line; therefore it can be used to acquire input from the user's
    terminal as well as from ordinary text files.

    Since the inner loop of |lua_input_ln| is part of \TeX's ``inner
    loop''---each character of input comes in at this place---it is wise to
    reduce system overhead by making use of special routines that read in an
    entire array of characters at once, if such routines are available.

*/

boolean lua_input_ln(alpha_file f, int n, boolean bypass_eoln)
{
    boolean lua_result;
    int last_ptr;
    int callback_id;
    /*tex Todo: variable can be removed: */
    (void) bypass_eoln;
    if (n == 0)
        callback_id = input_file_callback_id[iindex];
    else
        callback_id = read_file_callback_id[n];
    if (callback_id > 0) {
        last = first;
        last_ptr = first;
        lua_result =
            run_saved_callback(callback_id, "reader", "->l", &last_ptr);
        if ((lua_result == true) && (last_ptr != 0)) {
            last = last_ptr;
            if (last > max_buf_stack)
                max_buf_stack = last;
        } else {
            lua_result = false;
        }
    } else {
        lua_result = input_ln(f, bypass_eoln);
    }
    if (lua_result == true) {
        /*tex Fix up the input buffer using callbacks */
        if (last >= first) {
            callback_id = callback_defined(process_input_buffer_callback);
            if (callback_id > 0) {
                last_ptr = first;
                lua_result =
                    run_callback(callback_id, "l->l", (last - first),
                                 &last_ptr);
                if ((lua_result == true) && (last_ptr != 0)) {
                    last = last_ptr;
                    if (last > max_buf_stack)
                        max_buf_stack = last;
                }
            }
        }
        return true;
    }
    return false;
}

/*tex

    We need a special routine to read the first line of \TeX\ input from the
    user's terminal. This line is different because it is read before we have
    opened the transcript file; there is sort of a ``chicken and egg'' problem
    here. If the user types `\.{\\input paper}' on the first line, or if some
    macro invoked by that line does such an \.{\\input}, the transcript file will
    be named `\.{paper.log}'; but if no \.{\\input} commands are performed during
    the first line of terminal input, the transcript file will acquire its
    default name `\.{texput.log}'. (The transcript file will not contain error
    messages generated by the first line before the first \.{\\input} command.)

    The first line is special also because it may be read before \TeX\ has input
    a format file. In such cases, normal error messages cannot yet be given. The
    following code uses concepts that will be explained later.

    Different systems have different ways to get started. But regardless of what
    conventions are adopted, the routine that initializes the terminal should
    satisfy the following specifications:

    \startitemize[n]

        \startitem
            It should open file |term_in| for input from the terminal. (The file
            |term_out| will already be open for output to the terminal.)
        \stopitem

        \startitem
            If the user has given a command line, this line should be considered
            the first line of terminal input. Otherwise the user should be
            prompted with `\.{**}', and the first line of input should be
            whatever is typed in response.
        \stopitem

        \startitem
            The first line of input, which might or might not be a command line,
            should appear in locations |first| to |last-1| of the |buffer| array.
        \stopitem

        \startitem
            The global variable |loc| should be set so that the character to be
            read next by \TeX\ is in |buffer[loc]|. This character should not be
            blank, and we should have |loc<last|.
        \stopitem

    \stopitemize

    It may be necessary to prompt the user several times before a non-blank line
    comes in. The prompt is `\.{**}' instead of the later `\.*' because the
    meaning is slightly different: `\.{\\input}' need not be typed immediately
    after~`\.{**}'.)

    The following program does the required initialization. Iff anything has been
    specified on the command line, then |t_open_in| will return with |last >
    first|.

*/

boolean init_terminal(void)
{
    /*tex This gets the terminal input started. */
    t_open_in();
    if (last > first) {
        iloc = first;
        while ((iloc < last) && (buffer[iloc] == ' '))
            incr(iloc);
        if (iloc < last) {
            return true;
        }
    }
    while (1) {
        wake_up_terminal();
        fputs("**", term_out);
        update_terminal();
        if (!input_ln(term_in, true)) {
            /*tex This shouldn't happen. */
            fputs("\n! End of file on the terminal... why?\n", term_out);
            return false;
        }
        iloc = first;
        while ((iloc < last) && (buffer[iloc] == ' ')) {
            incr(iloc);
        }
        /*tex Return unless the line was all blank. */
        if (iloc < last) {
            return true;
        }
        fputs("Please type the name of your input file.\n", term_out);
    }
}


/*tex

    Here is a procedure that asks the user to type a line of input, assuming that
    the |selector| setting is either |term_only| or |term_and_log|. The input is
    placed into locations |first| through |last-1| of the |buffer| array, and
    echoed on the transcript file if appropriate.

*/

void term_input(void)
{
    /*tex Index into |buffer|: */
    int k;
    /*tex Now the user sees the prompt for sure: */
    update_terminal();
    if (!input_ln(term_in, true))
        fatal_error("End of file on the terminal!");
    /*tex The user's line ended with \.{<return>}: */
    term_offset = 0;
    /*tex Prepare to echo the input. */
    decr(selector);
    if (last != first) {
        for (k = first; k <= last - 1; k++)
            print_char(buffer[k]);
    }
    print_ln();
    /*tex Restore previous status. */
    incr(selector);
}

/*tex

    It's time now to fret about file names. Besides the fact that different
    operating systems treat files in different ways, we must cope with the fact
    that completely different naming conventions are used by different groups of
    people. The following programs show what is required for one particular
    operating system; similar routines for other systems are not difficult to
    devise.

    \TeX\ assumes that a file name has three parts: the name proper; its
    ``extension''; and a ``file area'' where it is found in an external file
    system. The extension of an input file or a write file is assumed to be
    `\.{.tex}' unless otherwise specified; it is `\.{.log}' on the transcript
    file that records each run of \TeX; it is `\.{.tfm}' on the font metric files
    that describe characters in the fonts \TeX\ uses; it is `\.{.dvi}' on the
    output files that specify typesetting information; and it is `\.{.fmt}' on
    the format files written by \.{INITEX} to initialize \TeX. The file area can
    be arbitrary on input files, but files are usually output to the user's
    current area. If an input file cannot be found on the specified area, \TeX\
    will look for it on a special system area; this special area is intended for
    commonly used input files like \.{webmac.tex}.

    Simple uses of \TeX\ refer only to file names that have no explicit extension
    or area. For example, a person usually says `\.{\\input} \.{paper}' or
    `\.{\\font\\tenrm} \.= \.{helvetica}' instead of `\.{\\input} \.{paper.new}'
    or `\.{\\font\\tenrm} \.= \.{<csd.knuth>test}'. Simple file names are best,
    because they make the \TeX\ source files portable; whenever a file name
    consists entirely of letters and digits, it should be treated in the same way
    by all implementations of \TeX. However, users need the ability to refer to
    other files in their environment, especially when responding to error
    messages concerning unopenable files; therefore we want to let them use the
    syntax that appears in their favorite operating system.

    The following procedures don't allow spaces to be part of file names; but
    some users seem to like names that are spaced-out. System-dependent changes
    to allow such things should probably be made with reluctance, and only when
    an entire file name that includes spaces is ``quoted'' somehow.

    Here are the global values that file names will be scanned into.

*/

/*tex name of file just scanned */

str_number cur_name;

/*tex file area just scanned, or \.{""} */

str_number cur_area;

/*tex file extension just scanned, or \.{""} */

str_number cur_ext;

/*tex

    The file names we shall deal with have the following structure: If the name
    contains `\./' or `\.:' (for Amiga only), the file area consists of all
    characters up to and including the final such character; otherwise the file
    area is null. If the remaining file name contains `\..', the file extension
    consists of all such characters from the last `\..' to the end, otherwise the
    file extension is null.

    We can scan such file names easily by using two global variables that keep
    track of the occurrences of area and extension delimiters:

*/

/*tex the most recent `\./', if any */

pool_pointer area_delimiter;

/*tex the relevant `\..', if any */

pool_pointer ext_delimiter;

/*tex

    Input files that can't be found in the user's area may appear in a standard
    system area called |TEX_area|. Font metric files whose areas are not given
    explicitly are assumed to appear in a standard system area called
    |TEX_font_area|. $\Omega$'s compiled translation process files whose areas
    are not given explicitly are assumed to appear in a standard system area.
    These system area names will, of course, vary from place to place.

*/

#define append_to_fn(A) do {                                    \
        c=(A);                                                  \
        if (c!='"') {                                           \
            if (k<file_name_size) fn[k++]=(unsigned char)(c);   \
        }                                                       \
    } while (0)


char *pack_file_name(str_number n, str_number a, str_number e)
{
    /*tex character being packed */
    ASCII_code c;
    /*tex index into |str_pool| */
    unsigned char *j;
    /*tex number of positions filled in |fn| */
    int k = 0;
    unsigned char *fn = xmallocarray(packed_ASCII_code, str_length(a) + str_length(n) + str_length(e) + 1);
    for (j = str_string(a); j < str_string(a) + str_length(a); j++)
        append_to_fn(*j);
    for (j = str_string(n); j < str_string(n) + str_length(n); j++)
        append_to_fn(*j);
    for (j = str_string(e); j < str_string(e) + str_length(e); j++)
        append_to_fn(*j);
    fn[k] = 0;
    return (char *) fn;
}

/*tex

    A messier routine is also needed, since format file names must be scanned
    before \TeX's string mechanism has been initialized. We shall use the global
    variable |TEX_format_default| to supply the text for default system areas and
    extensions related to format files.

    Under \UNIX\ we don't give the area part, instead depending on the path
    searching that will happen during file opening. Also, the length will be set
    in the main program.

*/

char *TEX_format_default;


/*tex

    This part of the program becomes active when a ``virgin'' \TeX\ is trying to
    get going, just after the preliminary initialization, or when the user is
    substituting another format file by typing `\.\&' after the initial `\.{**}'
    prompt. The buffer contains the first line of input in
    |buffer[loc..(last-1)]|, where |loc<last| and |buffer[loc]<>" "|.

*/

#if defined(MIKTEX)
char* open_fmt_file(int renew)
#else
char *open_fmt_file(void)
#endif
{
    /*tex The first space after the format file name: */
    int j;
    char *fmt = NULL;
    int dist;
    j = iloc;
    if (buffer[iloc] == '&') {
        incr(iloc);
        j = iloc;
        buffer[last] = ' ';
        while (buffer[j] != ' ')
            incr(j);
        fmt = xmalloc((unsigned) (j - iloc + 1));
        strncpy(fmt, (char *) (buffer + iloc), (size_t) (j - iloc));
        fmt[j - iloc] = 0;
        dist = (int) (strlen(fmt) - strlen(DUMP_EXT));
        if (!(strstr(fmt, DUMP_EXT) == fmt + dist))
            fmt = concat(fmt, DUMP_EXT);
#if defined(MIKTEX)
        if (zopen_w_input(&fmt_file, fmt, DUMP_FORMAT, FOPEN_RBIN_MODE, renew))
#else
        if (zopen_w_input(&fmt_file, fmt, DUMP_FORMAT, FOPEN_RBIN_MODE))
#endif
          goto FOUND;
        wake_up_terminal();
        fprintf(stdout, "Sorry, I can't find the format `%s'; will try `%s'.\n",
                fmt, TEX_format_default);
        update_terminal();
    }
    /*tex Now pull out all the stops: try for the system \.{plain} file. */
    fmt = TEX_format_default;
#if defined(MIKTEX)
    if (!zopen_w_input(&fmt_file, fmt, DUMP_FORMAT, FOPEN_RBIN_MODE, renew)) {
#else
    if (!zopen_w_input(&fmt_file, fmt, DUMP_FORMAT, FOPEN_RBIN_MODE)) {
#endif
        wake_up_terminal();
        fprintf(stdout, "I can't find the format file `%s'!\n",
                TEX_format_default);
        return NULL;
    }
  FOUND:
    iloc = j;
    return fmt;
}

/*tex

    The global variable |name_in_progress| is used to prevent recursive use of
    |scan_file_name|, since the |begin_name| and other procedures communicate via
    global variables. Recursion would arise only by devious tricks like
    `\.{\\input\\input f}'; such attempts at sabotage must be thwarted.
    Furthermore, |name_in_progress| prevents \.{\\input} from being initiated
    when a font size specification is being scanned.

    Another global variable, |job_name|, contains the file name that was first
    \.{\\input} by the user. This name is extended by `\.{.log}' and `\.{.dvi}'
    and `\.{.fmt}' in the names of \TeX's output files.

*/

/*tex is a file name being scanned? */

boolean name_in_progress;

/*tex principal file name */

str_number job_name;

/*tex has the transcript file been opened? */

boolean log_opened_global;

/*tex

    Initially |job_name=0|; it becomes nonzero as soon as the true name is known.
    We have |job_name=0| if and only if the `\.{log}' file has not been opened,
    except of course for a short time just after |job_name| has become nonzero.

*/

/*tex full name of the log file */

unsigned char *texmf_log_name;

/*tex

    The |open_log_file| routine is used to open the transcript file and to help
    it catch up to what has previously been printed on the terminal.

*/

void open_log_file(void)
{
    /*tex previous |selector| setting */
    int old_setting;
    /*tex index into |buffer| */
    int k;
    /*tex end of first input line */
    int l;
    char *fn;
    old_setting = selector;
    if (job_name == 0)
        job_name = getjobname(maketexstring("texput"));
    fn = pack_job_name(".fls");
    recorder_change_filename(fn);
    fn = pack_job_name(".log");
    while (!lua_a_open_out(&log_file, fn, 0)) {
        /*tex

            Try to get a different log file name. Sometimes |open_log_file| is
            called at awkward moments when \TeX\ is unable to print error
            messages or even to |show_context|. The |prompt_file_name| routine
            can result in a |fatal_error|, but the |error| routine will not be
            invoked because |log_opened| will be false.

            The normal idea of |batch_mode| is that nothing at all should be
            written on the terminal. However, in the unusual case that no log
            file could be opened, we make an exception and allow an explanatory
            message to be seen.

            Incidentally, the program always refers to the log file as a
            `\.{transcript file}', because some systems cannot use the extension
            `\.{.log}' for this file.
        */
        selector = term_only;
        fn = prompt_file_name("transcript file name", ".log");
    }
    texmf_log_name = (unsigned char *) xstrdup(fn);
    selector = log_only;
    log_opened_global = true;
    if (callback_defined(start_run_callback) == 0) {
        /*tex Print the banner line, including current date and time. */
        log_banner(luatex_version_string);
        /*tex Make sure bottom level is in memory. */
        input_stack[input_ptr] = cur_input;
        tprint_nl("**");
        /*tex The last position of first line. */
        l = input_stack[0].limit_field;
        if (buffer[l] == end_line_char_par) {
            /*tex maybe also handle multichar endlinechar */
            decr(l);
        }
        for (k = 1; k <= l; k++) {
            print_char(buffer[k]);
        }
        /*tex now the transcript file contains the first line of input */
        print_ln();
    }
    /*tex should be done always */
    flush_loggable_info();
    /*tex should be done always */
    selector = old_setting + 2;
}

/*tex

    This function is needed by synctex to make its log appear in the right spot
    when |output_directory| is set.

*/

char *get_full_log_name (void)
{
   if (output_directory) {
       char *ret  = xmalloc(strlen((char *)texmf_log_name)+2+strlen(output_directory));
       ret = strcpy(ret, output_directory);
       strcat(ret, "/");
       strcat(ret, (char *)texmf_log_name);
       return ret;
   } else {
       return xstrdup((const char*)texmf_log_name);
   }
}

/*tex Synctex uses this to get the anchored path of an input file. */

char *luatex_synctex_get_current_name (void)
{
  char *pwdbuf = NULL, *ret;
  if (kpse_absolute_p(fullnameoffile, false)) {
     return xstrdup(fullnameoffile);
  }
  pwdbuf = xgetcwd();
  ret = concat3(pwdbuf, DIR_SEP_STRING, fullnameoffile);
  free(pwdbuf) ;
  return ret;
}

/*tex

    Let's turn now to the procedure that is used to initiate file reading when an
    `\.{\\input}' command is being processed.

*/

void start_input(void)
{
    str_number temp_str;
    char *fn;
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));

    back_input();
    if (cur_cmd != left_brace_cmd) {
        /*tex Set |cur_name| to desired file name. */
        scan_file_name();
    } else {
        scan_file_name_toks();
    }
    fn = pack_file_name(cur_name, cur_area, cur_ext);
    while (1) {
        /*tex Set up |cur_file| and new level of input. */
        begin_file_reading();
        if (lua_a_open_in(&cur_file, fn, 0)) {
            break;
        }
        /*tex Remove the level that didn't work. */
        end_file_reading();
        fn = prompt_file_name("input file name", "");
    }
    iname = maketexstring(fullnameoffile);
    /*tex

        Now that we have |fullnameoffile|, it is time to post-adjust |cur_name|
        and |cur_ext| for trailing |.tex|.

    */
    {
        char *n, *p;
        n = p = fullnameoffile + strlen(fullnameoffile);
        while (p>fullnameoffile) {
            p--;
            if (IS_DIR_SEP(*p)) {
                break;
            }
        }
        if (IS_DIR_SEP(*p)) {
            p++;
        }
        while (n>fullnameoffile) {
            n--;
            if (*n == '.') {
                break;
            }
        }
        if (n>p) {
            int q = *n;
            cur_ext = maketexstring(n);
            *n = 0;
            cur_name = maketexstring(p);
            *n = q;
        }
    }
    source_filename_stack[in_open] = iname;
    full_source_filename_stack[in_open] = xstrdup(fullnameoffile);
    /*tex We can try to conserve string pool space now. */
    temp_str = search_string(iname);
    if (temp_str > 0) {
        flush_str(iname);
        iname = temp_str;
    }
    if (job_name == 0) {
        job_name = getjobname(cur_name);
        open_log_file();
    }
    /*tex

        |open_log_file| doesn't |show_context|, so |limit| and |loc| needn't be
        set to meaningful values yet.

    */
    report_start_file(filetype_tex,fullnameoffile);
    incr(open_parens);
    update_terminal();
    istate = new_line;
    /*tex Prepare new file {\sl Sync\TeX} information. */
    if (! synctex_get_no_files()) {
        /*tex Give control to the {\sl Sync\TeX} controller. */
        synctexstartinput();
    }
    /*tex

        Read the first line of the new file. Here we have to remember to tell the
        |lua_input_ln| routine not to start with a |get|. If the file is empty,
        it is considered to contain a single blank line.

    */
    line = 1;
    if (lua_input_ln(cur_file, 0, false)) {
        ;
    }
    firm_up_the_line();
    if (end_line_char_inactive)
        decr(ilimit);
    else
        buffer[ilimit] = (packed_ASCII_code) end_line_char_par;
    first = ilimit + 1;
    iloc = istart;
}

/*tex

    Because the format is zipped we read and write dump files through zlib.
    Earlier versions recast |*f| from |FILE *| to |gzFile|, but there is no
    guarantee that these have the same size, so a static variable is needed.

*/

static gzFile gz_fmtfile = NULL;

/*tex

    As distributed, the dump files are architecture dependent; specifically,
    BigEndian and LittleEndian architectures produce different files. These
    routines always output BigEndian files. This still does not guarantee them to
    be architecture-independent, because it is possible to make a format that
    dumps a glue ratio, i.e., a floating-point number. Fortunately, none of the
    standard formats do that.

*/

#if !defined (WORDS_BIGENDIAN) && !defined (NO_DUMP_SHARE)

/*tex

    This macro is always invoked as a statement. It assumes a variable `temp'.

*/

#  define SWAP(x, y) do { temp = x; x = y; y = temp; } while (0)

/*tex

    Make the NITEMS items pointed at by P, each of size SIZE, be the
    opposite-endianness of whatever they are now.

*/

static void swap_items(char *pp, int nitems, int size)
{
    char temp;
    unsigned total = (unsigned) (nitems * size);
    char *q = xmalloc(total);
    char *p = q;
    memcpy(p,pp,total);
    /*tex

        Since `size' does not change, we can write a while loop for each case,
        and avoid testing `size' for each time.

    */
    switch (size) {
        case 16:
            /*tex

                16-byte items happen on the DEC Alpha machine when we are not doing
                sharable memory dumps.

            */
            while (nitems--) {
                SWAP(p[0], p[15]);
                SWAP(p[1], p[14]);
                SWAP(p[2], p[13]);
                SWAP(p[3], p[12]);
                SWAP(p[4], p[11]);
                SWAP(p[5], p[10]);
                SWAP(p[6], p[9]);
                SWAP(p[7], p[8]);
                p += size;
            }
            break;

        case 12:
            while (nitems--) {
                SWAP(p[0], p[11]);
                SWAP(p[1], p[10]);
                SWAP(p[2], p[9]);
                SWAP(p[3], p[8]);
                SWAP(p[4], p[7]);
                SWAP(p[5], p[6]);
                p += size;
            }
            break;

        case 8:
            while (nitems--) {
                SWAP(p[0], p[7]);
                SWAP(p[1], p[6]);
                SWAP(p[2], p[5]);
                SWAP(p[3], p[4]);
                p += size;
            }
            break;

        case 4:
            while (nitems--) {
                SWAP(p[0], p[3]);
                SWAP(p[1], p[2]);
                p += size;
            }
            break;

        case 2:
            while (nitems--) {
                SWAP(p[0], p[1]);
                p += size;
            }
            break;
        case 1:
            /*tex Nothing to do. */
            break;
        default:
            FATAL1("Can't swap a %d-byte item for (un)dumping", size);
    }
    memcpy(pp,q,total);
    xfree(q);
}
#endif

/*tex

    That second swap is to make sure following calls don't get confused in the
    case of |dump_things|.

*/

void do_zdump(char *p, int item_size, int nitems, FILE * out_file)
{
    int err;
    (void) out_file;
    if (nitems == 0)
        return;
#if !defined (WORDS_BIGENDIAN) && !defined (NO_DUMP_SHARE)
    swap_items(p, nitems, item_size);
#endif
    if (gzwrite(gz_fmtfile, (void *) p, (unsigned) (item_size * nitems)) !=
        item_size * nitems) {
        fprintf(stderr, "! Could not write %d %d-byte item(s): %s.\n", nitems, item_size, gzerror(gz_fmtfile, &err));
        uexit(1);
    }
#if !defined (WORDS_BIGENDIAN) && !defined (NO_DUMP_SHARE)
    swap_items(p, nitems, item_size);
#endif
}

void do_zundump(char *p, int item_size, int nitems, FILE * in_file)
{
    int err;
    (void) in_file;
    if (nitems == 0)
        return;
    if (gzread(gz_fmtfile, (void *) p, (unsigned) (item_size * nitems)) <= 0) {
        fprintf(stderr, "Could not undump %d %d-byte item(s): %s.\n", nitems, item_size, gzerror(gz_fmtfile, &err));
        uexit(1);
    }
#if !defined (WORDS_BIGENDIAN) && !defined (NO_DUMP_SHARE)
    swap_items(p, nitems, item_size);
#endif
}

/*tex

    Tests has shown that a level 3 compression is the most optimal tradeoff
    between file size and load time.

*/

#define COMPRESSION "R3"

#if defined(MIKTEX)
boolean zopen_w_input(FILE** f, const char* fname, int format, const_string fopen_mode, int renew)
#else
boolean zopen_w_input(FILE ** f, const char *fname, int format, const_string fopen_mode)
#endif
{
    int callbackid;
    int res;
    char *fnam;
    callbackid = callback_defined(find_format_file_callback);
    if (callbackid > 0) {
        res = run_callback(callbackid, "S->R", fname, &fnam);
        if (res && fnam && strlen(fnam) > 0) {
            *f = fopen(fnam, fopen_mode);
            if (*f == NULL) {
                return 0;
            }
        } else {
            return 0;
        }
    } else {
#if defined(MIKTEX)
      if (format == DUMP_FORMAT)
      {
        res = miktex_open_format_file(fname, f, renew);
      }
      else
#endif
        res = luatex_open_input(f, fname, format, fopen_mode, true);
    }
    if (res) {
#if defined(MIKTEX)
        gz_fmtfile = gzdopen(dup(fileno(*f)), "rb" COMPRESSION);
#else
        gz_fmtfile = gzdopen(fileno(*f), "rb" COMPRESSION);
#endif
    }
    return res;
}

boolean zopen_w_output(FILE ** f, const char *s, const_string fopen_mode)
{
    int res = 1;
    if (luainit) {
        *f = fopen(s, fopen_mode);
        if (*f == NULL) {
            return 0;
        }
    } else {
        res = luatex_open_output(f, s, fopen_mode);
    }
    if (res) {
#if defined(MIKTEX)
        gz_fmtfile = gzdopen(dup(fileno(*f)), "wb" COMPRESSION);
#else
        gz_fmtfile = gzdopen(fileno(*f), "wb" COMPRESSION);
#endif
    }
    return res;
}

void zwclose(FILE * f)
{
    (void) f;
    gzclose(gz_fmtfile);
}

/*tex Create the \DVI\ or \PDF\ file. */

int open_outfile(FILE ** f, const char *name, const char *mode)
{
    FILE *res;
    res = fopen(name, mode);
    if (res != NULL) {
        *f = res;
        return 1;
    }
    return 0;
}

/*tex The caller should set |tfm_buffer=NULL| and |tfm_size=0|. */

int readbinfile(FILE * f, unsigned char **tfm_buffer, int *tfm_size)
{
    void *buf;
    int size;
    if (fseek(f, 0, SEEK_END) == 0) {
        size = (int) ftell(f);
        if (size > 0) {
            buf = xmalloc((unsigned) size);
            if (fseek(f, 0, SEEK_SET) == 0) {
                if (fread((void *) buf, (size_t) size, 1, f) == 1) {
                    *tfm_buffer = (unsigned char *) buf;
                    *tfm_size = size;
                    return 1;
                }
            }
        } else {
            *tfm_buffer = NULL;
            *tfm_size = 0;
            return 1;
        }
    }
    /*tex Either seek failed or we have a zero-sized file. */
    return 0;
}

/*tex

    Like |os.execute()|, the |runpopen()| function is called only when
    |shellenabledp == 1|. Unlike |os.execute()| we write errors to stderr, since
    we have nowhere better to use; and of course we return a file handle (or
    NULL) instead of a status indicator.

*/

static FILE *runpopen(char *cmd, const char *mode)
{
#if defined(MIKTEX)
  return miktex_emulate__runpopen(cmd, mode);
#else
    FILE *f = NULL;
    char *safecmd = NULL;
    char *cmdname = NULL;
    int allow;
#ifdef WIN32
    char *pp;

    for (pp = cmd; *pp; pp++) {
      if (*pp == '\'') *pp = '"';
    }
#endif
    /*tex If |restrictedshell| is zero, any command is allowed. */
    if (restrictedshell == 0) {
        allow = 1;
    } else {
        const char *thecmd = cmd;
        allow = shell_cmd_is_allowed(thecmd, &safecmd, &cmdname);
    }
    if (allow == 1)
        f = popen(cmd, mode);
    else if (allow == 2)
        f = popen(safecmd, mode);
    else if (allow == -1)
        fprintf(stderr, "\nrunpopen quotation error in command line: %s\n", cmd);
    else
        fprintf(stderr, "\nrunpopen command not allowed: %s\n", cmdname);
    if (safecmd)
        free(safecmd);
    if (cmdname)
        free(cmdname);
    return f;
#endif
}

/*tex

    The code that implements |popen()| needs an array for tracking possible pipe
    file pointers, because these need to be closed using |pclose()|.

*/

#if !defined(MIKTEX)
#define NUM_PIPES 16
static FILE *pipes[NUM_PIPES];

#ifdef WIN32
FILE *Poptr;
#endif
#endif

boolean open_in_or_pipe(FILE ** f_ptr, char *fn, int filefmt, const_string fopen_mode, boolean must_exist)
{
    string fname = NULL;
    int i;
    /*tex

        Opening a read pipe is straightforward, only have to skip past the pipe
        symbol in the file name. filename quoting is assumed to happen elsewhere
        (it does :-))

    */
    if (shellenabledp && *fn == '|') {
        /*tex The user requested a pipe. */
        *f_ptr = NULL;
        fname = (string) xmalloc((unsigned) (strlen(fn) + 1));
        strcpy(fname, fn);
        if (fullnameoffile)
            free(fullnameoffile);
        fullnameoffile = xstrdup(fname);
#if !defined(MIKTEX)
        recorder_record_input(fname + 1);
#endif
        *f_ptr = runpopen(fname + 1, "r");
        free(fname);
#if !defined(MIKTEX)
        for (i = 0; i < NUM_PIPES; i++) {
            if (pipes[i] == NULL) {
                pipes[i] = *f_ptr;
                break;
            }
        }
#endif
        if (*f_ptr)
            setvbuf(*f_ptr, (char *) NULL, _IONBF, 0);
#if !defined(MIKTEX)
#ifdef WIN32
        Poptr = *f_ptr;
#endif
#endif
        return *f_ptr != NULL;
    }
    return luatex_open_input(f_ptr, fn, filefmt, fopen_mode, must_exist);
}


boolean open_out_or_pipe(FILE ** f_ptr, char *fn, const_string fopen_mode)
{
    string fname;
    int i;
    /*tex

        Opening a write pipe takes a little bit more work, because TeX will
        perhaps have appended ".tex". To avoid user confusion as much as
        possible, this extension is stripped only when the command is a bare
        word. Some small string trickery is needed to make sure the correct
        number of bytes is free()-d afterwards.
    */
    if (shellenabledp && *fn == '|') {
        /*tex The user requested a pipe. */
        fname = (string) xmalloc((unsigned) (strlen(fn) + 1));
        strcpy(fname, fn);
        if (strchr(fname, ' ') == NULL && strchr(fname, '>') == NULL) {
            /*tex

                \METAPOST\ and \METAFIONT\ currently do not use this code, but it
                is better to be prepared. Hm, what has this todo with \LUATEX ?

            */
            if (STREQ((fname + strlen(fname) - 3), "tex"))
                *(fname + strlen(fname) - 4) = 0;
            *f_ptr = runpopen(fname + 1, "w");
            *(fname + strlen(fname)) = '.';
        } else {
            *f_ptr = runpopen(fname + 1, "w");
        }
#if !defined(MIKTEX)
        recorder_record_output(fname + 1);
#endif
        free(fname);
#if !defined(MIKTEX)
        for (i = 0; i < NUM_PIPES; i++) {
            if (pipes[i] == NULL) {
                pipes[i] = *f_ptr;
                break;
            }
        }
        if (*f_ptr)
            setvbuf(*f_ptr, (char *) NULL, _IONBF, 0);
#endif
        return *f_ptr != NULL;
    }
    return luatex_open_output(f_ptr, fn, fopen_mode);
}


void close_file_or_pipe(FILE * f)
{
#if defined(MIKTEX)
  miktex_emulate__close_file_or_pipe(f);
#else
    int i;
    if (shellenabledp) {
        for (i = 0; i <= 15; i++) {
            /*tex If this file was a pipe, |pclose()| it and return. */
            if (pipes[i] == f) {
                if (f) {
                    pclose(f);
#ifdef WIN32
                    Poptr = NULL;
#endif
                }
                pipes[i] = NULL;
                return;
            }
        }
    }
    close_file(f);
#endif
}
