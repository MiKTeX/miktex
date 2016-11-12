## CMakeLists.txt:                                      -*- CMake -*-
##
## Copyright (C) 2015-2016 Christian Schenk
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

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_ADMIN_FOLDER}/TPM")

set(common_tpms
  miktex-config.tpm.in
  miktex-doc.tpm.in
)

if(WITH_CONTEXT_SUPPORT)
  list(APPEND common_tpms
    miktex-context-config.tpm.in
  )
endif()

foreach(tpm ${common_tpms})
  get_filename_component(fn ${tpm} NAME_WE)
  configure_file(
    ${tpm}
    "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${fn}-${MIKTEX_SERIES_STR}.tpm"
  )
  list(APPEND tpms "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${fn}-${MIKTEX_SERIES_STR}.tpm")
endforeach()

if(${MIKTEX_TARGET_SYSTEM_TAG} STREQUAL "windows-x86")
  set(target_system_tpms
    windows-x86/miktex-arctrl-bin.tpm.in
    windows-x86/miktex-bibtex-bin.tpm.in
    windows-x86/miktex-bibtex8bit-bin.tpm.in
    windows-x86/miktex-bin.tpm.in
    windows-x86/miktex-bzip2-bin.tpm.in
    windows-x86/miktex-cairo-bin.tpm.in
    windows-x86/miktex-chktex-bin.tpm.in
    windows-x86/miktex-cjkutils-bin.tpm.in
    windows-x86/miktex-curl-bin.tpm.in
    windows-x86/miktex-cweb-bin.tpm.in
    windows-x86/miktex-devnag-bin.tpm.in
    windows-x86/miktex-dvicopy-bin.tpm.in
    windows-x86/miktex-dvipdfmx-bin.tpm.in
    windows-x86/miktex-dvipng-bin.tpm.in
    windows-x86/miktex-dvips-bin.tpm.in
    windows-x86/miktex-dvisvgm-bin.tpm.in
    windows-x86/miktex-epstopdf-bin.tpm.in
    windows-x86/miktex-expat-bin.tpm.in
    windows-x86/miktex-findtexmf-bin.tpm.in
    windows-x86/miktex-fontconfig-bin.tpm.in
    windows-x86/miktex-fonts-bin.tpm.in
    windows-x86/miktex-freetype2-bin.tpm.in
    windows-x86/miktex-fribidixetex-bin.tpm.in
    windows-x86/miktex-gd-bin.tpm.in
    windows-x86/miktex-graphite2-bin.tpm.in
    windows-x86/miktex-gsf2pk-bin.tpm.in
    windows-x86/miktex-hunspell-bin.tpm.in
    windows-x86/miktex-int-bin.tpm.in
    windows-x86/miktex-jpeg-bin.tpm.in
    windows-x86/miktex-kpathsea-bin.tpm.in
    windows-x86/miktex-libressl-bin.tpm.in
    windows-x86/miktex-log4cxx-bin.tpm.in
    windows-x86/miktex-lua52-bin.tpm.in
    windows-x86/miktex-luatex-bin.tpm.in
    windows-x86/miktex-lzma-bin.tpm.in
    windows-x86/miktex-makeindex-bin.tpm.in
    windows-x86/miktex-md5-bin.tpm.in
    windows-x86/miktex-metafont-bin.tpm.in
    windows-x86/miktex-metapost-bin.tpm.in
    windows-x86/miktex-mfware-bin.tpm.in
    windows-x86/miktex-mkfntmap-bin.tpm.in
    windows-x86/miktex-mktex-bin.tpm.in
    windows-x86/miktex-mo-bin.tpm.in
    windows-x86/miktex-mpm-bin.tpm.in
    windows-x86/miktex-mpmcli-bin.tpm.in
    windows-x86/miktex-mspack-bin.tpm.in
    windows-x86/miktex-mthelp-bin.tpm.in
    windows-x86/miktex-mtprint-bin.tpm.in
    windows-x86/miktex-omega-bin.tpm.in
    windows-x86/miktex-pdftex-bin.tpm.in
    windows-x86/miktex-png-bin.tpm.in
    windows-x86/miktex-poppler-bin.tpm.in
    windows-x86/miktex-popt-bin.tpm.in
    windows-x86/miktex-posix-bin.tpm.in
    windows-x86/miktex-ps2pk-bin.tpm.in
    windows-x86/miktex-psutils-bin.tpm.in
    windows-x86/miktex-runtime-bin.tpm.in
    windows-x86/miktex-tdsutil-bin.tpm.in
    windows-x86/miktex-teckit-bin.tpm.in
    windows-x86/miktex-tex-bin.tpm.in
    windows-x86/miktex-tex2xindy-bin.tpm.in
    windows-x86/miktex-tex4ht-bin.tpm.in
    windows-x86/miktex-texify-bin.tpm.in
    windows-x86/miktex-texware-bin.tpm.in
    windows-x86/miktex-texworks-bin.tpm.in
    windows-x86/miktex-ttf2pk2-bin.tpm.in
    windows-x86/miktex-uriparser-bin.tpm.in
    windows-x86/miktex-web-bin.tpm.in
    windows-x86/miktex-xetex-bin.tpm.in
    windows-x86/miktex-yap-bin.tpm.in
    windows-x86/miktex-zlib-bin.tpm.in
    windows-x86/miktex-zzip-bin.tpm.in
  )
  if(WITH_CONTEXT_SUPPORT)
    list(APPEND target_system_tpms
      windows-x86/miktex-context-bin.tpm.in
    )
  endif()

elseif(${MIKTEX_TARGET_SYSTEM_TAG} STREQUAL "windows-x64")
  set(target_system_tpms
    windows-x64/miktex-arctrl-bin-x64.tpm.in
    windows-x64/miktex-bibtex-bin-x64.tpm.in
    windows-x64/miktex-bibtex8bit-bin-x64.tpm.in
    windows-x64/miktex-bin-x64.tpm.in
    windows-x64/miktex-bzip2-bin-x64.tpm.in
    windows-x64/miktex-cairo-bin-x64.tpm.in
    windows-x64/miktex-chktex-bin-x64.tpm.in
    windows-x64/miktex-cjkutils-bin-x64.tpm.in
    windows-x64/miktex-curl-bin-x64.tpm.in
    windows-x64/miktex-cweb-bin-x64.tpm.in
    windows-x64/miktex-devnag-bin-x64.tpm.in
    windows-x64/miktex-dvicopy-bin-x64.tpm.in
    windows-x64/miktex-dvipdfmx-bin-x64.tpm.in
    windows-x64/miktex-dvipng-bin-x64.tpm.in
    windows-x64/miktex-dvips-bin-x64.tpm.in
    windows-x64/miktex-dvisvgm-bin-x64.tpm.in
    windows-x64/miktex-epstopdf-bin-x64.tpm.in
    windows-x64/miktex-expat-bin-x64.tpm.in
    windows-x64/miktex-findtexmf-bin-x64.tpm.in
    windows-x64/miktex-fontconfig-bin-x64.tpm.in
    windows-x64/miktex-fonts-bin-x64.tpm.in
    windows-x64/miktex-freetype2-bin-x64.tpm.in
    windows-x86/miktex-fribidixetex-bin-x64.tpm.in
    windows-x64/miktex-gd-bin-x64.tpm.in
    windows-x64/miktex-graphite2-bin-x64.tpm.in
    windows-x64/miktex-gsf2pk-bin-x64.tpm.in
    windows-x64/miktex-hunspell-bin-x64.tpm.in
    windows-x64/miktex-int-bin-x64.tpm.in
    windows-x64/miktex-jpeg-bin-x64.tpm.in
    windows-x64/miktex-kpathsea-bin-x64.tpm.in
    windows-x64/miktex-libressl-bin-x64.tpm.in
    windows-x64/miktex-log4cxx-bin-x64.tpm.in
    windows-x64/miktex-lua52-bin-x64.tpm.in
    windows-x64/miktex-luatex-bin-x64.tpm.in
    windows-x64/miktex-lzma-bin-x64.tpm.in
    windows-x64/miktex-makeindex-bin-x64.tpm.in
    windows-x64/miktex-md5-bin-x64.tpm.in
    windows-x64/miktex-metafont-bin-x64.tpm.in
    windows-x64/miktex-metapost-bin-x64.tpm.in
    windows-x64/miktex-mfware-bin-x64.tpm.in
    windows-x64/miktex-mkfntmap-bin-x64.tpm.in
    windows-x64/miktex-mktex-bin-x64.tpm.in
    windows-x64/miktex-mo-bin-x64.tpm.in
    windows-x64/miktex-mpm-bin-x64.tpm.in
    windows-x64/miktex-mpmcli-bin-x64.tpm.in
    windows-x64/miktex-mspack-bin-x64.tpm.in
    windows-x64/miktex-mthelp-bin-x64.tpm.in
    windows-x64/miktex-mtprint-bin-x64.tpm.in
    windows-x64/miktex-omega-bin-x64.tpm.in
    windows-x64/miktex-pdftex-bin-x64.tpm.in
    windows-x64/miktex-png-bin-x64.tpm.in
    windows-x64/miktex-poppler-bin-x64.tpm.in
    windows-x64/miktex-popt-bin-x64.tpm.in
    windows-x64/miktex-posix-bin-x64.tpm.in
    windows-x64/miktex-ps2pk-bin-x64.tpm.in
    windows-x64/miktex-psutils-bin-x64.tpm.in
    windows-x64/miktex-runtime-bin-x64.tpm.in
    windows-x64/miktex-tdsutil-bin-x64.tpm.in
    windows-x64/miktex-teckit-bin-x64.tpm.in
    windows-x64/miktex-tex-bin-x64.tpm.in
    windows-x64/miktex-tex2xindy-bin-x64.tpm.in
    windows-x64/miktex-tex4ht-bin-x64.tpm.in
    windows-x64/miktex-texify-bin-x64.tpm.in
    windows-x64/miktex-texware-bin-x64.tpm.in
    windows-x64/miktex-texworks-bin-x64.tpm.in
    windows-x64/miktex-ttf2pk2-bin-x64.tpm.in
    windows-x64/miktex-uriparser-bin-x64.tpm.in
    windows-x64/miktex-web-bin-x64.tpm.in
    windows-x64/miktex-xetex-bin-x64.tpm.in
    windows-x64/miktex-yap-bin-x64.tpm.in
    windows-x64/miktex-zlib-bin-x64.tpm.in
    windows-x64/miktex-zzip-bin-x64.tpm.in
  )
  if(WITH_CONTEXT_SUPPORT)
    list(APPEND target_system_tpms
      windows-x64/miktex-context-bin-x64.tpm.in
    )
  endif()
endif()

foreach(tpm ${target_system_tpms})
  get_filename_component(fn ${tpm} NAME_WE)
  configure_file(
    ${tpm}
    "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${MIKTEX_TARGET_SYSTEM_TAG}/${fn}-${MIKTEX_SERIES_STR}.tpm"
  )
  list(APPEND tpms
    "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${MIKTEX_TARGET_SYSTEM_TAG}/${fn}-${MIKTEX_SERIES_STR}.tpm")
endforeach()

install(
  FILES ${tpms}
  DESTINATION ${MIKTEX_TPM_DESTINATION_DIR}
)
