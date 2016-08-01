% dofont.w
%
% Copyright 2006-2010 Taco Hoekwater <taco@@luatex.org>
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
#include "lua/luatex-api.h"

@ a bit more interfacing is needed for proper error reporting

@c
static char *font_error_message(pointer u, char *nom, scaled s)
{
    char *str = xmalloc(256);
    char *c = makecstring(cs_text(u));
    const char *extra = "metric data not found or bad";
    if (s >= 0) {
        snprintf(str, 255, "Font \\%s=%s at %gpt not loadable: %s", c, nom,
                 (double) s / 65536, extra);
    } else if (s != -1000) {
        snprintf(str, 255, "Font \\%s=%s scaled %d not loadable: %s", c, nom,
                 (int) (-s), extra);
    } else {
        snprintf(str, 255, "Font \\%s=%s not loadable: %s", c, nom, extra);
    }
    free(c);
    return str;
}

static int do_define_font(int f, const char *cnom, scaled s, int natural_dir)
{

    boolean res;                /* was the callback successful? */
    int callback_id;
    char *cnam;
    int r, t;
    res = 0;

    callback_id = callback_defined(define_font_callback);
    if (callback_id > 0) {
        cnam = xstrdup(cnom);
        callback_id = run_and_save_callback(callback_id, "Sdd->", cnam, s, f);
        free(cnam);
        if (callback_id > 0) {  /* success */
            luaL_checkstack(Luas, 1, "out of stack space");
            lua_rawgeti(Luas, LUA_REGISTRYINDEX, callback_id);
            t = lua_type(Luas, -1);
            if (t == LUA_TTABLE) {
                res = font_from_lua(Luas, f);
                destroy_saved_callback(callback_id);
                /* |lua_pop(Luas, 1);| *//* done by |font_from_lua| */
            } else if (t == LUA_TNUMBER) {
                r = (int) lua_tointeger(Luas, -1);
                destroy_saved_callback(callback_id);
                delete_font(f);
                lua_pop(Luas, 1);
                return r;
            } else {
                lua_pop(Luas, 1);
                delete_font(f);
                return 0;
            }
        }
    } else if (callback_id == 0) {
        res = read_tfm_info(f, cnom, s);
        if (res) {
            set_hyphen_char(f, default_hyphen_char_par);
            set_skew_char(f, default_skew_char_par);
        }
    }
    if (font_name(f) && strlen(font_name(f)) > 255) {
        /* the font name has to fit in the dvi file's single byte storage */
        /* no need to test area, as we are never using it */
        res = 0;
    }
    if (res) {
        if (font_type(f) != virtual_font_type) {        /* implies lua */
            do_vf(f);
            set_font_natural_dir(f, natural_dir);
        }
        return f;
    } else {
        delete_font(f);
        return 0;
    }

}

int read_font_info(pointer u, char *cnom, scaled s, int natural_dir)
{
    int f;
    char *msg;

    f = new_font();
    if ((f = do_define_font(f, cnom, s, natural_dir))) {
        return f;
    } else {
        const char *help[] =
            { "I wasn't able to read the size data for this font,",
            "so I will ignore the font specification.",
            "[Wizards can fix TFM files using TFtoPL/PLtoTF.]",
            "You might try inserting a different font spec;",
            "e.g., type `I\\font<same font id>=<substitute font name>'.",
            NULL
        };
        if (suppress_fontnotfound_error_par == 0) {
            msg = font_error_message(u, cnom, s);
            tex_error(msg, help);
            free(msg);
        }
        return 0;
    }
}

@ TODO This function is a placeholder. There can easily appears holes in
   the |font_tables| array, and we could attempt to reuse those

@c
int find_font_id(const char *nom, scaled s)
{
    int f;
    f = new_font();
    if ((f = do_define_font(f, nom, s, -1))) {
        return f;
    } else {
        return 0;
    }
}
