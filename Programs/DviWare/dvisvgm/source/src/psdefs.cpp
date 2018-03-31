/*************************************************************************
** psdefs.cpp                                                           **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2018 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#if defined(MIKTEX)
#  include <config.h>
#endif
#include "PSInterpreter.hpp"

const char *PSInterpreter::PSDEFS =
"<</Install{matrix setmatrix}/HWResolution[72 72]/PageSize[10000 10000]/Imaging"
"BBox null>>setpagedevice/@dodraw true store/@patcnt 0 store/@SD systemdict def"
"/@UD userdict def true setglobal @SD/:save @SD/save get put @SD/:restore @SD/r"
"estore get put @SD/:gsave @SD/gsave get put @SD/:grestore @SD/grestore get put"
" @SD/:grestoreall @SD/grestoreall get put @SD/:newpath @SD/newpath get put @SD"
"/:stroke @SD/stroke get put @SD/:fill @SD/fill get put @SD/:eofill @SD/eofill "
"get put @SD/:clip @SD/clip get put @SD/:eoclip @SD/eoclip get put @SD/:charpat"
"h @SD/charpath get put @SD/:show @SD/show get put @SD/.setopacityalpha known n"
"ot{@SD/.setopacityalpha{pop}put}if @SD/prseq{-1 1{-1 roll =only( )print}for(\\"
"n)print}put @SD/prcmd{( )exch(\\ndvi.)3{print}repeat prseq}put @SD/cvxall{{cvx"
"}forall}put @SD/defpr{[exch[/copy @SD]cvxall 5 -1 roll dup 6 1 roll[/get/exec]"
"cvxall 6 -1 roll dup 7 1 roll 4 -1 roll dup 5 1 roll dup length string cvs/prc"
"md cvx]cvx def}put @SD/querypos{{currentpoint}stopped{$error/newerror false pu"
"t}{2(querypos)prcmd}ifelse}put @SD/applyscalevals{1 0 dtransform exch dup mul "
"exch dup mul add sqrt 0 1 dtransform exch dup mul exch dup mul add sqrt 1 0 dt"
"ransform dup mul exch dup dup mul 3 -1 roll add dup 0 eq{pop}{sqrt div}ifelse "
"3(applyscalevals)prcmd}put @SD/prpath{{2(moveto)prcmd}{2(lineto)prcmd}{6(curve"
"to)prcmd}{0(closepath)prcmd}pathforall}put @SD/charpath{/@dodraw false store :"
"charpath/@dodraw true store}put @SD/show{@dodraw{dup :gsave currentpoint 2{50 "
"mul exch}repeat :newpath moveto 50 50/scale sysexec true charpath eofill :gres"
"tore/@dodraw false store :show/@dodraw true store}{pop}ifelse}put @SD/awidthsh"
"ow{{1 string dup 0 5 index put :gsave show :grestore pop 0 rmoveto 3 index eq{"
"4 index 4 index rmoveto}if 1 index 1 index rmoveto}exch cshow 5{pop}repeat}put"
" @SD/widthshow{0 0 3 -1 roll pstack awidthshow}put @SD/ashow{0 0 0 6 3 roll aw"
"idthshow}put @SD/newpath{:newpath 0 1(newpath)prcmd}put @SD/stroke{@dodraw{1 1"
"(newpath)prcmd prpath 0(stroke)prcmd :newpath}{:stroke}ifelse}put @SD/fill{@do"
"draw{1 1(newpath)prcmd prpath 0(fill)prcmd :newpath}{:fill}ifelse}put @SD/eofi"
"ll{@dodraw{1 1(newpath)prcmd prpath 0(eofill)prcmd :newpath}{:eofill}ifelse}pu"
"t @SD/clip{:clip 0 1(newpath)prcmd prpath 0(clip)prcmd}put @SD/eoclip{:eoclip "
"1 1(newpath)prcmd prpath 0(eoclip)prcmd}put @SD/shfill{begin currentdict/Shadi"
"ngType known currentdict/ColorSpace known and currentdict/DataSource known and"
" currentdict/Function known not and ShadingType 4 ge and DataSource type/array"
"type eq and{<</DeviceGray 1/DeviceRGB 3/DeviceCMYK 4/bgknown currentdict/Backg"
"round known/bbknown currentdict/BBox known>>begin currentdict ColorSpace known"
"{ShadingType ColorSpace load bgknown{1 Background aload pop}{0}ifelse bbknown{"
"1 BBox aload pop}{0}ifelse ShadingType 5 eq{VerticesPerRow}if DataSource aload"
" length 4 add bgknown{ColorSpace load add}if bbknown{4 add}if ShadingType 5 eq"
"{1 add}if(shfill)prcmd}if end}if end}put/@rect{4 -2 roll moveto exch dup 0 rli"
"neto exch 0 exch rlineto neg 0 rlineto closepath}bind def/@rectcc{4 -2 roll mo"
"veto 2 copy 0 lt exch 0 lt xor{dup 0 exch rlineto exch 0 rlineto neg 0 exch rl"
"ineto}{exch dup 0 rlineto exch 0 exch rlineto neg 0 rlineto}ifelse closepath}b"
"ind def @SD/rectclip{:newpath dup type/arraytype eq{aload length 4 idiv{@rectc"
"c}repeat}{@rectcc}ifelse clip :newpath}put @SD/rectfill{gsave :newpath dup typ"
"e/arraytype eq{aload length 4 idiv{@rectcc}repeat}{@rectcc}ifelse fill grestor"
"e}put @SD/rectstroke{gsave :newpath dup type/arraytype eq{aload length 4 idiv{"
"@rect}repeat}{@rect}ifelse stroke grestore}put false setglobal @SD readonly po"
"p/initclip 0 defpr/clippath 0 defpr/sysexec{@SD exch get exec}def/adddot{dup l"
"ength 1 add string dup 0 46 put dup 3 -1 roll 1 exch putinterval}def/setlinewi"
"dth{dup/setlinewidth sysexec 1(setlinewidth)prcmd}def/setlinecap 1 defpr/setli"
"nejoin 1 defpr/setmiterlimit 1 defpr/setdash{mark 3 1 roll 2 copy/setdash syse"
"xec exch aload length 1 add -1 roll counttomark(setdash)prcmd pop}def/@setpage"
"device{pop<<>>/setpagedevice sysexec[1 0 0 -1 0 0]setmatrix newpath 0(setpaged"
"evice)prcmd}def/setgstate{currentlinewidth 1(setlinewidth)prcmd currentlinecap"
" 1(setlinecap)prcmd currentlinejoin 1(setlinejoin)prcmd currentmiterlimit 1(se"
"tmiterlimit)prcmd currentrgbcolor 3(setrgbcolor)prcmd matrix currentmatrix alo"
"ad pop 6(setmatrix)prcmd applyscalevals currentdash mark 3 1 roll exch aload l"
"ength 1 add -1 roll counttomark(setdash)prcmd pop}def/save{@UD begin/@saveID v"
"mstatus pop pop def end :save @saveID 1(save)prcmd}def/restore{:restore setgst"
"ate @UD/@saveID known{@UD begin @saveID end}{0}ifelse 1(restore)prcmd}def/gsav"
"e 0 defpr/grestore{:grestore setgstate 0(grestore)prcmd}def/grestoreall{:grest"
"oreall setstate 0(grestoreall)prcmd}def/rotate{dup type/arraytype ne{dup 1(rot"
"ate)prcmd}if/rotate sysexec applyscalevals}def/scale{dup type/arraytype ne{2 c"
"opy 2(scale)prcmd}if/scale sysexec applyscalevals}def/translate{dup type/array"
"type ne{2 copy 2(translate)prcmd}if/translate sysexec}def/setmatrix{dup/setmat"
"rix sysexec aload pop 6(setmatrix)prcmd applyscalevals}def/initmatrix{matrix s"
"etmatrix}def/concat{matrix currentmatrix matrix concatmatrix setmatrix}def/mak"
"epattern{gsave<</mx 3 -1 roll>>begin dup/XUID[1000000 @patcnt]put mx/makepatte"
"rn sysexec dup dup begin PatternType @patcnt BBox aload pop XStep YStep PaintT"
"ype mx aload pop 15(makepattern)prcmd :newpath matrix setmatrix PaintProc 0 1("
"makepattern)prcmd end/@patcnt @patcnt 1 add store end grestore}def/setpattern{"
"begin PatternType 1 eq{PaintType 1 eq{XUID aload pop exch pop 1}{:gsave[curren"
"tcolorspace aload length -1 roll pop]setcolorspace/setcolor sysexec XUID aload"
" pop exch pop currentrgbcolor :grestore 4}ifelse(setpattern)prcmd}{/setpattern"
" sysexec}ifelse end}def/setcolor{dup type/dicttype eq{setpattern}{/setcolor sy"
"sexec/currentrgbcolor sysexec setrgbcolor}ifelse}def/setgray 1 defpr/setcmykco"
"lor 4 defpr/sethsbcolor 3 defpr/setrgbcolor 3 defpr/.setopacityalpha{dup/.seto"
"pacityalpha sysexec 1(setopacityalpha)prcmd}def ";
