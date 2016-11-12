#include "fribidixetex-dict.h"
#include "fribidixetex-defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***********************
 *        D A T A      *
 ***********************/


typedef struct 
{
	wchar_t	hebrew[MAX_COMMAND_LEN];
	char 	ascii[MAX_COMMAND_LEN];
}
	dict_t;

#define LETTER_ALEPH	L'א'
#define LETTER_TAV		L'ת'

#define SIZEOF_ARRAY(x) (sizeof(x)/sizeof(x[0]))

dict_t dict_utf8_tags_perm[] = 
{
	{ L"ש", "L" },
	{ L"י", "R"  },
	{ L"התחל", "begin" },
	{ L"סיים", "end"  },
	{ L"כותרת", "title" },
	{ L"תוכןעניינים", "tableofcontents" },
	{ L"צורכותרת", "maketitle" },
	{ L"מחבר", "author" },
	{ L"תאריך", "date" },
	{ L"חלק", "part" },
	{ L"פרק", "chapter" },
	{ L"סעיף", "section" },
	{ L"תתסעיף", "subsection"},
	{ L"תתתתסעיף", "subsubsection"},
	{ L"הדגש", "emph" },
	{ L"פריט", "item" },
	{ L"נק", "item" },
	{ L"תג", "label" },
	{ L"ראה", "ref" },
	{ L"הערתשוליים", "footnote" },
	{ L"צטט", "cite" },
	{ L"נקודות", "ldots" },
};

dict_t dict_utf8_envs_perm[] = 
{
	{ L"פירוט","itemize"},
	{ L"תיאור","description" },
	{ L"מספור", "enumerate" },
	{ L"שיר", "verse" },
	{ L"מסמך", "document" },
	{ L"תקציר", "abstract" },
};

/*************/
/* User tags */
/*************/

int user_tags_size=0;
int user_envs_size=0;

dict_t dict_utf8_tags_user[MAX_USER_TRANSLATIONS];
dict_t dict_utf8_envs_user[MAX_USER_TRANSLATIONS];


/**************/


int dict_is_hebrew_letter(FriBidiChar ch)
{
	return (LETTER_ALEPH<=ch && ch<=LETTER_TAV);
	/* Range from alef to tav in Unicode */
}

char *dict_find(FriBidiChar *str,dict_t *dictionary,int size)
{
	wchar_t *wstr;
	int i;
	FriBidiChar *bstr;
	
	for(i=0;i<size;i++){
		wstr=dictionary[i].hebrew;
		bstr=str;
		while(*bstr && *wstr && *bstr == (FriBidiChar)*wstr){
			bstr++;
			wstr++;
		}
		if(*bstr==0 && *wstr==0){
			return dictionary[i].ascii;
		}
	}
	return NULL;
}


char *dict_translate_env(FriBidiChar *str)
{
	char *ptr;
	ptr=dict_find(str,dict_utf8_envs_perm,SIZEOF_ARRAY(dict_utf8_envs_perm));
	if(ptr== NULL)
		return dict_find(str,dict_utf8_envs_user,user_envs_size);
	return ptr;
}
char *dict_translate_tag(FriBidiChar *str)
{
	char *ptr;
	ptr=dict_find(str,dict_utf8_tags_perm,SIZEOF_ARRAY(dict_utf8_tags_perm));
	if(ptr==NULL)
		return dict_find(str,dict_utf8_tags_user,user_tags_size);
	return ptr;
}

void dict_set_tag(dict_t *entry,FriBidiChar *uni,char *ascii)
{
	int i;
	for(i=0;uni[i];i++)
		entry->hebrew[i]=uni[i];
	entry->hebrew[i]=0;
	strcpy(entry->ascii,ascii);
}

void dict_add_tans(FriBidiChar *tag_uni,char *tag_ascii,int type)
{
	if(type == DICT_ENV) {
		if(user_envs_size>=MAX_USER_TRANSLATIONS) {
			fprintf(stderr,"Too many evironments translations added\n");
			exit(1);
		}
		dict_set_tag(dict_utf8_envs_user+user_envs_size,tag_uni,tag_ascii);
		user_envs_size++;
	}
	else {
		if(user_tags_size>=MAX_USER_TRANSLATIONS) {
			fprintf(stderr,"Too many tags translations added\n");
			exit(1);
		}
		dict_set_tag(dict_utf8_tags_user+user_tags_size,tag_uni,tag_ascii);
		user_tags_size++;
	}
}
