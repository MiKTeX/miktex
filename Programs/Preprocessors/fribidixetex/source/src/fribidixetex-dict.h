#ifndef _DICT_H_
#define _DICT_H_
#include <fribidi/fribidi.h>

enum {DICT_ENV, DICT_TAG};

int dict_is_hebrew_letter(FriBidiChar ch);

typedef char *(*trans_func_t)(FriBidiChar *str);
char *dict_translate_env(FriBidiChar *str);
char *dict_translate_tag(FriBidiChar *str);

void dict_add_tans(FriBidiChar *tag_uni,char *tag_ascii,int type);


#endif
