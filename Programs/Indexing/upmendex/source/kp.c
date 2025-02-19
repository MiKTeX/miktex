/* Written by K.Asayayam  Sep. 1995 */

#include "mendex.h"
#include <kpathsea/expand.h>
#include <kpathsea/pathsearch.h>
#include <kpathsea/tex-file.h>

#include "kp.h"

/* KP_get_path(char *var, char *def_val)
     ARGUMENTS:
       char *var:     name of variable.
       char *def_val: default value.
 */
static const char *KP_get_path(const char *var, const char *def_val)
{
  char avar[264];
  const char *p;
  strcpy(avar, "${");
  strncat(avar, var, 260);
  strcat(avar, "}");
  p = kpse_path_expand(avar);
  return (p && *p) ? p : def_val;
}

/*
 */
void KP_entry_filetype(KpathseaSupportInfo *info)
{
  info->path = KP_get_path(info->var_name,info->path);
}

/* KP_find_file(KpathseaSupportInfo *info, char *name)
     ARGUMENTS:
       KpathseaSupportInfo *info: Informations about the type of files.
       char *name:                Name of file.
 */
const char *KP_find_file(KpathseaSupportInfo *info, const char *name)
{
  char *ret;
  ret = kpse_path_search(info->path,name,1);
  if (!ret && info->suffix && !find_suffix(name)) {
    char *suff_name;
    suff_name = concat3(name,".",info->suffix);
    ret = kpse_path_search(info->path,suff_name,1);
    free(suff_name);
  }
  return ret ? ret : name;
}

const char *KP_find_ist_file(const char *name)
{
  char *ret = kpse_find_file(name, kpse_ist_format, 1);
  return ret ? ret : name;
}

const char *KP_find_dict_file(const char *name)
{
  return KP_find_file(&kp_dict, name);
}
