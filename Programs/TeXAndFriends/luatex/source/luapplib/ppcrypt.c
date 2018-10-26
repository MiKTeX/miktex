
#include "utilmd5.h"
#include "utilsha.h"

#include "pplib.h"

/* crypt struct */

static ppcrypt * ppcrypt_create (ppheap **pheap)
{
  ppcrypt *crypt;
  crypt = (ppcrypt *)ppheap_take(pheap, sizeof(ppcrypt));
  memset(crypt, 0, sizeof(ppcrypt));
  return crypt;
}

int ppcrypt_type (ppcrypt *crypt, ppname cryptname, ppuint *length, int *cryptflags)
{
  ppdict *filterdict;
  ppname filtertype;
  int cryptmd = 0, default256 = 0;

  if (crypt->map == NULL || (filterdict = ppdict_rget_dict(crypt->map, cryptname)) == NULL)
    return 0;
  if ((filtertype = ppdict_get_name(filterdict, "CFM")) == NULL)
    return 0;
  *cryptflags = 0;
  if (ppname_is(filtertype, "V2"))
    *cryptflags |= PPCRYPT_INFO_RC4;
  else if (ppname_is(filtertype, "AESV2"))
    *cryptflags |= PPCRYPT_INFO_AES;
  else if (ppname_is(filtertype, "AESV3"))
    *cryptflags |= PPCRYPT_INFO_AES, default256 = 1;
  else
    return 0;
  /* pdf spec page. 134: /Length is said to be optional bit-length of the key, but it seems to be a mistake, as Acrobat
     produces /Length key with bytes lengths, opposite to /Length key of the main encrypt dict. */
  if (length != NULL)
    if (!ppdict_get_uint(filterdict, "Length", length))
      *length = (*cryptflags & PPCRYPT_INFO_RC4) ? 5 : (default256 ? 32 : 16);
  /* one of metadata flags is set iff there is an explicit EncryptMetadata key */
  if (ppdict_get_bool(filterdict, "EncryptMetadata", &cryptmd))
    *cryptflags |= (cryptmd ? PPCRYPT_INFO_MD : PPCRYPT_INFO_NOMD);
  return 1;
}

static const uint8_t padding_string[] = {
  0x28, 0xBF, 0x4E, 0x5E, 0x4E, 0x75, 0x8A, 0x41, 0x64, 0x00, 0x4E, 0x56, 0xFF, 0xFA, 0x01, 0x08,
  0x2E, 0x2E, 0x00, 0xB6, 0xD0, 0x68, 0x3E, 0x80, 0x2F, 0x0C, 0xA9, 0xFE, 0x64, 0x53, 0x69, 0x7A
};

static void ppcrypt_set_userpass (ppcrypt *crypt, const void *userpass, size_t userpasslength)
{
  crypt->userpasslength = userpasslength > 32 ? 32 : userpasslength;
  memcpy(crypt->userpass, userpass, crypt->userpasslength);
  memcpy(crypt->userpass + crypt->userpasslength, padding_string, 32 - crypt->userpasslength);
  crypt->flags |= PPCRYPT_USER_PASSWORD;
}

static void ppcrypt_set_ownerpass (ppcrypt *crypt, const void *ownerpass, size_t ownerpasslength)
{
  crypt->ownerpasslength = ownerpasslength > 32 ? 32 : ownerpasslength;
  memcpy(crypt->ownerpass, ownerpass, crypt->ownerpasslength);
  memcpy(crypt->ownerpass + crypt->ownerpasslength, padding_string, 32 - crypt->ownerpasslength);
  crypt->flags |= PPCRYPT_OWNER_PASSWORD;
}

/* retrieving user password from owner password and owner key (variant < 5) */

static void ppcrypt_retrieve_userpass (ppcrypt *crypt, const void *ownerkey, size_t ownerkeysize)
{
  uint8_t temp[16], rc4key[32], rc4key2[32];
  uint8_t i;
  ppuint k;

  md5init();
  md5add(crypt->ownerpass, 32);
  md5put(rc4key);
  if (crypt->algorithm_revision >= 3)
  {
    for (i = 0; i < 50; ++i)
    {
      pplib_md5(rc4key, 16, temp);
      memcpy(rc4key, temp, 16);
    }
  }
  rc4_decode_data(ownerkey, ownerkeysize, crypt->userpass, rc4key, crypt->filekeylength);
  if (crypt->algorithm_revision >= 3)
  {
    for (i = 1; i <= 19; ++i)
    {
      for (k = 0; k < crypt->filekeylength; ++k)
        rc4key2[k] = rc4key[k] ^ i;
      rc4_decode_data(crypt->userpass, 32, crypt->userpass, rc4key2, crypt->filekeylength);
    }
  }
  //crypt->userpasslength = 32;
  for (crypt->userpasslength = 0; crypt->userpasslength < 32; ++crypt->userpasslength)
    if (memcmp(&crypt->userpass[crypt->userpasslength], padding_string, 32 - crypt->userpasslength) == 0)
      break;
  crypt->flags |= PPCRYPT_USER_PASSWORD;
}

/* generating file key; pdf spec p. 125 */

static void ppcrypt_filekey (ppcrypt *crypt, const void *ownerkey, size_t ownerkeysize, const void *id, size_t idsize)
{
  uint32_t p;
  uint8_t permissions[4], temp[16];
  int i;

  md5init();
  md5add(crypt->userpass, 32);
  md5add(ownerkey, ownerkeysize);
  p = (uint32_t)crypt->permissions;
  permissions[0] = get_byte1(p);
  permissions[1] = get_byte2(p);
  permissions[2] = get_byte3(p);
  permissions[3] = get_byte4(p);
  md5add(permissions, 4);
  md5add(id, idsize);
  if (crypt->algorithm_revision >= 4 && (crypt->flags & PPCRYPT_NO_METADATA))
    md5add("\xFF\xFF\xFF\xFF", 4);
  md5put(crypt->filekey);
  if (crypt->algorithm_revision >= 3)
  {
    for (i = 0; i < 50; ++i)
    {
      pplib_md5(crypt->filekey, (size_t)crypt->filekeylength, temp);
      memcpy(crypt->filekey, temp, 16);
    }
  }
}

/* generating userkey for comparison with /U; requires a general file key and id; pdf spec page 126-127 */

static void ppcrypt_userkey (ppcrypt *crypt, const void *id, size_t idsize, uint8_t *password_hash)
{
  uint8_t rc4key2[32];
  uint8_t i;
  ppuint k;

  if (crypt->algorithm_revision <= 2)
  {
    rc4_encode_data(padding_string, 32, password_hash, crypt->filekey, crypt->filekeylength);
  }
  else
  {
    md5init();
    md5add(padding_string, 32);
    md5add(id, idsize);
    md5put(password_hash);
    rc4_encode_data(password_hash, 16, password_hash, crypt->filekey, crypt->filekeylength);
    for (i = 1; i <= 19; ++i)
    {
      for (k = 0; k < crypt->filekeylength; ++k)
        rc4key2[k] = crypt->filekey[k] ^ i;
      rc4_encode_data(password_hash, 16, password_hash, rc4key2, crypt->filekeylength);
    }
    for (i = 16; i < 32; ++i)
      password_hash[i] = password_hash[i - 16] ^ i; /* arbitrary 16-bytes padding */
  }
}

/* validating /Perms key (pdf 1.7, /V 5 /R 5 crypt) */

static const uint8_t nulliv[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; /* AES-256 initialization vector */

static ppcrypt_status ppcrypt_authenticate_perms (ppcrypt *crypt, ppstring perms)
{ /* decode /Perms string overriding crypt setup (should match anyway) */
  uint8_t permsdata[16];
  //int64_t p;
  //int i;

  aes_decode_data(perms, ppstring_size(perms), permsdata, crypt->filekey, crypt->filekeylength, nulliv, AES_NULL_PADDING);

  if (permsdata[9] != 'a' || permsdata[10] != 'd' || permsdata[11] != 'b')
    return PPCRYPT_FAIL;

  // do not update permissions flags; they seem to be different inside crypt string
  //for (p = 0, i = 0; i < 8; ++i)
  //  p = p + (permsdata[i] << (i << 3)); /* low order bytes first */
  //crypt->permissions = (ppint)(int32_t)(p & 0x00000000FFFFFFFFLL); /* unset bits 33..64, treat as 32-bit signed int */

  if (permsdata[8] == 'T')
    crypt->flags &= ~PPCRYPT_NO_METADATA;
  else if (permsdata[8] == 'F')
    crypt->flags |= PPCRYPT_NO_METADATA;

  return PPCRYPT_DONE;
}

ppcrypt_status ppdoc_crypt_init (ppdoc *pdf, const void *userpass, size_t userpasslength, const void *ownerpass, size_t ownerpasslength)
{
  ppcrypt *crypt;
  ppdict *trailer, *encrypt;
  ppobj *obj;
  ppname name, *pkey;
  ppstring userkey, ownerkey, userkey_e = NULL, ownerkey_e = NULL;
  size_t hashlength;
  pparray *idarray;
  ppstring id = NULL, perms = NULL;
  int cryptflags, encryptmd;
  size_t strkeylength, stmkeylength;

  uint8_t password_hash[32]; /* /U and /O are 48 bytes strings for AES-256, but here we use only 32 */
  uint8_t *validation_salt, *key_salt;

  /* Every xref could theoretically have a separate encryption info. Not clarified in pdf spec but it seems that the top
     level xref encryption info is the one to be applied to all objects in all xrefs, including older. */
  trailer = ppxref_trailer(pdf->xref);
  if ((obj = ppdict_get_obj(trailer, "Encrypt")) == NULL)
    return PPCRYPT_NONE;
  /* Typically this is all done early, before loading body, so if /Encrypt is indirect reference, it points nothing. We have to load it here. */
  obj = ppobj_preloaded(pdf, obj);
  if (obj->type != PPDICT)
    return PPCRYPT_FAIL;
  encrypt = obj->dict;
  for (ppdict_first(encrypt, pkey, obj); *pkey != NULL; ppdict_next(pkey, obj))
    (void)ppobj_preloaded(pdf, obj);

  if ((name = ppdict_get_name(encrypt, "Filter")) != NULL && !ppname_is(name, "Standard"))
    return PPCRYPT_FAIL;

  if ((crypt = pdf->crypt) == NULL)
    crypt = pdf->crypt = ppcrypt_create(&pdf->heap);
  if (!ppdict_get_uint(encrypt, "V", &crypt->algorithm_variant))
    crypt->algorithm_variant = 0;
  if (crypt->algorithm_variant < 1 || crypt->algorithm_variant > 5)
    return PPCRYPT_FAIL;
  if (!ppdict_get_uint(encrypt, "R", &crypt->algorithm_revision))
    return PPCRYPT_FAIL;
  if (crypt->algorithm_revision >= 3)
    crypt->flags |= PPCRYPT_OBSCURITY;
  if (!ppdict_get_int(encrypt, "P", &crypt->permissions))
    return PPCRYPT_FAIL;
  if ((userkey = ppdict_get_string(encrypt, "U")) == NULL || (ownerkey = ppdict_get_string(encrypt, "O")) == NULL)
    return PPCRYPT_FAIL;
  userkey = ppstring_decoded(userkey);
  ownerkey = ppstring_decoded(ownerkey);
  /* for some reason acrobat pads /O and /U to 127 bytes with NULL, so we don't check the exact length but ensure the minimal */
  hashlength = crypt->algorithm_variant < 5 ? 32 : 48;
  if (ppstring_size(userkey) < hashlength || ppstring_size(ownerkey) < hashlength)
    return PPCRYPT_FAIL;
  if (crypt->algorithm_variant < 5)
  { // get first string from /ID (must not be ref)
    if ((idarray = ppdict_get_array(trailer, "ID")) == NULL || (id = pparray_get_string(idarray, 0)) == NULL)
      return PPCRYPT_FAIL;
    id = ppstring_decoded(id);
  }
  else
  {
    if ((userkey_e = ppdict_get_string(encrypt, "UE")) == NULL || (ownerkey_e = ppdict_get_string(encrypt, "OE")) == NULL)
      return PPCRYPT_FAIL;
    userkey_e = ppstring_decoded(userkey_e);
    ownerkey_e = ppstring_decoded(ownerkey_e);
    if (ppstring_size(userkey_e) < 32 || ppstring_size(ownerkey_e) < 32)
      return PPCRYPT_FAIL;
    if ((perms = ppdict_get_string(encrypt, "Perms")) == NULL)
      return PPCRYPT_FAIL;
    perms = ppstring_decoded(perms);
    if (ppstring_size(perms) != 16)
      return PPCRYPT_FAIL;
  }

  switch (crypt->algorithm_revision)
  {
    case 1:
      crypt->filekeylength = 5;
      crypt->flags |= PPCRYPT_RC4;
      break;
    case 2: case 3:
      if (ppdict_get_uint(encrypt, "Length", &crypt->filekeylength))
        crypt->filekeylength >>= 3; /* 40..256 bits, 5..32 bytes*/
      else
        crypt->filekeylength = 5; /* 40 bits, 5 bytes */
      crypt->flags |= PPCRYPT_RC4;
      break;
    case 4: case 5:
      if ((crypt->map = ppdict_rget_dict(encrypt, "CF")) == NULL)
        return PPCRYPT_FAIL;
      for (ppdict_first(crypt->map, pkey, obj); *pkey != NULL; ppdict_next(pkey, obj))
        (void)ppobj_preloaded(pdf, obj);
      /* /EncryptMetadata relevant only for version >=4, may be also provided in crypt filter dictionary; which takes a precedence then?
         we assume that if there is an explicit EncryptMetadata key, it overrides main encrypt dict flag or default flag (the default is true,
         meaning that Metadata stream is encrypted as others) */
      if (ppdict_get_bool(encrypt, "EncryptMetadata", &encryptmd) && !encryptmd)
        crypt->flags |= PPCRYPT_NO_METADATA;

      strkeylength = stmkeylength = 0;
      /* streams filter */
      if ((name = ppdict_get_name(encrypt, "StmF")) != NULL && ppcrypt_type(crypt, name, &stmkeylength, &cryptflags))
      {
        if (cryptflags & PPCRYPT_INFO_AES)
          crypt->flags |= PPCRYPT_STREAM_AES;
        else if (cryptflags & PPCRYPT_INFO_RC4)
          crypt->flags |= PPCRYPT_STREAM_RC4;
        if (cryptflags & PPCRYPT_INFO_NOMD)
          crypt->flags |= PPCRYPT_NO_METADATA;
        else if (cryptflags & PPCRYPT_INFO_MD)
          crypt->flags &= ~PPCRYPT_NO_METADATA;
      } /* else identity */
      /* strings filter */
      if ((name = ppdict_get_name(encrypt, "StrF")) != NULL && ppcrypt_type(crypt, name, &strkeylength, &cryptflags))
      {
        if (cryptflags & PPCRYPT_INFO_AES)
          crypt->flags |= PPCRYPT_STRING_AES;
        else if (cryptflags & PPCRYPT_INFO_RC4)
          crypt->flags |= PPCRYPT_STRING_RC4;
      } /* else identity */

      /* /Length of encrypt dict is irrelevant here, theoretically every crypt filter may have own length... It means that we should
         actually keep a different file key for streams and strings. But it leads to nonsense, as /U and /O entries refers to a single
         keylength, without a distinction for strings/streams. So we have to assume /Length is consistent. To expose the limitation: */
      if ((crypt->flags & PPCRYPT_STREAM) && (crypt->flags & PPCRYPT_STRING))
        if (strkeylength != stmkeylength)
          return PPCRYPT_FAIL;
      crypt->filekeylength = stmkeylength ? stmkeylength : strkeylength;
      if ((crypt->flags & PPCRYPT_STREAM) || (crypt->flags & PPCRYPT_STRING))
        if (crypt->filekeylength == 0)
          return PPCRYPT_FAIL;
      break;
    default:
      return PPCRYPT_FAIL;
  }

  /* password */

  if (userpass != NULL)
  {
    ppcrypt_set_userpass(crypt, userpass, userpasslength);
  }
  else if (ownerpass != NULL)
  {
    if (crypt->algorithm_variant < 5) // fetch user password from owner password
      ppcrypt_retrieve_userpass(crypt, ownerkey, ppstring_size(ownerkey));
    else                              // open the document using owner password
      ppcrypt_set_ownerpass(crypt, ownerpass, ownerpasslength);
  }
  else
  {
    return PPCRYPT_FAIL;
  }

  if (crypt->algorithm_variant < 5)
  { /* authenticate by comparing a generated vs present /U entry; depending on variant 16 or 32 bytes to compare */
    ppcrypt_filekey(crypt, ownerkey, ppstring_size(ownerkey), id, ppstring_size(id));
    ppcrypt_userkey(crypt, id, ppstring_size(id), password_hash); /* needs file key so comes after key generation */
    if (memcmp(userkey, password_hash, (crypt->algorithm_revision >= 3 ? 16 : 32)) == 0)
      return PPCRYPT_DONE;
    return PPCRYPT_PASS;
  }
  if (crypt->flags & PPCRYPT_USER_PASSWORD)
  {
    validation_salt = (uint8_t *)userkey + 32;
    key_salt = validation_salt + 8;
    sha256init();
    sha256add(crypt->userpass, crypt->userpasslength);
    sha256add(validation_salt, 8);
    sha256put(password_hash);
    if (memcmp(userkey, password_hash, 32) != 0)
      return PPCRYPT_PASS;
    sha256init();
    sha256add(crypt->userpass, crypt->userpasslength);
    sha256add(key_salt, 8);
    sha256put(password_hash);
    aes_decode_data(userkey_e, 32, crypt->filekey, password_hash, 32, nulliv, AES_NULL_PADDING);
    return ppcrypt_authenticate_perms(crypt, perms);
  }
  if (crypt->flags & PPCRYPT_OWNER_PASSWORD)
  {
    validation_salt = (uint8_t *)ownerkey + 32;
    key_salt = validation_salt + 8;
    sha256init();
    sha256add(crypt->ownerpass, crypt->ownerpasslength);
    sha256add(validation_salt, 8);
    sha256add(userkey, 48);
    sha256put(password_hash);
    if (memcmp(ownerkey, password_hash, 32) != 0)
      return PPCRYPT_PASS;
    sha256init();
    sha256add(crypt->ownerpass, crypt->ownerpasslength);
    sha256add(key_salt, 8);
    sha256add(userkey, 48);
    sha256put(password_hash);
    aes_decode_data(ownerkey_e, 32, crypt->filekey, password_hash, 32, nulliv, AES_NULL_PADDING);
    return ppcrypt_authenticate_perms(crypt, perms);
  }
  return PPCRYPT_FAIL; // should never get here
}

/* decrypting strings */

/*
Since strings are generally rare, but might occur in mass (name trees). We generate decryption key when needed.
All strings within the same reference are crypted with the same key. Both RC4 and AES algorithms expands
the crypt key in some way and the result of expansion is the same for the same crypt key. Instead of recreating
the ky for every string, we backup the initial decryption state.
*/

static void ppcrypt_strkey (ppcrypt *crypt, ppref *ref, int aes)
{
  if (crypt->cryptkeylength > 0)
  { /* crypt key already generated, just reinitialize crypt states */
    if (aes)
    { /* aes codecs that works on c-strings do not modify aes_state flags at all, so we actually don't need to revitalize the state,
         we only rewrite an initialization vector, which is modified during crypt procedure */
    }
    else
    { /* rc4 crypt map is modified during crypt procedure, so here we reinitialize rc4 bytes map */
      rc4_map_restore(&crypt->rc4state, &crypt->rc4copy);
    }
    return;
  }

  if (crypt->algorithm_variant < 5)
  {
    crypt->filekey[crypt->filekeylength + 0] = get_byte1(ref->number);
    crypt->filekey[crypt->filekeylength + 1] = get_byte2(ref->number);
    crypt->filekey[crypt->filekeylength + 2] = get_byte3(ref->number);
    crypt->filekey[crypt->filekeylength + 3] = get_byte1(ref->version);
    crypt->filekey[crypt->filekeylength + 4] = get_byte2(ref->version);

    if (aes)
    {
      crypt->filekey[crypt->filekeylength + 5] = 0x73;
      crypt->filekey[crypt->filekeylength + 6] = 0x41;
      crypt->filekey[crypt->filekeylength + 7] = 0x6C;
      crypt->filekey[crypt->filekeylength + 8] = 0x54;
    }

    pplib_md5(crypt->filekey, crypt->filekeylength + (aes ? 9 : 5), crypt->cryptkey);
    crypt->cryptkeylength = crypt->filekeylength + 5 >= 16 ? 16 : crypt->filekeylength + 5;
  }
  else
  {
    memcpy(crypt->cryptkey, crypt->filekey, 32);
    crypt->cryptkeylength = 32;
  }

  if (aes)
  {
    aes_decode_initialize(&crypt->aesstate, &crypt->aeskeyblock, crypt->cryptkey, crypt->cryptkeylength, NULL);
    aes_pdf_mode(&crypt->aesstate);
  }
  else
  {
    rc4_state_initialize(&crypt->rc4state, &crypt->rc4map, crypt->cryptkey, crypt->cryptkeylength);
    rc4_map_save(&crypt->rc4state, &crypt->rc4copy);
  }
}

int ppstring_decrypt (ppcrypt *crypt, const void *input, size_t size, void *output, size_t *newsize)
{
  int aes, rc4;
  aes = crypt->flags & PPCRYPT_STRING_AES;
  rc4 = crypt->flags & PPCRYPT_STRING_RC4;
  if (aes || rc4)
  {
    ppcrypt_strkey(crypt, crypt->ref, aes);
    if (aes)
      *newsize = aes_decode_state_data(&crypt->aesstate, input, size, output);
    else // if (rc4)
      *newsize = rc4_decode_state_data(&crypt->rc4state, input, size, output);
    return 1;
  }
  return 0; // identity crypt
}

/* decrypting streams */

/*
Streams are decrypted everytime when accessing the stream data. We need to be able to get or make
the key for decryption as long as the stream is alive. And to get the key we need the reference
number and version, plus document crypt info. First thought was to keep the reference to which
the stream belongs; stream->ref and accessing the crypt info stream->ref->xref->pdf->crypt.
It would be ok as long as absolutelly nothing happens with ref and crypt. At some point pplib
may drift into rewriting support, which would imply ref/xref/crypt/pdf structures modifications.
So I feel better with generating a crypt key for every stream in encrypted document, paying a cost
of pplib_md5() for all streams, not necessarily those actually read.

Key generation is the same as for strings, but different for distinct encryption methods (rc4 vs aes).
Since streams and strings might theoretically be encrypted with different filters. No reason to cacche
decryption state here.
*/

ppstring ppcrypt_stmkey (ppcrypt *crypt, ppref *ref, int aes, ppheap **pheap)
{
  ppstring cryptkeystring;
  //if (crypt->cryptkeylength > 0)
  //  return;

  if (crypt->algorithm_variant < 5)
  {
    crypt->filekey[crypt->filekeylength + 0] = get_byte1(ref->number);
    crypt->filekey[crypt->filekeylength + 1] = get_byte2(ref->number);
    crypt->filekey[crypt->filekeylength + 2] = get_byte3(ref->number);
    crypt->filekey[crypt->filekeylength + 3] = get_byte1(ref->version);
    crypt->filekey[crypt->filekeylength + 4] = get_byte2(ref->version);

    if (aes)
    {
      crypt->filekey[crypt->filekeylength + 5] = 0x73;
      crypt->filekey[crypt->filekeylength + 6] = 0x41;
      crypt->filekey[crypt->filekeylength + 7] = 0x6C;
      crypt->filekey[crypt->filekeylength + 8] = 0x54;
    }

    pplib_md5(crypt->filekey, crypt->filekeylength + (aes ? 9 : 5), crypt->cryptkey);
    crypt->cryptkeylength = crypt->filekeylength + 5 >= 16 ? 16 : crypt->filekeylength + 5; // how about 256bits AES??
  }
  else
  { // we could actually generate this string once, but.. aes itself is way more expensive that we can earn here
    memcpy(crypt->cryptkey, crypt->filekey, 32); // just for the record
    crypt->cryptkeylength = 32;
  }
  cryptkeystring = ppstring_internal(crypt->cryptkey, crypt->cryptkeylength, pheap);
  return ppstring_decoded(cryptkeystring);
}
