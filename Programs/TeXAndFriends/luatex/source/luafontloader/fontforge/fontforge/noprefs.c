/* Copyright (C) 2000-2008 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "pfaedit.h"
#include "plugins.h"
#include <charset.h>
#include <gfile.h>
#include <ustring.h>

#include <sys/types.h>
#include <dirent.h>
#include <locale.h>
#include <time.h>
#if defined(MIKTEX_WINDOWS)
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif
#include <stdlib.h>

#include "ttf.h"

#if HAVE_LANGINFO_H
# include <langinfo.h>
#endif

static char *othersubrsfile = NULL;

extern int adjustwidth;
extern int adjustlbearing;
extern Encoding *default_encoding;
extern int autohint_before_rasterize;
extern int autohint_before_generate;
extern int accent_offset;
extern int GraveAcuteCenterBottom;
extern int PreferSpacingAccents;
extern int CharCenterHighest;
extern int recognizePUA;
extern int snaptoint;
extern float joinsnap;
extern char *BDFFoundry;
extern char *TTFFoundry;
extern char *xuid;
extern char *SaveTablesPref;
extern int maxundoes;			/* in cvundoes */
extern int prefer_cjk_encodings;	/* in parsettf */
extern int onlycopydisplayed, copymetadata, copyttfinstr;
extern int oldformatstate;		/* in savefontdlg.c */
extern int oldbitmapstate;		/* in savefontdlg.c */
extern int old_ttf_flags;		/* in savefontdlg.c */
extern int old_ps_flags;		/* in savefontdlg.c */
extern int old_otf_flags;		/* in savefontdlg.c */
extern int preferpotrace;		/* in autotrace.c */
extern int autotrace_ask;		/* in autotrace.c */
extern int mf_ask;			/* in autotrace.c */
extern int mf_clearbackgrounds;		/* in autotrace.c */
extern int mf_showerrors;		/* in autotrace.c */
extern char *mf_args;			/* in autotrace.c */
extern int coverageformatsallowed;	/* in tottfgpos.c */
extern int hint_diagonal_ends;		/* in stemdb.c */
extern int hint_diagonal_intersections;	/* in stemdb.c */
extern int hint_bounding_boxes;		/* in stemdb.c */
extern int detect_diagonal_stems;	/* in stemdb.c */
extern int new_em_size;				/* in splineutil2.c */
extern int new_fonts_are_order2;		/* in splineutil2.c */
extern int loaded_fonts_same_as_new;		/* in splineutil2.c */
extern int use_second_indic_scripts;		/* in tottfgpos.c */
extern int allow_utf8_glyphnames;		/* in charinfo.c */
extern int ask_user_for_cmap;			/* in parsettf.c */
extern NameList *force_names_when_opening;
extern NameList *force_names_when_saving;
extern NameList *namelist_for_new_fonts;
extern int default_fv_row_count;		/* in splineutil2.c */
extern int default_fv_col_count;		/* in splineutil2.c */
extern int use_freetype_to_rasterize_fv;	/* in bitmapchar.c */

/* UI preferences which we don't use, but will preserve to so we can read/write */
/*  UI preference files without loss of data */
static char *xdefs_filename;
static char *helpdir=NULL;				/* in uiutil.c */
static int splash=1;
static int cv_auto_goto=1;
static int OpenCharsInNewWindow=1;
static float arrowAmount=1;
static float arrowAccelFactor=10;
static float snapdistance=3.5;
static int stop_at_join=0;
static int updateflex=0;				/* in charview.c */
static int ask_user_for_resolution=1;
static int default_fv_showhmetrics=0;	/* in fontview */
static int default_fv_showvmetrics=0;	/* in fontview */
static int default_fv_glyphlabel=0;	/* in fontview */
static int save_to_dir=0;			/* in fontview, use sfdir rather than sfd */
static int palettes_docked=1;		/* in cvpalettes */
static int cvvisible[2]={1,1}, bvvisible[3]={1,1,1};	/* in cvpalettes.c */
static int infowindowdistance=10;		/* in cvruler.c */
static int loacal_markextrema, loacal_markpoi, loacal_showrulers,
    loacal_showcpinfo, loacal_showsidebearings, loacal_showpoints,
    loacal_showfilled, loacal_showtabs, loacal_showrefnames;
static int oldsystem=100;
static char *oflib_username;
static char *oflib_password;
static int rectelipse=0, polystar=0, regular_star=0;	/* from cvpalettes.c */
static int center_out[2]={0,0};			/* from cvpalettes.c */
static float rr_radius=0;				/* from cvpalettes.c */
static int ps_pointcnt=5;				/* from cvpalettes.c */
static float star_percent=100;			/* from cvpalettes.c */
static int debug_wins=0;			/* in cvdebug.c */
static int gridfit_dpi=100, gridfit_depth=1;	/* in cvgridfit.c */
static float gridfit_pointsizex=12;		/* in cvgridfit.c */
static float gridfit_pointsizey=12;		/* in cvgridfit.c */
static int  gridfit_x_sameas_y=true;		/* in cvgridfit.c */
static int default_font_filter_index=0;
static int ItalicConstrained = true;
extern int clear_tt_instructions_when_needed;	/* cvundoes.c */
static int default_cv_width;			/* in charview.c */
static int default_cv_height;			/* in charview.c */
static int mv_width;				/* in metricsview.c */
static int mv_height;				/* in metricsview.c */
static int bv_width;				/* in bitmapview.c */
static int bv_height;				/* in bitmapview.c */
static int mvshowgrid;				/* in metricsview.c */
static int old_validate = true;
static int old_fontlog = false;
static int home_char = 'A';
static int compact_font_on_open=0;
static int oflib_automagic_preview;		/* from oflib.c */
static int aa_pixelsize;			/* from anchorsaway.c */

static int gfc_showhidden, gfc_dirplace;
static char *gfc_bookmarks=NULL;
static char *pixmapdir=NULL;

enum pref_types { pr_int, pr_real, pr_bool, pr_enum, pr_encoding, pr_string,
	pr_file, pr_namelist, pr_unicode };

int unused = 0;
char *unused_string = NULL;

static struct prefs_list {
    char *name;
    	/* In the prefs file the untranslated name will always be used, but */
	/* in the UI that name may be translated. */
    enum pref_types type;
    void *val;
    void *(*get)(void);
    void (*set)(void *);
    char mn;
    struct enums *enums;
    unsigned int dontdisplay: 1;
    char *popup;
} core_list[] = {
	{ N_("OtherSubrsFile"), pr_file, &othersubrsfile, NULL, NULL, 'O', NULL, 0, N_("If you wish to replace Adobe's OtherSubrs array (for Type1 fonts)\nwith an array of your own, set this to point to a file containing\na list of up to 14 PostScript subroutines. Each subroutine must\nbe preceded by a line starting with '%%%%' (any text before the\nfirst '%%%%' line will be treated as an initial copyright notice).\nThe first three subroutines are for flex hints, the next for hint\nsubstitution (this MUST be present), the 14th (or 13 as the\nnumbering actually starts with 0) is for counter hints.\nThe subroutines should not be enclosed in a [ ] pair.") },
	{ N_("AutoHint"), pr_bool, &unused, NULL, NULL, 'A', NULL, 0, N_("AutoHint before rasterizing") },
	{ N_("NewCharset"), pr_encoding, &default_encoding, NULL, NULL, 'N', NULL, 0, N_("Default encoding for\nnew fonts") },
	{ N_("NewEmSize"), pr_int, &new_em_size, NULL, NULL, 'S', NULL, 0, N_("The default size of the Em-Square in a newly created font.") },
	{ N_("NewFontsQuadratic"), pr_bool, &new_fonts_are_order2, NULL, NULL, 'Q', NULL, 0, N_("Whether new fonts should contain splines of quadratic (truetype)\nor cubic (postscript & opentype).") },
	{ N_("FreeTypeInFontView"), pr_bool, &unused, NULL, NULL, 'O', NULL, 0, N_("Use the FreeType rasterizer (when available)\nto rasterize glyphs in the font view.\nThis generally results in better quality.") },
	{ N_("LoadedFontsAsNew"), pr_bool, &loaded_fonts_same_as_new, NULL, NULL, 'L', NULL, 0, N_("Whether fonts loaded from the disk should retain their splines\nwith the original order (quadratic or cubic), or whether the\nsplines should be converted to the default order for new fonts\n(see NewFontsQuadratic).") },
	{ N_("PreferCJKEncodings"), pr_bool, &prefer_cjk_encodings, NULL, NULL, 'C', NULL, 0, N_("When loading a truetype or opentype font which has both a unicode\nand a CJK encoding table, use this flag to specify which\nshould be loaded for the font.") },
	{ N_("AskUserForCMap"), pr_bool, &ask_user_for_cmap, NULL, NULL, 'O', NULL, 0, N_("When loading a font in sfnt format (TrueType, OpenType, etc.),\nask the user to specify which cmap to use initially.") },
	{ N_("PreserveTables"), pr_string, &SaveTablesPref, NULL, NULL, 'P', NULL, 0, N_("Enter a list of 4 letter table tags, separated by commas.\nFontForge will make a binary copy of these tables when it\nloads a True/OpenType font, and will output them (unchanged)\nwhen it generates the font. Do not include table tags which\nFontForge thinks it understands.") },
	{ N_("ItalicConstrained"), pr_bool, &ItalicConstrained, NULL, NULL, '\0', NULL, 0, N_("In the Outline View, the Shift key constrains motion to be parallel to the ItalicAngle rather than constraining it to be vertical.") },
	{ N_("SnapToInt"), pr_bool, &snaptoint, NULL, NULL, '\0', NULL, 0, N_("When the user clicks in the editing window, round the location to the nearest integers.") },
	{ N_("JoinSnap"), pr_real, &joinsnap, NULL, NULL, '\0', NULL, 0, N_("The Edit->Join command will join points which are this close together\nA value of 0 means they must be coincident") },
	{ N_("CopyMetaData"), pr_bool, &copymetadata, NULL, NULL, '\0', NULL, 0, N_("When copying glyphs from the font view, also copy the\nglyphs' metadata (name, encoding, comment, etc).") },
	{ N_("UndoDepth"), pr_int, &maxundoes, NULL, NULL, '\0', NULL, 0, N_("The maximum number of Undoes/Redoes stored in a glyph") },
	{ N_("AutoWidthSync"), pr_bool, &adjustwidth, NULL, NULL, '\0', NULL, 0, N_("Changing the width of a glyph\nchanges the widths of all accented\nglyphs based on it.") },
	{ N_("AutoLBearingSync"), pr_bool, &adjustlbearing, NULL, NULL, '\0', NULL, 0, N_("Changing the left side bearing\nof a glyph adjusts the lbearing\nof other references in all accented\nglyphs based on it.") },
	{ N_("ClearInstrsBigChanges"), pr_bool, &clear_tt_instructions_when_needed, NULL, NULL, 'C', NULL, 0, N_("Instructions in a TrueType font refer to\npoints by number, so if you edit a glyph\nin such a way that some points have different\nnumbers (add points, remove them, etc.) then\nthe instructions will be applied to the wrong\npoints with disasterous results.\n  Normally FontForge will remove the instructions\nif it detects that the points have been renumbered\nin order to avoid the above problem. You may turn\nthis behavior off -- but be careful!") },
	{ N_("CopyTTFInstrs"), pr_bool, &copyttfinstr, NULL, NULL, '\0', NULL, 0, N_("When copying glyphs from the font view, also copy the\nglyphs' metadata (name, encoding, comment, etc).") },
	{ N_("AccentOffsetPercent"), pr_int, &accent_offset, NULL, NULL, '\0', NULL, 0, N_("The percentage of an em by which an accent is offset from its base glyph in Build Accent") },
	{ N_("AccentCenterLowest"), pr_bool, &GraveAcuteCenterBottom, NULL, NULL, '\0', NULL, 0, N_("When placing grave and acute accents above letters, should\nFontForge center them based on their full width, or\nshould it just center based on the lowest point\nof the accent.") },
	{ N_("CharCenterHighest"), pr_bool, &CharCenterHighest, NULL, NULL, '\0', NULL, 0, N_("When centering an accent over a glyph, should the accent\nbe centered on the highest point(s) of the glyph,\nor the middle of the glyph?") },
	{ N_("PreferSpacingAccents"), pr_bool, &PreferSpacingAccents, NULL, NULL, '\0', NULL, 0, N_("Use spacing accents (Unicode: 02C0-02FF) rather than\ncombining accents (Unicode: 0300-036F) when\nbuilding accented glyphs.") },
	{ N_("PreferPotrace"), pr_bool, &unused, NULL, NULL, '\0', NULL, 0, N_("FontForge supports two different helper applications to do autotracing\n autotrace and potrace\nIf your system only has one it will use that one, if you have both\nuse this option to tell FontForge which to pick.") },
	{ N_("AutotraceArgs"), pr_string, &unused_string, NULL, NULL, '\0', NULL, 0, N_("Extra arguments for configuring the autotrace program\n(either autotrace or potrace)") },
	{ N_("AutotraceAsk"), pr_bool, &unused, NULL, NULL, '\0', NULL, 0, N_("Ask the user for autotrace arguments each time autotrace is invoked") },
	{ N_("MfArgs"), pr_string, &unused_string, NULL, NULL, '\0', NULL, 0, N_("Commands to pass to mf (metafont) program, the filename will follow these") },
	{ N_("MfAsk"), pr_bool, &unused, NULL, NULL, '\0', NULL, 0, N_("Ask the user for mf commands each time mf is invoked") },
	{ N_("MfClearBg"), pr_bool, &unused, NULL, NULL, '\0', NULL, 0, N_("FontForge loads large images into the background of each glyph\nprior to autotracing them. You may retain those\nimages to look at after mf processing is complete, or\nremove them to save space") },
	{ N_("MfShowErr"), pr_bool, &unused, NULL, NULL, '\0', NULL, 0, N_("MetaFont (mf) generates lots of verbiage to stdout.\nMost of the time I find it an annoyance but it is\nimportant to see if something goes wrong.") },
	{ N_("FoundryName"), pr_string, &unused_string, NULL, NULL, 'F', NULL, 0, N_("Name used for foundry field in bdf\nfont generation") },
	{ N_("TTFFoundry"), pr_string, &TTFFoundry, NULL, NULL, 'T', NULL, 0, N_("Name used for Vendor ID field in\nttf (OS/2 table) font generation.\nMust be no more than 4 characters") },
	{ N_("NewFontNameList"), pr_namelist, &namelist_for_new_fonts, NULL, NULL, '\0', NULL, 0, N_("FontForge will use this namelist when assigning\nglyph names to code points in a new font.") },
	{ N_("RecognizePUANames"), pr_bool, &recognizePUA, NULL, NULL, 'U', NULL, 0, N_("Once upon a time, Adobe assigned PUA (public use area) encodings\nfor many stylistic variants of characters (small caps, old style\nnumerals, etc.). Adobe no longer believes this to be a good idea,\nand recommends that these encodings be ignored.\n\n The assignments were originally made because most applications\ncould not handle OpenType features for accessing variants. Adobe\nnow believes that all apps that matter can now do so. Applications\nlike Word and OpenOffice still can't handle these features, so\n fontforge's default behavior is to ignore Adobe's current\nrecommendations.\n\nNote: This does not affect figuring out unicode from the font's encoding,\nit just controls determining unicode from a name.") },
	{ N_("UnicodeGlyphNames"), pr_bool, &allow_utf8_glyphnames, NULL, NULL, 'O', NULL, 0, N_("Allow the full unicode character set in glyph names.\nThis does not conform to adobe's glyph name standard.\nSuch names should be for internal use only and\nshould NOT end up in production fonts." ) },
	{ N_("XUID-Base"), pr_string, &xuid, NULL, NULL, 'X', NULL, 0, N_("If specified this should be a space separated list of integers each\nless than 16777216 which uniquely identify your organization\nFontForge will generate a random number for the final component.") },
	{ N_("AskBDFResolution"), pr_bool, &ask_user_for_resolution, NULL, NULL, 'B', NULL, 0, N_("When generating a set of BDF fonts ask the user\nto specify the screen resolution of the fonts\notherwise FontForge will guess depending on the pixel size.") },
	{ N_("HintForGen"), pr_bool, &autohint_before_generate, NULL, NULL, 'H', NULL, 0, N_("AutoHint changed glyphs before generating a font") },
	{ N_("HintBoundingBoxes"), pr_bool, &hint_bounding_boxes, NULL, NULL, '\0', NULL, 0, N_("FontForge will place vertical or horizontal hints to describe the bounding boxes of suitable glyphs.") },
	{ N_("HintDiagonalEnds"), pr_bool, &hint_diagonal_ends, NULL, NULL, '\0', NULL, 0, N_("FontForge will place vertical or horizontal hints at the ends of diagonal stems.") },
	{ N_("HintDiagonalInter"), pr_bool, &hint_diagonal_intersections, NULL, NULL, '\0', NULL, 0, N_("FontForge will place vertical or horizontal hints at the intersections of diagonal stems.") },
	{ N_("DetectDiagonalStems"), pr_bool, &detect_diagonal_stems, NULL, NULL, '\0', NULL, 0, N_("FontForge will generate diagonal stem hints, which then can be used by the AutoInstr command.") },
	{ N_("UseNewIndicScripts"), pr_bool, &use_second_indic_scripts, NULL, NULL, 'C', NULL, 0, N_("MS has changed (in August 2006) the inner workings of their Indic shaping\nengine, and to disambiguate this change has created a parallel set of script\ntags (generally ending in '2') for Indic writing systems. If you are working\nwith the new system set this flag, if you are working with the old unset it.\n(if you aren't doing Indic work, this flag is irrelevant).") },
	{ "AntiAlias", pr_bool, &default_fv_antialias, NULL, NULL, '\0', NULL, 1, NULL },
	{ "DefaultFVSize", pr_int, &default_fv_font_size, NULL, NULL, 'S', NULL, 1, NULL },
	{ "DefaultFVRowCount", pr_int, &default_fv_row_count, NULL, NULL, 'S', NULL, 1, NULL },
	{ "DefaultFVColCount", pr_int, &default_fv_col_count, NULL, NULL, 'S', NULL, 1, NULL },
	{ "OnlyCopyDisplayed", pr_bool, &onlycopydisplayed, NULL, NULL, '\0', NULL, 1, NULL },
	{ "DefaultOutputFormat", pr_int, &unused, NULL, NULL, '\0', NULL, 1, NULL },
	{ "DefaultBitmapFormat", pr_int, &unused, NULL, NULL, '\0', NULL, 1, NULL },
	{ "SaveValidate", pr_int, &old_validate, NULL, NULL, '\0', NULL, 1, NULL },
	{ "SaveFontLogAsk", pr_int, &old_fontlog, NULL, NULL, '\0', NULL, 1, NULL },
	{ "DefaultTTFflags", pr_int, &unused, NULL, NULL, '\0', NULL, 1, NULL },
	{ "DefaultPSflags", pr_int, &unused, NULL, NULL, '\0', NULL, 1, NULL },
	{ "DefaultOTFflags", pr_int, &unused, NULL, NULL, '\0', NULL, 1, NULL },
	{ "PageWidth", pr_int, &unused, NULL, NULL, '\0', NULL, 1, NULL },
	{ "PageHeight", pr_int, &unused, NULL, NULL, '\0', NULL, 1, NULL },
	{ "PrintType", pr_int, &unused, NULL, NULL, '\0', NULL, 1, NULL },
	{ "PrintCommand", pr_string, &unused_string, NULL, NULL, '\0', NULL, 1, NULL },
	{ "PageLazyPrinter", pr_string, &unused_string, NULL, NULL, '\0', NULL, 1, NULL },
	{ "CoverageFormatsAllowed", pr_int, &coverageformatsallowed, NULL, NULL, '\0', NULL, 1, NULL },
	{ "ForceNamesWhenOpening", pr_namelist, &force_names_when_opening, NULL, NULL, '\0', NULL, 1, NULL },
	{ "ForceNamesWhenSaving", pr_namelist, &force_names_when_saving, NULL, NULL, '\0', NULL, 1, NULL },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, 0, NULL },
},
   extras[] = {
	{ N_("ResourceFile"), pr_file, &xdefs_filename, NULL, NULL, 'R', NULL, 0, N_("When FontForge starts up, it loads display related resources from a\nproperty on the screen. Sometimes it is useful to be able to store\nthese resources in a file. These resources are only read at start\nup, so changing this has no effect until the next time you start\nFontForge.") },
	{ N_("HelpDir"), pr_file, &helpdir, NULL, NULL, 'H', NULL, 0, N_("The directory on your local system in which FontForge will search for help\nfiles.  If a file is not found there, then FontForge will look for it on the net.") },
	{ N_("SplashScreen"), pr_bool, &splash, NULL, NULL, 'S', NULL, 0, N_("Show splash screen on start-up") },
	{ N_("GlyphAutoGoto"), pr_bool, &cv_auto_goto, NULL, NULL, '\0', NULL, 0, N_("Typing a normal character in the glyph view window changes the window to look at that character") },
	{ N_("OpenCharsInNewWindow"), pr_bool, &OpenCharsInNewWindow, NULL, NULL, '\0', NULL, 0, N_("When double clicking on a character in the font view\nopen that character in a new window, otherwise\nreuse an existing one.") },
	{ N_("ArrowMoveSize"), pr_real, &arrowAmount, NULL, NULL, '\0', NULL, 0, N_("The number of em-units by which an arrow key will move a selected point") },
	{ N_("ArrowAccelFactor"), pr_real, &arrowAccelFactor, NULL, NULL, '\0', NULL, 0, N_("Holding down the Alt (or Meta) key will speed up arrow key motion by this factor") },
	{ N_("SnapDistance"), pr_real, &snapdistance, NULL, NULL, '\0', NULL, 0, N_("When the mouse pointer is within this many pixels\nof one of the various interesting features (baseline,\nwidth, grid splines, etc.) the pointer will snap\nto that feature.") },
	{ N_("StopAtJoin"), pr_bool, &stop_at_join, NULL, NULL, '\0', NULL, 0, N_("When dragging points in the outline view a join may occur\n(two open contours may connect at their endpoints). When\nthis is On a join will cause FontForge to stop moving the\nselection (as if the user had released the mouse button).\nThis is handy if your fingers are inclined to wiggle a bit.") },
	{ N_("UpdateFlex"), pr_bool, &updateflex, NULL, NULL, '\0', NULL, 0, N_("Figure out flex hints after every change") },
	{ N_("AskBDFResolution"), pr_bool, &ask_user_for_resolution, NULL, NULL, 'B', NULL, 0, N_("When generating a set of BDF fonts ask the user\nto specify the screen resolution of the fonts\notherwise FontForge will guess depending on the pixel size.") },
	{ "DefaultFVShowHmetrics", pr_int, &default_fv_showhmetrics, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultFVShowVmetrics", pr_int, &default_fv_showvmetrics, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultFVGlyphLabel", pr_int, &default_fv_glyphlabel, NULL, NULL, 'S', NULL, 1, NULL  },
	{ "SaveToDir", pr_int, &save_to_dir, NULL, NULL, 'S', NULL, 1, NULL  },
	{ "PalettesDocked", pr_bool, &palettes_docked, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "CVVisible0", pr_bool, &cvvisible[0], NULL, NULL, '\0', NULL, 1, NULL  },
	{ "CVVisible1", pr_bool, &cvvisible[1], NULL, NULL, '\0', NULL, 1, NULL  },
	{ "BVVisible0", pr_bool, &bvvisible[0], NULL, NULL, '\0', NULL, 1, NULL  },
	{ "BVVisible1", pr_bool, &bvvisible[1], NULL, NULL, '\0', NULL, 1, NULL  },
	{ "BVVisible2", pr_bool, &bvvisible[2], NULL, NULL, '\0', NULL, 1, NULL  },
	{ "InfoWindowDistance", pr_int, &infowindowdistance, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "MarkExtrema", pr_int, &loacal_markextrema, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "MarkPointsOfInflect", pr_int, &loacal_markpoi, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "ShowRulers", pr_bool, &loacal_showrulers, NULL, NULL, '\0', NULL, 1, N_("Display rulers in the Outline Glyph View") },
	{ "ShowCPInfo", pr_int, &loacal_showcpinfo, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "ShowSideBearings", pr_int, &loacal_showsidebearings, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "ShowRefNames", pr_int, &loacal_showrefnames, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "ShowPoints", pr_bool, &loacal_showpoints, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "ShowFilled", pr_int, &loacal_showfilled, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "ShowTabs", pr_int, &loacal_showtabs, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultScreenDpiSystem", pr_int, &oldsystem, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "OFLibUsername", pr_string, &oflib_username, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "OFLibPassword", pr_string, &oflib_password, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "RegularStar", pr_bool, &regular_star, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "PolyStar", pr_bool, &polystar, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "RectEllipse", pr_bool, &rectelipse, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "RectCenterOut", pr_bool, &center_out[0], NULL, NULL, '\0', NULL, 1, NULL  },
	{ "EllipseCenterOut", pr_bool, &center_out[1], NULL, NULL, '\0', NULL, 1, NULL  },
	{ "PolyStartPointCnt", pr_int, &ps_pointcnt, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "RoundRectRadius", pr_real, &rr_radius, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "StarPercent", pr_real, &star_percent, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DebugWins", pr_int, &debug_wins, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "GridFitDpi", pr_int, &gridfit_dpi, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "GridFitDepth", pr_int, &gridfit_depth, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "GridFitPointSize", pr_real, &gridfit_pointsizey, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "GridFitPointSizeX", pr_real, &gridfit_pointsizex, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "GridFitSameAs", pr_int, &gridfit_x_sameas_y, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "MVShowGrid", pr_int, &mvshowgrid, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultFontFilterIndex", pr_int, &default_font_filter_index, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "SeekChar", pr_unicode, &home_char, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "CompactOnOpen", pr_bool, &compact_font_on_open, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "PixmapDir", pr_file, &pixmapdir, NULL, NULL, 'R', NULL, 0, NULL },
	{ "DefaultCVWidth", pr_int, &default_cv_width, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultCVHeight", pr_int, &default_cv_height, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "FCShowHidden", pr_bool, &gfc_showhidden, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "FCDirPlacement", pr_int, &gfc_dirplace, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "FCBookmarks", pr_string, &gfc_bookmarks, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "OFLibAutomagicPreview", pr_int, &oflib_automagic_preview, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultMVWidth", pr_int, &mv_width, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultMVHeight", pr_int, &mv_height, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultBVWidth", pr_int, &bv_width, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "DefaultBVHeight", pr_int, &bv_height, NULL, NULL, '\0', NULL, 1, NULL  },
	{ "AnchorControlPixelSize", pr_int, &aa_pixelsize, NULL, NULL, '\0', NULL, 1, NULL  },
	{ NULL, 0, NULL, NULL, NULL, 0, NULL, 0, NULL },
},
 *prefs_list[] = { core_list, extras, NULL };

static int NOUI_GetPrefs(char *name,Val *val) {
    int i,j;
    
    for ( i=0; prefs_list[i]!=NULL; ++i ) for ( j=0; prefs_list[i][j].name!=NULL; ++j ) {
	if ( strcmp(prefs_list[i][j].name,name)==0 ) {
	    struct prefs_list *pf = &prefs_list[i][j];
	    if ( pf->type == pr_bool || pf->type == pr_int || pf->type == pr_unicode ) {
		val->type = v_int;
		val->u.ival = *((int *) (pf->val));
	    } else if ( pf->type == pr_string || pf->type == pr_file ) {
		val->type = v_str;
		val->u.sval = copy( *((char **) (pf->val)));
	    } else if ( pf->type == pr_encoding ) {
		val->type = v_str;
		if ( *((NameList **) (pf->val))==NULL )
		    val->u.sval = copy( "NULL" );
		else
		    val->u.sval = copy( (*((Encoding **) (pf->val)))->enc_name );
	    } else if ( pf->type == pr_namelist ) {
		val->type = v_str;
		val->u.sval = copy( (*((NameList **) (pf->val)))->title );
	    } else if ( pf->type == pr_real ) {
		val->type = v_real;
		val->u.fval = *((float *) (pf->val));
	    } else
return( false );

return( true );
	}
    }
return( false );
}

static int NOUI_SetPrefs(char *name,Val *val1, Val *val2) {
    int i,j;

    for ( i=0; prefs_list[i]!=NULL; ++i ) for ( j=0; prefs_list[i][j].name!=NULL; ++j ) {
	if ( strcmp(prefs_list[i][j].name,name)==0 ) {
	    struct prefs_list *pf = &prefs_list[i][j];
	    if ( pf->type == pr_bool || pf->type == pr_int || pf->type == pr_unicode ) {
		if ( (val1->type!=v_int && val1->type!=v_unicode) || val2!=NULL )
return( -1 );
		*((int *) (pf->val)) = val1->u.ival;
	    } else if ( pf->type == pr_real ) {
		if ( val1->type==v_real && val2==NULL )
		    *((float *) (pf->val)) = val1->u.fval;
		else if ( val1->type!=v_int || (val2!=NULL && val2->type!=v_int ))
return( -1 );
		else
		    *((float *) (pf->val)) = (val2==NULL ? val1->u.ival : val1->u.ival / (double) val2->u.ival);
	    } else if ( pf->type == pr_string || pf->type == pr_file ) {
		if ( val1->type!=v_str || val2!=NULL )
return( -1 );
		if ( pf->set ) {
		    pf->set( val1->u.sval );
		} else {
		    free( *((char **) (pf->val)));
		    *((char **) (pf->val)) = copy( val1->u.sval );
		}
	    } else if ( pf->type == pr_encoding ) {
		if ( val2!=NULL )
return( -1 );
		else if ( val1->type==v_str && pf->val == &default_encoding) {
		    Encoding *enc = FindOrMakeEncoding(val1->u.sval);
		    if ( enc==NULL )
return( -1 );
		    *((Encoding **) (pf->val)) = enc;
		} else
return( -1 );
	    } else if ( pf->type == pr_namelist ) {
		if ( val2!=NULL )
return( -1 );
		else if ( val1->type==v_str ) {
		    NameList *nl = NameListByName(val1->u.sval);
		    if ( strcmp(val1->u.sval,"NULL")==0 && pf->val != &namelist_for_new_fonts )
			nl = NULL;
		    else if ( nl==NULL )
return( -1 );
		    *((NameList **) (pf->val)) = nl;
		} else
return( -1 );
	    } else
return( false );

	    SavePrefs(true);
return( true );
	}
    }
return( false );
}

static char *NOUI_getFontForgeShareDir(void) {
#if defined(SHAREDIR)
return( SHAREDIR "/fontforge" );
#elif defined(PREFIX)
return( PREFIX "/share/fontforge" );
#else
return( NULL );
#endif
}

static void DefaultXUID(void) {
    /* Adobe has assigned PfaEdit a base XUID of 1021. Each new user is going */
    /*  to get a couple of random numbers appended to that, hoping that will */
    /*  make for a fairly safe system. */
    /* FontForge will use the same scheme */
    int r1, r2;
    char buffer[50];
    struct timeval tv;

    gettimeofday(&tv,NULL);
    srand(tv.tv_usec);
    do {
	r1 = rand()&0x3ff;
    } while ( r1==0 );		/* I reserve "0" for me! */
    gettimeofday(&tv,NULL);
    r2 = rand();
    sprintf( buffer, "1021 %d %d", r1, r2 );
    free(xuid);
    xuid = copy(buffer);
}

static void NOUI_SetDefaults(void) {
    DefaultXUID();
}

static void NOUI_LoadPrefs(void) {
}

static void NOUI_SavePrefs(int not_if_script) {
  (void)not_if_script;
}

static struct prefs_interface prefsnoui = {
    NOUI_SavePrefs,
    NOUI_LoadPrefs,
    NOUI_GetPrefs,
    NOUI_SetPrefs,
    NOUI_getFontForgeShareDir,
    NOUI_SetDefaults
};

struct prefs_interface *prefs_interface = &prefsnoui;

void FF_SetPrefsInterface(struct prefs_interface *prefsi) {
    prefs_interface = prefsi;
}
