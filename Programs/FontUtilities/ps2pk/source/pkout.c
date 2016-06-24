/* FILE:    pkout.c
 * PURPOSE: implementation of PK output functions
 * COMMENT: functions are derived from `The GFtoPK processor' but
 *          presented in a modular fashion so they can be used in other
 *          programs. 
 *          (Porting `top-down' WEB code into modular readable C-code is
 *          not a trivial exercise.)
 *
 * AUTHOR:  Piet Tutelaers
 * (see README for license)
 */

/* Some constants */
#define PK_ID	 89
#define PK_SPC1	240		/* All we need, max length always < 255 */
#define PK_NUMSPC 244
#define PK_NOOP	246
#define PK_PRE	247
#define PK_POST	245

#define DPI	72.27
#define MIN(a,b) ( (a<b)? (a): (b))

#include "basics.h"	/* definitions and fatal() */
#include <stdio.h>
#include <stdarg.h>	/* va_start(), va_end() */
#include <stdlib.h>	/* malloc() */
#include "pkout.h"

#undef DEBUG
static int pk_len = 0;
static FILE *pkfile;

void pk_open(char *pkname)
{
   pkfile = fopen(pkname, WB);
   if (pkfile == NULL) fatal("Can not open %s\n", pkname);
}

void pk_close(void)
{
   fclose(pkfile);
}

/* Portable Big Endian output functions */

/* Nybble buffer */
static int pk_output_byte, bitweight;
static void pk1(int);

static void pk_nybble(int x)
{
   if (bitweight == 16) {
      bitweight = 1; pk_output_byte = x*16;
   }
   else {
      bitweight = 16; pk1(pk_output_byte + x);
   }
}

static void pk1(int x)
{
   if (x < 0) x += 256; 
   putc(x & 0x00ff, pkfile); 
   pk_len++;
}

static void pk2(int x)
{
   if (x < 0) x += 65536; 
   pk1((x & 0x00ff00) >> 8);
   pk1(x & 0x0000ff);
}

static void pk3(int32_t x)
{
   pk1((x & 0x00ff0000) >> 16);
   pk1((x & 0x0000ff00) >> 8);
   pk1(x & 0x000000ff);
}

static void pk4(int32_t x)
{
   if (x < 0) { /* next two lines from mackay@cs.washington.edu */
      x += 2<<29; x += 2<<29;
      pk1(((x & 0x7f000000) >> 24) + 128);
   }
   else pk1((x & 0x7f000000) >> 24); 
   pk1((x & 0x00ff0000) >> 16);
   pk1((x & 0x0000ff00) >> 8);
   pk1(x & 0x000000ff);
}

static int MAX_COUNTS; /* length of array to hold runlengths */

void pk_preamble(char *comment, 	/* source of the font */
		 float pointsize,	/* pointsize in points */
		 int32_t checksum,      /* should equal to tfm-value */
		 unsigned int h_res,    /* horizontal resolution (dpi) */
		 unsigned int v_res)	/* vertical resolution (dpi) */
{ 
   int i, len;

   /* compute MAX_COUNTS based upon pointsize, h_res and v_res */
   MAX_COUNTS = pointsize / DPI * h_res * (pointsize / DPI * v_res + 1);
   
   pk1(PK_PRE);
   pk1(PK_ID);
   len = strlen(comment);
   len = len>255? 255: len;
   pk1(len);
   for (i=0; i<len; i++) pk1(*comment++);

   pk4((int32_t)(pointsize * (1<<20) + 0.5));
   pk4(checksum);
   pk4((int32_t)(h_res / DPI * (1<<16))); pk4((int32_t)(v_res / DPI * (1<<16)));
}

/* From `The GFtoPK processor', pp. 231 */

static int optimal_size(int W, int H, int cnt, int count[], int *dyn_f)
{  int comp_size = 0, b_comp_size, deriv[14], i, j, k;

   i = (count[0] == 0? 1: 0); /* skip first empty runlength */
   /* compute comp_size and deriv[1..13] */
   for (j=1; j<14; j++) deriv[j] = 0;
   while (i<cnt) {
      j = count[i++];
      if (j == -1) { comp_size++; continue; }
      if (j < 0) { j = -j; comp_size++; }
      if (j < 209) comp_size += 2;
      else {
      	 k = j - 193;
      	 while (k >= 16) { k=k/16; comp_size += 2; }
      	 comp_size++;
      }
      if (j < 14) deriv[j]--;
      else {
      	 if (j < 209) deriv[(223 - j) / 15]++;
      	 else {
      	    k = 16;
      	    while (k*16 < j+3) k *= 16;
      	    if (j-k <= 192) deriv[(207-j+k)/15] += 2;
      	 }
      }
   }
   /* find minimal value */
   b_comp_size = comp_size; *dyn_f = 0;
   for (i=1; i<14; i++) {
      comp_size += deriv[i];
      if (comp_size <= b_comp_size) {
      	 b_comp_size = comp_size; *dyn_f = i;
      }
   }
   comp_size = (b_comp_size + 1) / 2;
   if (H*W == 0 || (comp_size > (H*W + 7) / 8)) {
      comp_size = (H*W + 7) / 8; *dyn_f = 14;
   }
   return comp_size;
}

/* Next array contains the runlengths */
static int *count = NULL;

/* Global variables (interface between pk_runlengths() and pk_char() */
static int dyn_f, comp_size, cnt;

static void pk_runlengths(int W, int H, int (*next_pixel)())
{  int current_value, pixel, i, j, max_counts, first_count;
   int total_pixels, rc, row, col, runlength;
   int color2, total_pixels2, /* counting equal rows variables */
       i1, i2, newrow;

   first_count = H; max_counts = first_count + H * W;
   if (count == NULL) {
      count = malloc(MAX_COUNTS * sizeof(int));
      if (count == NULL) fatal("Out of memory\n");
   }
   
   if (max_counts > MAX_COUNTS) {
      free(count);
      count = malloc(max_counts * sizeof(int));
      if (count == NULL) fatal("Out of memory\n");
      MAX_COUNTS = max_counts;
   }

   /* 1: compute transitions BLACK<->WHITE */

   cnt = first_count;
   runlength = 0; current_value = BLACK;
   for (row=0; row < H; row++) {
      for (col=0; col<W; col++) {
      	 pixel = (*next_pixel)();
         if (pixel == current_value) runlength++;
         else if (pixel == OTHER(current_value)) {
            count[cnt++] = runlength;
            current_value = OTHER(current_value);
            runlength = 1;
         }
      }
   }
   if (runlength>0)
      count[cnt++] = runlength;

   /* for an empty glyph */
   if (cnt == first_count) count[cnt++] = 0;

#ifdef DEBUG
   current_value = BLACK;
   for (i=first_count; i<cnt; i++)
      if (count[i] < 0) printf("[%d]", -count[i]);
      else {
	 if (current_value == BLACK) printf("%d", count[i]);
	 else printf("(%d)", count[i]);
	 current_value = OTHER(current_value);
      }
   putchar('\n');
#endif

   /* 2: Now remove equal lines and add a repeat count at the first
         transition of a row */

   i = first_count; j = 0;
   total_pixels = 0; row = -1;
   current_value = BLACK;
   if (count[i] == 0) {
      count[j++] = count[i++];
      current_value = WHITE;
   }
   while (i < cnt) {
      if (j >= i) fatal("Program error: report to author!\n");
      count[j++] = count[i];
      total_pixels += count[i++];
      current_value = OTHER(current_value);
      newrow = total_pixels / W; col = total_pixels % W;
      /* if transition starts in column zero or in a previous row 
         then continue */
      if (newrow == row || row == H-1 || col == 0) continue;
      row = newrow;

      /* count equal rows */

         /* 1: goto first transition of next row */
      color2 = current_value;
      total_pixels2 = total_pixels; i2 = i;
      while (i2 < cnt && total_pixels2 / W == row) {
	 total_pixels2 += count[i2++];
	 color2 = OTHER(color2);
      }

         /* 2: do we need to compute a repeat count? */
      if (color2 != current_value 
          || total_pixels2 - total_pixels != W) continue;

	 /* 3: count them */
      rc = 0; i1 = i;
      while (i2 < cnt && count[i1] == count[i2]) {
#ifdef DEBUG
         printf("%d (%d)", count[i1], count[i2]);
#endif
	 total_pixels2 += count[i2++]; i1++; 
      }
      rc = total_pixels2 / W - row;  /* enclosed rows */
      if (MIN(count[i1], count[i2]) + total_pixels2 % W < W)
            rc--;
#ifdef DEBUG
      printf(" row %d: rc = %d\n", row, rc);
#endif

	/* 3: now remove the equal rows and finish last row */      
      if (rc > 0) {
      	 /* insert a repeat count */
         if (j >= i) fatal("Program error: report to author\n");
         count[j++] = -rc;

         /* finish runlengths of current row */
         while (total_pixels + count[i] < row*W+W) {
            if (j >= i) fatal("Program error: increase FIRST_COUNT!\n");
            count[j++] = count[i];
            total_pixels += count[i++];
            current_value = OTHER(current_value);
         }

        /* skip runlengths of equal rows */
         while (total_pixels + count[i] < (row+rc+1)*W) {
            total_pixels += count[i++];
            current_value = OTHER(current_value);
         }
         row += rc;
      }
   }
   cnt = j; /* that is what we have now */

   /* 3: compute optimal packing size */

   comp_size = optimal_size(H, W, cnt, count, &dyn_f);
#ifdef DEBUG
   current_value = BLACK;
   for (i=0; i<cnt; i++)
      if (count[i] < 0) printf("[%d]", -count[i]);
      else {
	 if (current_value == BLACK) printf("%d", count[i]);
	 else printf("(%d)", count[i]);
	 current_value = OTHER(current_value);
      }
   printf("\nOptimal packing with dyn_f = %d total size %d\n", 
   dyn_f, comp_size);
#endif
}

#define MAX_TWOBYTE_NYBBLE (208 - 15*dyn_f)

static void pk_number(int x)
{  int k;

   if (x < 0) { /* repeat count */
      if (x == -1) pk_nybble(0xF);
      else { pk_nybble(0xE);
         pk_number(-x);
      }
   } 
   else if (x <= dyn_f) pk_nybble(x);
   else if (x <= MAX_TWOBYTE_NYBBLE) { /* two nybble values */
      x-= dyn_f+1;
      pk_nybble(dyn_f + 1 + x / 16);
      pk_nybble(x % 16);
   }
   else { /* huge counts */
      x = x - MAX_TWOBYTE_NYBBLE + 15; k = 16;
      while (k <= x) { k *= 16; pk_nybble(0x0); }
      while (k > 1) { k = k / 16; pk_nybble(x / k); x = x % k; }
   } 
}

/* in case runlength encoding is not optimal we send a compressed 
   bitmap in stead of the packed runlengths
   See GFtoPK processor, pp. 235 */
static void pk_bitmap(int width, int cnt, int runlength[])
{  int count,		/* number of bits in the current state to send */
       buff,		/* byte buffer */
       h_bit,		/* horizontal bit count for each runlength */
       p_bit,   	/* what bit are we about to send out? */
       r_on, s_on,	/* state saving variables */
       r_count, s_count,/* dito */
       r_i, s_i,	/* dito */
       rc,		/* repeat count */
       state,		/* state variable (what state?) */
       on,		/* starting with black? */
       i;		/* points to next runlength */
   static int power[9] = {1, 2, 4, 8, 16, 32, 64, 128, 256};

   buff = 0; p_bit = 8; h_bit = width; 
   state = 0; rc = 0;
   r_on = s_on = r_count = s_count = r_i = s_i = 0; /* avoid warnings */
   on = 1; count = runlength[0]; i = 1;
   while (i < cnt || state || count>0) {
      if (state) { 
         count = r_count; i = r_i; on = r_on; rc--; 
      }
      else { 
         r_count = count; r_i = i; r_on = on; 
      }

      /* send one row of width bits */
      do {
      	 if (count == 0) {
      	    if (runlength[i] < 0) {
	       if (!state) rc = - runlength[i];
	       i++;
      	    }
      	    count = runlength[i]; i++; on = !on;
      	 }
      	 if (count >= p_bit && p_bit < h_bit) {
      	    /* we end a byte, we don't end a runlength */
      	    if (on) buff += power[p_bit] - 1;
      	    pk1(buff); 
      	    buff = 0; h_bit -= p_bit; 
      	    count -= p_bit; p_bit = 8;
      	 }
      	 else {
      	    if (count < p_bit && count < h_bit) {
	       /* we neither end the row nor the byte */
	       if (on) buff += power[p_bit] - power[p_bit-count];
	       p_bit -= count; h_bit -= count; count = 0;
      	    }
      	    else { /* we end a row and maybe a byte */
	       if (on) buff += power[p_bit] - power[p_bit-h_bit];
	       count -= h_bit; p_bit -= h_bit; h_bit = width;
	       if (p_bit == 0) {
	          pk1(buff); buff = 0; p_bit = 8;
	       }
      	    }
      	 }
      } while (h_bit != width);

      if (state && rc == 0) { 
         count = s_count; i = s_i; on = s_on; state = 0;
      }
      else if (!state && rc >0) {
      	 s_count = count; s_i = i; s_on = on; state = 1;
      }
   }
   if (p_bit != 8) pk1(buff);
}

/* For packing a character */
void pk_char(int char_code, 	/* character code 0..255 */
        int32_t tfm_width,      /* TFM width of character */
        int h_escapement,       /* horizontal escapement in pixels */
   	unsigned int width, 	/* width of bounding box */
   	unsigned int height, 	/* height of bounding box */
   	int h_offset, 		/* horizontal offset to reference point */
   	int v_offset, 		/* vertical offset to reference point */
        int (*next_pixel)())	/* user's next pixel generator */
{  int i;
   unsigned short flag_byte;

   pk_runlengths(width, height, next_pixel);

   /* write a character preamble */

   flag_byte = dyn_f * 16;
   if (count[0] > 0) flag_byte += 8; /* starting in BLACK */
   if (tfm_width > 0XFFFFFF || width > 65535 || height > 65535
       || h_offset > 32767 || v_offset > 32767
       || h_offset < -32768 || v_offset < -32768
       || comp_size > 196594) 
   { /* write long format */
      fatal("Can't handle long format yet!\n");
   }
   else if (h_escapement > 255 || width > 255 || height > 255
            || h_offset > 127 || v_offset > 127
            || h_offset < -128 || v_offset < -128 
            || comp_size > 1015)
   { /* write two-byte short character preamble */
      comp_size += 13; flag_byte += comp_size / 65535 + 4; 
      pk1(flag_byte);
      pk2(comp_size % 65535);
      pk1(char_code);
      pk3(tfm_width);
      pk2(h_escapement); /* pixels ! */
      pk2(width);
      pk2(height);
      pk2(h_offset);
      pk2(v_offset);
   }
   else { /* write one-byte short character preamble */
      comp_size += 8;  flag_byte += comp_size / 256; 
      pk1(flag_byte);
      pk1(comp_size % 256);
      pk1(char_code);
      pk3(tfm_width);
      pk1(h_escapement); /* pixels ! */
      pk1(width);
      pk1(height);
      pk1(h_offset);
      pk1(v_offset);
   }

   /* and now the character itself */
   if (dyn_f != 14) { /* send compressed format */ 
      if (count[0] == 0) i = 1; else i = 0;
      bitweight = 16;
      for (i=i; i<cnt; i++) pk_number(count[i]);
      if (bitweight != 16) pk1(pk_output_byte);
   }
   else /* send bitmap */
      if (height > 0) pk_bitmap(width, cnt, count);

}

/*
 * Output small string special (<255 chars)
 */
static void pkstring(const char *fmt, ...) {
   char buf[256]; int i, len;
   va_list args;

   va_start(args, fmt);
#ifdef CHARSPRINTF
   (void) vsprintf(buf, fmt, args);
   len = strlen(buf);
#else
   len = vsprintf(buf, fmt, args);
#endif
   if (len > 255 || len < 0) fatal("PK string exceeds 255 characters\n");
   va_end(args);

   pk1(PK_SPC1);
   pk1(len); i = 0;
   for (i = 0; i < len; i++) pk1(buf[i]);
}

/*
 * Compute METAFONT magnification string for <dpi>
 */

static int PSPKINT(float x) {
   return (int) x;
}

static char mag_str[64];

static char *magnification (int dpi, int BDPI) {
   double size, magstep;

   if (dpi == BDPI) {
      sprintf(mag_str, "magstep(0)");
      return mag_str;
   }
   size = BDPI; magstep = 0; 
   while (dpi < size) {
      size = size / 1.095445115;
      magstep -= 0.5;
      if (dpi == PSPKINT(size + 0.5)) {
         sprintf(mag_str, "magstep(%.1f)", magstep);
         return mag_str;
      }
      if (dpi > size) {
         sprintf(mag_str, "%d+%d/%d", PSPKINT(dpi/BDPI), dpi%BDPI, BDPI);
         return mag_str;
      }
   }
   while (dpi > size) {
      size = size * 1.095445115;
      magstep += 0.5;
      if (dpi == PSPKINT(size + 0.5)) {
         sprintf(mag_str, "magstep(%.1f)", magstep);
         return mag_str;
      }
      if (dpi < size) {
         sprintf(mag_str, "%d+%d/%d", PSPKINT(dpi/BDPI), dpi%BDPI, BDPI);
         return mag_str;
      }
   }
   fatal("PK could not determine magnification\n");
}

/*
 * Barebone postample
 */ 
void pk_postamble(void)
{ 	
   pk1(PK_POST);
   while (pk_len % 4 != 0) pk1(PK_NOOP);
}

/*
 * Write special PK strings in the postamble for identification
 * purposes as proposed by mackay@cs.washington.edu.
 */ 
void ps2pk_postamble(char *fontname, /* The real FontName from the afm */
		  char *encname,     /* The actual name, not the filename */
		  int base_res,      /* basic resolution */
		  int h_res,	     /* Match against base_res for mag */
		  int v_res,	     /* Match against h_res for aspect_ratio */
		  float pointsize,   /* Used for fontfacebyte calculation */
		  char *args)        /* Essential ps2pk args */
{ 	
   int i;

   pkstring("ps2pk options: %s", args);
   pkstring("fontid=%s", fontname);
   if (encname) pkstring("codingscheme=%s", encname);
   pkstring("fontfacebyte"); 
   pk1(PK_NUMSPC);
   i = (pointsize < 127.0) ?
     ((254 - (int)((2 * pointsize)+0.5)) * (1 << 16)) : 0;
   pk4(i);
   pkstring("pixels_per_inch=%d", base_res);
   pkstring("mag=%s", magnification(h_res, base_res));
   if (v_res != h_res)
      pkstring("aspect ratio=%d / %d", 
	  (int)((1.0 * v_res / h_res * base_res) + 0.5), base_res);

   pk1(PK_POST);
   while (pk_len % 4 != 0) pk1(PK_NOOP);
}
