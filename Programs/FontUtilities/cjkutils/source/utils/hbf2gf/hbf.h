/*
 * Copyright 1993,1994,1995,2005 by Ross Paterson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Two interfaces to HBF files -- take your pick.
 *
 * Ross Paterson <ross@soi.city.ac.uk>
 *
 * Ross no longer maintains this code.  Please send bug reports to
 * Werner Lemberg <wl@gnu.org>.
 *
 */
#ifndef _HBF_
#define _HBF_

#ifndef __STDC__
#	ifndef const
#		define const
#	endif
#endif

/*
 *	#1: a lightweight C interface.
 */

typedef	unsigned int	HBF_CHAR;

typedef struct {
	unsigned short	hbf_width;
	unsigned short	hbf_height;
	short		hbf_xDisplacement;
	short		hbf_yDisplacement;
} HBF_BBOX;

typedef struct {
	/* fields corresponding to the definition */
	HBF_BBOX	hbf_bitmap_bbox;	/* HBF_BITMAP_BOUNDING_BOX */
	HBF_BBOX	hbf_font_bbox;		/* FONTBOUNDINGBOX */
} HBF;

extern	HBF *hbfOpen(
#ifdef __STDC__
			const	char	*filename
#endif
		);

extern	void	hbfClose(
#ifdef __STDC__
			HBF		*hbf
#endif
		);

extern	const	char	*hbfProperty(
#ifdef __STDC__
			HBF		*hbf,
			const	char	*propName
#endif
		);

extern	const	unsigned char	*hbfGetBitmap(
#ifdef __STDC__
			HBF		*hbf,
			HBF_CHAR	code
#endif
		);

extern	void	hbfForEach(
#ifdef __STDC__
			HBF	*hbf,
			void	(*func)(HBF *sameHbf, HBF_CHAR code)
#endif
		);

extern	const	char	*hbfFileName(
#ifdef __STDC__
			HBF	*hbf
#endif
		);

extern	long	hbfChars(
#ifdef __STDC__
			HBF	*hbf
#endif
		);

extern	HBF_BBOX *hbfBitmapBBox(
#ifdef __STDC__
			HBF	*hbf
#endif
		);
/* but defined here as a macro */
#define	hbfBitmapBBox(hbf)	(&((hbf)->hbf_bitmap_bbox))

extern	HBF_BBOX *hbfFontBBox(
#ifdef __STDC__
			HBF	*hbf
#endif
		);
/* but defined here as a macro */
#define	hbfFontBBox(hbf)	(&((hbf)->hbf_font_bbox))

#define	HBF_RowSize(hbf)\
	((hbfBitmapBBox(hbf)->hbf_width + 7)/8)

#define	HBF_BitmapSize(hbf)\
	(HBF_RowSize(hbf) * hbfBitmapBBox(hbf)->hbf_height)

#define	HBF_GetBit(hbf,bitmap,x,y)\
	(((bitmap)[(y)*HBF_RowSize(hbf) + (x)/8]>>(7 - (x)%8))&01)

extern	int	hbfDebug;	/* set non-zero for error reporting */

extern const void *hbfGetCodeRange(
#ifdef __STDC__
			HBF			*hbfFile,
			const void		*code_pointer,
			HBF_CHAR		*startp,
			HBF_CHAR		*finishp
#endif
		);

extern const void *hbfGetByte2Range(
#ifdef __STDC__
			HBF			*hbfFile,
			const void		*b2r_pointer,
			unsigned char		*startp,
			unsigned char		*finishp
#endif
		);

/*
 *	#2: taken from Appendix 2 of the HBF draft.
 */

typedef	unsigned int	HBF_HzCode;
typedef unsigned char	HBF_Byte ;
typedef HBF_Byte *	HBF_BytePtr ;
typedef HBF *	        HBF_Handle ;
typedef HBF_Handle *    HBF_HandlePtr ;
typedef char *	        String ;

extern	int	HBF_OpenFont(
#ifdef __STDC__
		const	char *        filename,
		HBF_HandlePtr ptrHandleStorage
#endif
);

extern	int	HBF_CloseFont(
#ifdef __STDC__
		HBF_Handle handle
#endif
);

extern	const char * HBF_GetProperty(
#ifdef __STDC__
		HBF_Handle	handle,
		const	char *	propertyName
#endif
);

extern	int	HBF_GetFontBoundingBox(
#ifdef __STDC__
		HBF_Handle   handle,
		unsigned int *width,
		unsigned int *height,
		int *xDisplacement,
		int *yDisplacement
#endif
);

extern	int	HBF_GetBitmapBoundingBox(
#ifdef __STDC__
		HBF_Handle   handle,
		unsigned int *width,
		unsigned int *height,
		int *xDisplacement,
		int *yDisplacement
#endif
);

extern	int	HBF_GetBitmap(
#ifdef __STDC__
		HBF_Handle  handle,
		HBF_HzCode  hanziCode,
		HBF_BytePtr ptrBitmapBuffer
#endif
);

#endif /* ! _HBF_ */
