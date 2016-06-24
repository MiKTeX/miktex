/* commands.h */

/************************************************************************

  Part of the dvipng distribution

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program. If not, see
  <http://www.gnu.org/licenses/>.

  Copyright (C) 2002-2008 Jan-Åke Larsson

************************************************************************/

/* DVI COMMANDS */
#define  DVIFORMAT     2

#define  SETC_000         0     /* typeset character 0 and move right */
#define  SETC_127       127     /* typeset character 127 and move right */
#define  SET1           128     /* typeset a character and move right */
#define  SET2           129     /* ??? */
#define  SET3           130     /* ??? */
#define  SET4           131     /* ??? */
#define  SET_RULE       132     /* typeset a rule and move right */
#define  PUT1           133     /* typeset a character */
#define  PUT2           134     /* ??? */
#define  PUT3           135     /* ??? */
#define  PUT4           136     /* ??? */
#define  PUT_RULE       137     /* typeset a rule */
#define  NOP            138     /* no operation */
#define  BOP            139     /* beginning of page */
#define  EOP            140     /* ending of page */
#define  PUSH           141     /* save the current positions */
#define  POP            142     /* restore previous positions */
#define  RIGHT1         143     /* move right */
#define  RIGHT2         144     /* ??? */
#define  RIGHT3         145     /* ??? */
#define  RIGHT4         146     /* ??? */
#define  W0             147     /* move right by |w| */
#define  W1             148     /* move right and set |w| */
#define  W2             149     /* ??? */
#define  W3             150     /* ??? */
#define  W4             151     /* ??? */
#define  X0             152     /* move right by |x| */
#define  X1             153     /* move right and set |x| */
#define  X2             154     /* ??? */
#define  X3             155     /* ??? */
#define  X4             156     /* ??? */
#define  DOWN1          157     /* move down */
#define  DOWN2          158     /* ??? */
#define  DOWN3          159     /* ??? */
#define  DOWN4          160     /* ??? */
#define  Y0             161     /* move down by |y| */
#define  Y1             162     /* move down and set |y| */
#define  Y2             163     /* ??? */
#define  Y3             164     /* ??? */
#define  Y4             165     /* ??? */
#define  Z0             166     /* move down by |z| */
#define  Z1             167     /* move down and set |z| */
#define  Z2             168     /* ??? */
#define  Z3             169     /* ??? */
#define  Z4             170     /* ??? */
#define  FONT_00        171     /* set current font to 0 */
#define  FONT_63        234     /* set current font to 63 */
#define  FNT1           235     /* set current font */
#define  FNT2           236     /* Same as FNT1, except that arg is 2 bytes */
#define  FNT3           237     /* Same as FNT1, except that arg is 3 bytes */
#define  FNT4           238     /* Same as FNT1, except that arg is 4 bytes */
#define  XXX1           239     /* extension to \.DVI primitives */
#define  XXX2           240     /* Like XXX1, but 0<=k<65536 */
#define  XXX3           241     /* Like XXX1, but 0<=k<@t$2^{24}$@> */
#define  XXX4           242     /* potentially long extension to \.DVI
                                   primitives */
#define  FNT_DEF1       243     /* define the meaning of a font number */
#define  FNT_DEF2       244     /* ??? */
#define  FNT_DEF3       245     /* ??? */
#define  FNT_DEF4       246     /* ??? */
#define  PRE            247     /* preamble */
#define  POST           248     /* postamble beginning */
#define  POST_POST      249     /* postamble ending */

/*  undefined_commands           250,251,252,253,254,255 */

EXTERN const int8_t dvi_commandlength[256] 
#ifdef MAIN
={
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,            /* SETC_000 --- SETC_127    */
  2,3,4,5,9,                  /* SET1 --- SET4, SET_RULE  */ 
  2,3,4,5,9,                  /* PUT1 --- PUT4, PUT_RULE  */ 
  1,45,1,1,1,                 /* NOP, BOP, EOP, PUSH, POP */
  2,3,4,5,                    /* RIGHT1 --- RIGHT4        */
  1,2,3,4,5,                  /* W0 --- W4                */
  1,2,3,4,5,                  /* X0 --- X4                */
  2,3,4,5,                    /* DOWN1 --- DOWN4          */
  1,2,3,4,5,                  /* Y0 --- Y4                */
  1,2,3,4,5,                  /* Z0 --- Z4                */
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,                    /* FONT_00 --- FONT_63      */
  2,3,4,5,                    /* FNT1 --- FNT4            */
  2,3,4,5,                    /* XXX1 --- XXX4 + special string    */
  16,17,18,19,                /* FNT_DEF1 --- FNT_DEF4 + font name */
  15,                         /* PRE + TeX comment        */
  29,                         /* POST                     */
  10,                         /* POST_POST minimum        */
  -1,-1,-1,-1,-1,-1           /* undefined                */
}
#endif
;

EXTERN const char* dvi_commands[256] 
#ifdef MAIN
={
"SETC_000","SETC_001","SETC_002","SETC_003","SETC_004",
"SETC_005","SETC_006","SETC_007","SETC_008","SETC_009",
"SETC_010","SETC_011","SETC_012","SETC_013","SETC_014",
"SETC_015","SETC_016","SETC_017","SETC_018","SETC_019",
"SETC_020","SETC_021","SETC_022","SETC_023","SETC_024",
"SETC_025","SETC_026","SETC_027","SETC_028","SETC_029",
"SETC_030","SETC_031","SETC_032","SETC_033","SETC_034",
"SETC_035","SETC_036","SETC_037","SETC_038","SETC_039",
"SETC_040","SETC_041","SETC_042","SETC_043","SETC_044",
"SETC_045","SETC_046","SETC_047","SETC_048","SETC_049",
"SETC_050","SETC_051","SETC_052","SETC_053","SETC_054",
"SETC_055","SETC_056","SETC_057","SETC_058","SETC_059",
"SETC_060","SETC_061","SETC_062","SETC_063","SETC_064",
"SETC_065","SETC_066","SETC_067","SETC_068","SETC_069",
"SETC_070","SETC_071","SETC_072","SETC_073","SETC_074",
"SETC_075","SETC_076","SETC_077","SETC_078","SETC_079",
"SETC_080","SETC_081","SETC_082","SETC_083","SETC_084",
"SETC_085","SETC_086","SETC_087","SETC_088","SETC_089",
"SETC_090","SETC_091","SETC_092","SETC_093","SETC_094",
"SETC_095","SETC_096","SETC_097","SETC_098","SETC_099",
"SETC_100","SETC_101","SETC_102","SETC_103","SETC_104",
"SETC_105","SETC_106","SETC_107","SETC_108","SETC_109",
"SETC_110","SETC_111","SETC_112","SETC_113","SETC_114",
"SETC_115","SETC_116","SETC_117","SETC_118","SETC_119",
"SETC_120","SETC_121","SETC_122","SETC_123","SETC_124",
"SETC_125","SETC_126","SETC_127",
"SET1","SET2","SET3","SET4","SET_RULE",
"PUT1","PUT2","PUT3","PUT4","PUT_RULE",
"NOP","BOP","EOP","PUSH","POP",
"RIGHT1","RIGHT2","RIGHT3","RIGHT4",
"W0","W1","W2","W3","W4",
"X0","X1","X2","X3","X4",
"DOWN1","DOWN2","DOWN3","DOWN4",
"Y0","Y1","Y2","Y3","Y4",
"Z0","Z1","Z2","Z3","Z4",
"FONT_00","FONT_01","FONT_02","FONT_03","FONT_04",
"FONT_05","FONT_06","FONT_07","FONT_08","FONT_09",
"FONT_10","FONT_11","FONT_12","FONT_13","FONT_14",
"FONT_15","FONT_16","FONT_17","FONT_18","FONT_19",
"FONT_20","FONT_21","FONT_22","FONT_23","FONT_24",
"FONT_25","FONT_26","FONT_27","FONT_28","FONT_29",
"FONT_30","FONT_31","FONT_32","FONT_33","FONT_34",
"FONT_35","FONT_36","FONT_37","FONT_38","FONT_39",
"FONT_40","FONT_41","FONT_42","FONT_43","FONT_44",
"FONT_45","FONT_46","FONT_47","FONT_48","FONT_49",
"FONT_50","FONT_51","FONT_52","FONT_53","FONT_54",
"FONT_55","FONT_56","FONT_57","FONT_58","FONT_59",
"FONT_60","FONT_61","FONT_62","FONT_63",
"FNT1","FNT2","FNT3","FNT4",
"XXX1","XXX2","XXX3","XXX4",
"FNT_DEF1","FNT_DEF2","FNT_DEF3","FNT_DEF4",
"PRE","POST","POST_POST",
"UNDEF_250","UNDEF_251","UNDEF_252","UNDEF_253","UNDEF_254","UNDEF_255"
}
#endif
;

