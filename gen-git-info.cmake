## gen-git-info.cmake
##
## Copyright (C) 2020-2022 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

if("$Format:%%$" STREQUAL "%")
    set(MIKTEX_HAVE_GIT_INFO 1)
    set(MIKTEX_GIT_COMMIT "$Format:%H$")
    set(MIKTEX_GIT_COMMIT_ABBREV "$Format:%h$")
    set(MIKTEX_GIT_AUTHOR_DATE $Format:%at$)
elseif(IS_DIRECTORY "${SOURCE_DIR}/.git" AND GIT_FOUND)
    set(MIKTEX_HAVE_GIT_INFO 1)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} show --no-patch --pretty=%H
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE MIKTEX_GIT_COMMIT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} show --no-patch --pretty=%h
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE MIKTEX_GIT_COMMIT_ABBREV
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} show --no-patch --pretty=%at
        WORKING_DIRECTORY "${SOURCE_DIR}"
        OUTPUT_VARIABLE MIKTEX_GIT_AUTHOR_DATE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(MIKTEX_HAVE_GIT_INFO 0)
endif()

configure_file(
    ${GITINFO_H_IN}
    ${GITINFO_H}
)
