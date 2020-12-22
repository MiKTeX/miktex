/*
 *   This routine handles the PostScript prologs that might
 *   be included through:
 *
 *      - Default
 *      - Use of PostScript fonts
 *      - Specific inclusion through specials, etc.
 *      - Use of graphic specials that require them.
 *
 *   Things are real simple.  We build a linked list of headers to
 *   include.  Then, when the time comes, we simply copy those
 *   headers down.
 */
#include "dvips.h" /* The copyright notice in that file is included too! */
struct header_list *header_head;
/*
 *   The external declarations:
 */
#include "protos.h"

static int add_name_general(const char *s, struct header_list **what,
                            char *pre, char *post);

int
add_name(const char *s, struct header_list **what)
{
  return (int) add_name_general (s, what, NULL, NULL);
}

/*
 *   This more general routine adds a name to a list of unique
 *   names.
 */
static int
add_name_general(const char *s, struct header_list **what, char *pre, char *post)
{
   struct header_list *p, *q;

   for (p = *what; p != NULL; p = p->next)
      if (strcmp(p->name, s)==0)
         return 0;
   q = (struct header_list *)mymalloc((integer)(sizeof(struct header_list)));
   q->name = mymalloc(strlen(s) + 1);
   q->Hname = infont;
   q->next = NULL;
   q->precode = pre;
   q->postcode = post;
   strcpy(q->name, s);
   if (*what == NULL)
      *what = q;
   else {
      for (p = *what; p->next != NULL; p = p->next);
      p->next = q;
   }
   return 1;
}
/*
 *   This function checks the virtual memory usage of a header file.
 *   If we can find a VMusage comment, we use that; otherwise, we use
 *   length of the file.
 */
void
checkhmem(const char *s, char *pre, char *post)
{
   FILE *f;

   f = search(headerpath, s, READBIN);
   if (pre || post) {
      if (f==NULL)
	 f = search(figpath, s, READBIN);
   }
   if (f==0) {
      char *msg = concat ("! Couldn't find header file: ", s);

      if (secure == 2) {
        msg = concat (msg,
                   "\nAbsolute and ../relative paths are denied in -R2 mode.");
      }
      error(msg);
   } else {
      int len, i, j;
      long mem = -1;
      char buf[1024];

      len = fread(buf, sizeof(char), 1024, f);
      for (i=0; i<len-20; i++)
         if (buf[i]=='%' && strncmp(buf+i, "%%VMusage:", 10)==0) {
            if (sscanf(buf+i+10, "%d %ld", &j, &mem) != 2)
               mem = -1;
            break;
         }
      if (mem == -1) {
         mem = 0;
         while (len > 0) {
            mem += len;
            len = fread(buf, sizeof(char), 1024, f);
         }
      }
      if (mem < 0)
         mem = DNFONTCOST;
      (*close_file) (f);
#ifdef DEBUG
      if (dd(D_HEADER))
         fprintf_str(stderr, "Adding header file \"%s\" %ld\n",
                                s, mem);
#endif
      fontmem -= mem;
      if (fontmem > 0) /* so we don't count it twice. */
         swmem -= mem;
   }
}
/*
 *   This routine is responsible for adding a header file.  We also
 *   calculate the VM usage.  If we can find a VMusage comment, we
 *   use that; otherwise, we use the length of the file.
 */
int
add_header(const char *s)
{
  return (int) add_header_general (s, NULL, NULL);
}

int
add_header_general(const char *s, char *pre, char *post)
{
   int r;

   r = add_name_general(s, &header_head, pre, post);
   if (r) {
      if (headersready == 1) {
         struct header_list *p = header_head;

         while (p) {
            checkhmem(p->name, p->precode, p->postcode);
            p = p->next;
         }
         headersready = 2;
      } else if (headersready == 2) {
         checkhmem(s, pre, post);
      }
   }
   return r;
}
/*
 *   This routine runs down a list, returning each in order.
 */
static struct header_list *CUR_head = NULL;
char *
get_name(struct header_list **what)
{
   if (what && *what) {
      char *p = (*what)->name;
      infont = (*what)->Hname;
      CUR_head = *what;
      *what =  (*what)->next;
      return p;
   } else
      return 0;
}
/*
 *   This routine actually sends the headers.
 */
void
send_headers(void) {
   struct header_list *p = header_head;
   char *q;

   while (0 != (q=get_name(&p))) {
#ifdef DEBUG
      if (dd(D_HEADER))
         fprintf_str(stderr, "Sending header file \"%s\"\n", q);
#endif
#ifdef HPS
     if (HPS_FLAG) {
 	     if (strcmp(q,"target.dct")==0) noprocset = 1;
       }
#endif
      copyfile_general(q, CUR_head);
   }
   infont = NULL;
}
