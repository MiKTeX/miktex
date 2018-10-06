
#include "pplib.h"

pparray * pparray_create (const ppobj *stackpos, size_t size, ppheap **pheap)
{
	pparray *array;
	ppobj *data;
  array = (pparray *)ppheap_take(pheap, sizeof(pparray) + size * sizeof(ppobj));
  array->size = size;
  array->data = data = (ppobj *)(array + 1);
  memcpy(data, stackpos, size * sizeof(ppobj));
  return array;
}

ppobj * pparray_get_obj (pparray *array, size_t index)
{
  return pparray_get(array, index);
}

ppobj * pparray_rget_obj (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_obj(obj) : NULL;
}

int pparray_get_bool (pparray *array, size_t index, int *v)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_bool(obj, *v) : 0;
}

int pparray_rget_bool (pparray *array, size_t index, int *v)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_bool(obj, *v) : 0;
}

int pparray_get_int (pparray *array, size_t index, ppint *v)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_int(obj, *v) : 0;
}

int pparray_rget_int (pparray *array, size_t index, ppint *v)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_int(obj, *v) : 0;
}

int pparray_get_uint (pparray *array, size_t index, ppuint *v)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_uint(obj, *v) : 0;
}

int pparray_rget_uint (pparray *array, size_t index, ppuint *v)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_uint(obj, *v) : 0;
}

int pparray_get_num (pparray *array, size_t index, ppnum *v)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_num(obj, *v) : 0;
}

int pparray_rget_num (pparray *array, size_t index, ppnum *v)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_num(obj, *v) : 0;
}

ppname pparray_get_name (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_name(obj) : NULL;
}

ppname pparray_rget_name (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_name(obj) : NULL;
}

ppstring pparray_get_string (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_string(obj) : NULL;
}

ppstring pparray_rget_string (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_string(obj) : NULL;
}

pparray * pparray_get_array (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_array(obj) : NULL;
}

pparray * pparray_rget_array (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_array(obj) : NULL;
}

ppdict * pparray_get_dict (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_dict(obj) : NULL;
}

ppdict * pparray_rget_dict (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_dict(obj) : NULL;
}

/*
ppstream * pparray_get_stream (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_stream(obj) : NULL;
}
*/

ppstream * pparray_rget_stream (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_stream(obj) : NULL;
}

ppref * pparray_get_ref (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_get_ref(obj) : NULL;
}

ppref * pparray_rget_ref (pparray *array, size_t index)
{
  ppobj *obj;
  return (obj = pparray_get(array, index)) != NULL ? ppobj_rget_ref(obj) : NULL;
}
