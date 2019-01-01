## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2018 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/log4cxx")

set(apu_have_crypto_10 0)
set(apu_have_apr_iconv_10 0)
SET(apr_has_ldap_10 0)

add_definitions(
  -DWINNT
)

include_directories(BEFORE
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/private
)

set(public_include_directories
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_headers
  include/apr_anylock.h
  include/apr_base64.h
  include/apr_buckets.h
  include/apr_crypto.h
  include/apr_date.h
  include/apr_dbd.h
  include/apr_dbm.h
  include/apr_hooks.h
  include/apr_ldap_init.h
  include/apr_ldap_option.h
  include/apr_ldap_rebind.h
  include/apr_ldap_url.h
  include/apr_md4.h
  include/apr_md5.h
  include/apr_memcache.h
  include/apr_optional.h
  include/apr_optional_hooks.h
  include/apr_queue.h
  include/apr_reslist.h
  include/apr_rmm.h
  include/apr_sdbm.h
  include/apr_sha1.h
  include/apr_strmatch.h
  include/apr_thread_pool.h
  include/apr_uri.h
  include/apr_uuid.h
  include/apr_xlate.h
  include/apr_xml.h
  include/apu_errno.h
  include/apu_version.h
)

foreach(h ${public_headers})
  configure_file(
    source/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/${h}
    COPYONLY
  )
endforeach()

configure_file(
  source/include/apu.hwc
  ${CMAKE_CURRENT_BINARY_DIR}/include/apu.h
)

configure_file(
  source/include/apr_ldap.hwc
  ${CMAKE_CURRENT_BINARY_DIR}/include/apr_ldap.h
)

configure_file(
  source/include/private/apu_config.hw
  ${CMAKE_CURRENT_BINARY_DIR}/include/apu_config.h
  COPYONLY
)

configure_file(
  source/include/private/apu_select_dbm.hw
  ${CMAKE_CURRENT_BINARY_DIR}/include/apu_select_dbm.h
  COPYONLY
)

configure_file(
  source/include/apu_want.hw
  ${CMAKE_CURRENT_BINARY_DIR}/include/apu_wantg.h
  COPYONLY
)

set(apu_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_brigade.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_alloc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_eos.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_file.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_flush.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_heap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_mmap.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_pipe.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_pool.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_refcount.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_simple.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/buckets/apr_buckets_socket.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/apr_crypto.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/apr_md4.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/apr_md5.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/apr_passwd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/apr_sha1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/apr_siphash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/crypt_blowfish.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/getuuid.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/crypto/uuid.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dbd/apr_dbd.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dbm/apr_dbm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dbm/apr_dbm_sdbm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dbm/sdbm/sdbm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dbm/sdbm/sdbm_hash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dbm/sdbm/sdbm_lock.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/dbm/sdbm/sdbm_pair.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/encoding/apr_base64.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/hooks/apr_hooks.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/memcache/apr_memcache.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/apr_date.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/apr_queue.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/apr_reslist.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/apr_rmm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/apr_thread_pool.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/apu_dso.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/misc/apu_version.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/redis/apr_redis.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/strmatch/apr_strmatch.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/uri/apr_uri.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/xlate/xlate.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/xml/apr_xml.c

  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_anylock.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_base64.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_buckets.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_crypto.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_date.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_dbd.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_dbm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_hooks.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_ldap_init.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_ldap_option.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_ldap_rebind.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_ldap_url.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_md4.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_md5.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_memcache.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_optional.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_optional_hooks.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_queue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_redis.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_reslist.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_rmm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_sdbm.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_sha1.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_siphash.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_strmatch.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_thread_pool.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_uri.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_uuid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_xlate.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apr_xml.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apu_errno.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/include/apu_version.h
)

if(NOT LINK_EVERYTHING_STATICALLY)
  add_subdirectory(shared)
endif()

add_subdirectory(static)
