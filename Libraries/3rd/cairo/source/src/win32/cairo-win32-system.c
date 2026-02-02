/* -*- Mode: c; tab-width: 8; c-basic-offset: 4; indent-tabs-mode: t; -*- */
/* Cairo - a vector graphics library with display and print output
 *
 * Copyright © 2005 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Red Hat, Inc.
 *
 * Contributor(s):
 *	Owen Taylor <otaylor@redhat.com>
 *	Stuart Parmenter <stuart@mozilla.com>
 *	Vladimir Vukicevic <vladimir@pobox.com>
 */

/* This file should include code that is system-specific, not
 * feature-specific.  For example, the DLL initialization/finalization
 * code on Win32 or OS/2 must live here (not in cairo-whatever-surface.c).
 * Same about possible ELF-specific code.
 *
 * And no other function should live here.
 */

#include "cairoint.h"

#include <windows.h>

/**
 * _cairo_win32_print_api_error:
 * @context: context string to display along with the error
 * @api: name of the failing api
 *
 * Helper function to dump out a human readable form of the
 * current error code.
 *
 * Return value: A cairo status code for the error code
 **/
cairo_status_t
_cairo_win32_print_api_error (const char *context, const char *api)
{
    const DWORD lang_id = MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT);
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_IGNORE_INSERTS |
                        FORMAT_MESSAGE_FROM_SYSTEM;
    const DWORD last_error = GetLastError ();
    void *lpMsgBuf = NULL;

    if (!FormatMessageW (flags, NULL, last_error, lang_id, (LPWSTR) &lpMsgBuf, 0, NULL)) {
       fprintf (stderr, "%s: %s failed with error code %lu\n", context, api, last_error);
    }
    else {
       fprintf (stderr, "%s: %s failed - %S\n", context, api, (wchar_t *)lpMsgBuf);
       LocalFree (lpMsgBuf);
    }

    return _cairo_error (CAIRO_STATUS_WIN32_GDI_ERROR);
}

/**
 * _cairo_win32_load_library_from_system32:
 * @name: name of the module to load from System32
 *
 * Helper function to load system modules in the System32
 * folder.
 *
 * Return value: An module HANDLE, NULL on error.
 **/
HMODULE
_cairo_win32_load_library_from_system32 (const wchar_t *name)
{
    HMODULE module_handle;

    module_handle = LoadLibraryExW (name, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (module_handle == NULL) {
        DWORD code = GetLastError();
        if (code == ERROR_INVALID_PARAMETER) {
            /* Support for flag LOAD_LIBRARY_SEARCH_SYSTEM32 was backported
             * to Windows Vista / 7 with Update KB2533623. If the flag is
             * not supported, simply use LoadLibrary */
            return LoadLibraryW (name);
        }
    }

    return module_handle;
}

#if CAIRO_MUTEX_IMPL_WIN32

static void NTAPI
cairo_win32_tls_callback (PVOID hinstance, DWORD dwReason, PVOID lpvReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            CAIRO_MUTEX_INITIALIZE ();
            break;

        case DLL_PROCESS_DETACH:
            if (lpvReserved == NULL) {
                CAIRO_MUTEX_FINALIZE ();
            }
            break;
    }
}

#ifdef _MSC_VER

#ifdef _M_IX86
# define SYMBOL_PREFIX "_"
#else
# define SYMBOL_PREFIX ""
#endif

#ifdef __cplusplus
# define EXTERN_C_BEGIN extern "C" {
# define EXTERN_C_END }
# define EXTERN_CONST extern const
#else
# define EXTERN_C_BEGIN
# define EXTERN_C_END
# define EXTERN_CONST const
#endif

#define DEFINE_TLS_CALLBACK(func) \
__pragma (section (".CRT$XLD", long, read))                          \
                                                                     \
static void NTAPI func (PVOID, DWORD, PVOID);                        \
                                                                     \
EXTERN_C_BEGIN                                                       \
__declspec (allocate (".CRT$XLD"))                                   \
EXTERN_CONST PIMAGE_TLS_CALLBACK _ptr_##func = func;                 \
EXTERN_C_END                                                         \
                                                                     \
__pragma (comment (linker, "/INCLUDE:" SYMBOL_PREFIX "_tls_used"))   \
__pragma (comment (linker, "/INCLUDE:" SYMBOL_PREFIX "_ptr_" #func))

#else /* _MSC_VER */

#define DEFINE_TLS_CALLBACK(func) \
static void NTAPI func (PVOID, DWORD, PVOID);        \
                                                     \
__attribute__ ((used, section (".CRT$XLD")))         \
static const PIMAGE_TLS_CALLBACK _ptr_##func = func;


#endif /* !_MSC_VER */

DEFINE_TLS_CALLBACK (cairo_win32_tls_callback);

#endif /* CAIRO_MUTEX_IMPL_WIN32 */
