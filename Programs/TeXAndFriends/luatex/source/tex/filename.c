/*

filename.w

Copyright 2009-2010 Taco Hoekwater <taco@@luatex.org>

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


/*tex

    In order to isolate the system-dependent aspects of file names, the @^system
    dependencies@> system-independent parts of \TeX\ are expressed in terms of
    three system-dependent procedures called |begin_name|, |more_name|, and
    |end_name|. In essence, if the user-specified characters of the file name are
    $c_1\ldots c_n$, the system-independent driver program does the operations
    $$|begin_name|;\,|more_name|(c_1);\,\ldots\,;\,|more_name|(c_n);
    \,|end_name|.$$

    These three procedures communicate with each other via global variables.
    Afterwards the file name will appear in the string pool as three strings
    called |cur_name|\penalty10000\hskip-.05em, |cur_area|, and |cur_ext|; the
    latter two are null (i.e., |""|), unless they were explicitly specified by
    the user.

    Actually the situation is slightly more complicated, because \TeX\ needs to
    know when the file name ends. The |more_name| routine is a function (with
    side effects) that returns |true| on the calls |more_name|$(c_1)$, \dots,
    |more_name|$(c_{n-1})$. The final call |more_name|$(c_n)$ returns |false|;
    or, it returns |true| and the token following $c_n$ is something like
    `\.{\\hbox}' (i.e., not a character). In other words, |more_name| is supposed
    to return |true| unless it is sure that the file name has been completely
    scanned; and |end_name| is supposed to be able to finish the assembly of
    |cur_name|, |cur_area|, and |cur_ext| regardless of whether
    $|more_name|(c_n)$ returned |true| or |false|.


    Here now is the first of the system-dependent routines for file name
    scanning. @^system dependencies@>

*/

static void begin_name(void)
{
    area_delimiter = 0;
    ext_delimiter = 0;
    quoted_filename = false;
}

/*tex

    And here's the second. The string pool might change as the file name is being
    scanned, since a new \.{\\csname} might be entered; therefore we keep
    |area_delimiter| and |ext_delimiter| relative to the beginning of the current
    string, instead of assigning an absolute address like |pool_ptr| to them.
    @^system dependencies@>

*/

static boolean more_name(ASCII_code c)
{
    if (c == ' ' && stop_at_space && (!quoted_filename)) {
        return false;
    } else if (c == '"') {
        quoted_filename = !quoted_filename;
        return true;
    } else {
        str_room(1);
        append_char(c);
        if (IS_DIR_SEP(c)) {
            area_delimiter = (pool_pointer) cur_length;
            ext_delimiter = 0;
        } else if (c == '.')
            ext_delimiter = (pool_pointer) cur_length;
        return true;
    }
}

/*tex

    The third. @^system dependencies@>

*/

static void end_name(void)
{
    unsigned char *s;
    if (str_ptr + 3 > (max_strings + STRING_OFFSET))
        overflow(
            "number of strings",
            (unsigned) (max_strings - init_str_ptr + STRING_OFFSET)
        );
    /*tex At this point, the full string lives in |cur_string|. */
    if (area_delimiter == 0) {
        cur_area = get_nullstr();
    } else {
        s = (unsigned char *) xstrdup((char *) (cur_string + area_delimiter));
        cur_string[area_delimiter] = '\0';
        cur_length = (unsigned) strlen((char *) cur_string);
        cur_area = make_string();
        xfree(cur_string);
        cur_length = (unsigned) strlen((char *) s);
        cur_string = s;
    }
    if (ext_delimiter == 0) {
        cur_name = make_string();
        cur_ext = get_nullstr();
    } else {
        int l = (ext_delimiter - area_delimiter - 1);
        s = (unsigned char *) xstrdup((char *) (cur_string + l));
        cur_string[l] = '\0';
        cur_length = (unsigned) strlen((char *) cur_string);
        cur_name = make_string();
        xfree(cur_string);
        cur_length = (unsigned) strlen((char *) s);
        cur_string = s;
        cur_ext = make_string();
    }
}

/*tex

   Now let's consider the ``driver'' routines by which \TeX\ deals with file
   names in a system-independent manner. First comes a procedure that looks for a
   file name in the input by calling |get_x_token| for the information.

*/

void scan_file_name(void)
{
    str_number u = 0;
    name_in_progress = true;
    begin_name();
    /*tex Get the next non-blank non-call token: */
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));

    while (true) {
        if ((cur_cmd > other_char_cmd) || (cur_chr > biggest_char)) {   /* not a character */
            back_input();
            break;
        }
        /*tex
            If |cur_chr| is a space and we're not scanning a token list, check
            whether we're at the end of the buffer. Otherwise we end up adding
            spurious spaces to file names in some cases.
        */
        if ((cur_chr == ' ') && (istate != token_list) && (iloc > ilimit)
            && !quoted_filename)
            break;
        if (cur_chr > 127) {
            unsigned char *bytes;
            unsigned char *thebytes;
            thebytes = uni2str((unsigned) cur_chr);
            bytes = thebytes;
            while (*bytes) {
                if (!more_name(*bytes))
                    break;
                bytes++;
            }
            xfree(thebytes);
        } else {
            if (!more_name(cur_chr))
                break;
        }
        u = save_cur_string();
        get_x_token();
        restore_cur_string(u);
    }
    end_name();
    name_in_progress = false;
}

/*
    This function constructs a the three file name strings from a token list.
*/

void scan_file_name_toks(void)
{
    char *a, *n, *e, *s = NULL;
    int i, l = 0;
    (void) scan_toks(false, true);
    s = tokenlist_to_cstring(def_ref, true, &l);
    a = n = s;
    e = NULL;
    for (i = 0; i < l; i++) {
        if (IS_DIR_SEP(s[i])) {
            n = s + i + 1;
            e = NULL;
        } else if (s[i] == '.') {
            e = s + i;
        }
    }
    if (n != s) {
        /*tex explicit area */
        cur_area = maketexlstring(a, (size_t) (n - a));
    } else {
        cur_area = get_nullstr();
    }
    if (e != NULL) {
        /*tex explicit extension */
        cur_name = maketexlstring(n, (size_t) (e - n));
        cur_ext = maketexstring(e);
    } else {
        cur_name = maketexstring(n);
        cur_ext = get_nullstr();
    }
    flush_list(def_ref);
    xfree(s);
}

/*tex

    Here is a routine that manufactures the output file names, assuming that
    |job_name<>0|. It ignores and changes the current settings of |cur_area| and
    |cur_ext|; |s = ".log"|, |".dvi"|, or |format_extension|
*/

char *pack_job_name(const char *s)
{
    cur_area = get_nullstr();
    cur_ext = maketexstring(s);
    cur_name = job_name;
    return pack_file_name(cur_name, cur_area, cur_ext);
}

/*tex

    If some trouble arises when \TeX\ tries to open a file, the following routine
    calls upon the user to supply another file name. Parameter~|s| is used in the
    error message to identify the type of file; parameter~|e| is the default
    extension if none is given. Upon exit from the routine, variables |cur_name|,
    |cur_area|, and |cur_ext| are ready for another attempt at file opening.

*/

char *prompt_file_name(const char *s, const char *e)
{
    int k;                      /* index into |buffer| */
    str_number saved_cur_name;  /* to catch empty terminal input */
    int callback_id ;
    char prompt[256];
    char *ar, *na, *ex;
    saved_cur_name = cur_name;
    if (interaction == scroll_mode) {
        wake_up_terminal();
    }
    ar = makecstring(cur_area);
    na = makecstring(cur_name);
    ex = makecstring(cur_ext);
    if (strcmp(s, "input file name") == 0) {
        snprintf(prompt, 255, "I can't find file `%s%s%s'.", ar, na, ex);
    } else {
        snprintf(prompt, 255, "I can't write on file `%s%s%s'.", ar, na, ex);
    }
    free(ar);
    free(na);
    free(ex);
    print_err(prompt);
    callback_id = callback_defined(show_error_hook_callback);
    if (callback_id > 0) {
        flush_err();
        run_callback(callback_id, "->");
    } else {
        if ((strcmp(e, ".tex") == 0) || (strcmp(e, "") == 0))
            show_context();
        if (strcmp(s, "input file name") == 0)
            tprint_nl(promptfilenamehelpmsg ")");
    }
    tprint_nl("Please type another ");
    tprint(s);
    if (interaction < scroll_mode)
        fatal_error("*** (job aborted, file error in nonstop mode)");
    clear_terminal();
    prompt_input(": ");
    begin_name();
    k = first;
    while ((buffer[k] == ' ') && (k < last))
        k++;
    while (true) {
        if (k == last)
            break;
        if (!more_name(buffer[k]))
            break;
        k++;
    }
    end_name();
    if (cur_ext == get_nullstr())
        cur_ext = maketexstring(e);
    if (str_length(cur_name) == 0)
        cur_name = saved_cur_name;
    return pack_file_name(cur_name, cur_area, cur_ext);
}

void tprint_file_name(unsigned char *n, unsigned char *a, unsigned char *e)
{
    boolean must_quote; /* whether to quote the filename */
    unsigned char *j;   /* index into string */
    must_quote = false;
    if (a != NULL) {
        j = a;
        while ((!must_quote) && (*j)) {
            must_quote = (*j == ' ');
            j++;
        }
    }
    if (n != NULL) {
        j = n;
        while ((!must_quote) && (*j)) {
            must_quote = (*j == ' ');
            j++;
        }
    }
    if (e != NULL) {
        j = e;
        while ((!must_quote) && (*j)) {
            must_quote = (*j == ' ');
            j++;
        }
    }
    /*tex
        Alternative is to assume that any filename that has to be quoted has at
        least one quoted component...if we pick this, a number of insertions of
        |print_file_name| should go away.
    */
    if (must_quote)
        print_char('"');
    if (a != NULL) {
        for (j = a; *j; j++)
            if (*j != '"')
                print_char(*j);
    }
    if (n != NULL) {
        for (j = n; *j; j++)
            if (*j != '"')
                print_char(*j);
    }
    if (e != NULL) {
        for (j = e; *j; j++)
            if (*j != '"')
                print_char(*j);
    }
    if (must_quote)
        print_char('"');
}

void print_file_name(str_number n, str_number a, str_number e)
{
    char *nam, *are, *ext;
    nam = makecstring(n);
    are = makecstring(a);
    ext = makecstring(e);
    tprint_file_name(
        (unsigned char *) nam,
        (unsigned char *) are,
        (unsigned char *) ext
    );
    free(nam);
    free(are);
    free(ext);
}
