arith.c 
/* void DLmult(struct *product,unsigned long u,unsigned long v); */
/* void DLdiv(struct *quotient,unsigned long divisor); */
/* void DLadd(struct *u,struct *v); */
/* void DLsub(struct *u,struct *v); */
extern long FPmult(long u,long v);
extern long FPdiv(long dividend,long divisor);
extern long FPstarslash(long a,long b,long c);
basename.c 
extern int main(int argc,char **argv );
basics.c 
extern void fatal(char *fmt,...);
extern void msg(char *fmt,...);
bstring.c 
extern int memset(void *s,int c,int length);
curves.c 
extern int BezierTerminationTest(long xa,long ya,long xb,long yb,long xc,long yc,long xd,long yd);
static struct segment *StepBezierRecurse(struct bezierinfo *I,long xA,long yA,long xB,long yB,long xC,long yC,long xD,long yD);
extern struct segment *t1_StepBezier(struct region *R,long xA,long yA,long xB,long yB,long xC,long yC,long xD,long yD);
emxrun.c 
extern int main(int argc,char **argv );
extern int emx_system(char *cmd);
encoding.c 
static char *value_after(char *s,char *t);
extern int decimal(char *s);
extern char nextsymbol(void);
extern char *nextpsname(void);
extern void getenc(char **fontname ,char **encname ,char **ev ,int *width);
extern void addcc(char *name,int charcode);
extern int getcc(char *name);
extern char *string(char *s);
extern void remove_string(void);
filenames.c 
static char *charptr(char *name,char c);
extern int equal(char *s,char *t);
extern char *extension(char *str);
extern char *newname(char *name,char *ext);
extern char *ps2pk_basename(char *str,char *suffix);
extern int ps_resource(char *name);
flisearch.c 
extern int main(int argc,char **argv );
extern int matching(char *font,int size);
fontfcn.c 
extern int SearchDictName(struct ps_dict *dictP,struct ps_obj *keyP);
extern int initFont(void);
extern void resetFont(char *env);
extern int readFont(char *env);
extern unsigned char *fontfcnB(unsigned char *S,unsigned char *code,int *lenP,int *mode);
extern int fontfcnA(char *env,int *mode);
extern void QueryFontLib(char *env,char *infoName,unsigned char *infoValue,int *rcodeP);
hints.c 
extern void t1_InitHints(void);
extern void t1_CloseHints(struct fractpoint *hintP);
static void ComputeHint(struct hintsegment *hP,long currX,long currY,struct fractpoint *hintP);
extern void t1_ProcessHint(struct hintsegment *hP,long currX,long currY,struct fractpoint *hintP);
static short SearchXofY(struct edgelist *edge,short y);
static int ImpliedHorizontalLine(struct edgelist *e1,struct edgelist *e2,int y);
static void FixSubPaths(struct region *R);
static void DumpSubPaths(struct edgelist *anchor);
static struct edgelist *before(struct edgelist *e);
static void writeXofY(struct edgelist *e,int y,int x);
static void CollapseWhiteRun(struct edgelist *anchor,short yblack,struct edgelist *left,struct edgelist *right,short ywhite);
extern void t1_ApplyContinuity(struct region *R);
lines.c 
extern void t1_StepLine(struct region *R,long x1,long y1,long x2,long y2);
extern void t1_Bresenham(short *edgeP,long x1,long y1,long x2,long y2);
lspsres.c 
mag.c 
extern int main(int argc,char **argv );
extern int fontsize(double x);
extern double stepsize(double x);
objects.c 
extern struct xobject *t1_Allocate(int size,struct xobject *template,int extra);
extern void t1_Free(struct xobject *obj);
extern struct xobject *t1_Permanent(struct xobject *obj);
extern struct xobject *xiTemporary(struct xobject *obj);
extern struct xobject *t1_Dup(struct xobject *obj);
extern struct xobject *t1_Copy(struct xobject *obj);
extern struct xobject *t1_Destroy(struct xobject *obj);
extern struct xobject *t1_Unique(struct xobject *obj);
extern void t1_Pragmatics(char *username,int value);
extern void t1_Consume(int n,...);
extern struct xobject *t1_TypeErr(char *name,struct xobject *obj,int expect,struct xobject *ret);
static char *TypeFmt(int type);
static int ObjectPostMortem(struct xobject *obj);
extern struct xobject *t1_ArgErr(char *str,struct xobject *obj,struct xobject *ret);
extern void t1_abort(char *str);
extern char *t1_ErrorMsg(void);
extern void t1_InitImager(void);
extern void t1_TermImager(void);
extern void reportusage(void);
paths.c 
extern struct segment *t1_CopyPath(struct segment *p0);
extern void t1_KillPath(struct segment *p);
extern struct segment *t1_Loc(struct XYspace *S,double x,double y);
extern struct segment *t1_ILoc(struct XYspace *S,int x,int y);
extern struct segment *t1_SubLoc(struct segment *p1,struct segment *p2);
extern struct segment *t1_PathSegment(int type,long x,long y);
extern struct segment *t1_Line(struct segment *P);
extern struct beziersegment *t1_Bezier(struct segment *B,struct segment *C,struct segment *D);
extern struct hintsegment *t1_Hint(struct XYspace *S,double ref,double width,char orientation,char hinttype,char adjusttype,char direction,int label);
extern struct segment *t1_Join(struct segment *p1,struct segment *p2);
extern struct segment *t1_JoinSegment(struct segment *before,int type,long x,long y,struct segment *after);
extern struct segment *t1_ClosePath(struct segment *p0,int lastonly);
extern struct segment *t1_Reverse(struct segment *p);
static struct segment *ReverseSubPath(struct segment *p);
static struct segment *DropSubPath(struct segment *p0);
static struct segment *SplitPath(struct segment *anchor,struct segment *before);
extern struct segment *t1_ReverseSubPaths(struct segment *p);
static int UnClose(struct segment *p0);
extern struct segment *t1_PathXform(struct segment *p0,struct XYspace *S);
extern void t1_PathDelta(struct segment *p,struct fractpoint *pt);
extern struct segment *t1_BoundingBox(short h,short w);
extern void t1_QueryLoc(struct segment *P,struct XYspace *S,double *xP,double *yP);
extern void t1_QueryPath(struct segment *path,int *typeP,struct segment **Bp ,struct segment **Cp ,struct segment **Dp ,double *fP);
extern void t1_QueryBounds(struct segment *p0,struct XYspace *S,double *xminP,double *yminP,double *xmaxP,double *ymaxP);
extern struct segment *t1_BoxPath(struct XYspace *S,int h,int w);
extern struct segment *t1_DropSegment(struct segment *path);
extern struct segment *t1_HeadSegment(struct segment *path);
extern void t1_DumpPath(struct segment *p);
pfb2pfa.c 
extern int main(int argc,char **argv );
pk2bm.c 
extern int main(int argc,char **argv );
extern unsigned char lsbf(unsigned char u);
extern void dots(unsigned char u,int n);
extern int atoo(char *oct);
pkin.c 
static short pkbyte(void);
static long pkquad(void);
static long pktrio(void);
static long pklong(void);
static short pkopen(char *name);
static short getnyb(void);
static short getbit(void);
static unsigned short pkpackednum(void);
static unsigned short rest(void);
static unsigned short handlehuge(unsigned short i,unsigned short k);
/* void unpack(struct *cd); */
/* int readchar(char *name,short c,struct *cd); */
static void error(char *s);
pkout.c 
extern void pk_open(char *pkname);
extern void pk_close(void);
static void pk_nybble(int x);
static void pk1(int x);
static void pk2(int x);
static void pk3(int x);
static void pk4(int x);
extern void pk_preamble(char *comment,float pointsize,int checksum,unsigned int h_res,unsigned int v_res);
extern int optimal_size(int W,int H,int cnt,int *count,int *dyn_f);
static void pk_runlengths(int W,int H,int (*next_pixel)(void));
static void pk_number(int x);
static void pk_bitmap(int width,int cnt,int *runlength);
extern void pk_char(int char_code,int tfm_width,int h_escapement,unsigned int width,unsigned int height,int h_offset,int v_offset,int (*next_pixel)(void));
static void pkstring(char *fmt,...);
extern int INT(float x);
extern char *magnification(int dpi,int BDPI);
extern void pk_postamble(void);
extern void ps2pk_postamble(char *fontname,char *encname,int base_res,int h_res,int v_res,float pointsize,char *args);
pktest.c 
extern int main(int argc,char **argv );
extern int next_pixel(void);
ps2pk.c 
psargs.c 
psearch.c 
regions.c 
extern void t1_KillRegion(struct region *area);
extern struct region *t1_CopyRegion(struct region *area);
static struct edgelist *NewEdge(short xmin,short xmax,short ymin,short ymax,short *xvalues,int isdown);
extern struct region *t1_Interior(struct segment *p,int fillrule);
static int Unwind(struct edgelist *area);
extern void t1_ChangeDirection(int type,struct region *R,long x,long y,long dy);
static void newfilledge(struct region *R,long xmin,long xmax,long ymin,long ymax,int isdown);
extern struct edgelist *t1_SortSwath(struct edgelist *anchor,struct edgelist *edge,struct edgelist *(*swathfcn)(void));
static struct edgelist *splitedge(struct edgelist *list,short y);
static void vertjoin(struct edgelist *top,struct edgelist *bottom);
extern struct edgelist *swathxsort(struct edgelist *before0,struct edgelist *edge);
extern struct edgelist *t1_SwathUnion(struct edgelist *before0,struct edgelist *edge);
extern struct edgelist *swathrightmost(struct edgelist *before,struct edgelist *edge);
static int touches(int h,short *left,short *right);
static int crosses(int h,short *left,short *right);
static void cedgemin(int, pel *, pel);
static void cedgemax(int, pel *, pel);
static void edgemin(int, pel *, pel *);
static void edgemax(int, pel *, pel *);
static void discard(struct edgelist *left,struct edgelist *right);
extern void t1_MoveEdges(struct region *R,long dx,long dy);
extern void t1_UnJumble(struct region *region);
static int OptimizeRegion(struct region *R);
extern void t1_MoreWorkArea(struct region *R,long x1,long y1,long x2,long y2);
extern struct region *t1_BoxClip(struct region *R,short xmin,short ymin,short xmax,short ymax);
extern struct segment *t1_RegionBounds(struct region *R);
extern void t1_DumpArea(struct region *area);
extern void t1_DumpEdges(struct edgelist *edges);
static int edgecheck(struct edgelist *edge,int oldmin,int oldmax);
scanfont.c 
/* struct ps_obj *MakeEncodingArrayP(struct *encodingTable); */
extern int Init_BuiltInEncoding(void);
static int getNextValue(int valueType);
static int getInt(void);
static int getEncoding(struct ps_obj *arrayP);
static int getArray(struct ps_obj *arrayP);
static int getName(char *nameP);
static int getNbytes(int N);
static int getLiteralName(struct ps_obj *nameObjP);
static int BuildSubrs(struct ps_font *FontP);
static int BuildCharStrings(struct ps_font *FontP);
static int BuildFontInfo(struct ps_font *fontP);
static int BuildPrivate(struct ps_font *fontP);
static int GetType1Blues(struct ps_font *fontP);
extern struct ps_obj *GetType1CharString(struct ps_font *fontP,unsigned char code);
static int FindDictValue(struct ps_dict *dictP);
extern int scan_font(struct ps_font *FontP);
sexpr.c 
extern int main(int argc,char **argv );
spaces.c 
extern struct XYspace *t1_CopySpace(struct XYspace *S);
static void ConsiderContext(struct xobject *obj,double (*M)[2]);
extern void FXYConvert(struct fractpoint *pt,struct XYspace *S,double x,double y);
extern void IXYConvert(struct fractpoint *pt,struct XYspace *S,long x,long y);
extern void ForceFloat(struct fractpoint *pt,struct XYspace *S,long x,long y);
extern long FXYboth(double cx,double cy,double x,double y);
extern long FXonly(double cx,double cy,double x,double y);
extern long FYonly(double cx,double cy,double x,double y);
extern long IXYboth(long cx,long cy,long x,long y);
extern long IXonly(long cx,long cy,long x,long y);
extern long IYonly(long cx,long cy,long x,long y);
extern long FPXYboth(long cx,long cy,long x,long y);
extern long FPXonly(long cx,long cy,long x,long y);
extern long FPYonly(long cx,long cy,long x,long y);
static void FillOutFcns(struct XYspace *S);
static void FindFfcn(double cx,double cy,long (**fcnP )(void));
static void FindIfcn(double cx,double cy,long *icxP,long *icyP,long (**fcnP )(void));
extern void t1_UnConvert(struct XYspace *S,struct fractpoint *pt,double *xp,double *yp);
extern struct xobject *t1_Xform(struct xobject *obj,double (*M)[2]);
extern struct xobject *t1_Transform(struct xobject *obj,double cxx,double cyx,double cxy,double cyy);
extern struct xobject *t1_Scale(struct xobject *obj,double sx,double sy);
extern void t1_PseudoSpace(struct XYspace *S,double (*M)[2]);
extern void t1_MMultiply(double (*A)[2],double (*B)[2],double (*C)[2]);
extern void t1_MInvert(double (*M)[2],double (*Mprime)[2]);
extern void t1_InitSpaces(void);
extern void t1_QuerySpace(struct XYspace *S,double *cxxP,double *cyxP,double *cxyP,double *cyyP);
extern void t1_FormatFP(char *str,long fpel);
extern void t1_DumpSpace(struct XYspace *S);
strexpr.c 
extern struct val *make_int(int i);
extern struct val *make_str(char *s);
extern void free_value(struct val *vp);
extern int is_integer(struct val *vp,int *r);
extern int to_integer(struct val *vp);
extern void to_string(struct val *vp);
extern void nexttoken(void);
extern struct val *eval6(void);
extern struct val *eval4(void);
extern struct val *eval3(void);
extern int strexpr(int *result,char *expression);
t1funcs.c 
extern int Type1OpenScalable(char **ev ,struct _Font **ppFont ,int flags,struct _FontEntry *entry,char *fileName,struct _FontScalable *vals,unsigned long format,unsigned long fmask,double efactor,double slant);
static int Type1GetGlyphs(struct _Font *pFont,unsigned long count,unsigned char *chars,int charEncoding,unsigned long *glyphCount,struct _CharInfo **glyphs );
/* int Type1GetMetrics(struct _Font *pFont,unsigned long count,unsigned char *chars,int charEncoding,unsigned long *glyphCount,struct **glyphs ); */
extern void Type1CloseFont(struct _Font *pFont);
static void fill(char *dest,int h,int w,struct region *area,int byte,int bit,int wordsize);
static void fillrun(char *p,short x0,short x1,int bit);
extern int Type1RegisterFontFileFunctions(void);
t1info.c 
static void FillHeader(struct _FontInfo *pInfo,struct _FontScalable *Vals);
/* void adjust_min_max(struct *minc,struct *maxc,struct *tmp); */
static void ComputeBounds(struct _FontInfo *pInfo,struct _CharInfo *pChars,struct _FontScalable *Vals);
static void ComputeProps(struct _FontInfo *pInfo,struct _FontScalable *Vals,char *Filename);
static void ComputeStdProps(struct _FontInfo *pInfo,struct _FontScalable *Vals,char *Filename,char *Fontname);
extern int Type1GetInfoScalable(struct _FontPathElement *fpe,struct _FontInfo *pInfo,struct _FontEntry *entry,struct _FontName *fontName,char *fileName,struct _FontScalable *Vals);
extern void T1FillFontInfo(struct _Font *pFont,struct _FontScalable *Vals,char *Filename,char *Fontname);
extern void T1InitStdProps(void);
t1io.c 
extern struct F_FILE *T1Open(char *fn,char *mode);
extern int T1Getc(struct F_FILE *f);
extern int T1Ungetc(int c,struct F_FILE *f);
extern int T1Read(char *buffP,int size,int n,struct F_FILE *f);
extern int T1Close(struct F_FILE *f);
extern struct F_FILE *T1eexec(struct F_FILE *f);
static int T1Decrypt(unsigned char *p,int len);
static int T1Fill(struct F_FILE *f);
t1snap.c 
extern unsigned __int64 __stdcall Int64ShllMod32(unsigned __int64 Value,unsigned long ShiftCount);
extern __int64 __stdcall Int64ShraMod32(__int64 Value,unsigned long ShiftCount);
extern unsigned __int64 __stdcall Int64ShrlMod32(unsigned __int64 Value,unsigned long ShiftCount);
extern void *GetFiberData(void );
extern void *GetCurrentFiber(void );
extern struct segment *t1_Phantom(struct xobject *obj);
extern struct xobject *t1_Snap(struct segment *p);
t1stub.c 
extern int xiStub(void);
extern void t1_DumpText(struct segment *foo);
t1test.c 
extern int main(int argc,char **argv );
extern void Display(struct _CharInfo *glyph);
extern int T1FillVals(struct _FontScalable *vals);
extern int CheckFSFormat(int format,int fmask,int *bit,int *byte,int *scan,int *glyph,int *image);
extern char *MakeAtom(char *p);
extern int GetClientResolutions(int *resP);
extern char *Xalloc(int size);
extern void Xfree(void);
extern int FontDefaultFormat(void);
extern int FontFileRegisterRenderer(void);
extern int GenericGetBitmaps(void);
extern int GenericGetExtents(void);
extern int FontParseXLFDName(void);
extern int FontComputeInfoAccelerators(void);
texfiles.c 
extern unsigned long num(struct _iobuf *fp,int size);
extern int snum(struct _iobuf *fp,int size);
token.c 
static double P10(long exponent);
static int next_char(int ch);
static int add_char(int ch);
static int skip_space(int ch);
static int skip_comment(int ch);
static int add_sign(int ch);
static int add_1st_digits(int ch);
static int add_digits(int ch);
static int add_1st_decpt(int ch);
static int add_decpt(int ch);
static int add_fraction(int ch);
static int add_e_sign(int ch);
static int add_exponent(int ch);
static int add_radix(int ch);
static int add_r_digits(int ch);
static int RADIX_NUMBER(int ch);
static int INTEGER(int ch);
static int REAL(int ch);
static int HEX_STRING(int ch);
static void save_digraph(int ch);
static int STRING(int ch);
static int AAH_NAME(int ch);
static int NAME(int ch);
static int LITERAL_NAME(int ch);
static int IMMED_NAME(int ch);
static int OOPS_NAME(int ch);
static int RIGHT_ANGLE(int ch);
static int RIGHT_PAREN(int ch);
static int LEFT_BRACE(int ch);
static int RIGHT_BRACE(int ch);
static int LEFT_BRACKET(int ch);
static int RIGHT_BRACKET(int ch);
static int BREAK_SIGNAL(int ch);
static int NO_TOKEN(int ch);
extern void scan_token(struct ps_obj *inputP);
type1.c 
static int ComputeAlignmentZones(void);
static int InitStems(void);
static int FinitStems(void);
static void ComputeStem(int stemno);
static struct segment *Applyhint(struct segment *p,int stemnumber,int half);
static struct segment *Applyrevhint(struct segment *p,int stemnumber,int half);
static struct segment *FindStems(double x,double y,double dx,double dy);
static void ClearStack(void);
static void Push(double Num);
static void ClearCallStack(void);
static void PushCall(struct ps_obj *CurrStrP,int CurrIndex,unsigned short CurrKey);
static void PopCall(struct ps_obj **CurrStrPP ,int *CurrIndexP,unsigned short *CurrKeyP);
static void ClearPSFakeStack(void);
static void PSFakePush(double Num);
static double PSFakePop(void);
static struct segment *CenterStem(double edge1,double edge2);
static unsigned char Decrypt(unsigned char cipher);
static int DoRead(int *CodeP);
static void StartDecrypt(void);
static void Decode(int Code);
static void DoCommand(int Code);
static void Type1Escape(int Code);
static void HStem(double y,double dy);
static void VStem(double x,double dx);
static void RLineTo(double dx,double dy);
static void RRCurveTo(double dx1,double dy1,double dx2,double dy2,double dx3,double dy3);
static void DoClosePath(void);
static void CallSubr(int subrno);
static void Return(void);
static void EndChar(void);
static void RMoveTo(double dx,double dy);
static void DotSection(void);
static void Seac(double asb,double adx,double ady,unsigned char bchar,unsigned char achar);
static void Sbw(double sbx,double sby,double wx,double wy);
static double Div(double num1,double num2);
static void FlxProc(double c1x2,double c1y2,double c3x0,double c3y0,double c3x1,double c3y1,double c3x2,double c3y2,double c4x0,double c4y0,double c4x1,double c4y1,double c4x2,double c4y2,double epY,double epX,int idmin);
static void FlxProc1(void);
static void FlxProc2(void);
static void HintReplace(void);
static void CallOtherSubr(int othersubrno);
static void SetCurrentPoint(double x,double y);
extern struct xobject *Type1Char(char *env,struct XYspace *S,struct ps_obj *charstrP,struct ps_obj *subrsP,struct ps_obj *osubrsP,struct blues_struct *bluesP,int *modeP);
util.c 
extern int vm_init(void);
extern char *vm_alloc(unsigned int bytes);
extern void objFormatInteger(struct ps_obj *objP,int value);
extern void objFormatReal(struct ps_obj *objP,double value);
extern void objFormatBoolean(struct ps_obj *objP,int value);
extern void objFormatEncoding(struct ps_obj *objP,int length,struct ps_obj *valueP);
extern void objFormatArray(struct ps_obj *objP,int length,struct ps_obj *valueP);
extern void objFormatString(struct ps_obj *objP,int length,char *valueP);
extern void objFormatName(struct ps_obj *objP,int length,char *valueP);
extern void objFormatFile(struct ps_obj *objP,struct _iobuf *valueP);
