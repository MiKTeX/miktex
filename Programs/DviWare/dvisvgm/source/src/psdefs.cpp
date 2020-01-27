/*************************************************************************
** psdefs.cpp                                                           **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2019 Martin Gieseking <martin.gieseking@uos.de>   **
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
"BBox null>>setpagedevice/@dodraw true store/@patcnt 0 store/@GD globaldict def"
"/@SD systemdict def/@UD userdict def @GD/@nulldev false put true setglobal @SD"
"/:save @SD/save get put @SD/:restore @SD/restore get put @SD/:gsave @SD/gsave "
"get put @SD/:grestore @SD/grestore get put @SD/:grestoreall @SD/grestoreall ge"
"t put @SD/:newpath @SD/newpath get put @SD/:stroke @SD/stroke get put @SD/:fil"
"l @SD/fill get put @SD/:eofill @SD/eofill get put @SD/:clip @SD/clip get put @"
"SD/:eoclip @SD/eoclip get put @SD/:charpath @SD/charpath get put @SD/:show @SD"
"/show get put @SD/:stringwidth @SD/stringwidth get put @SD/:nulldevice @SD/nul"
"ldevice get put @SD/.setopacityalpha known not{@SD/.setopacityalpha{pop}put}if"
" @SD/.setshapealpha known not{@SD/.setshapealpha{pop}put}if @SD/.setblendmode "
"known not{@SD/.setblendmode{pop}put}if @SD/prseq{-1 1{-1 roll =only( )print}fo"
"r(\\n)print}put @SD/prcmd{( )exch(\\ndvi.)3{print}repeat prseq}put @SD/cvxall{"
"{cvx}forall}put @SD/defpr{[exch[/copy @SD]cvxall 5 -1 roll dup 6 1 roll[/get/e"
"xec]cvxall 6 -1 roll dup 7 1 roll 4 -1 roll dup 5 1 roll dup length string cvs"
"/prcmd cvx]cvx def}put @SD/querypos{{currentpoint}stopped{$error/newerror fals"
"e put}{2(querypos)prcmd}ifelse}put @SD/applyscalevals{1 0 dtransform exch dup "
"mul exch dup mul add sqrt 0 1 dtransform exch dup mul exch dup mul add sqrt 1 "
"0 dtransform dup mul exch dup dup mul 3 -1 roll add dup 0 eq{pop}{sqrt div}ife"
"lse 3(applyscalevals)prcmd}put @SD/prpath{{2(moveto)prcmd}{2(lineto)prcmd}{6(c"
"urveto)prcmd}{0(closepath)prcmd}pathforall}put @SD/nulldevice{@GD/@nulldev tru"
"e put :nulldevice 1 1(setnulldevice)prcmd}put @SD/charpath{/@dodraw false stor"
"e :charpath/@dodraw true store}put @SD/stringwidth{/@dodraw false store :strin"
"gwidth/@dodraw true store}put @SD/show{@dodraw @GD/@nulldev get not and{dup :g"
"save currentpoint 2{50 mul exch}repeat :newpath moveto 50 50/scale sysexec tru"
"e charpath fill :grestore/@dodraw false store :show/@dodraw true store}{:show}"
"ifelse}put @SD/varxyshow{exch dup type/arraytype eq{<</arr 3 -1 roll/prc 5 -1 "
"roll/chr 1 string/idx 0>>begin{chr 0 3 -1 roll put :gsave chr show :grestore c"
"urrentpoint prc moveto/idx idx 1 add store}forall end}{pop show}ifelse}put @SD"
"/xyshow{{exch arr idx 2 mul get add exch arr idx 2 mul 1 add get add}varxyshow"
"}put @SD/xshow{{exch arr idx get add exch}varxyshow}put @SD/yshow{{arr idx get"
" add}varxyshow}put @SD/awidthshow{{1 string dup 0 5 index put :gsave show :gre"
"store pop 0 rmoveto 3 index eq{4 index 4 index rmoveto}if 1 index 1 index rmov"
"eto}exch cshow 5{pop}repeat}put @SD/widthshow{0 0 3 -1 roll pstack awidthshow}"
"put @SD/ashow{0 0 0 6 3 roll awidthshow}put @SD/newpath{:newpath 1 1(newpath)p"
"rcmd}put @SD/stroke{@dodraw @GD/@nulldev get not and{prcolor 0 1(newpath)prcmd"
" prpath 0(stroke)prcmd :newpath}{:stroke}ifelse}put @SD/fill{@dodraw @GD/@null"
"dev get not and{prcolor 0 1(newpath)prcmd prpath 0(fill)prcmd :newpath}{:fill}"
"ifelse}put @SD/eofill{@dodraw @GD/@nulldev get not and{prcolor 0 1(newpath)prc"
"md prpath 0(eofill)prcmd :newpath}{:eofill}ifelse}put @SD/clip{:clip @GD/@null"
"dev get not{0 1(newpath)prcmd prpath 0(clip)prcmd}if}put @SD/eoclip{:eoclip @G"
"D/@nulldev get not{0 1(newpath)prcmd prpath 0(eoclip)prcmd}}put @SD/shfill{beg"
"in currentdict/ShadingType known currentdict/ColorSpace known and currentdict/"
"DataSource known and currentdict/Function known not and ShadingType 4 ge and D"
"ataSource type/arraytype eq and{<</DeviceGray 1/DeviceRGB 3/DeviceCMYK 4/bgkno"
"wn currentdict/Background known/bbknown currentdict/BBox known>>begin currentd"
"ict ColorSpace known{ShadingType ColorSpace load bgknown{1 Background aload po"
"p}{0}ifelse bbknown{1 BBox aload pop}{0}ifelse ShadingType 5 eq{VerticesPerRow"
"}if DataSource aload length 4 add bgknown{ColorSpace load add}if bbknown{4 add"
"}if ShadingType 5 eq{1 add}if(shfill)prcmd}if end}if end}put/@rect{4 -2 roll m"
"oveto exch dup 0 rlineto exch 0 exch rlineto neg 0 rlineto closepath}bind def/"
"@rectcc{4 -2 roll moveto 2 copy 0 lt exch 0 lt xor{dup 0 exch rlineto exch 0 r"
"lineto neg 0 exch rlineto}{exch dup 0 rlineto exch 0 exch rlineto neg 0 rlinet"
"o}ifelse closepath}bind def @SD/rectclip{:newpath dup type/arraytype eq{aload "
"length 4 idiv{@rectcc}repeat}{@rectcc}ifelse clip :newpath}put @SD/rectfill{gs"
"ave :newpath dup type/arraytype eq{aload length 4 idiv{@rectcc}repeat}{@rectcc"
"}ifelse fill grestore}put @SD/rectstroke{gsave :newpath dup type/arraytype eq{"
"aload length 4 idiv{@rect}repeat}{@rect}ifelse stroke grestore}put false setgl"
"obal @SD readonly pop/initclip 0 defpr/clippath 0 defpr/sysexec{@SD exch get e"
"xec}def/adddot{dup length 1 add string dup 0 46 put dup 3 -1 roll 1 exch putin"
"terval}def/setlinewidth{dup/setlinewidth sysexec 1(setlinewidth)prcmd}def/setl"
"inecap 1 defpr/setlinejoin 1 defpr/setmiterlimit 1 defpr/setdash{mark 3 1 roll"
" 2 copy/setdash sysexec exch aload length 1 add -1 roll counttomark(setdash)pr"
"cmd pop}def/@setpagedevice{pop<<>>/setpagedevice sysexec matrix setmatrix newp"
"ath 0(setpagedevice)prcmd}def/@checknulldev{@GD/@nulldev get{currentpagedevice"
" maxlength 0 ne{@GD/@nulldev false put 0 1(setnulldevice)prcmd}if}if}def/prcol"
"or{currentrgbcolor 3(setrgbcolor)prcmd}def/printgstate{@dodraw @GD/@nulldev ge"
"t not and{matrix currentmatrix aload pop 6(setmatrix)prcmd applyscalevals curr"
"entlinewidth 1(setlinewidth)prcmd currentlinecap 1(setlinecap)prcmd currentlin"
"ejoin 1(setlinejoin)prcmd currentmiterlimit 1(setmiterlimit)prcmd currentrgbco"
"lor 3(setrgbcolor)prcmd currentdash mark 3 1 roll exch aload length 1 add -1 r"
"oll counttomark(setdash)prcmd pop}if}def/setgstate{/setgstate sysexec printgst"
"ate}def/save{@UD begin/@saveID vmstatus pop pop def end :save @saveID 1(save)p"
"rcmd}def/restore{:restore @checknulldev printgstate @UD/@saveID known{@UD begi"
"n @saveID end}{0}ifelse 1(restore)prcmd}def/gsave 0 defpr/grestore{:grestore @"
"checknulldev printgstate 0(grestore)prcmd}def/grestoreall{:grestoreall @checkn"
"ulldev setstate 0(grestoreall)prcmd}def/rotate{dup type/arraytype ne @dodraw a"
"nd{dup 1(rotate)prcmd}if/rotate sysexec applyscalevals}def/scale{dup type/arra"
"ytype ne @dodraw and{2 copy 2(scale)prcmd}if/scale sysexec applyscalevals}def/"
"translate{dup type/arraytype ne @dodraw and{2 copy 2(translate)prcmd}if/transl"
"ate sysexec}def/setmatrix{dup/setmatrix sysexec @dodraw{aload pop 6(setmatrix)"
"prcmd applyscalevals}{pop}ifelse}def/initmatrix{matrix setmatrix}def/concat{ma"
"trix currentmatrix matrix concatmatrix setmatrix}def/makepattern{gsave<</mx 3 "
"-1 roll>>begin dup/XUID[1000000 @patcnt]put mx/makepattern sysexec dup dup beg"
"in PatternType @patcnt BBox aload pop XStep YStep PaintType mx aload pop 15(ma"
"kepattern)prcmd :newpath matrix setmatrix PaintProc 0 1(makepattern)prcmd end/"
"@patcnt @patcnt 1 add store end grestore}def/setpattern{begin PatternType 1 eq"
"{PaintType 1 eq{XUID aload pop exch pop 1}{:gsave[currentcolorspace aload leng"
"th -1 roll pop]setcolorspace/setcolor sysexec XUID aload pop exch pop currentr"
"gbcolor :grestore 4}ifelse(setpattern)prcmd}{/setpattern sysexec}ifelse end}de"
"f/setcolor{dup type/dicttype eq{setpattern}{/setcolor sysexec/currentrgbcolor "
"sysexec setrgbcolor}ifelse}def/setgray 1 defpr/setcmykcolor 4 defpr/sethsbcolo"
"r 3 defpr/setrgbcolor 3 defpr/.setopacityalpha{dup/.setopacityalpha sysexec 1("
"setopacityalpha)prcmd}def/.setshapealpha{dup/.setshapealpha sysexec 1(setshape"
"alpha)prcmd}def/.setblendmode{dup/.setblendmode sysexec<</Normal 0/Compatible "
"0/Multiply 1/Screen 2/Overlay 3/SoftLight 4/HardLight 5/ColorDodge 6/ColorBurn"
" 7/Darken 8/Lighten 9/Difference 10/Exclusion 11/Hue 12/Saturation 13/Color 14"
"/Luminosity 15/CompatibleOverprint 16>>exch get 1(setblendmode)prcmd}def/@pdfp"
"agecount{(r)file runpdfbegin pdfpagecount runpdfend}def/@pdfpagebox{(r)file ru"
"npdfbegin dup dup 1 lt exch pdfpagecount gt or{pop}{pdfgetpage/MediaBox pget p"
"op aload pop}ifelse runpdfend}def DELAYBIND{.bindnow}if ";

