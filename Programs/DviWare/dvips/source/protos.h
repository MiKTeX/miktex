#ifndef _PROTOS_H_

/* see dvips.h for copyright notice */

#define _PROTOS_H_

/* some types need declaration before being used */
struct nlink;
struct hps_link;
struct Char;
struct String;
struct tcd;

/******* prototypes for functions *******/

/* prototypes for functions from bbox.c */
extern void findbb(int bop);

/* prototypes for functions from bitmapenc.c */
extern void bmenc_startsection(void) ;
extern void bitmapencopt(int) ;
extern int downloadbmencoding(const char *name, double scale, fontdesctype *curfnt) ;
extern void finishbitmapencoding(const char *name, double scale) ;

/* prototypes for functions from color.c */
extern void initcolor(void);
extern void background(char *bkgrnd);
extern void resetcolorstack(char *p, int outtops);
extern void bopcolor(int outtops);
extern void pushcolor(char *p, Boolean outtops);
extern void popcolor(Boolean outtops);

/* prototypes for functions from dopage.c */
extern void dopage(void);

/* prototypes for functions from dosection.c */
extern void dosection(sectiontype *s, int c);
extern int InPageList(int i);
extern int ParsePages(char *s);

/* prototypes for functions from dospecial.c */
extern void specerror(const char *s);
extern void outbangspecials(void);
extern void predospecial(int numbytes, Boolean scanning);
extern void dospecial(int numbytes);
extern float *bbdospecial(int nbytes);

/* prototypes for functions from download.c */
quarterword *unpack_bb(chardesctype *c, integer *cwidth, integer *cheight,
                                        integer *xoff, integer *yoff) ;
extern void download(charusetype *p, int psfont);
extern void makepsname(char *s, int n);
extern void lfontout(int n);
extern void dopsfont(sectiontype *fs);

/* prototypes for functions from dpicheck.c */
extern unsigned short dpicheck(unsigned short dpi);

/* prototypes for functions from drawPS.c */
#ifdef TPIC
extern void setPenSize(char *cp);
extern void addPath(char *cp);
extern void arc(char *cp, int invis);
extern void flushPath(int invis);
extern void flushDashed(char *cp, int dotted);
extern void flushSpline(char *cp);
extern void SetShade(char *cp);
extern void shadeLast(char *cp);
extern void whitenLast(void);
extern void blackenLast(void);
#endif /* TPIC */

/* prototypes for functions from dviinput.c */
extern short dvibyte(void);
extern unsigned short twobytes(void);
extern int threebytes(void);
extern short signedbyte(void);
extern short signedpair(void);
extern int signedtrio(void);
extern int signedquad(void);
extern void skipover(int i);

/* prototypes for functions from dvips.c */
extern void help(int status);
extern void error_with_perror(const char *s, const char *fname);
extern void error(const char *s);
extern void check_checksum(unsigned int c1, unsigned int c2, const char *name);
extern char *mymalloc(int n);
extern void checkstrings(void);
extern char *newstring(const char *s);
extern void *revlist(void *p);

/* prototypes for functions from emspecial.c */
extern void emclear(void);
extern void emspecial(char *p);

/* prototypes for functions from finclude.c */
extern void scanfontcomments(const char *filename);
extern void fonttableout(void);

/* prototypes for functions from flib.c */
#ifdef FONTLIB
extern void fliload(void);
extern char *fliparse(char *path, char *name)
#endif

/* prototypes for functions from fontdef.c */
extern fontdesctype *newfontdesc(int cksum, int scsize, int dssize, char *name, char *area);
extern fontdesctype *matchfont(char *name, char *area, int scsize, char *scname);
extern void fontdef(int siz);
extern int skipnop(void);

/* prototypes for functions from header.c */
extern int add_name(const char *s, struct header_list **what );
extern void checkhmem(const char *s, char *p, char *q);
extern int add_header(const char *s);
extern int add_header_general(const char *s, char *pre, char* post);
extern char *get_name(struct header_list **what );
extern void send_headers(void);

/* prototypes for functions from hps.c */
#ifdef HPS
extern void do_html(char *s);
extern void finish_hps(void);
extern void set_bitfile(const char *s, int mode);
extern void vertical_in_hps(void);
extern void end_current_box(void);
extern void start_new_box(void);
#endif /* HPS */

/* prototypes for functions from loadfont.c */
extern void badpk(const char *s);
extern short pkbyte(void);
extern int pkquad(void);
extern int pktrio(void);
extern void loadfont(fontdesctype *curfnt);

/* prototypes for functions from makefont.c */
extern void makefont(char *name, int dpi, int bdpi);

/* prototypes for functions from output.c */
extern void copyfile(const char *s);
extern void copyfile_general(const char *s, struct header_list *h);
extern void figcopyfile(char *s, int systemtype);
extern void specialout(char c);
extern void stringend(void);
#ifdef SHIFTLOWCHARS
extern int T1Char(int c);
#endif
extern void scout(unsigned char c);
extern void cmdout(const char *s);
extern void floatout(float n);
extern void doubleout(double n);
extern void numout(int n);
extern void mhexout(unsigned char *p, long len);
extern void hvpos(void);
extern void newline(void);
extern void open_output(void);
extern void nlcmdout(const char *s);
extern void initprinter(sectiontype *sect);
extern void setup(void);
extern void cleanprinter(void);
extern void psflush(void);
extern void pslineout(const char *s);
extern void psnameout(const char *s);
extern void pageinit(void);
extern void pageend(void);
extern void drawrule(int rw, int rh);
extern void cmddir(void);
extern void drawchar(chardesctype * c, int cc);

/* prototypes for functions from papersiz.c */
extern void handlepapersize(char *p, int *x, int *y);

/* prototypes for functions from pprescan.c */
extern void pprescanpages(void);

/* prototypes for functions from prescan.c */
extern void readpreamble(void);
extern void prescanpages(void);

/* prototypes for functions from repack.c */
extern long getlong(unsigned char *a);
extern void dochar(unsigned char *from, short width, short height);
extern void repack(struct tcd *cp);

/* prototypes for functions from resident.c */
extern unsigned int hash(char *s);
extern void revpslists(void);
extern void cleanres(void);
extern struct resfont *lookup(char *name);
extern struct resfont *findPSname(char *name);
extern int residentfont(fontdesctype *curfnt);
extern Boolean getdefaults(const char *s);
extern void getpsinfo(const char *name);
extern void checkenv(int which);

/* prototypes for functions from scalewidth.c */
extern int scalewidth(int a, int b);

/* prototypes for functions from scanpage.c */
extern Boolean prescanchar(chardesctype *cd);
extern Boolean preselectfont(fontdesctype *f);
extern short scanpage(void);

/* prototypes for functions from search.c */
#ifdef KPATHSEA
extern FILE *search(kpse_file_format_type format, const char *file, const char *mode);
extern FILE *pksearch(const char *file, const char *mode, halfword dpi, const char **name_ret, int *dpi_ret);
#else /* !KPATSHEA */
extern FILE *search(char *path, const char *file, const char *mode);
extern FILE *pksearch(char *path, const char *file, const char *mode, char *n, halfword dpi, halfword vdpi);
#endif /* KPATHSEA */
extern FILE *my_real_fopen(const char *n, const char *t);
extern int close_file(FILE *f);

/* prototypes for functions from skippage.c */
extern void skippage(void);

/* prototypes for functions from t1part.c */
extern void *getmem(unsigned int size);
extern struct Char *UnDefineChars(struct Char *TmpChar);
extern int FontPart(FILE *fout, unsigned char *fontfile, unsigned char *vectfile);

/* prototypes for functions from tfmload.c */
extern void badtfm(const char *s);
extern void tfmopen(fontdesctype *fd);
extern unsigned short tfm16(void);
extern int tfm32(void);
extern int tfmload(fontdesctype *curfnt);

/* prototypes for functions from unpack.c */
extern long unpack(unsigned char *pack, unsigned short *raster,
                    unsigned short cwidth, unsigned short cheight, unsigned short cmd);

/* prototypes for functions from virtualfont.c */
extern Boolean virtualfont(fontdesctype *curfnt);

/* prototypes for functions from writet1.c */
extern char **load_enc_file(char *);
extern boolean t1_subset_2(char *, unsigned char *, char *);

/*********** global variables ***********/

/* global variables from bitmapenc.c */
extern int encodetype3 ;

/* global variables from dopage.c */
extern integer dir;
#ifdef HPS
extern integer hhmem, vvmem;
extern integer pushcount;
extern Boolean PAGEUS_INTERUPPTUS;
extern Boolean NEED_NEW_BOX;
#endif

/* global variables from dosection.c */
#ifdef HPS
extern int pagecounter;
#endif

/* global variables from dvips.c */
extern int found_problems;
extern char *downloadedpsnames[];
extern int unused_top_of_psnames;
extern fontdesctype *fonthead;
extern fontdesctype *curfnt;
extern sectiontype *sections;
extern Boolean partialdownload;
extern Boolean manualfeed;
extern Boolean compressed;
extern Boolean downloadpspk;
extern Boolean safetyenclose;
extern Boolean removecomments;
extern Boolean nosmallchars;
extern Boolean cropmarks;
extern Boolean abspage;
extern Boolean tryepsf;
extern int secure;
extern int secure_option;
extern int collatedcopies;
extern integer pagecopies;
extern shalfword linepos;
extern integer maxpages;
extern Boolean notfirst, notlast;
extern Boolean evenpages, oddpages, pagelist;
extern Boolean sendcontrolD;
extern Boolean shiftlowchars;
extern integer firstpage, lastpage;
extern integer firstseq, lastseq;
extern integer hpapersize, vpapersize;
extern integer hoff, voff;
extern integer maxsecsize;
extern integer firstboploc;
extern Boolean sepfiles;
extern int numcopies;
extern char *titlename;
extern const char *oname;
extern char *iname;
extern char *fulliname;
extern char *nextstring, *maxstring;
extern FILE *dvifile, *bitfile;
extern quarterword *curpos, *curlim;
extern fontmaptype *ffont;
extern real conv;
extern real vconv;
extern real alpha;
extern double mag;
extern integer num, den;
extern int overridemag;
extern int actualdpi, vactualdpi;
extern int maxdrift, vmaxdrift;
extern char *paperfmt;
extern int landscape;
extern integer fontmem;
extern integer pagecount;
extern integer pagenum;
extern long bytesleft;
extern quarterword *raster;
extern integer hh, vv;
extern Boolean noomega;
extern Boolean noptex;
extern Boolean lastpsizwins;
extern const char *infont;
#ifndef KPATHSEA
extern char *tfmpath;
extern char *pkpath;
extern char *vfpath;
extern char *figpath;
extern char *headerpath;
extern char *configpath;
extern char *pictpath;
#ifdef SEARCH_SUBDIRECTORIES
extern char *fontsubdirpath;
#endif
#endif /* ! KPATHSEA */
#ifdef FONTLIB
extern char *flipath;
extern char *fliname;
#endif
extern integer swmem;
extern int quiet;
extern int filter;
extern int dvips_debug_flag;
extern int prettycolumn;
extern int gargc;
extern char **gargv;
extern int totalpages;
extern Boolean reverse;
extern Boolean usesPSfonts;
extern Boolean usesspecial;
extern Boolean headers_off;
extern Boolean usescolor;
extern char *warningmsg;
extern Boolean multiplesects;
extern Boolean disablecomments;
extern char *printer;
extern char *mfmode;
extern char *mflandmode;
extern int mfmode_option;
extern int oname_option;
extern frametype frames[];
extern integer pagecost;
extern integer fsizetol;
extern Boolean includesfonts;
extern fontdesctype *fonthd[MAXFONTHD];
extern int nextfonthd;
extern char xdig[256];
extern char banner[], banner2[];
extern Boolean noenv;
extern Boolean dopprescan;
extern int dontmakefont;
extern struct papsiz *papsizes;
extern int headersready;
#if defined(MSDOS) || defined(OS2) || defined(ATARIST)
extern char *mfjobname;
extern FILE *mfjobfile;
#endif
#ifdef DEBUG
extern integer debug_flag;
#endif
#ifdef HPS
extern Boolean HPS_FLAG;
#endif
#if defined(MIKTEX)
extern int miktex_no_glyphs;
extern int miktex_no_rules;
extern int miktex_no_landscape;
extern int miktex_pedantic;
extern int miktex_allow_all_paths;
#endif

/* global variables from flib.c */
#ifdef FONTLIB
extern Boolean flib;
#endif

/* global variables from hps.c */
#ifdef HPS
extern Boolean inHTMLregion;
extern integer HREF_COUNT;
extern int current_pushcount;
extern Boolean noprocset;
#endif

/* global variables from loadfont.c */
extern char errbuf[1500];
extern int lastresortsizes[40];
extern FILE *pkfile;

/* global variables from output.c */
extern char preamblecomment[256];
extern integer rdir, fdir;

/* global variables from pprescan.c */
extern Boolean pprescan;

/* global variables from repack.c */
extern long mbytesleft;
extern quarterword *mraster;

/* global variables from resident.c */
extern struct header_list *ps_fonts_used;
extern const char *psmapfile;

/* global variables from search.c */
extern int to_close;
#ifdef KPATHSEA
extern char *realnameoffile;
#else
extern char realnameoffile[];
#endif

/* global variables from tfmload.c */
extern FILE *tfmfile;

/* global variables from loadfont.c */
extern int bitmapfontseen ;

#endif
