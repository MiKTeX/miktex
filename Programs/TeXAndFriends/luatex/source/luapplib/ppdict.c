
#include "pplib.h"

ppdict * ppdict_create (const ppobj *stackpos, size_t size, ppheap **pheap)
{
	ppdict *dict;
	ppobj *data;
	ppname *pkey;
	size_t i;
	size >>= 1;
	dict = (ppdict *)ppheap_take(pheap, sizeof(ppdict) + size * sizeof(ppobj) + (size + 1) * sizeof(ppname *)); // ? + size * sizeof(ppdict_map_node)
	dict->size = 0;
	dict->data = data = (ppobj *)(dict + 1);
	dict->keys = pkey = (ppname *)(dict->data + size);
	for (i = 0; i < size; ++i, stackpos += 2)
	{
	  if (stackpos->type != PPNAME) // we need this check at lest for trailer hack
	    continue;
    *pkey = stackpos->name;
    *data = *(stackpos + 1);
    ++pkey, ++data, ++dict->size;
	}
	*pkey = NULL; // sentinel for convinient iteration
	return dict;
}

ppobj * ppdict_get_obj (ppdict *dict, const char *name)
{ // some lookup? will see
  ppname *pkey;
  ppobj *obj;
  for (ppdict_first(dict, pkey, obj); *pkey != NULL; ppdict_next(pkey, obj))
    if (strcmp(*pkey, name) == 0) // not ppname_eq() or ppname_is()!!
      return obj;
  return NULL;
}

ppobj * ppdict_rget_obj (ppdict *dict, const char *name)
{
	ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_obj(obj) : NULL;
}

int ppdict_get_bool (ppdict *dict, const char *name, int *v)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_bool(obj, *v) : 0;
}

int ppdict_rget_bool (ppdict *dict, const char *name, int *v)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_bool(obj, *v) : 0;
}

int ppdict_get_int (ppdict *dict, const char *name, ppint *v)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_int(obj, *v) : 0;
}

int ppdict_rget_int (ppdict *dict, const char *name, ppint *v)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_int(obj, *v) : 0;
}

int ppdict_get_uint (ppdict *dict, const char *name, ppuint *v)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_uint(obj, *v) : 0;
}

int ppdict_rget_uint (ppdict *dict, const char *name, ppuint *v)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_uint(obj, *v) : 0;
}

int ppdict_get_num (ppdict *dict, const char *name, ppnum *v)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_num(obj, *v) : 0;
}

int ppdict_rget_num (ppdict *dict, const char *name, ppnum *v)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_num(obj, *v) : 0;
}

ppname ppdict_get_name (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_name(obj) : NULL;
}

ppname ppdict_rget_name (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_name(obj) : NULL;
}

ppstring ppdict_get_string (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_string(obj) : NULL;
}

ppstring ppdict_rget_string (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_string(obj) : NULL;
}

pparray * ppdict_get_array (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_array(obj) : NULL;
}

pparray * ppdict_rget_array (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_array(obj) : NULL;
}

ppdict * ppdict_get_dict (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_dict(obj) : NULL;
}

ppdict * ppdict_rget_dict (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_dict(obj) : NULL;
}

/*
ppstream * ppdict_get_stream (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_stream(obj) : NULL;
}
*/

ppstream * ppdict_rget_stream (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_stream(obj) : NULL;
}

ppref * ppdict_get_ref (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_get_ref(obj) : NULL;
}

ppref * ppdict_rget_ref (ppdict *dict, const char *name)
{
  ppobj *obj;
  return (obj = ppdict_get_obj(dict, name)) != NULL ? ppobj_rget_ref(obj) : NULL;
}
