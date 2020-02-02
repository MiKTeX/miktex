# include "process_score.h"

# define APPOGG_SPACING SP(64)+SP(256)  /* not a legitimate spacing */
# define notespp "\\vnotes2.95\\elemskip"
# define Notespp "\\vnotes3.95\\elemskip"
# define NOtespp "\\vnotes4.95\\elemskip"
# define NOTespp "\\vnotes6.95\\elemskip"
# define NOTEsp  "\\vnotes9.52\\elemskip"
# define APPOGG_NOTES "\\vnotes1.45\\elemskip"

char terminator[MAX_STAFFS];    /* one of '&' "|', '$'                */

char *notes[MAX_STAFFS];        /* note segment for ith staff         */
char *current[MAX_STAFFS];

int spacings[MAX_STAFFS];       /* spacing for ith staff              */

int vspacing[MAX_STAFFS];       /* virtual-note (skip) spacing        */
bool vspacing_active[MAX_STAFFS]; /* virtual-note spacing active?       */
                      /* used to preclude unnecessary pre-accidental skips    */

bool nonvirtual_notes;          /* used to preclude output of *only* virtual notes */

int cspacing[MAX_STAFFS];       /* nominal collective-note spacing    */
char collective[MAX_STAFFS][SHORT_LEN];
                                    /* prefixes for collective note sequences */
bool first_collective[MAX_STAFFS];

char deferred_bar[SHORT_LEN];   /* deferred \bar (or \endpiece etc.)  */

int beaming[MAX_STAFFS];        /* spacing for beamed notes           */
int new_beaming;
int semiauto_beam_notes[MAX_STAFFS]; /* semi-automatic beam notes     */

int spacing_staff;              /* staff that determines current spacing  */

/* save-restore state for a staff; used in process_xtuplet */
int beamingi;  
char *currenti;
int cspacingi;
int vspacingi;
char collectivei[SHORT_LEN];
bool first_collectivei;

int xtuplet[MAX_STAFFS];        /* x for xtuplet in staff i          */

bool appoggiatura;

char outstrings[MAX_STAFFS][LINE_LEN];  
                                     /* accumulate commands to be output    */
char *n_outstrings[MAX_STAFFS];

int global_skip;  
   /* = 1, 2, 3, or 4 for (non-standard) commands \QQsk \HQsk \TQsk \Qsk    */
   /* = 5 for five commas and double-flat accidental spacing                */
   /* = 6 for six commas                                                    */


char s[SHORT_LEN];  /* string for ps()  */
char *s_n;
char *ps(int spacing);
void note_segment (char *s);
void status (int i);
void status_spacing (void);
void status_all (void);
void status_collective (int i);
void status_beam (int i);


int spacing_note (int i);
void output_notes (int i);
void initialize_notes ();
