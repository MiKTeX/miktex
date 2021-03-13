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

#include "fontforgevw.h"
#include <utype.h>
#include <ustring.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gfile.h>
#include <time.h>
#include "unicoderange.h"
#include "psfont.h"
#if defined(MIKTEX)
#include <miktex/Core/c/api.h>
#endif

#ifdef _WIN32
#define MKDIR(A,B) mkdir(A)
#else
#define MKDIR(A,B) mkdir(A,B)
#endif

#if defined(MIKTEX) && (!defined(P_tmpdir) || _MSC_VER > 1700)
#  define P_tmpdir "/tmp"
#endif

void SFUntickAll(SplineFont *sf) {
    int i;

    for ( i=0; i<sf->glyphcnt; ++i ) if ( sf->glyphs[i]!=NULL )
	sf->glyphs[i]->ticked = false;
}

SplineChar *SCBuildDummy(SplineChar *dummy,SplineFont *sf,EncMap *map,int i) {
    static char namebuf[100];
    static Layer layers[2];

    memset(dummy,'\0',sizeof(*dummy));
    dummy->color = COLOR_DEFAULT;
    dummy->layer_cnt = 2;
    dummy->layers = layers;
    if ( sf->cidmaster!=NULL ) {
	/* CID fonts don't have encodings, instead we must look up the cid */
	if ( sf->cidmaster->loading_cid_map )
	    dummy->unicodeenc = -1;
	else
	    dummy->unicodeenc = CID2NameUni(FindCidMap(sf->cidmaster->cidregistry,sf->cidmaster->ordering,sf->cidmaster->supplement,sf->cidmaster),
		    i,namebuf,sizeof(namebuf));
    } else
	dummy->unicodeenc = UniFromEnc(i,map->enc);

    if ( sf->cidmaster!=NULL )
	dummy->name = namebuf;
    else if ( map->enc->psnames!=NULL && i<map->enc->char_cnt &&
	    map->enc->psnames[i]!=NULL )
	dummy->name = map->enc->psnames[i];
    else if ( dummy->unicodeenc==-1 )
	dummy->name = NULL;
    else
	dummy->name = (char *) StdGlyphName(namebuf,dummy->unicodeenc,sf->uni_interp,sf->for_new_glyphs);
    if ( dummy->name==NULL ) {
	/*if ( dummy->unicodeenc!=-1 || i<256 )
	    dummy->name = ".notdef";
	else*/ {
	    int j;
	    sprintf( namebuf, "NameMe.%d", i);
	    j=0;
	    while ( SFFindExistingSlot(sf,-1,namebuf)!=-1 )
		sprintf( namebuf, "NameMe.%d.%d", i, ++j);
	    dummy->name = namebuf;
	}
    }
    dummy->width = dummy->vwidth = sf->ascent+sf->descent;
    if ( dummy->unicodeenc>0 && dummy->unicodeenc<0x10000 &&
	    iscombining(dummy->unicodeenc)) {
	/* Mark characters should be 0 width */
	dummy->width = 0;
	/* Except in monospaced fonts on windows, where they should be the */
	/*  same width as everything else */
    }
    /* Actually, in a monospace font, all glyphs should be the same width */
    /*  whether mark or not */
    if ( sf->pfminfo.panose_set && sf->pfminfo.panose[3]==9 &&
	    sf->glyphcnt>0 ) {
	for ( i=sf->glyphcnt-1; i>=0; --i )
	    if ( SCWorthOutputting(sf->glyphs[i])) {
		dummy->width = sf->glyphs[i]->width;
	break;
	    }
    }
    dummy->parent = sf;
    dummy->orig_pos = 0xffff;
return( dummy );
}

static SplineChar *_SFMakeChar(SplineFont *sf,EncMap *map,int enc) {
    SplineChar dummy, *sc;
    SplineFont *ssf;
    int j, real_uni, gid;
    extern const int cns14pua[], amspua[];

    if ( enc>=map->enccount )
	gid = -1;
    else
	gid = map->map[enc];
    if ( sf->subfontcnt!=0 && gid!=-1 ) {
	ssf = NULL;
	for ( j=0; j<sf->subfontcnt; ++j )
	    if ( gid<sf->subfonts[j]->glyphcnt ) {
		ssf = sf->subfonts[j];
		if ( ssf->glyphs[gid]!=NULL ) {
return( ssf->glyphs[gid] );
		}
	    }
	sf = ssf;
    }

    if ( gid==-1 || (sc = sf->glyphs[gid])==NULL ) {
	if (( map->enc->is_unicodebmp || map->enc->is_unicodefull ) &&
		( enc>=0xe000 && enc<=0xf8ff ) &&
		( sf->uni_interp==ui_ams || sf->uni_interp==ui_trad_chinese ) &&
		( real_uni = (sf->uni_interp==ui_ams ? amspua : cns14pua)[enc-0xe000])!=0 ) {
	    if ( real_uni<map->enccount ) {
		SplineChar *sc;
		/* if necessary, create the real unicode code point */
		/*  and then make us be a duplicate of it */
		sc = _SFMakeChar(sf,map,real_uni);
		map->map[enc] = gid = sc->orig_pos;
		SCCharChangedUpdate(sc,ly_all);
return( sc );
	    }
	}

	SCBuildDummy(&dummy,sf,map,enc);
	/* Let's say a user has a postscript encoding where the glyph ".notdef" */
	/*  is assigned to many slots. Once the user creates a .notdef glyph */
	/*  all those slots should fill in. If they don't they damn well better*/
	/*  when the user clicks on one to edit it */
	/* Used to do that with all encodings. It just confused people */
	if ( map->enc->psnames!=NULL &&
		(sc = SFGetChar(sf,dummy.unicodeenc,dummy.name))!=NULL ) {
	    map->map[enc] = sc->orig_pos;
return( sc );
	}
	sc = SFSplineCharCreate(sf);
	sc->unicodeenc = dummy.unicodeenc;
	sc->name = copy(dummy.name);
	sc->width = dummy.width;
	sc->orig_pos = 0xffff;
	/*SCLigDefault(sc);*/
	SFAddGlyphAndEncode(sf,sc,map,enc);
    }
return( sc );
}

SplineChar *SFMakeChar(SplineFont *sf,EncMap *map, int enc) {
    int gid;

    if ( enc==-1 )
return( NULL );
    if ( enc>=map->enccount )
	gid = -1;
    else
	gid = map->map[enc];
    if ( sf->mm!=NULL && (gid==-1 || sf->glyphs[gid]==NULL) ) {
	int j;
	_SFMakeChar(sf->mm->normal,map,enc);
	for ( j=0; j<sf->mm->instance_count; ++j )
	    _SFMakeChar(sf->mm->instances[j],map,enc);
    }
return( _SFMakeChar(sf,map,enc));
}

struct unicoderange specialnames[] = {
    { NULL, 0, 0, 0, 0, 0, 0 }
};


static SplineFont *_SFReadPostscript(FILE *file,char *filename) {
    FontDict *fd=NULL;
    SplineFont *sf=NULL;

    ff_progress_change_stages(2);
    fd = _ReadPSFont(file);
    ff_progress_next_stage();
    ff_progress_change_line2(_("Interpreting Glyphs"));
    if ( fd!=NULL ) {
	sf = SplineFontFromPSFont(fd);
	PSFontFree(fd);
	if ( sf!=NULL )
	    CheckAfmOfPostscript(sf,filename,sf->map);
    }
return( sf );
}

static SplineFont *SFReadPostscript(char *filename) {
    FontDict *fd=NULL;
    SplineFont *sf=NULL;

    ff_progress_change_stages(2);
    fd = ReadPSFont(filename);
    ff_progress_next_stage();
    ff_progress_change_line2(_("Interpreting Glyphs"));
    if ( fd!=NULL ) {
	sf = SplineFontFromPSFont(fd);
	PSFontFree(fd);
	if ( sf!=NULL )
	    CheckAfmOfPostscript(sf,filename,sf->map);
    }
return( sf );
}

    
struct compressors compressors[] = {
    { ".gz", "gunzip", "gzip" },
    { ".bz2", "bunzip2", "bzip2" },
    { ".bz", "bunzip2", "bzip2" },
    { ".Z", "gunzip", "compress" },
/* file types which are both archived and compressed (.tgz, .zip) are handled */
/*  by the archiver above */
    { NULL, NULL, NULL }
};

char *Decompress(char *name, int compression) {
    char *dir = getenv("TMPDIR");
    char buf[1500];
    char *tmpfile;

    if (   strchr(name,'$') || strchr(name,'!') || strchr(name,'*') || strchr(name,'?')
        || strchr(name,'[') || strchr(name,'[') || strchr(name,']') || strchr(name,']') 
        || strchr(name,';') || strchr(name,'&') || strchr(name,'>') || strchr(name,'<')
        || strchr(name,'`') || strchr(name,'"') || strchr(name,'|') || strchr(name,'\'')
        || strchr(name,'\\')) 
return( NULL );
    if ( dir==NULL ) dir = P_tmpdir;
    tmpfile = galloc(strlen(dir)+strlen(GFileNameTail(name))+2);
    strcpy(tmpfile,dir);
    strcat(tmpfile,"/");
    strcat(tmpfile,GFileNameTail(name));
    *strrchr(tmpfile,'.') = '\0';
#if defined( _NO_SNPRINTF ) || defined( __VMS )
    sprintf( buf, "%s < \"%s\" > \"%s\"", compressors[compression].decomp, name, tmpfile );
#else
    snprintf( buf, sizeof(buf), "%s < \"%s\" > \"%s\"", compressors[compression].decomp, name, tmpfile );
#endif
#if defined(MIKTEX)
    if (miktex_system(buf) == 0)
#else
    if ( system(buf)==0 )
#endif
return( tmpfile );
    free(tmpfile);
return( NULL );
}

static char *ForceFileToHaveName(FILE *file, char *exten) {
    char tmpfilename[L_tmpnam+100];
    static int try=0;
    FILE *newfile;

    forever {
	sprintf( tmpfilename, P_tmpdir "/fontforge%d-%d", getpid(), try++ );
	if ( exten!=NULL )
	    strcat(tmpfilename,exten);
	if ( access( tmpfilename, F_OK )==-1 &&
		(newfile = fopen(tmpfilename,"w"))!=NULL ) {
	    char buffer[1024];
	    int len;
	    while ( (len = fread(buffer,1,sizeof(buffer),file))>0 )
		fwrite(buffer,1,len,newfile);
	    fclose(newfile);
	}
return(copy(tmpfilename));			/* The filename does not exist */
    }
}

/* This does not check currently existing fontviews, and should only be used */
/*  by LoadSplineFont (which does) and by RevertFile (which knows what it's doing) */
SplineFont *_ReadSplineFont(FILE *file,char *filename,enum openflags openflags) {
    SplineFont *sf;
    char ubuf[250], *temp;
    int fromsfd = false;
    int i;
    char *pt, *strippedname, *oldstrippedname, *tmpfile=NULL, *paren=NULL, *fullname=filename, *rparen;
    int len;
    int checked;
    int compression=0;
    int wasurl = false, nowlocal = true;

    if ( filename==NULL )
return( NULL );

    strippedname = filename;
    pt = strrchr(filename,'/');
    if ( pt==NULL ) pt = filename;
    /* Someone gave me a font "Nafees Nastaleeq(Updated).ttf" and complained */
    /*  that ff wouldn't open it */
    /* Now someone will complain about "Nafees(Updated).ttc(fo(ob)ar)" */
    if ( (paren = strrchr(pt,'('))!=NULL &&
	    (rparen = strrchr(paren,')'))!=NULL &&
	    rparen[1]=='\0' ) {
	strippedname = copy(filename);
	strippedname[paren-filename] = '\0';
    }

    pt = strrchr(strippedname,'.');

    i = -1;
    if ( pt!=NULL ) for ( i=0; compressors[i].ext!=NULL; ++i )
	if ( strcmp(compressors[i].ext,pt)==0 )
    break;
    oldstrippedname = strippedname;
    if ( i==-1 || compressors[i].ext==NULL )
	i=-1;
    else {
	if ( file!=NULL ) {
	    char *spuriousname = ForceFileToHaveName(file,compressors[i].ext);
	    tmpfile = Decompress(spuriousname,i);
	    fclose(file); file = NULL;
	    unlink(spuriousname); free(spuriousname);
	} else
	    tmpfile = Decompress(strippedname,i);
	if ( tmpfile!=NULL ) {
	    strippedname = tmpfile;
	} else {
	    ff_post_error(_("Decompress Failed!"),_("Decompress Failed!"));
return( NULL );
	}
	compression = i+1;
	if ( strippedname!=filename && paren!=NULL ) {
	    fullname = galloc(strlen(strippedname)+strlen(paren)+1);
	    strcpy(fullname,strippedname);
	    strcat(fullname,paren);
	} else
	    fullname = strippedname;
    }

    /* If there are no pfaedit windows, give them something to look at */
    /*  immediately. Otherwise delay a bit */
    strcpy(ubuf,_("Loading font from "));
    len = strlen(ubuf);
    if ( !wasurl || i==-1 )	/* If it wasn't compressed, or it wasn't an url, then the fullname is reasonable, else use the original name */
	strncat(ubuf,temp = copy(GFileNameTail(fullname)),100);
    else
	strncat(ubuf,temp = copy(GFileNameTail(filename)),100);
    free(temp);
    ubuf[100+len] = '\0';
    ff_progress_start_indicator(FontViewFirst()==NULL?0:10,_("Loading..."),ubuf,_("Reading Glyphs"),0,1);
    ff_progress_enable_stop(0);

    if ( file==NULL ) {
	file = fopen(strippedname,"rb");
	nowlocal = true;
    }

    sf = NULL;
    checked = false;
/* checked == false => not checked */
/* checked == 'u'   => UFO */
/* checked == 't'   => TTF/OTF */
/* checked == 'p'   => pfb/general postscript */
/* checked == 'P'   => pdf */
/* checked == 'c'   => cff */
/* checked == 'S'   => svg */
/* checked == 'f'   => sfd */
/* checked == 'F'   => sfdir */
/* checked == 'b'   => bdf */
/* checked == 'i'   => ikarus */
    if ( file!=NULL ) {
	/* Try to guess the file type from the first few characters... */
	int ch1 = getc(file);
	int ch2 = getc(file);
	int ch3 = getc(file);
	int ch4 = getc(file);
	int ch9, ch10;
	fseek(file, 98, SEEK_SET);
	ch9 = getc(file);
	ch10 = getc(file);
	rewind(file);
	if (( ch1==0 && ch2==1 && ch3==0 && ch4==0 ) ||
		(ch1=='O' && ch2=='T' && ch3=='T' && ch4=='O') ||
		(ch1=='t' && ch2=='r' && ch3=='u' && ch4=='e') ||
		(ch1=='t' && ch2=='t' && ch3=='c' && ch4=='f') ) {
	    sf = _SFReadTTF(file,0,openflags,fullname,NULL);
	    checked = 't';
	} else if (( ch1=='%' && ch2=='!' ) ||
		    ( ch1==0x80 && ch2=='\01' ) ) {	/* PFB header */
	    sf = _SFReadPostscript(file,fullname);
	    checked = 'p';
	} else if ( ch1==1 && ch2==0 && ch3==4 ) {
	    int len;
	    fseek(file,0,SEEK_END);
	    len = ftell(file);
	    fseek(file,0,SEEK_SET);
	    sf = _CFFParse(file,len,NULL);
	    checked = 'c';
	} /* Too hard to figure out a valid mark for a mac resource file */
	if ( file!=NULL ) fclose(file);
    }

    if ( sf!=NULL )
	/* good */;
    else if (( strmatch(fullname+strlen(fullname)-4, ".ttf")==0 ||
		strmatch(fullname+strlen(strippedname)-4, ".ttc")==0 ||
		strmatch(fullname+strlen(fullname)-4, ".gai")==0 ||
		strmatch(fullname+strlen(fullname)-4, ".otf")==0 ||
		strmatch(fullname+strlen(fullname)-4, ".otb")==0 ) && checked!='t') {
	sf = SFReadTTF(fullname,0,openflags);
    } else if ( strmatch(fullname+strlen(strippedname)-4, ".bin")==0 ||
		strmatch(fullname+strlen(strippedname)-4, ".hqx")==0 ||
		strmatch(fullname+strlen(strippedname)-6, ".dfont")==0 ) {
	sf = SFReadMacBinary(fullname,0,openflags);
    } else if ( (strmatch(fullname+strlen(fullname)-4, ".pfa")==0 ||
		strmatch(fullname+strlen(fullname)-4, ".pfb")==0 ||
		strmatch(fullname+strlen(fullname)-4, ".pf3")==0 ||
		strmatch(fullname+strlen(fullname)-4, ".cid")==0 ||
		strmatch(fullname+strlen(fullname)-4, ".gsf")==0 ||
		strmatch(fullname+strlen(fullname)-4, ".pt3")==0 ||
		strmatch(fullname+strlen(fullname)-3, ".ps")==0 ) && checked!='p' ) {
	sf = SFReadPostscript(fullname);
    } else if ( strmatch(fullname+strlen(fullname)-4, ".cff")==0 && checked!='c' ) {
	sf = CFFParse(fullname);
    } else {
	sf = SFReadMacBinary(fullname,0,openflags);
    }
    ff_progress_end_indicator();

    if ( sf!=NULL ) {
	SplineFont *norm = sf->mm!=NULL ? sf->mm->normal : sf;
	if ( compression!=0 ) {
	    free(sf->filename);
	    *strrchr(oldstrippedname,'.') = '\0';
	    sf->filename = copy( oldstrippedname );
	}
	if ( fromsfd )
	    sf->compression = compression;
	free( norm->origname );
	if ( sf->chosenname!=NULL && strippedname==filename ) {
	    norm->origname = galloc(strlen(filename)+strlen(sf->chosenname)+8);
	    strcpy(norm->origname,filename);
	    strcat(norm->origname,"(");
	    strcat(norm->origname,sf->chosenname);
	    strcat(norm->origname,")");
	} else
	    norm->origname = copy(filename);
	free( norm->chosenname ); norm->chosenname = NULL;
	if ( sf->mm!=NULL ) {
	    int j;
	    for ( j=0; j<sf->mm->instance_count; ++j ) {
		free(sf->mm->instances[j]->origname);
		sf->mm->instances[j]->origname = copy(norm->origname);
	    }
	}
    } else if ( !GFileExists(filename) )
	ff_post_error(_("Couldn't open font"),_("The requested file, %.100s, does not exist"),GFileNameTail(filename));
    else if ( !GFileReadable(filename) )
	ff_post_error(_("Couldn't open font"),_("You do not have permission to read %.100s"),GFileNameTail(filename));
    else
	ff_post_error(_("Couldn't open font"),_("%.100s is not in a known format (or is so badly corrupted as to be unreadable)"),GFileNameTail(filename));

    if ( oldstrippedname!=filename )
	free(oldstrippedname);
    if ( fullname!=filename && fullname!=strippedname )
	free(fullname);
    if ( tmpfile!=NULL ) {
	unlink(tmpfile);
	free(tmpfile);
    }
    if ( (openflags&of_fstypepermitted) && sf!=NULL && (sf->pfminfo.fstype&0xff)==0x0002 ) {
	/* Ok, they have told us from a script they have access to the font */
    } else if ( !fromsfd && sf!=NULL && (sf->pfminfo.fstype&0xff)==0x0002 ) {
	char *buts[3];
	buts[0] = _("_Yes"); buts[1] = _("_No"); buts[2] = NULL;
	if ( ff_ask(_("Restricted Font"),(const char **) buts,1,1,_("This font is marked with an FSType of 2 (Restricted\nLicense). That means it is not editable without the\npermission of the legal owner.\n\nDo you have such permission?"))==1 ) {
	    SplineFontFree(sf);
return( NULL );
	}
    }
return( sf );
}

SplineFont *ReadSplineFont(char *filename,enum openflags openflags) {
return( _ReadSplineFont(NULL,filename,openflags));
}


SplineFont *ReadSplineFontInfo(char *filename,enum openflags openflags) {
  SplineFont *sf, *sf_ptr;
	char **fontlist; 
    char *pt =NULL, *strippedname=filename, *paren=NULL, *rparen=NULL, *fullname=filename;
    FILE *foo = NULL;
    int checked = 0;
	char s[512] = {0};

    if ( filename==NULL )
return( NULL );

    pt = strrchr(filename,'/');
    if ( pt==NULL ) pt = filename;
    /* Someone gave me a font "Nafees Nastaleeq(Updated).ttf" and complained */
    /*  that ff wouldn't open it */
    /* Now someone will complain about "Nafees(Updated).ttc(fo(ob)ar)" */
    if ( (paren = strrchr(pt,'('))!=NULL &&
	    (rparen = strrchr(paren,')'))!=NULL &&
	    rparen[1]=='\0' ) {
	strippedname = copy(filename);
	strippedname[paren-filename] = '\0';
    }

    sf = NULL;
    foo = fopen(strippedname,"rb");
    checked = false;
    if ( foo!=NULL ) {
	/* Try to guess the file type from the first few characters... */
	int ch1 = getc(foo);
	int ch2 = getc(foo);
	int ch3 = getc(foo);
	int ch4 = getc(foo);
	fclose(foo);
	if (( ch1==0 && ch2==1 && ch3==0 && ch4==0 ) ||
		(ch1=='O' && ch2=='T' && ch3=='T' && ch4=='O') ||
		(ch1=='t' && ch2=='r' && ch3=='u' && ch4=='e') ) {
	    sf = SFReadTTFInfo(fullname,0,openflags);
	    checked = 't';
	} else if ((ch1=='t' && ch2=='t' && ch3=='c' && ch4=='f')) {
          char **old_fontlist;
          int i;
          /* read all fonts in a collection */
	  fontlist = NamesReadTTF(fullname);
          old_fontlist = fontlist;
	  if (fontlist) {
		while (*fontlist != NULL) {
		  snprintf(s,511, "%s(%s)", fullname,*fontlist);
		  sf_ptr = SFReadTTFInfo(s,0,openflags);
		  if (sf != NULL)
			sf_ptr->next = sf;	  
		  sf = sf_ptr;
		  fontlist++;
		}
                /* fontlist is (g)allocated */
                fontlist = old_fontlist;
                for(i=0; fontlist[i]; i++)
                      free(fontlist[i]);
                free(fontlist);
                old_fontlist = NULL;
	  }
    } else if ( strmatch(fullname+strlen(strippedname)-4, ".bin")==0 ||
                strmatch(fullname+strlen(strippedname)-4, ".hqx")==0 ||
                strmatch(fullname+strlen(strippedname)-6, ".dfont")==0 ) {
	  fontlist = NamesReadMacBinary(fullname);
	  if (fontlist) {
		while (*fontlist != NULL) {
		  snprintf(s,511, "%s(%s)", fullname,*fontlist);
		  sf_ptr = SFReadMacBinaryInfo(s,0,openflags);
		  if (sf != NULL)
			sf_ptr->next = sf;	  
		  sf = sf_ptr;
		  fontlist++;
		}
	  }
	} else {
      sf = ReadSplineFont (fullname, openflags);
    }
    }
    if ( strippedname!=filename )
      free(strippedname);
return( sf );
}


/* Use URW 4 letter abbreviations */
char *knownweights[] = { "Demi", "Bold", "Regu", "Medi", "Book", "Thin",
	"Ligh", "Heav", "Blac", "Ultr", "Nord", "Norm", "Gras", "Stan", "Halb",
	"Fett", "Mage", "Mitt", "Buch", NULL };
char *realweights[] = { "Demi", "Bold", "Regular", "Medium", "Book", "Thin",
	"Light", "Heavy", "Black", "Ultra", "Nord", "Normal", "Gras", "Standard", "Halbfett",
	"Fett", "Mager", "Mittel", "Buchschrift", NULL};
static char *moreweights[] = { "ExtraLight", "VeryLight", NULL };
char **noticeweights[] = { moreweights, realweights, knownweights, NULL };

static char *modifierlist[] = { "Ital", "Obli", "Kursive", "Cursive", "Slanted",
	"Expa", "Cond", NULL };
static char *modifierlistfull[] = { "Italic", "Oblique", "Kursive", "Cursive", "Slanted",
    "Expanded", "Condensed", NULL };
static char **mods[] = { knownweights, modifierlist, NULL };
static char **fullmods[] = { realweights, modifierlistfull, NULL };

char *_GetModifiers(char *fontname, char *familyname,char *weight) {
    char *pt, *fpt;
    int i, j;

    /* URW fontnames don't match the familyname */
    /* "NimbusSanL-Regu" vs "Nimbus Sans L" (note "San" vs "Sans") */
    /* so look for a '-' if there is one and use that as the break point... */

    if ( (fpt=strchr(fontname,'-'))!=NULL ) {
	++fpt;
	if ( *fpt=='\0' )
	    fpt = NULL;
    } else if ( familyname!=NULL ) {
	for ( pt = fontname, fpt=familyname; *fpt!='\0' && *pt!='\0'; ) {
	    if ( *fpt == *pt ) {
		++fpt; ++pt;
	    } else if ( *fpt==' ' )
		++fpt;
	    else if ( *pt==' ' )
		++pt;
	    else if ( *fpt=='a' || *fpt=='e' || *fpt=='i' || *fpt=='o' || *fpt=='u' )
		++fpt;	/* allow vowels to be omitted from family when in fontname */
	    else
	break;
	}
	if ( *fpt=='\0' && *pt!='\0' )
	    fpt = pt;
	else
	    fpt = NULL;
    }

    if ( fpt == NULL ) {
	for ( i=0; mods[i]!=NULL; ++i ) for ( j=0; mods[i][j]!=NULL; ++j ) {
	    pt = strstr(fontname,mods[i][j]);
	    if ( pt!=NULL && (fpt==NULL || pt<fpt))
		fpt = pt;
	}
    }
    if ( fpt!=NULL ) {
	for ( i=0; mods[i]!=NULL; ++i ) for ( j=0; mods[i][j]!=NULL; ++j ) {
	    if ( strcmp(fpt,mods[i][j])==0 )
return( fullmods[i][j]);
	}
	if ( strcmp(fpt,"BoldItal")==0 )
return( "BoldItalic" );
	else if ( strcmp(fpt,"BoldObli")==0 )
return( "BoldOblique" );

return( fpt );
    }

return( weight==NULL || *weight=='\0' ? "Regular": weight );
}

char *SFGetModifiers(SplineFont *sf) {
return( _GetModifiers(sf->fontname,sf->familyname,sf->weight));
}

