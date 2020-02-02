# include "autosp.h"

# define MAX_STAFFS 9

# define SMALL_NOTE 512
# define SP(note) (SMALL_NOTE/note)  
/* note = { 256 | 128 | 64 | 32 | 16 | 8 | 4 | 2 | 1 }  */
# define MAX_SPACING 2*SP(1)
int staffs[MAX_STAFFS]; /* number of staffs for ith instrument*/
bool active[MAX_STAFFS];        /* is staff i active?                 */
bool bar_rest[MAX_STAFFS];
int spacing;                    /* spacing for current notes          */
int restbars;
char global_skip_str[7][16];   
int ninstr;                 /* number of instruments              */
int nstaffs;                /* number of staffs                   */
int nastaffs;               /* number of active staffs;           */
int old_spacing;
int oldspacing_staff;
bool Changeclefs;               /* output \Changeclefs after \def\atnextbar */
char TransformNotes2[SHORT_LEN];      /* 2nd argument of \TransformNotes    */
bool TransformNotesDefined;
char instrument_size[MAX_STAFFS][SHORT_LEN];
     /* instrument size as set by setsize or by default ("\\@one") */
float instr_numsize[MAX_STAFFS];
     /* nummerical intrument size */
int staff_instr[MAX_STAFFS];  /* instrument for each staff */

char line[LINE_LEN];            /* line of input                      */

void process_line ();

void process_command (char **ln);
