
#ifndef PP_CRYPT_H
#define PP_CRYPT_H

#include "ppfilter.h"
#include "utilcrypt.h"
#include "utilcryptdef.h"

typedef struct {
  ppuint algorithm_variant;  /* /V entry of encrypt dict */
  ppuint algorithm_revision; /* /R entry of encrypt dict */
  ppint permissions;         /* /P entry of encrypt dict */
  ppdict *map;               /* /CF filters map of encrypt dict */
  uint8_t userpass[32];      /* padded user password */
  size_t userpasslength;     /* the length of unpadded user password */
  uint8_t ownerpass[32];     /* padded owner password */
  size_t ownerpasslength;    /* the length of unpadded owner password */
  uint8_t filekey[32+5+4];   /* generated file key with extra space of 5..9 bytes for salt */
  size_t filekeylength;      /* key length; usually 5, 16 or 32 bytes */
  uint8_t cryptkey[32];      /* final crypt key for a given reference */
  size_t cryptkeylength;     /* final crypt key length; usually keylength + 5 */
  ppref *ref;                /* currently loaded ref (each ref may have a different key) */
  union {                    /* cached crypt states for strings encrypted/decrypted with the same key */
    struct {
      rc4_state rc4state;
      rc4_map rc4map;
      rc4_map rc4copy;
    };
    struct {
      aes_state aesstate;
      aes_keyblock aeskeyblock;
      uint8_t ivcopy[16];
    };
  };
  int flags;
} ppcrypt;

#define PPCRYPT_NO_METADATA (1<<0)
#define PPCRYPT_USER_PASSWORD (1<<1)
#define PPCRYPT_OWNER_PASSWORD (1<<2)
#define PPCRYPT_STREAM_RC4 (1<<3)
#define PPCRYPT_STRING_RC4 (1<<4)
#define PPCRYPT_STREAM_AES (1<<5)
#define PPCRYPT_STRING_AES (1<<6)
#define PPCRYPT_OBSCURITY  (1<<7)

#define PPCRYPT_STREAM (PPCRYPT_STREAM_AES|PPCRYPT_STREAM_RC4)
#define PPCRYPT_STRING (PPCRYPT_STRING_AES|PPCRYPT_STRING_RC4)
#define PPCRYPT_RC4 (PPCRYPT_STREAM_RC4|PPCRYPT_STRING_RC4)
#define PPCRYPT_AES (PPCRYPT_STREAM_AES|PPCRYPT_STRING_AES)

#define PPCRYPT_INFO_AES  (1<<0)
#define PPCRYPT_INFO_RC4  (1<<1)
#define PPCRYPT_INFO_MD   (1<<2)
#define PPCRYPT_INFO_NOMD (1<<3)

ppcrypt_status ppdoc_crypt_init (ppdoc *pdf, const void *userpass, size_t userpasslength, const void *ownerpass, size_t ownerpasslength);
int ppstring_decrypt (ppcrypt *crypt, const void *input, size_t size, void *output, size_t *newsize);

#define ppcrypt_start_ref(crypt, r) ((crypt)->ref = r, (crypt)->cryptkeylength = 0)
#define ppcrypt_end_ref(crypt) ((crypt)->ref = NULL, (crypt)->cryptkeylength = 0)
#define ppcrypt_ref(pdf, crypt) ((crypt = (pdf)->crypt) != NULL && crypt->ref != NULL)

int ppcrypt_type (ppcrypt *crypt, ppname cryptname, ppuint *length, int *cryptflags);
ppstring ppcrypt_stmkey (ppcrypt *crypt, ppref *ref, int aes, ppheap **pheap);

#endif