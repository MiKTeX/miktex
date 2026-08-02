## CMakeLists.txt
##
## Copyright (C) 2015-2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_ADMIN_FOLDER}/TPM")

set(common_tpms
    miktex-config.tpm.in
)

set(common_tpms_no_version
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
        "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${fn}-${MIKTEX_LEGACY_MAJOR_MINOR_STR}.tpm"
    )
    list(APPEND tpms "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${fn}-${MIKTEX_LEGACY_MAJOR_MINOR_STR}.tpm")
endforeach()

foreach(tpm ${common_tpms_no_version})
    get_filename_component(fn ${tpm} NAME_WE)
    configure_file(
        ${tpm}
        "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${fn}.tpm"
    )
    list(APPEND tpms "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${fn}.tpm")
endforeach()

if(${MIKTEX_TARGET_SYSTEM_NAME} STREQUAL "windows")
    set(target_system_tpms
        windows/miktex-arctrl-bin.tpm.in
        windows/miktex-asymptote-bin.tpm.in
        windows/miktex-autosp-bin.tpm.in
        windows/miktex-axohelp-bin.tpm.in
        windows/miktex-bibarts-bin.tpm.in
        windows/miktex-bibtex-bin.tpm.in
        windows/miktex-bibtex8bit-bin.tpm.in
        windows/miktex-bzip2-bin.tpm.in
        windows/miktex-cairo-bin.tpm.in
        windows/miktex-chktex-bin.tpm.in
        windows/miktex-cjkutils-bin.tpm.in
        windows/miktex-console-bin.tpm.in
        windows/miktex-curl-bin.tpm.in
        windows/miktex-cweb-bin.tpm.in
        windows/miktex-devnag-bin.tpm.in
        windows/miktex-dvicopy-bin.tpm.in
        windows/miktex-dvipdfmx-bin.tpm.in
        windows/miktex-dvipng-bin.tpm.in
        windows/miktex-dvips-bin.tpm.in
        windows/miktex-dvisvgm-bin.tpm.in
        windows/miktex-epstopdf-bin.tpm.in
        windows/miktex-expat-bin.tpm.in
        windows/miktex-findtexmf-bin.tpm.in
        windows/miktex-fmt-bin.tpm.in
        windows/miktex-fontconfig-bin.tpm.in
        windows/miktex-fonts-bin.tpm.in
        windows/miktex-freeglut-bin.tpm.in
        windows/miktex-freetype2-bin.tpm.in
        windows/miktex-fribidixetex-bin.tpm.in
        windows/miktex-gd-bin.tpm.in
        windows/miktex-graphite2-bin.tpm.in
        windows/miktex-gregorio-bin.tpm.in
        windows/miktex-gsf2pk-bin.tpm.in
        windows/miktex-harfbuzz-bin.tpm.in
        windows/miktex-hunspell-bin.tpm.in
        windows/miktex-jpeg-bin.tpm.in
        windows/miktex-kpathsea-bin.tpm.in
        windows/miktex-lacheck-bin.tpm.in
        windows/miktex-lcdf-typetools-bin.tpm.in
        windows/miktex-libressl-bin.tpm.in
        windows/miktex-log4cxx-bin.tpm.in
        windows/miktex-lua53-bin.tpm.in
        windows/miktex-luatex-bin.tpm.in
        windows/miktex-lzma-bin.tpm.in
        windows/miktex-m-tx-bin.tpm.in
        windows/miktex-makeindex-bin.tpm.in
        windows/miktex-md5-bin.tpm.in
        windows/miktex-metafont-bin.tpm.in
        windows/miktex-metapost-bin.tpm.in
        windows/miktex-mfware-bin.tpm.in
        windows/miktex-mktex-bin.tpm.in
        windows/miktex-mo-bin.tpm.in
        windows/miktex-mpmcli-bin.tpm.in
        windows/miktex-mspack-bin.tpm.in
        windows/miktex-mthelp-bin.tpm.in
        windows/miktex-mtprint-bin.tpm.in
        windows/miktex-omegaware-bin.tpm.in
        windows/miktex-pdftex-bin.tpm.in
        windows/miktex-pixman-bin.tpm.in
        windows/miktex-pmx-bin.tpm.in
        windows/miktex-png-bin.tpm.in
        windows/miktex-poppler-bin.tpm.in
        windows/miktex-popt-bin.tpm.in
        windows/miktex-posix-bin.tpm.in
        windows/miktex-ps2pk-bin.tpm.in
        windows/miktex-psutils-bin.tpm.in
        windows/miktex-runtime-bin.tpm.in
        windows/miktex-synctex-bin.tpm.in
        windows/miktex-teckit-bin.tpm.in
        windows/miktex-tex-bin.tpm.in
        windows/miktex-tex2xindy-bin.tpm.in
        windows/miktex-tex4ht-bin.tpm.in
        windows/miktex-texify-bin.tpm.in
        windows/miktex-texware-bin.tpm.in
        windows/miktex-texworks-bin.tpm.in
        windows/miktex-ttf2pk2-bin.tpm.in
        windows/miktex-uriparser-bin.tpm.in
        windows/miktex-web-bin.tpm.in
        windows/miktex-xetex-bin.tpm.in
        windows/miktex-yap-bin.tpm.in
        windows/miktex-zlib-bin.tpm.in
        windows/miktex-zzip-bin.tpm.in
    )
    set(target_system_tpms_no_version
        windows/miktex-hitex-bin.tpm.in
        windows/miktex-patgen-bin.tpm.in
        windows/miktex-ptex-bin.tpm.in
        windows/miktex-tdsutil-bin.tpm.in
        windows/miktex-upmendex-bin.tpm.in
        windows/miktex-xml2pmx-bin.tpm.in
    )
    if(WITH_CONTEXT_SUPPORT)
        list(APPEND target_system_tpms
            windows/miktex-context-bin.tpm.in
        )
    endif()
endif()

foreach(tpm ${target_system_tpms})
    get_filename_component(fn ${tpm} NAME_WE)
    configure_file(
        ${tpm}
        "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${MIKTEX_TARGET_SYSTEM_TAG}/${fn}-${MIKTEX_LEGACY_MAJOR_MINOR_STR}.tpm"
    )
    list(APPEND tpms
        "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${MIKTEX_TARGET_SYSTEM_TAG}/${fn}-${MIKTEX_LEGACY_MAJOR_MINOR_STR}.tpm"
    )
endforeach()

foreach(tpm ${target_system_tpms_no_version})
    get_filename_component(fn ${tpm} NAME_WE)
    configure_file(
        ${tpm}
        "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${MIKTEX_TARGET_SYSTEM_TAG}/${fn}.tpm"
    )
    list(APPEND tpms
        "${CMAKE_BINARY_DIR}/${MIKTEX_REL_TPM_DIR}/${MIKTEX_TARGET_SYSTEM_TAG}/${fn}.tpm"
    )
endforeach()

install(
    FILES ${tpms}
    DESTINATION ${MIKTEX_TPM_DESTINATION_DIR}
)
