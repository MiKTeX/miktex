% texdeffont.w
%
% Copyright 2008-2010 Taco Hoekwater <taco@@luatex.org>
%
% This file is part of LuaTeX.
%
% LuaTeX is free software; you can redistribute it and/or modify it under
% the terms of the GNU General Public License as published by the Free
% Software Foundation; either version 2 of the License, or (at your
% option) any later version.
%
% LuaTeX is distributed in the hope that it will be useful, but WITHOUT
% ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
% FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
% License for more details.
%
% You should have received a copy of the GNU General Public License along
% with LuaTeX; if not, see <http://www.gnu.org/licenses/>.

@ @c


#include "ptexlib.h"

@ When the user defines \.{\\font\\f}, say, \TeX\ assigns an internal number
to the user's font~\.{\\f}. Adding this number to |font_id_base| gives the
|eqtb| location of a ``frozen'' control sequence that will always select
the font.

@c
int font_bytes;

void set_cur_font(internal_font_number f)
{
    int a = 0;                  /* never global */
    define(cur_font_loc, data_cmd, f);
}

@ @c
static char *scaled_to_string(scaled s)
{                               /* prints scaled real, rounded to five digits */
    static char result[16];
    int n, k;
    scaled delta;               /* amount of allowable inaccuracy */
    k = 0;
    if (s < 0) {
        result[k++] = '-';
        s = -s;                 /* print the sign, if negative */
    }
    {
        int l = 0;
        char digs[8] = { 0 };
        n = s / unity;
        /* process the integer part */
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
        if (delta > unity)
            s = s + 0100000 - 050000;   /* round the last digit */
        result[k++] = (char) ('0' + (s / unity));
        s = 10 * (s % unity);
        delta = delta * 10;
    } while (s > delta);

    result[k] = 0;
    return (char *) result;
}

@ @c
void tex_def_font(small_number a)
{
    pointer u;                  /* user's font identifier */
    internal_font_number f;     /* runs through existing fonts */
    str_number t;               /* name for the frozen font identifier */
    int old_setting;            /* holds |selector| setting */
    scaled s = -1000;           /* stated ``at'' size, or negative of scaled magnification */
    int natural_dir = -1;       /* the natural direction of the font */
    char *fn;
    if (job_name == 0)
        open_log_file();        /* avoid confusing \.{texput} with the font name */
    get_r_token();
    u = cur_cs;
    if (u >= null_cs)
        t = cs_text(u);
    else
        t = maketexstring("FONT");
    if (a >= 4) {
        geq_define(u, set_font_cmd, null_font);
    } else {
        eq_define(u, set_font_cmd, null_font);
    }
    scan_optional_equals();
    /* Get the next non-blank non-call token; */
    do {
        get_x_token();
    } while ((cur_cmd == spacer_cmd) || (cur_cmd == relax_cmd));

    if (cur_cmd != left_brace_cmd) {
        back_input();
        scan_file_name();
        if (cur_area != get_nullstr() || cur_ext != get_nullstr()) {
            /* Have to do some rescue-ing here, fonts only have a name,
               no area nor extension */
            old_setting = selector;
            selector = new_string;
            if (cur_area != get_nullstr()) {
                print(cur_area);
            }
            if (cur_name != get_nullstr()) {
                print(cur_name);
            }
            if (cur_ext != get_nullstr()) {
                print(cur_ext);
            }
            selector = old_setting;
            cur_name = make_string();
            cur_ext = get_nullstr();
            cur_area = get_nullstr();
        }
    } else {
        back_input();
        (void) scan_toks(false, true);
        old_setting = selector;
        selector = new_string;
        token_show(def_ref);
        selector = old_setting;
        flush_list(def_ref);
        /* |str_room(1)|; *//* what did that do ? */
        cur_name = make_string();
        cur_ext = get_nullstr();
        cur_area = get_nullstr();
    }
    /* Scan the font size specification; */
    name_in_progress = true;    /* this keeps |cur_name| from being changed */
    if (scan_keyword("at")) {
        /* Put the positive `at' size into |s| */
        scan_normal_dimen();
        s = cur_val;
        if ((s <= 0) || (s >= 01000000000)) {
            char err[256];
            const char *errhelp[] =
                { "I can only handle fonts at positive sizes that are",
                "less than 2048pt, so I've changed what you said to 10pt.",
                NULL
            };
            snprintf(err, 255, "Improper `at' size (%spt), replaced by 10pt",
                     scaled_to_string(s));
            tex_error(err, errhelp);
            s = 10 * unity;
        }
    } else if (scan_keyword("scaled")) {
        scan_int();
        s = -cur_val;
        if ((cur_val <= 0) || (cur_val > 32768)) {
            char err[256];
            const char *errhelp[] =
                { "The magnification ratio must be between 1 and 32768.",
                NULL
            };
            snprintf(err, 255,
                     "Illegal magnification has been changed to 1000 (%d)",
                     (int) cur_val);
            tex_error(err, errhelp);
            s = -1000;
        }
    }
    if (scan_keyword("naturaldir")) {
        scan_direction();
        natural_dir = cur_val;
    }
    name_in_progress = false;
    fn = makecstring(cur_name);
    f = read_font_info(u, fn, s, natural_dir);
    xfree(fn);
    equiv(u) = f;

    eqtb[font_id_base + f] = eqtb[u];
    cs_text(font_id_base + f) = t;
}
