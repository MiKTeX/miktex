/* limglib.c

   Copyright 2006-2013 Taco Hoekwater <taco@luatex.org>

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
   with LuaTeX; if not, see <http://www.gnu.org/licenses/>. */

#include "ptexlib.h"
#include "lua/luatex-api.h"
#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"

#define img_types_max 7

const char *img_types[] = {
    "none",
    "pdf",
    "png",
    "jpg",
    "jp2",
    "jbig2",
    "stream",
    "memstream",
    NULL
};

static void copy_image(lua_State * L, lua_Number scale)
{
    image *a, **aa, *b, **bb;
    image_dict *d;
    if (lua_gettop(L) == 0)
        luaL_error(L, "img.copy needs an image as argument");
    aa = (image **) luaL_checkudata(L, 1, TYPE_IMG);       /* a */
    lua_pop(L, 1);                                         /* - */
    a = *aa;
    bb = (image **) lua_newuserdata(L, sizeof(image *));   /* b */
    luaL_getmetatable(L, TYPE_IMG);                        /* m b */
    lua_setmetatable(L, -2);                               /* b */
    b = *bb = new_image();
    if (!is_wd_running(a))
        img_width(b) = do_zround(img_width(a) * scale);
    if (!is_ht_running(a))
        img_height(b) = do_zround(img_height(a) * scale);
    if (!is_dp_running(a))
        img_depth(b) = do_zround(img_depth(a) * scale);
    img_transform(b) = img_transform(a);
    img_dict(b) = img_dict(a);
    if (img_dictref(a) != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, img_dictref(a)); /* ad b */
        img_dictref(b) = luaL_ref(L, LUA_REGISTRYINDEX);   /* b */
        d = img_dict(*aa);
        img_luaref(d) += 1;
    } else if (img_state(img_dict(a)) < DICT_REFERED) {
        luaL_error(L, "img.copy needs an proper image as argument");
    }
}

static void lua_to_image(lua_State * L, image * a, image_dict * d);

int l_new_image(lua_State * L)
{
    image *a, **aa;
    image_dict **add;
    if (lua_gettop(L) > 0 && ! lua_istable(L, -1)) {
        luaL_error(L, "img.new needs table as optional argument");  /* (t) */
    }
    aa = (image **) lua_newuserdata(L, sizeof(image *));            /* i (t) */
    luaL_getmetatable(L, TYPE_IMG);                                 /* m i (t) */
    lua_setmetatable(L, -2);                                        /* i (t) */
    a = *aa = new_image();
    add = (image_dict **) lua_newuserdata(L, sizeof(image_dict *)); /* ad i (t) */
    luaL_getmetatable(L, TYPE_IMG_DICT);                            /* m ad i (t) */
    lua_setmetatable(L, -2);                                        /* ad i (t) */
    img_dict(a) = *add = new_image_dict();
    img_dictref(a) = luaL_ref(L, LUA_REGISTRYINDEX);                /* i (t) */
    img_luaref(*add) += 1;
    if (lua_gettop(L) == 2) {                                       /* i t, else just i */
        lua_insert(L, -2);                                          /* t i */
        lua_pushnil(L);                                             /* n t i (1st key for iterator) */
        while (lua_next(L, -2) != 0) {                              /* v k t i */
            lua_to_image(L, a, *add);                               /* v k t i */
            lua_pop(L, 1);                                          /* k t i */
        }                                                           /* t i */
        lua_pop(L, 1);                                              /* i */
    }                                                               /* i */
    return 1;                                                       /* i */
}

static int l_copy_image(lua_State * L)
{
    if (lua_gettop(L) != 1) {
        luaL_error(L, "img.copy needs an image as argument");
    } else if (lua_istable(L, 1)) {
        (void) l_new_image(L);
    } else {
        (void) copy_image(L, 1.0);
    }
    return 1;
}

static void read_scale_img(image * a)
{
    image_dict *ad;
    if (a == NULL) {
        luaL_error(Luas, "the image scaler needs a valid image");
    } else {
        ad = img_dict(a);
        if (a == NULL) {
            luaL_error(Luas, "the image scaler needs a valid dictionary");
        } else {
            if (img_state(ad) == DICT_NEW) {
                if (img_type(ad) == IMG_TYPE_PDFSTREAM)
                    check_pdfstream_dict(ad);
                else {
                    read_img(ad);
                }
            }
            if ((img_type(ad) == IMG_TYPE_NONE) || (img_state(ad) == DICT_NEW)) {
                normal_warning("image","don't rely on the image data to be okay");
                img_width(a) = 0;
                img_height(a) = 0;
                img_depth(a) = 0;
            } else if (is_wd_running(a) || is_ht_running(a) || is_dp_running(a)) {
                img_dimen(a) = scale_img(ad, img_dimen(a), img_transform(a));
            }
        }
    }
}

static int l_scan_image(lua_State * L)
{
    image *a, **aa;
    if (lua_gettop(L) != 1)
        luaL_error(L, "img.scan needs exactly 1 argument");
    if (lua_istable(L, 1))
        (void) l_new_image(L);
    aa = (image **) luaL_checkudata(L, 1, TYPE_IMG);
    a = *aa;
    check_o_mode(static_pdf, "img.scan", 1 << OMODE_PDF, false);
    read_scale_img(a);
    return 1;
}

static halfword img_to_node(lua_State * L, image * a)
{
    image_dict *ad;
    halfword n = null;
    if (a == NULL) {
        luaL_error(L, "img.tonode needs a valid image");
    } else {
        ad = img_dict(a);
        if (a == NULL) {
            luaL_error(L, "img.tonode image has no dictionary");
        } else if (img_objnum(ad) == 0) {
            luaL_error(L, "img.tonode got image without object number");
        } else {
            n = new_rule(image_rule);
            rule_index(n) = img_index(ad);
            width(n) = img_width(a);
            height(n) = img_height(a);
            depth(n) = img_depth(a);
            rule_transform(n) = img_transform(a);
        }
    }
    return n;
}

typedef enum {
    WR_WRITE,
    WR_IMMEDIATEWRITE,
    WR_NODE,
    WR_VF_IMG
} wrtype_e;

const char *wrtype_s[] = {
    "img.write()",
    "img.immediatewrite()",
    "img.node()",
    "write vf image"
};

static void setup_image(PDF pdf, image * a, wrtype_e writetype)
{
    image_dict *ad;
    if (a == NULL)
        luaL_error(Luas, "no valid image passed"); /* todo, also check in caller */
    ad = img_dict(a);
    check_o_mode(pdf, wrtype_s[writetype], 1 << OMODE_PDF, false);
    read_scale_img(a);
    if (img_objnum(ad) == 0) { /* latest needed just before out_img() */
        pdf->ximage_count++;
        img_objnum(ad) = pdf_create_obj(pdf, obj_type_ximage, pdf->ximage_count);
        img_index(ad) = pdf->ximage_count;
        idict_to_array(ad); /* from now on ad is read-only */
        obj_data_ptr(pdf, pdf->obj_ptr) = img_index(ad);
    }
}

static void write_image_or_node(lua_State * L, wrtype_e writetype)
{
    image *a, **aa;
    image_dict *ad;
    halfword n;
    if (lua_gettop(L) != 1)
        luaL_error(L, "%s expects an argument", wrtype_s[writetype]);
    if (lua_istable(L, 1))
        (void) l_new_image(L);
    aa = (image **) luaL_checkudata(L, 1, TYPE_IMG);
    a = *aa;
    ad = img_dict(a);
    setup_image(static_pdf, a, writetype);
    switch (writetype) {
        case WR_WRITE:
            n = img_to_node(L, a);
            tail_append(n);
            break;
        case WR_IMMEDIATEWRITE:
            write_img(static_pdf, ad);
            break;
        case WR_NODE:
            lua_pop(L, 1); /* - */
            n = img_to_node(L, a);
            lua_nodelib_push_fast(L, n);
            break;
        default:
            luaL_error(L, "%s expects an valid image", wrtype_s[writetype]);
    }
    if (img_state(ad) < DICT_REFERED)
        img_state(ad) = DICT_REFERED;
}

static int l_write_image(lua_State * L)
{
    write_image_or_node(L, WR_WRITE);
    return 1;
}

static int l_immediatewrite_image(lua_State * L)
{
    check_o_mode(static_pdf, "img.immediatewrite", 1 << OMODE_PDF, true);
    if (global_shipping_mode != NOT_SHIPPING) {
        luaL_error(L, "img.immediatewrite can not be used with \\latelua");
    } else {
        write_image_or_node(L, WR_IMMEDIATEWRITE);
    }
    return 1;
}

static int l_image_node(lua_State * L)
{
    write_image_or_node(L, WR_NODE);
    return 1;
}

static int l_image_keys(lua_State * L)
{
    return lua_show_valid_keys(L, img_parms, img_parms_max);
}

static int l_image_types(lua_State * L)
{
    return lua_show_valid_list(L, img_types, img_types_max);
}

static int l_image_boxes(lua_State * L)
{
    return lua_show_valid_keys(L, img_pageboxes, img_pageboxes_max);
}

static const struct luaL_Reg imglib_f[] = {
    { "new", l_new_image },
    { "copy", l_copy_image },
    { "scan", l_scan_image },
    { "write", l_write_image },
    { "immediatewrite", l_immediatewrite_image },
    { "node", l_image_node },
    { "keys", l_image_keys },
    { "types", l_image_types },
    { "boxes", l_image_boxes },
    { NULL, NULL }
};

void vf_out_image(PDF pdf, unsigned i)
{
    image *a, **aa;
    image_dict *ad;
    lua_State *L = Luas;
    lua_rawgeti(L, LUA_REGISTRYINDEX, (int) i);
    aa = (image **) luaL_checkudata(L, -1, TYPE_IMG);
    a = *aa;
    ad = img_dict(a);
    if (ad == NULL) {
        luaL_error(L, "invalid image dictionary");
    }
    setup_image(pdf, a, WR_VF_IMG);
    place_img(pdf, ad, img_dimen(a), img_transform(a));
    lua_pop(L, 1);
}

/* metamethods for image */

static int m_img_get(lua_State * L)
{
    int j;
    const char *s;
    image **a = (image **) luaL_checkudata(L, 1, TYPE_IMG); /* k u */
    image_dict *d = img_dict(*a);
    if (d == NULL) {
        luaL_error(L, "invalid image dictionary");
    }
    s = lua_tostring(L, 2);
    if (lua_key_eq(s,width)) {
        if (is_wd_running(*a)) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, img_width(*a));
        }
    } else if (lua_key_eq(s,height)) {
        if (is_ht_running(*a)) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, img_height(*a));
        }
    } else if (lua_key_eq(s,depth)) {
        if (is_dp_running(*a)) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, img_depth(*a));
        }
    } else if (lua_key_eq(s,transform)) {
        lua_pushinteger(L, img_transform(*a));
    } else if (lua_key_eq(s,filename)) {
        if (img_filename(d) == NULL || strlen(img_filename(d)) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, img_filename(d));
        }
    } else if (lua_key_eq(s,visiblefilename)) {
        if (img_visiblefilename(d) == NULL || strlen(img_visiblefilename(d)) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, img_visiblefilename(d));
        }
    } else if (lua_key_eq(s,keepopen)) {
        lua_pushboolean(L, img_keepopen(d));
    } else if (lua_key_eq(s,filepath)) {
        if (img_filepath(d) == NULL || strlen(img_filepath(d)) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, img_filepath(d));
        }
    } else if (lua_key_eq(s,attr)) {
        if (img_attr(d) == NULL || strlen(img_attr(d)) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, img_attr(d));
        }
    } else if (lua_key_eq(s,page)) {
        if (img_pagename(d) != NULL && strlen(img_pagename(d)) != 0) {
            lua_pushstring(L, img_pagename(d));
        } else {
            lua_pushinteger(L, img_pagenum(d));
        }
    } else if (lua_key_eq(s,pages)) {
        lua_pushinteger(L, img_totalpages(d));
    } else if (lua_key_eq(s,xsize)) {
        if ((img_rotation(d) & 1) == 0) {
            lua_pushinteger(L, img_xsize(d));
        } else {
            lua_pushinteger(L, img_ysize(d));
        }
    } else if (lua_key_eq(s,ysize)) {
        if ((img_rotation(d) & 1) == 0) {
            lua_pushinteger(L, img_ysize(d));
        } else {
            lua_pushinteger(L, img_xsize(d));
        }
    } else if (lua_key_eq(s,xres)) {
        lua_pushinteger(L, img_xres(d));
    } else if (lua_key_eq(s,yres)) {
        lua_pushinteger(L, img_yres(d));
    } else if (lua_key_eq(s,rotation)) {
        lua_pushinteger(L, img_rotation(d));
    } else if (lua_key_eq(s,colorspace)) {
        if (img_colorspace(d) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, img_colorspace(d));
        }
    } else if (lua_key_eq(s,colordepth)) {
        if (img_colordepth(d) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, img_colordepth(d));
        }
    } else if (lua_key_eq(s,imagetype)) {
        j = img_type(d);
        if (j >= 0 && j <= img_types_max) {
            if (j == IMG_TYPE_NONE) {
                lua_pushnil(L);
            } else {
                lua_pushstring(L, img_types[j]);
            }
        } else {
            lua_pushnil(L);
        }
    } else if (lua_key_eq(s,pagebox)) {
        j = img_pagebox(d);
        if (j < 0 || j >= img_pageboxes_max) {
            j = 0;
        }
        lua_push_img_pagebox(L, j);
    } else if (lua_key_eq(s,bbox)) {
        if (!img_is_bbox(d)) {
            img_bbox(d)[0] = img_xorig(d);
            img_bbox(d)[1] = img_yorig(d);
            img_bbox(d)[2] = img_xorig(d) + img_xsize(d);
            img_bbox(d)[3] = img_yorig(d) + img_ysize(d);
        }
        lua_newtable(L);
        lua_pushinteger(L, 1);
        lua_pushinteger(L, img_bbox(d)[0]);
        lua_settable(L, -3);
        lua_pushinteger(L, 2);
        lua_pushinteger(L, img_bbox(d)[1]);
        lua_settable(L, -3);
        lua_pushinteger(L, 3);
        lua_pushinteger(L, img_bbox(d)[2]);
        lua_settable(L, -3);
        lua_pushinteger(L, 4);
        lua_pushinteger(L, img_bbox(d)[3]);
        lua_settable(L, -3);
    } else if (lua_key_eq(s,objnum)) {
        if (img_objnum(d) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, img_objnum(d));
        }
    } else if (lua_key_eq(s,index)) {
        if (img_index(d) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushinteger(L, img_index(d));
        }
    } else if (lua_key_eq(s,stream)) {
        if (img_type(d) != IMG_TYPE_PDFSTREAM
                || img_pdfstream_ptr(d) == NULL
                || img_pdfstream_stream(d) == NULL
                || strlen(img_pdfstream_stream(d)) == 0) {
            lua_pushnil(L);
        } else {
            lua_pushstring(L, img_pdfstream_stream(d));
        }
    } else if (lua_key_eq(s,ref_count)) {
        lua_pushinteger(L, img_luaref(d));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static void lua_to_image(lua_State * L, image * a, image_dict * d)
{
    int i, t;
    const char *s;
    s = lua_tostring(L,-2);
    t = lua_type(L, -1);
    if (lua_key_eq(s,width)) {
        if (t == LUA_TNIL) {
            set_wd_running(a);
        } else if (t == LUA_TNUMBER) {
            img_width(a) = (int) lua_tointeger(L, -1);
        } else if (t == LUA_TSTRING) {
            img_width(a) = dimen_to_number(L, lua_tostring(L, -1));
        } else {
            luaL_error(L, "image.width needs integer or nil value or dimension string");
        }
    } else if (lua_key_eq(s,height)) {
        if (t == LUA_TNIL) {
            set_ht_running(a);
        } else if (t == LUA_TNUMBER) {
            img_height(a) = (int) lua_tointeger(L, -1);
        } else if (t == LUA_TSTRING) {
            img_height(a) = dimen_to_number(L, lua_tostring(L, -1));
        } else {
            luaL_error(L, "image.height needs integer or nil value or dimension string");
        }
    } else if (lua_key_eq(s,depth)) {
        if (t == LUA_TNIL) {
            set_dp_running(a);
        } else if (t == LUA_TNUMBER) {
            img_depth(a) = (int) lua_tointeger(L, -1);
        } else if (t == LUA_TSTRING) {
            img_depth(a) = dimen_to_number(L, lua_tostring(L, -1));
        } else {
            luaL_error(L, "image.depth needs integer or nil value or dimension string");
        }
    } else if (lua_key_eq(s,transform)) {
        if (t == LUA_TNUMBER) {
            img_transform(a) = (int) lua_tointeger(L, -1);
        } else {
            luaL_error(L, "image.transform needs integer value");
        }
    } else if (lua_key_eq(s,filename)) {
        if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.filename is now read-only");
        } else if (img_type(d) == IMG_TYPE_PDFSTREAM) {
            luaL_error(L, "image.filename can't be used with image.stream");
        } else if (t == LUA_TSTRING) {
            xfree(img_filename(d));
            img_filename(d) = xstrdup(lua_tostring(L, -1));
        } else {
            luaL_error(L, "image.filename needs string value");
        }
    } else if (lua_key_eq(s,visiblefilename)) {
        if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.visiblefilename is now read-only");
        } else if (img_type(d) == IMG_TYPE_PDFSTREAM) {
            luaL_error(L, "image.visiblefilename can't be used with image.stream");
        } else if (t == LUA_TSTRING) {
            xfree(img_visiblefilename(d));
            img_visiblefilename(d) = xstrdup(lua_tostring(L, -1));
        } else {
            luaL_error(L, "image.visiblefilename needs string value");
        }
    } else if (lua_key_eq(s,attr)) {
        if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.attr is now read-only");
        } else if (t == LUA_TSTRING) {
            xfree(img_attr(d));
            img_attr(d) = xstrdup(lua_tostring(L, -1));
        } else if (t == LUA_TNIL) {
            xfree(img_attr(d));
        } else {
            luaL_error(L, "image.attr needs string or nil value");
        }
    } else if (lua_key_eq(s,page)) {
        if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.page is now read-only");
        } else if (t == LUA_TSTRING) {
            xfree(img_pagename(d));
            img_pagename(d) = xstrdup(lua_tostring(L, -1));
            img_pagenum(d) = 0;
        } else if (t == LUA_TNUMBER) {
            img_pagenum(d) = (int) lua_tointeger(L, -1);
            xfree(img_pagename(d));
        } else {
            luaL_error(L, "image.page needs integer or string value");
        }
    } else if (lua_key_eq(s,colorspace)) {
        if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.colorspace is now read-only");
        } else if (t == LUA_TNIL) {
            img_colorspace(d) = 0;
        } else if (t == LUA_TNUMBER) {
            img_colorspace(d) = (int) lua_tointeger(L, -1);
        } else {
            luaL_error(L, "image.colorspace needs integer or nil value");
        }
    } else if (lua_key_eq(s,pagebox)) {
        if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.pagebox is now read-only");
        } else if (t == LUA_TNIL) {
            img_pagebox(d) = PDF_BOX_SPEC_MEDIA;
        } else if (t == LUA_TNUMBER) {
            i = lua_tointeger(L,-1);
            if (i < 0 || i >= img_pageboxes_max) {
                img_pagebox(d) = PDF_BOX_SPEC_MEDIA;
            } else {
                img_pagebox(d) = i;
            }
        } else if (t == LUA_TSTRING) {
            img_pagebox(d) = PDF_BOX_SPEC_MEDIA;
            for (i = 0; i < img_pageboxes_max; i++) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, img_pageboxes[i]);
                if (lua_rawequal(L,-1,-2)) {
                    img_pagebox(d) = i;
                    lua_pop(L, 1);
                    break;
                } else {
                    lua_pop(L, 1);
                }
            }
        } else {
            luaL_error(L, "image.pagebox needs string, number or nil value");
        }
    } else if (lua_key_eq(s,keepopen)) {
        if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.keepopen is now read-only");
        } else if (t != LUA_TBOOLEAN) {
            luaL_error(L, "image.bbox needs boolean value");
        } else {
            img_keepopen(d) = lua_toboolean(L, -1);
        }
    } else if (lua_key_eq(s,bbox)) {
        if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.bbox is now read-only");
        } else if (t != LUA_TTABLE) {
            luaL_error(L, "image.bbox needs table value");
        } else if (lua_rawlen(L, -1) != 4) {
            luaL_error(L, "image.bbox table must have exactly 4 elements");
        } else {
            for (i = 1; i <= 4; i++) {      /* v k t ... */
                lua_pushinteger(L, i);      /* idx v k t ... */
                lua_gettable(L, -2);        /* int v k t ... */
                t = lua_type(L, -1);
                if (t == LUA_TNUMBER) {
                    img_bbox(d)[i - 1] = (int) lua_tointeger(L, -1);
                } else if (t == LUA_TSTRING) {
                    img_bbox(d)[i - 1] = dimen_to_number(L, lua_tostring(L, -1));
                } else {
                    luaL_error(L, "image.bbox table needs integer value or dimension string elements");
                }
                lua_pop(L, 1);      /* v k t ... */
            }
            img_set_bbox(d);
        }
    } else if (lua_key_eq(s,stream)) {
        if (img_filename(d) != NULL) {
            luaL_error(L, "image.stream can't be used with image.filename");
        } else if (img_state(d) >= DICT_FILESCANNED) {
            luaL_error(L, "image.stream is now read-only");
        } else {
            if (img_pdfstream_ptr(d) == NULL) {
                new_img_pdfstream_struct(d);
            }
            xfree(img_pdfstream_stream(d));
            img_pdfstream_stream(d) = xstrdup(lua_tostring(L, -1));
            img_type(d) = IMG_TYPE_PDFSTREAM;
        }
    } else {
        luaL_error(L, "image.%s can not be set", s);
    }
}

static int m_img_set(lua_State * L)
{
    image **a = (image **) luaL_checkudata(L, 1, TYPE_IMG); /* v k u */
    image_dict *d = img_dict(*a);
    if (d == NULL) {
        luaL_error(L, "invalid image dictionary");
    } else {
        lua_to_image(L, *a, d);
    }
    return 0;
}

static int m_img_mul(lua_State * L)
{
    lua_Number scale;
    if (lua_type(L, 1) == LUA_TNUMBER) {         /* u? n */
        (void) luaL_checkudata(L, 2, TYPE_IMG);  /* u n */
        lua_insert(L, -2);                       /* n a */
    } else if (lua_type(L, 2) != LUA_TNUMBER) {  /* n u? */
        (void) luaL_checkudata(L, 1, TYPE_IMG);  /* n a */
    }                                            /* n a */
    scale = lua_tonumber(L, 2); /* float */      /* n a */
    lua_pop(L, 1);                               /* a */
    copy_image(L, scale);                        /* b */
    return 1;
}

static int m_img_print(lua_State * L)
{
    image **aa;
    image_dict *d;
    aa = (image **) luaL_checkudata(L, 1, TYPE_IMG);
    d = img_dict(*aa);
    /* formatted a bit like a node */
    if (img_filename(d) == NULL) {
        if (img_pagename(d) != NULL && strlen(img_pagename(d)) != 0) {
            lua_pushfstring(L, "<img unset : %d >", img_dictref(*aa));
        } else {
            lua_pushfstring(L, "<img unset : %d >", img_dictref(*aa));
        }
    } else {
        if (img_pagename(d) != NULL && strlen(img_pagename(d)) != 0) {
            lua_pushfstring(L, "<img %s : %s : %d >", img_filename(d), img_pagename(d), img_dictref(*aa));
        } else {
            lua_pushfstring(L, "<img %s : %d : %d >", img_filename(d), img_pagenum(d), img_dictref(*aa));
        }
    }
    return 1;
}

/* this finalizes instance */

static int m_img_gc(lua_State * L)
{
    image *a, **aa;
    image_dict *d;
    aa = (image **) luaL_checkudata(L, 1, TYPE_IMG);
    a = *aa;
    d = img_dict(*aa);
    luaL_unref(L, LUA_REGISTRYINDEX, img_dictref(a));
    img_luaref(d) -= 1;
    /* we need to check this */
    if (!img_is_refered(d)) {
        xfree(a);
    }
    /* till here */
    return 0;
}

/* the instance */

static const struct luaL_Reg img_m[] = {
    {"__index", m_img_get},
    {"__newindex", m_img_set},
    {"__mul", m_img_mul},
    {"__tostring", m_img_print},
    {"__gc", m_img_gc},
    {NULL, NULL}
};

/* this finalizes the dict */

static int m_img_dict_gc(lua_State * L)
{
    image_dict *ad, **add;
    add = (image_dict **) luaL_checkudata(L, 1, TYPE_IMG_DICT);
    ad = *add;
    if (img_luaref(ad) > 0) {
        luaL_error(L, "disposing image dict that has references");
    } else {
        /* we need to check this */
        if (img_state(ad) < DICT_REFERED) {
            free_image_dict(ad);
        }
        /* till here */
    }
    return 0;
}

/* the (shared) dict */

static const struct luaL_Reg img_dict_m[] = {
    {"__gc", m_img_dict_gc},
    {NULL, NULL}
};

int luaopen_img(lua_State * L)
{
    luaL_newmetatable(L, TYPE_IMG);
#ifdef LuajitTeX
    luaL_register(L, NULL, img_m);
    luaL_newmetatable(L, TYPE_IMG_DICT);
    luaL_register(L, NULL, img_dict_m);
    luaL_register(L, "img", imglib_f);
#else
    luaL_setfuncs(L, img_m, 0);
    luaL_newmetatable(L, TYPE_IMG_DICT);
    luaL_setfuncs(L, img_dict_m, 0);
    luaL_newlib(L, imglib_f);
#endif
    return 1;
}
