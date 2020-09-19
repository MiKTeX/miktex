
#ifndef UTIL_CRYPTDEF_H
#define UTIL_CRYPTDEF_H

struct rc4_state {
  union {
    rc4_map *map;
    uint8_t *smap;
  };
  int i, j;
  int flush;
  int flags;
};

struct aes_state {
  size_t keylength;
  int rounds;
  //int keywords;
  union {
    aes_block block;
    uint8_t data[16];
  };
  aes_keyblock *keyblock;
  uint8_t iv[16];
  uint8_t buffered;
  int flush;
  int flags;
};

typedef union { rc4_state *rc4state; aes_state *aesstate; void *voidstate; } crypt_state_pointer; // to avoid 'dereferencing type-puned ...' warnings

#endif