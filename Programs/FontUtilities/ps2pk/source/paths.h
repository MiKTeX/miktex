/* $XConsortium: paths.h,v 1.2 91/10/10 11:18:50 rws Exp $ */
/* Copyright International Business Machines, Corp. 1991
 * All Rights Reserved
 * Copyright Lexmark International, Inc. 1991
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM or Lexmark not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM AND LEXMARK PROVIDE THIS SOFTWARE "AS IS", WITHOUT ANY WARRANTIES OF
 * ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  THE ENTIRE RISK AS TO THE
 * QUALITY AND PERFORMANCE OF THE SOFTWARE, INCLUDING ANY DUTY TO SUPPORT
 * OR MAINTAIN, BELONGS TO THE LICENSEE.  SHOULD ANY PORTION OF THE
 * SOFTWARE PROVE DEFECTIVE, THE LICENSEE (NOT IBM OR LEXMARK) ASSUMES THE
 * ENTIRE COST OF ALL SERVICING, REPAIR AND CORRECTION.  IN NO EVENT SHALL
 * IBM OR LEXMARK BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/*SHARED*/

#include "types.h"

#define   Loc(S,x,y)                   t1_Loc(S,(DOUBLE)x,(DOUBLE)y)
#define   ILoc(S,x,y)                  t1_ILoc(S,x,y)
#define   Line(P)                      t1_Line(P)
#define   Join(p1,p2)                  t1_Join(p1,p2)
#define   ClosePath(p)                 t1_ClosePath(p,0)
#define   CloseLastSubPath(p)          t1_ClosePath(p,1)
#define   Conic(B,C,s)                 t1_Conic(B,C,(DOUBLE)s)
#define   RoundConic(M,C,r)            t1_RoundConic(M,C,(DOUBLE)r)
#define   ArcP3(S,P2,P3)               t1_ArcP3(S,P2,P3)
#define   ArcCA(S,C,d)                 t1_ArcCA(S,C,(DOUBLE)d)
#define   Bezier(B,C,D)                t1_Bezier(B,C,D)
#define   Hint(S,r,w,o,h,a,d,l)        t1_Hint(S,r,w,o,h,a,d,l)
#define   Reverse(p)                   t1_Reverse(p)
#define   ReverseSubPaths(p)           t1_ReverseSubPaths(p)
#define   AddLoc(p1,p2)                t1_Join(p1,p2)
#if 0
#define   SubLoc(p1,p2)                t1_SubLoc(p1,p2)
#endif
#define   DropSegment(p)               t1_DropSegment(p)
#define   HeadSegment(p)               t1_HeadSegment(p)
#define   QueryLoc(P,S,x,y)            t1_QueryLoc(P,S,x,y)
#define   QueryPath(p,t,B,C,D,r)       t1_QueryPath(p,t,B,C,D,r)
#define   QueryBounds(p,S,x1,y1,x2,y2)  t1_QueryBounds(p,S,x1,y1,x2,y2)
 
 
struct segment *t1_Loc(struct XYspace *, DOUBLE, DOUBLE);
                              /* create a location object (or "move" segment) */
struct segment *t1_ILoc(struct XYspace *, int, int);
                              /* integer argument version of same             */
struct segment *t1_Line(struct segment *);    /* straight line path segment                   */
struct segment *t1_Join(struct segment *, struct segment *);
                             /* join two paths or regions together           */
struct segment *t1_ClosePath(struct segment *, int);
                                 /* close a path or path set                  */
struct conicsegment *t1_Conic();  /* conic curve path segment                 */
struct conicsegment *t1_RoundConic();  /* ditto, specified another way        */
struct conicsegment *t1_ArcP3(); /* circular path segment with three points   */
struct conicsegment *t1_ArcCA(); /* ditto, with center point and angle        */
struct beziersegment *t1_Bezier(struct segment *, struct segment *,
       struct segment *);           /* Bezier third order curve path segment  */
struct hintsegment *t1_Hint(struct XYspace *, float, float, char,
        char, char, char, int);  /* produce a font 'hint' path segment        */
struct segment *t1_Reverse(struct segment *);
                              /* reverse the complete order of paths          */
struct segment *t1_ReverseSubPaths(struct segment *);
                                 /* reverse only sub-paths; moves unchanged   */
struct segment *t1_DropSegment(struct segment *);
                                   /* Drop the first segment in a path        */
struct segment *t1_HeadSegment(struct segment *);
                                   /* return the first segment in a path      */
void t1_QueryLoc(struct segment *, struct XYspace *, DOUBLE *, DOUBLE *);
                              /* Query location; return its (x,y)             */
void t1_QueryPath(struct segment *, int *typeP, struct segment **,
        struct segment **, struct segment **, DOUBLE *);
                              /* Query segment at head of a path              */
void t1_QueryBounds(struct segment *, struct XYspace *, DOUBLE *, DOUBLE *,
       DOUBLE *, DOUBLE *);   /* Query the bounding box of a path             */
 
/*END SHARED*/
/*SHARED*/
 
#define   CopyPath(p)             t1_CopyPath(p)
#define   KillPath(p)             t1_KillPath(p)
#define   PathTransform(p,m)      t1_PathXform(p,m)
#define   PathDelta(p,pt)         t1_PathDelta(p,pt)
#define   BoundingBox(h,w)        t1_BoundingBox(h,w)
#define   PathSegment(t,x,y)      t1_PathSegment(t,(fractpel)x,(fractpel)y)
#define   JoinSegment(b,t,x,y,a)  t1_JoinSegment(b,t,(fractpel)x,(fractpel)y,a)
#define   Hypoteneuse(dx,dy)      t1_Hypoteneuse(dx,dy)
#define   BoxPath(S,h,w)          t1_BoxPath(S,h,w)
 
struct segment *t1_CopyPath(struct segment *); /* duplicate a path            */
void t1_KillPath(struct segment *);           /* destroy a path               */
struct segment *t1_PathXform(struct segment *, struct XYspace *);
                                 /* transform a path arbitrarily              */
void t1_PathDelta(struct segment *, struct fractpoint *);
                              /* calculate the ending point of a path         */
struct segment *t1_BoundingBox(pel, pel); /* produce a Bounding Box Path      */
struct segment *t1_PathSegment(int, fractpel, fractpel);
                                  /* produce a MOVE or LINE segment           */
struct segment *t1_JoinSegment(struct segment *, int, fractpel, fractpel,
       struct segment *);         /* join a MOVE or LINE segment to a path    */
DOUBLE t1_Hypoteneuse();      /* returns the length of a line                 */
struct segment *t1_BoxPath(struct XYspace *, int, int);
                                /* returns a rectangular path                 */
 
/*END SHARED*/
/*SHARED*/
 
#define    ConsumePath(p)    MAKECONSUME(p,KillPath(p))
#define    UniquePath(p)     MAKEUNIQUE(p,CopyPath(p))
 
/*END SHARED*/
/*SHARED*/
 
struct segment {
       XOBJ_COMMON     /* xobject common data define 3-26-91 PNM             */
       unsigned char size;   /* size of the structure                        */
       unsigned char context;  /* index to device context                    */
       struct segment *link; /* pointer to next structure in linked list     */
       struct segment *last; /* pointer to last structure in list            */
       struct fractpoint dest; /* relative ending location of path segment   */
} ;
 
#define   ISCLOSED(flag)   ((flag)&0x80)  /* subpath is closed               */
#define   LASTCLOSED(flag) ((flag)&0x40)  /* last segment in closed subpath  */
 
/*
NOTE: The ISCLOSED flag is set on the MOVETYPE segment before the
subpath proper; the LASTCLOSED flag is set on the last segment (LINETYPE)
in the subpath
 
We define the ISPATHANCHOR predicate to test that a path handle
passed by the user is valid:
*/
 
#define   ISPATHANCHOR(p)  (ISPATHTYPE(p->type)&&p->last!=NULL)
 
/*
For performance reasons, a user's "location" object is identical to
a path whose only segment is a move segment.  We define a predicate
to test for this case.  See also :hdref refid=location..
*/
 
#define   ISLOCATION(p)    ((p)->type == MOVETYPE && (p)->link == NULL)
 
/*END SHARED*/
/*SHARED*/
 
struct conicsegment {
       XOBJ_COMMON          /* xobject common data define 3-26-91 PNM        */
                            /* type = CONICTYPE			             */
       unsigned char size;   /* as with any 'segment' type                   */
       unsigned char context;  /* as with any 'segment' type                 */
       struct segment *link; /* as with any 'segment' type                   */
       struct segment *last; /* as with any 'segment' type                   */
       struct fractpoint dest;  /* Ending point (C point)                    */
       struct fractpoint M;  /* "midpoint" of conic explained above          */
       float roundness;      /* explained above                              */
} ;
/*END SHARED*/
/*SHARED*/
 
struct beziersegment {
       XOBJ_COMMON           /* xobject common data define 3-26-91 PNM       */
     			     /* type = BEZIERTYPE		             */
       unsigned char size;   /* as with any 'segment' type                   */
       unsigned char context;  /* as with any 'segment' type                 */
       struct segment *link; /* as with any 'segment' type                   */
       struct segment *last; /* as with any 'segment' type                   */
       struct fractpoint dest;  /* ending point (D)                          */
       struct fractpoint B;  /* control point B                              */
       struct fractpoint C;  /* control point C                              */
} ;
 
/*END SHARED*/
/*SHARED*/
 
struct hintsegment {
       XOBJ_COMMON            /* xobject common data define 3-26-91 PNM      */
                              /* type = HINTTYPE			     */
       unsigned char size;   /* size of the structure                        */
       unsigned char context;  /* device context                             */
       struct segment *link; /* pointer to next structure in linked list     */
       struct segment *last; /* pointer to last structure in list            */
       struct fractpoint dest; /* ALWAYS 0,0                                 */
       struct fractpoint ref;
       struct fractpoint width;
       char orientation;
       char hinttype;
       char adjusttype;
       char direction;
       int label;
} ;
 
/*END SHARED*/
/*SHARED*/
 
/*
CONCAT links the 'p2' path chain on the end of the 'p1' chain.  (This macro
is also used by the STROKES module.)
*/
#define  CONCAT(p1, p2)  { \
       p1->last->link = p2;     /* link p2 on end of p1                      */ \
       p1->last = p2->last;    /* last of new is last of p2                  */ \
       p2->last = NULL; }    /* only first segment has non-NULL "last"       */
 
/*END SHARED*/
