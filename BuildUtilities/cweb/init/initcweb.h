/*84:*/


/* Prototypes for functions, either
 * declared in common.w and used in ctangle.w and cweave.w, or
 * used in common.w and declared in ctangle.w and cweave.w.  */
/*35:*/


extern int names_match(name_pointer,const char*,int,char);


/*:35*//*40:*/


extern void init_p(name_pointer p,char t);


/*:40*//*48:*/


extern void init_node(name_pointer node);


/*:48*//*59:*/


extern void err_print(const char*);


/*:59*//*62:*/


extern int wrap_up(void);
extern void print_stats(void);


/*:62*//*65:*/


extern void fatal(const char*,const char*);
extern void overflow(const char*);


/*:65*//*83:*/


extern void common_init(void);
extern int input_ln(FILE*fp);
extern void reset_input(void);
extern int get_line(void);
extern void check_complete(void);
extern name_pointer id_lookup(const char*first,const char*last,char t);
extern void print_section_name(name_pointer p);
extern void sprint_section_name(char*dest,name_pointer p);
extern name_pointer section_lookup(char*first,char*last,int ispref);



/*:83*/


#if !defined(MIKTEX)
extern const char*versionstring;
#endif

/*:84*/
