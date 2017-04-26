## CMakeLists.txt                                       -*- CMake -*-
##
## Copyright (C) 2015-2017 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/libressl")

set(libressl_dir ${CMAKE_CURRENT_SOURCE_DIR})
set(libressl_source_dir ${CMAKE_CURRENT_SOURCE_DIR}/source)

set(public_include_directory
  ${CMAKE_CURRENT_BINARY_DIR}/include
)

set(public_include_directories ${public_include_directory})

include_directories(BEFORE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${libressl_source_dir}/include
  ${libressl_source_dir}/include/compat
)

set(public_headers
  openssl/aes.h
  openssl/asn1.h
  openssl/asn1_mac.h
  openssl/asn1t.h
  openssl/bio.h
  openssl/blowfish.h
  openssl/bn.h
  openssl/buffer.h
  openssl/camellia.h
  openssl/cast.h
  openssl/chacha.h
  openssl/cmac.h
  openssl/comp.h
  openssl/conf.h
  openssl/conf_api.h
  openssl/crypto.h
  openssl/curve25519.h
  openssl/des.h
  openssl/dh.h
  openssl/dsa.h
  openssl/dso.h
  openssl/dtls1.h
  openssl/ec.h
  openssl/ecdh.h
  openssl/ecdsa.h
  openssl/engine.h
  openssl/err.h
  openssl/evp.h
  openssl/gost.h
  openssl/hmac.h
  openssl/idea.h
  openssl/lhash.h
  openssl/md4.h
  openssl/md5.h
  openssl/modes.h
  openssl/obj_mac.h
  openssl/objects.h
  openssl/ocsp.h
  openssl/opensslconf.h
  openssl/opensslfeatures.h
  openssl/opensslv.h
  openssl/ossl_typ.h
  openssl/pem.h
  openssl/pem2.h
  openssl/pkcs12.h
  openssl/pkcs7.h
  openssl/poly1305.h
  openssl/rand.h
  openssl/rc2.h
  openssl/rc4.h
  openssl/ripemd.h
  openssl/rsa.h
  openssl/safestack.h
  openssl/sha.h
  openssl/srtp.h
  openssl/ssl.h
  openssl/ssl2.h
  openssl/ssl23.h
  openssl/ssl3.h
  openssl/stack.h
  openssl/tls1.h
  openssl/ts.h
  openssl/txt_db.h
  openssl/ui.h
  openssl/ui_compat.h
  openssl/whrlpool.h
  openssl/x509.h
  openssl/x509_vfy.h
  openssl/x509v3.h
)

foreach(h ${public_headers})
  configure_file(
    source/include/${h}
    ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
    COPYONLY
  )
endforeach()

add_definitions(-DLIBRESSL_INTERNAL)
add_definitions(-DOPENSSL_NO_HW_PADLOCK)
add_definitions(-D__BEGIN_HIDDEN_DECLS=)
add_definitions(-D__END_HIDDEN_DECLS=)

if(MSVC)
#  add_definitions(-Dinline=__inline)
#  add_definitions(-Drestrict)
#  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#  add_definitions(-D_CRT_DEPRECATED_NO_WARNINGS)
#  add_definitions(-D_REENTRANT -D_POSIX_THREAD_SAFE_FUNCTIONS)
#  add_definitions(-DWIN32_LEAN_AND_MEAN -D_WIN32_WINNT=0x0501)
#  add_definitions(-DCPPFLAGS -DOPENSSL_NO_SPEED -DNO_SYSLOG -DNO_CRYPT)
endif()

if(HAVE_STRNLEN)
  add_definitions(-DHAVE_STRNLEN)
endif()

add_subdirectory(crypto)
#add_subdirectory(ssl)
