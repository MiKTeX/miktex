## CMakeLists.txt
##
## Copyright (C) 2015-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

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

configure_file(
    miktex-libressl-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-libressl-version.h
)

list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/miktex-libressl-version.h)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
)

if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "(aarch64|arm64)")
	set(HOST_AARCH64 TRUE)
elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm")
	set(HOST_ARM TRUE)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS" AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "i386")
	set(HOST_X86_64 TRUE)
elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "(x86_64|amd64)")
	set(HOST_X86_64 TRUE)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "[i?86|x86]")
	set(ENABLE_ASM FALSE)
	set(HOST_I386 TRUE)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "mips64")
	set(HOST_MIPS64 TRUE)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "mips")
	set(HOST_MIPS TRUE)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "powerpc")
	set(HOST_POWERPC TRUE)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "ppc64")
	set(HOST_PPC64 TRUE)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "riscv64")
	set(HOST_RISCV64 TRUE)
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "sparc64")
	set(HOST_SPARC64 TRUE)
else()
	set(ENABLE_ASM FALSE)
endif()

set(public_headers
    openssl/aes.h
    openssl/asn1.h
    openssl/asn1t.h
    openssl/bio.h
    openssl/blowfish.h
    openssl/bn.h
    openssl/buffer.h
    openssl/camellia.h
    openssl/cast.h
    openssl/chacha.h
    openssl/cmac.h
    openssl/cms.h
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
    openssl/hkdf.h
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
    openssl/sm3.h
    openssl/sm4.h
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
    list(APPEND configured_headers ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
endforeach()

add_definitions(-DLIBRESSL_INTERNAL)
add_definitions(-DOPENSSL_NO_HW_PADLOCK)
add_definitions(-D__BEGIN_HIDDEN_DECLS=)
add_definitions(-D__END_HIDDEN_DECLS=)

if(HAVE_STRNLEN)
    add_definitions(-DHAVE_STRNLEN)
endif()

add_definitions(-DHAVE_GETOPT)

set(HAVE_INET_NTOP TRUE)
set(HAVE_INET_PTON TRUE)

if(HAVE_INET_NTOP)
    add_definitions(-DHAVE_INET_NTOP)
endif()

if(HAVE_INET_PTON)
    add_definitions(-DHAVE_INET_PTON)
endif()

add_subdirectory(crypto)
