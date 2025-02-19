/* Written by K.Asayayam  Sep. 1995 */
#ifndef KSUPPORT_H_INCLUDED
#define KSUPPORT_H_INCLUDED
typedef struct {
  const char *var_name;
  const char *path;
  const char *suffix;
} KpathseaSupportInfo;

extern KpathseaSupportInfo kp_ist, kp_dict;

extern void KP_entry_filetype(KpathseaSupportInfo *);
extern const char *KP_find_file(KpathseaSupportInfo *, const char *);
extern const char *KP_find_ist_file(const char *);
extern const char *KP_find_dict_file(const char *);
#endif /* ! KSUPPORT_H_INCLUDED */
