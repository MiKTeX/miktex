/*

texdeffont.w

Copyright 2008-2010 Taco Hoekwater <taco@@luatex.org>

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

When the user defines \.{\\font\\f}, say, \TeX\ assigns an internal number to the
user's font~\.{\\f}. Adding this number to |font_id_base| gives the |eqtb|
location of a ``frozen'' control sequence that will always select the font.

The variable |a| in the following code indicates the global nature of the value
to be set. It's used in the |define| macro. Here we're never global.

There's not much scanner code here because the other scanners are defined where
they make most sense.

*/

int font_bytes;

void set_cur_font(internal_font_number f)
{
    int a = 0;
    define(cur_font_loc, data_cmd, f);
}

/*tex

    This prints a scaled real, rounded to five digits.

*/

static char *scaled_to_string(scaled s)
{
    static char result[16];
    int n, k;
    /*tex The amount of allowable inaccuracy: */
    scaled delta;
    k = 0;
    if (s < 0) {
        /*tex Only print the sign, if negative */
        result[k++] = '-';
        s = -s;
    }
    {
        int l = 0;
        char digs[8] = { 0 };
        n = s / unity;
        /*tex Process the integer part: */
        do {
            digs[l++] = (char) (n % 10);
            n = n / 10;;
        } while (n > 0);
        while (l > 0) {
            result[k++] = (char) (digs[--l] + '0');
        }
    }
    result[k++] = '.';
    s = 10 * (s % unity) + 5;
    delta = 10;
    do {
        if (delta > unity) {
            /*tex Round the last digit: */
            s = s + 0100000 - 050000;
        }
        result[k++] = (char) ('0' + (s / unity));
        s = 10 * (s % unity);
        delta = delta * 10;
    } while (s > delta);
    result[k] = 0;
    return (char *) result;
}

void tex_def_font(small_number a)
{
    /*tex The user's font identifier. */
    pointer u;
    /*tex This runs through existing fonts. */
    internal_font_number f;
    /*tex The name for the frozen font identifier. */
    str_number t, d;
    /*tex Thos holds the |selector| setting. */
    int old_setting;
    /*tex Stated `at' size, or negative of scaled magnification. */
    scaled s = -1000;
    /*tex The natural direction of the font. */
    int natural_dir = -1;
    char *fn;
    if (job_name == 0) {
        /*tex Avoid confusing \.{texput} with the font name. */
        open_log_file();
    }
    get_r_token();
    u = cur_cs;
    if (a >= 4) {
        geq_define(u, set_font_cmd, null_font);
    } else {
        eq_define(u, set_font_cmd, null_font);
    }
    scan_optional_equals();
    /*tex Get the next non-blank non-call token. */
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));

    if (cur_cmd != left_brace_cmd) {
        back_input();
        scan_file_name();
        if (cur_area != get_nullstr() || cur_ext != get_nullstr()) {
            /*tex
                Have to do some rescue-ing here, fonts only have a name, no area
                nor extension.
            */
            old_setting = selector;
            selector = new_string;
            if (cur_area != get_nullstr()) {
                print(cur_area);
                flush_str(cur_area);
            }
            if (cur_name != get_nullstr()) {
                print(cur_name);
                flush_str(cur_name);
            }
            if (cur_ext != get_nullstr()) {
                print(cur_ext);
                flush_str(cur_ext);
            }
            selector = old_setting;
            cur_area = get_nullstr();
            cur_name = make_string();
            cur_ext = get_nullstr();
        }
    } else {
        back_input();
        (void) scan_toks(false, true);
        old_setting = selector;
        selector = new_string;
        token_show(def_ref);
        selector = old_setting;
        flush_list(def_ref);
        cur_area = get_nullstr();
        cur_name = make_string();
        cur_ext = get_nullstr();
    }
    /*tex
        Scan the font size specification. The next variable keeps |cur_name| from
        being changed
    */
    name_in_progress = true;
    if (scan_keyword("at")) {
        /*tex Put the positive `at' size into |s|. */
        scan_normal_dimen();
        s = cur_val;
        if ((s <= 0) || (s >= 01000000000)) {
            char err[256];
            const char *errhelp[] = {
                "I can only handle fonts at positive sizes that are",
                "less than 2048pt, so I've changed what you said to 10pt.",
                NULL
            };
            snprintf(err, 255, "Improper `at' size (%spt), replaced by 10pt", scaled_to_string(s));
            tex_error(err, errhelp);
            s = 10 * unity;
        }
    } else if (scan_keyword("scaled")) {
        scan_int();
        s = -cur_val;
        if ((cur_val <= 0) || (cur_val > 32768)) {
            char err[256];
            const char *errhelp[] = {
                "The magnification ratio must be between 1 and 32768.",
                NULL
            };
            snprintf(err, 255, "Illegal magnification has been changed to 1000 (%d)", (int) cur_val);
            tex_error(err, errhelp);
            s = -1000;
        }
    }
    /*tex
        There is no real reason to support this obsolete key as there are no useful
        fonts out there so let's get rid of this overhead. This also means that
        |natural_dir| can go away.
    */
    /*
    if (scan_keyword("naturaldir")) {
        scan_direction();
        natural_dir = cur_val;
    }
    */
    name_in_progress = false;
    fn = makecstring(cur_name);
    f = read_font_info(u, fn, s, natural_dir);
    xfree(fn);
    equiv(u) = f;
    eqtb[font_id_base + f] = eqtb[u];
    /*tex

        This is tricky: when we redefine a string we loose the old one. So this
        will change as it's only used to display the |\fontname| so we can store
        that with the font.

    */
    d = cs_text(font_id_base + f);
    t = (u >= null_cs) ? cs_text(u) : maketexstring("FONT");
    if (!d) {
        /*tex We have a new string. */
        cs_text(font_id_base + f) = t;
    } else if (str_eq_str(d,t)){
        /*tex We have the same string. */
        flush_str(t);
    } else {
        d = search_string(t);
        if (d) {
            /*tex We have already such a string. */
            cs_text(font_id_base + f) = d;
            flush_str(t);
        } else {
            /*tex The old value is lost but still in the pool. */
            cs_text(font_id_base + f) = t;
        }
    }
    if (cur_name == str_ptr-1) {
        flush_str(cur_name);
        cur_name = get_nullstr();
    }
}
