# include "autosp.h"

# define MAX_STAFFS 9

# define SMALL_NOTE 512
# define SP(note) (SMALL_NOTE/note)  
/* note = { 256 | 128 | 64 | 32 | 16 | 8 | 4 | 2 | 1 }  */
# define MAX_SPACING 2*SP(1)
extern int staffs[MAX_STAFFS]; /* number of staffs for ith instrument*/
extern bool active[MAX_STAFFS];        /* is staff i active?                 */
extern bool bar_rest[MAX_STAFFS];
extern int spacing;                    /* spacing for current notes          */
extern int restbars;
extern char global_skip_str[7][16];   
extern int ninstr;                 /* number of instruments              */
extern int nstaffs;                /* number of staffs                   */
extern int nastaffs;               /* number of active staffs;           */
extern int old_spacing;
extern int oldspacing_staff;
extern bool Changeclefs;               /* output \Changeclefs after \def\atnextbar */
extern char TransformNotes2[SHORT_LEN];      /* 2nd argument of \TransformNotes    */
extern bool TransformNotesDefined;
extern char instrument_size[MAX_STAFFS][SHORT_LEN];
     /* instrument size as set by setsize or by default ("\\@one") */
extern float instr_numsize[MAX_STAFFS];
     /* nummerical intrument size */
extern int staff_instr[MAX_STAFFS];  /* instrument for each staff */

extern char line[LINE_LEN];            /* line of input                      */

extern void process_line ();

extern void process_command (char **ln);
