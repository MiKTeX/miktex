/* t1fontskel.cc -- Type 1 font skeleton
 *
 * Copyright (c) 1998-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <efont/t1font.hh>
#include <efont/t1item.hh>
#include <efont/t1rw.hh>
#include <efont/t1mm.hh>
#include <lcdf/error.hh>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
namespace Efont {

#if HAVE_ADOBE_CODE

static const char* othersubrs_code =
// This version of the OtherSubrs code for old PostScript interpreters comes from
// https://github.com/adobe-type-tools/afdko/blob/master/FDK/Tools/Programs/public/lib/source/t1write/t1write_flexothers.txt
// (commit 3f4eeec). The Adobe Font Development Kit is licensed by the Apache
// License, 2.0, which permits this embedding.
"% Copyright 1987-1990 Adobe Systems Incorporated. All rights reserved.\n"
"/OtherSubrs[systemdict/internaldict known{1183615869 systemdict/internaldict\n"
"get exec/FlxProc known{save true}{false}ifelse}{userdict/internaldict known\n"
"not{userdict/internaldict{count 0 eq{/internaldict errordict/invalidaccess get\n"
"exec}if dup type/integertype ne{/internaldict errordict/invalidaccess get exec\n"
"}if dup 1183615869 eq{pop 0}{/internaldict errordict/invalidaccess get exec}\n"
"ifelse}dup 14 get 1 25 dict put bind executeonly put}if 1183615869 userdict\n"
"/internaldict get exec/FlxProc known{save true}{false}ifelse}ifelse[systemdict\n"
"/internaldict known not{100 dict/begin cvx/mtx matrix/def cvx}if systemdict\n"
"/currentpacking known{currentpacking true setpacking}if{systemdict\n"
"/internaldict known{1183615869 systemdict/internaldict get exec dup/$FlxDict\n"
"known not{dup dup length exch maxlength eq{pop userdict dup/$FlxDict known not\n"
"{100 dict begin/mtx matrix def dup/$FlxDict currentdict put end}if}{100 dict\n"
"begin/mtx matrix def dup/$FlxDict currentdict put end}ifelse}if/$FlxDict get\n"
"begin}if grestore/exdef{exch def}def/dmin exch abs 100 div def/epX exdef/epY\n"
"exdef/c4y2 exdef/c4x2 exdef/c4y1 exdef/c4x1 exdef/c4y0 exdef/c4x0 exdef/c3y2\n"
"exdef/c3x2 exdef/c3y1 exdef/c3x1 exdef/c3y0 exdef/c3x0 exdef/c1y2 exdef/c1x2\n"
"exdef/c2x2 c4x2 def/c2y2 c4y2 def/yflag c1y2 c3y2 sub abs c1x2 c3x2 sub abs gt\n"
"def/PickCoords{{c1x0 c1y0 c1x1 c1y1 c1x2 c1y2 c2x0 c2y0 c2x1 c2y1 c2x2 c2y2}{\n"
"c3x0 c3y0 c3x1 c3y1 c3x2 c3y2 c4x0 c4y0 c4x1 c4y1 c4x2 c4y2}ifelse/y5 exdef/x5\n"
"exdef/y4 exdef/x4 exdef/y3 exdef/x3 exdef/y2 exdef/x2 exdef/y1 exdef/x1 exdef\n"
"/y0 exdef/x0 exdef}def mtx currentmatrix pop mtx 0 get abs 1e-05 lt mtx 3 get\n"
"abs 1e-05 lt or{/flipXY -1 def}{mtx 1 get abs 1e-05 lt mtx 2 get abs 1e-05 lt\n"
"or{/flipXY 1 def}{/flipXY 0 def}ifelse}ifelse/erosion 1 def systemdict\n"
"/internaldict known{1183615869 systemdict/internaldict get exec dup/erosion\n"
"known{/erosion get/erosion exch def}{pop}ifelse}if yflag{flipXY 0 eq c3y2 c4y2\n"
"eq or{false PickCoords}{/shrink c3y2 c4y2 eq{0}{c1y2 c4y2 sub c3y2 c4y2 sub\n"
"div abs}ifelse def/yshrink{c4y2 sub shrink mul c4y2 add}def/c1y0 c3y0 yshrink\n"
"def/c1y1 c3y1 yshrink def/c2y0 c4y0 yshrink def/c2y1 c4y1 yshrink def/c1x0\n"
"c3x0 def/c1x1 c3x1 def/c2x0 c4x0 def/c2x1 c4x1 def/dY 0 c3y2 c1y2 sub round\n"
"dtransform flipXY 1 eq{exch}if pop abs def dY dmin lt PickCoords y2 c1y2 sub\n"
"abs .001 gt{c1x2 c1y2 transform flipXY 1 eq{exch}if/cx exch def/cy exch def/dY\n"
"0 y2 c1y2 sub round dtransform flipXY 1 eq{exch}if pop def dY round dup 0 ne{\n"
"/dY exdef}{pop dY 0 lt{-1}{1}ifelse/dY exdef}ifelse/erode PaintType 2 ne\n"
"erosion .5 ge and def erode{/cy cy .5 sub def}if/ey cy dY add def/ey ey\n"
"ceiling ey sub ey floor add def erode{/ey ey .5 add def}if ey cx flipXY 1 eq{\n"
"exch}if itransform exch pop y2 sub/eShift exch def/y1 y1 eShift add def/y2 y2\n"
"eShift add def/y3 y3 eShift add def}if}ifelse}{flipXY 0 eq c3x2 c4x2 eq or{\n"
"false PickCoords}{/shrink c3x2 c4x2 eq{0}{c1x2 c4x2 sub c3x2 c4x2 sub div abs}\n"
"ifelse def/xshrink{c4x2 sub shrink mul c4x2 add}def/c1x0 c3x0 xshrink def/c1x1\n"
"c3x1 xshrink def/c2x0 c4x0 xshrink def/c2x1 c4x1 xshrink def/c1y0 c3y0 def\n"
"/c1y1 c3y1 def/c2y0 c4y0 def/c2y1 c4y1 def/dX c3x2 c1x2 sub round 0 dtransform\n"
"flipXY -1 eq{exch}if pop abs def dX dmin lt PickCoords x2 c1x2 sub abs .001 gt\n"
"{c1x2 c1y2 transform flipXY -1 eq{exch}if/cy exch def/cx exch def/dX x2 c1x2\n"
"sub round 0 dtransform flipXY -1 eq{exch}if pop def dX round dup 0 ne{/dX\n"
"exdef}{pop dX 0 lt{-1}{1}ifelse/dX exdef}ifelse/erode PaintType 2 ne erosion\n"
".5 ge and def erode{/cx cx .5 sub def}if/ex cx dX add def/ex ex ceiling ex sub\n"
"ex floor add def erode{/ex ex .5 add def}if ex cy flipXY -1 eq{exch}if\n"
"itransform pop x2 sub/eShift exch def/x1 x1 eShift add def/x2 x2 eShift add\n"
"def/x3 x3 eShift add def}if}ifelse}ifelse x2 x5 eq y2 y5 eq or{x5 y5 lineto}{\n"
"x0 y0 x1 y1 x2 y2 curveto x3 y3 x4 y4 x5 y5 curveto}ifelse epY epX}systemdict\n"
"/currentpacking known{exch setpacking}if/exec cvx/end cvx]cvx executeonly exch\n"
"{pop true exch restore}{systemdict/internaldict known not{1183615869 userdict\n"
"/internaldict get exec exch/FlxProc exch put true}{1183615869 systemdict\n"
"/internaldict get exec dup length exch maxlength eq{false}{1183615869\n"
"systemdict/internaldict get exec exch/FlxProc exch put true}ifelse}ifelse}\n"
"ifelse{systemdict/internaldict known{{1183615869 systemdict/internaldict get\n"
"exec/FlxProc get exec}}{{1183615869 userdict/internaldict get exec/FlxProc get\n"
"exec}}ifelse executeonly}if{gsave currentpoint newpath moveto}executeonly{\n"
"currentpoint grestore gsave currentpoint newpath moveto}executeonly{systemdict\n"
"/internaldict known not{pop 3}{1183615869 systemdict/internaldict get exec dup\n"
"/startlock known{/startlock get exec}{dup/strtlck known{/strtlck get exec}{pop\n"
"3}ifelse}ifelse}ifelse}executeonly]def";

#else

static const char* othersubrs_code = "/OtherSubrs\n"
"[\n"
"{pop pop pop grestore 12 6 roll curveto curveto pop pop} executeonly\n"
"{gsave currentpoint newpath moveto} executeonly\n"
"{currentpoint currentpoint newpath moveto} executeonly\n"
"{} executeonly\n"
"] noaccess def";

#endif


Type1Font *
Type1Font::skeleton_make(PermString font_name, const String &version)
{
    Type1Font *output = new Type1Font(font_name);

    // %!PS-Adobe-Font comment
    StringAccum sa;
    sa << "%!PS-AdobeFont-1.0: " << font_name;
    if (version)
        sa << ' ' << version;
    output->add_item(new Type1CopyItem(sa.take_string()));

    output->_dict_deltas[dF] = 3; // Private, FontInfo, Encoding
    output->_dict_deltas[dP] = 3; // OtherSubrs, Subrs, CharStrings

    return output;
}

void
Type1Font::skeleton_comments_end()
{
    // count members of font dictionary
    add_definition(dF, new Type1Definition("FontName", "/" + String(_font_name), "def"));
}

void
Type1Font::skeleton_fontinfo_end()
{
    if (first_dict_item(Type1Font::dFI) >= 0)
        add_item(new Type1CopyItem("end readonly def"));
    else
        add_item(new Type1CopyItem("% no FontInfo dict"));
}

void
Type1Font::skeleton_fontdict_end()
{
    // switch to eexec
    add_item(new Type1CopyItem("currentdict end"));
    add_item(new Type1EexecItem(true));

    // Private dictionary
    add_definition(Type1Font::dP, Type1Definition::make_literal("-|", "{string currentfile exch readstring pop}", "executeonly def"));
    set_charstring_definer(" -| ");
    add_definition(Type1Font::dP, Type1Definition::make_literal("|-", "{noaccess def}", "executeonly def"));
    add_definition(Type1Font::dP, Type1Definition::make_literal("|", "{noaccess put}", "executeonly def"));
}

void
Type1Font::skeleton_private_end()
{
    add_item(new Type1CopyItem(othersubrs_code));

    // Subrs
    add_item(new Type1SubrGroupItem(this, true, "/Subrs 0 array"));
    add_item(new Type1CopyItem("|-"));

    // CharStrings
    add_item(new Type1SubrGroupItem(this, false, "2 index /CharStrings 0 dict dup begin"));

    // completion
    add_item(new Type1CopyItem("end\n\
end\n\
readonly put\n\
noaccess put\n\
dup /FontName get exch definefont pop\n\
mark currentfile closefile"));
    add_item(new Type1EexecItem(false));
    add_item(new Type1CopyItem("\
0000000000000000000000000000000000000000000000000000000000000000\n\
0000000000000000000000000000000000000000000000000000000000000000\n\
0000000000000000000000000000000000000000000000000000000000000000\n\
0000000000000000000000000000000000000000000000000000000000000000\n\
0000000000000000000000000000000000000000000000000000000000000000\n\
0000000000000000000000000000000000000000000000000000000000000000\n\
0000000000000000000000000000000000000000000000000000000000000000\n\
0000000000000000000000000000000000000000000000000000000000000000\n\
cleartomark"));
}

void
Type1Font::skeleton_common_subrs()
{
    // - first four Subrs have fixed definitions
    // - 0: "3 0 callothersubr pop pop setcurrentpoint return"
    set_subr(0, Type1Charstring(String::make_stable("\216\213\014\020\014\021\014\021\014\041\013", 11)), " |");
    // - 1: "0 1 callothersubr return"
    set_subr(1, Type1Charstring(String::make_stable("\213\214\014\020\013", 5)), " |");
    // - 2: "0 2 callothersubr return"
    set_subr(2, Type1Charstring(String::make_stable("\213\215\014\020\013", 5)), " |");
    // - 3: "return"
    set_subr(3, Type1Charstring(String::make_stable("\013", 1)), " |");
    // - 4: "1 3 callothersubr pop callsubr return"
    set_subr(4, Type1Charstring(String::make_stable("\214\216\014\020\014\021\012\013", 8)), " |");
}


static void
add_number_def(Type1Font *output, int dict, PermString name, const Type1Font *font)
{
    double v;
    if (Type1Definition *t1d = font->dict(dict, name))
        if (t1d->value_num(v))
            output->add_definition(dict, Type1Definition::make(name, v, "def"));
}

static void
add_copy_def(Type1Font *output, int dict, PermString name, const Type1Font *font, const char *definer = "def")
{
    if (Type1Definition *t1d = font->dict(dict, name))
        output->add_definition(dict, Type1Definition::make_literal(name, t1d->value(), definer));
}

static String
font_dict_string(const Type1Font *font, int dict, PermString name)
{
    String s;
    if (Type1Definition *d = font->dict(dict, name))
        if (d->value_string(s))
            return s;
    return String();
}

Type1Font *
Type1Font::skeleton_make_copy(const Type1Font *font, PermString font_name, const Vector<double> *xuid_extension)
{
    String version = font_dict_string(font, dFI, "version");
    Type1Font *output = skeleton_make(font_name, version);

    // other comments from font header
    for (int i = 0; i < font->nitems(); i++)
        if (Type1CopyItem *c = font->item(i)->cast_copy()) {
            if (c->length() > 1 && c->value()[0] == '%') {
                if (c->value()[1] != '!')
                    output->add_item(new Type1CopyItem(c->value()));
            } else
                break;
        } else
            break;

    output->skeleton_comments_end();

    // FontInfo dictionary
    if (version)
        output->add_definition(dFI, Type1Definition::make_string("version", version, "readonly def"));
    if (String s = font_dict_string(font, dFI, "Notice"))
        output->add_definition(dFI, Type1Definition::make_string("Notice", s, "readonly def"));
    if (String s = font_dict_string(font, dFI, "Copyright"))
        output->add_definition(dFI, Type1Definition::make_string("Copyright", s, "readonly def"));
    if (String s = font_dict_string(font, dFI, "FullName"))
        output->add_definition(dFI, Type1Definition::make_string("FullName", s, "readonly def"));
    if (String s = font_dict_string(font, dFI, "FamilyName"))
        output->add_definition(dFI, Type1Definition::make_string("FamilyName", s, "readonly def"));
    if (String s = font_dict_string(font, dFI, "Weight"))
        output->add_definition(dFI, Type1Definition::make_string("Weight", s, "readonly def"));
    if (Type1Definition *t1d = font->fi_dict("isFixedPitch")) {
        bool v;
        if (t1d->value_bool(v))
            output->add_definition(dFI, Type1Definition::make_literal("isFixedPitch", (v ? "true" : "false"), "def"));
    }
    add_number_def(output, dFI, "ItalicAngle", font);
    add_number_def(output, dFI, "UnderlinePosition", font);
    add_number_def(output, dFI, "UnderlineThickness", font);
    output->skeleton_fontinfo_end();

    // Encoding, other font dictionary entries
    output->add_type1_encoding(new Type1Encoding(*font->type1_encoding()));
    add_number_def(output, dF, "PaintType", font);
    add_number_def(output, dF, "FontType", font);
    add_copy_def(output, dF, "FontMatrix", font, "readonly def");
    add_number_def(output, dF, "StrokeWidth", font);
    if (!xuid_extension)
        add_number_def(output, dF, "UniqueID", font);
    add_copy_def(output, dF, "XUID", font, "readonly def");
    if (xuid_extension) {
        Vector<double> xuid;
        if (Type1Definition *xuid_def = output->dict("XUID"))
            xuid_def->value_numvec(xuid);
        if (!xuid.size()) {
            Type1Definition *uid_def = font->dict("UniqueID");
            int uid;
            if (uid_def && uid_def->value_int(uid)) {
                xuid.push_back(1);
                xuid.push_back(uid);
            }
        }
        if (xuid.size()) {
            for (int i = 0; i < xuid_extension->size(); i++)
                xuid.push_back((*xuid_extension)[i]);
            Type1Definition *xuid_def = output->ensure(dF, "XUID");
            xuid_def->set_numvec(xuid);
        }
    }
    add_copy_def(output, dF, "FontBBox", font, "readonly def");
    output->skeleton_fontdict_end();

    // Private dictionary
    add_copy_def(output, dP, "BlueValues", font);
    add_copy_def(output, dP, "OtherBlues", font);
    add_copy_def(output, dP, "FamilyBlues", font);
    add_copy_def(output, dP, "FamilyOtherBlues", font);
    add_number_def(output, dP, "BlueScale", font);
    add_number_def(output, dP, "BlueShift", font);
    add_number_def(output, dP, "BlueFuzz", font);
    add_copy_def(output, dP, "StdHW", font);
    add_copy_def(output, dP, "StdVW", font);
    add_copy_def(output, dP, "StemSnapH", font);
    add_copy_def(output, dP, "StemSnapV", font);
    add_copy_def(output, dP, "ForceBold", font);
    add_number_def(output, dP, "LanguageGroup", font);
    add_number_def(output, dP, "ExpansionFactor", font);
    if (!xuid_extension)
        add_number_def(output, dP, "UniqueID", font);
    output->add_definition(dP, Type1Definition::make_literal("MinFeature", "{16 16}", "|-"));
    output->add_definition(dP, Type1Definition::make_literal("password", "5839", "def"));
    output->add_definition(dP, Type1Definition::make_literal("lenIV", "0", "def"));
    output->skeleton_private_end();

    return output;
}

}
