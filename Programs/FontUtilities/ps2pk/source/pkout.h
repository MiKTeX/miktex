/* FILE:    pkout.h
 * PURPOSE: interface to te PK output functions (pkfile.c)
 * COMMENT: See the pktest.c example program how to use the PK functions.
 * (see README for license)
 */

/* A function for opening a pk file */
void pk_open(char *name);

/* A function for closing a pk file */
void pk_close(void);

/* A function for writing the preamble */
void pk_preamble(char *comment, /* comment string: source of font */
        float pointsize,        /* design size in points */
        int32_t checksum,       /* checksum */
        unsigned int h_res,     /* horizontal resolution (dpi) */
        unsigned int v_res);    /* vertical resolution (dpi) */

/* For packing a character */
void pk_char(int char_code, 	/* character code 0..255 */
        int32_t tfm_width,      /* TFM width of character */
        int h_escapement,       /* horizontal escapement in pixels */
   	unsigned int width, 	/* width of bounding box */
   	unsigned int height, 	/* height of bounding box */
   	int h_offset, 		/* horizontal offset to reference point */
   	int v_offset, 		/* vertical offset to reference point */
	int (*next_pixel)());	/* user's pixel generator */

/* Barebone postample */ 
void pk_postamble(void);

/* PS2PK postamble */
void ps2pk_postamble(char *fontname, /* The real FontName from the afm */
		  char *encname,     /* The actual name, not the filename */
		  int base_res,      /* basic resolution */
		  int h_res,	     /* Match against base_res for mag */
		  int v_res,	     /* Match against h_res for aspect_ratio */
		  float pointsize,   /* Used for fontfacebyte calculation */
		  char *args);       /* Essential ps2pk args */

/* Here are some definitions to play with pixels */
#define BLACK	1
#define WHITE   0
#define OTHER(pixel) (pixel==BLACK? WHITE: BLACK)
