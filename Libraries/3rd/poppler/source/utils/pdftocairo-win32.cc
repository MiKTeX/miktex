//========================================================================
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2014 Rodrigo Rivas Costa <rodrigorivascosta@gmail.com>
// Copyright (C) 2014, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2017, 2018, 2022 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <cairo.h>
#ifdef CAIRO_HAS_WIN32_SURFACE

#    include <cairo-win32.h>

#    include "parseargs.h"
#    include "pdftocairo-win32.h"
#    include "Win32Console.h"

#    include <dlgs.h>
#    include <commctrl.h>
#    include <commdlg.h>
#    include <windowsx.h>
#    include <winspool.h>

static HDC hdc;
static HGLOBAL hDevmode = nullptr;
static HGLOBAL hDevnames = nullptr;
static DEVMODEA *devmode;
static char *printerName;

struct Win32Option
{
    const char *name;
    int value;
};

static const Win32Option win32PaperSource[] = { { "upper", DMBIN_UPPER },           { "onlyone", DMBIN_ONLYONE },
                                                { "lower", DMBIN_LOWER },           { "middle", DMBIN_MIDDLE },
                                                { "manual", DMBIN_MANUAL },         { "envelope", DMBIN_ENVELOPE },
                                                { "envmanual", DMBIN_ENVMANUAL },   { "auto", DMBIN_AUTO },
                                                { "tractor", DMBIN_TRACTOR },       { "smallfmt", DMBIN_SMALLFMT },
                                                { "largefmt", DMBIN_LARGEFMT },     { "largecapacity", DMBIN_LARGECAPACITY },
                                                { "formsource", DMBIN_FORMSOURCE }, { nullptr, 0 } };

static void parseSource(GooString *source)
{
    const Win32Option *option = win32PaperSource;
    while (option->name) {
        if (source->cmp(option->name) == 0) {
            devmode->dmDefaultSource = option->value;
            devmode->dmFields |= DM_DEFAULTSOURCE;
            return;
        }
        option++;
    }
    fprintf(stderr, "Warning: Unknown paper source \"%s\"\n", source->c_str());
}

static const Win32Option win32DuplexMode[] = { { "off", DMDUP_SIMPLEX }, { "short", DMDUP_HORIZONTAL }, { "long", DMDUP_VERTICAL }, { nullptr, 0 } };

static void parseDuplex(GooString *mode)
{
    const Win32Option *option = win32DuplexMode;
    while (option->name) {
        if (mode->cmp(option->name) == 0) {
            devmode->dmDuplex = option->value;
            devmode->dmFields |= DM_DUPLEX;
            return;
        }
        option++;
    }
    fprintf(stderr, "Warning: Unknown duplex mode \"%s\"\n", mode->c_str());
}

static void fillCommonPrinterOptions(bool duplex)
{
    if (duplex) {
        devmode->dmDuplex = DMDUP_HORIZONTAL;
        devmode->dmFields |= DM_DUPLEX;
    }
}

static void fillPagePrinterOptions(double w, double h)
{
    w *= 254.0 / 72.0; // units are 0.1mm
    h *= 254.0 / 72.0;
    if (w > h) {
        devmode->dmOrientation = DMORIENT_LANDSCAPE;
        devmode->dmPaperWidth = static_cast<short>(h);
        devmode->dmPaperLength = static_cast<short>(w);
    } else {
        devmode->dmOrientation = DMORIENT_PORTRAIT;
        devmode->dmPaperWidth = static_cast<short>(w);
        devmode->dmPaperLength = static_cast<short>(h);
    }
    devmode->dmPaperSize = 0;
    devmode->dmFields |= DM_ORIENTATION | DM_PAPERWIDTH | DM_PAPERLENGTH;
}

static void fillPrinterOptions(bool duplex, GooString *printOpt)
{
    // printOpt format is: <opt1>=<val1>,<opt2>=<val2>,...
    const char *nextOpt = printOpt->c_str();
    while (nextOpt && *nextOpt) {
        const char *comma = strchr(nextOpt, ',');
        GooString opt;
        if (comma) {
            opt.Set(nextOpt, static_cast<int>(comma - nextOpt));
            nextOpt = comma + 1;
        } else {
            opt.Set(nextOpt);
            nextOpt = NULL;
        }
        // here opt is "<optN>=<valN> "
        const char *equal = strchr(opt.c_str(), '=');
        if (!equal) {
            fprintf(stderr, "Warning: unknown printer option \"%s\"\n", opt.c_str());
            continue;
        }
        const int iequal = static_cast<int>(equal - opt.c_str());
        GooString value(&opt, iequal + 1, opt.getLength() - iequal - 1);
        opt.del(iequal, opt.getLength() - iequal);
        // here opt is "<optN>" and value is "<valN>"

        if (opt.cmp("source") == 0) {
            parseSource(&value);
        } else if (opt.cmp("duplex") == 0) {
            if (duplex)
                fprintf(stderr, "Warning: duplex mode is specified both as standalone and printer options\n");
            else
                parseDuplex(&value);
        } else {
            fprintf(stderr, "Warning: unknown printer option \"%s\"\n", opt.c_str());
        }
    }
}

static void getLocalPos(HWND wind, HWND dlg, RECT *rect)
{
    GetClientRect(wind, rect);
    MapWindowPoints(wind, dlg, (LPPOINT)rect, 2);
}

static HWND createGroupBox(HWND parent, HINSTANCE hInstance, HMENU id, const char *label, RECT *rect)
{
    HWND hwnd = CreateWindowA(WC_BUTTONA, label, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, parent, id, hInstance, NULL);
    HFONT hFont = (HFONT)SendMessage(parent, WM_GETFONT, (WPARAM)0, (LPARAM)0);
    if (hFont)
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)0);
    return hwnd;
}

static HWND createCheckBox(HWND parent, HINSTANCE hInstance, HMENU id, const char *label, RECT *rect)
{
    HWND hwnd = CreateWindowA(WC_BUTTONA, label, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, parent, id, hInstance, NULL);
    HFONT hFont = (HFONT)SendMessage(parent, WM_GETFONT, (WPARAM)0, (LPARAM)0);
    if (hFont)
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)0);
    return hwnd;
}

static HWND createStaticText(HWND parent, HINSTANCE hinstance, HMENU id, const char *text, RECT *rect)
{
    HWND hwnd = CreateWindowA(WC_STATICA, text, WS_CHILD | WS_VISIBLE | SS_LEFT, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, parent, id, hinstance, NULL);
    HFONT hFont = (HFONT)SendMessage(parent, WM_GETFONT, (WPARAM)0, (LPARAM)0);
    if (hFont)
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)0);
    return hwnd;
}

static HWND createPageScaleComboBox(HWND parent, HINSTANCE hinstance, HMENU id, RECT *rect)
{
    HWND hwnd = CreateWindowA(WC_COMBOBOX, "", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP | CBS_DROPDOWNLIST, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top, parent, id, hinstance, NULL);
    HFONT hFont = (HFONT)SendMessage(parent, WM_GETFONT, (WPARAM)0, (LPARAM)0);
    if (hFont)
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)0);
    ComboBox_AddString(hwnd, "None");
    ComboBox_AddString(hwnd, "Shrink to Printable Area");
    ComboBox_AddString(hwnd, "Fit to Printable Area");
    return hwnd;
}

enum PageScale
{
    NONE = 0,
    SHRINK = 1,
    FIT = 2
};

// used to set/get option values in printDialogHookProc
static PageScale pageScale;
static bool centerPage;
static bool useOrigPageSize;

// PrintDlg callback to customize the print dialog with additional controls.
static UINT_PTR CALLBACK printDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    if (uiMsg == WM_INITDIALOG) {
        // Get the extant controls. See dlgs.h and prnsetup.dlg for the PrintDlg control ids.
        HWND printerGroupWind = GetDlgItem(hdlg, grp4);
        HWND printerComboWind = GetDlgItem(hdlg, cmb4);
        HWND nameLabelWind = GetDlgItem(hdlg, stc6);
        HWND statusLabelWind = GetDlgItem(hdlg, stc8);
        HWND printRangeGroupWind = GetDlgItem(hdlg, grp1);
        HWND radio1Wind = GetDlgItem(hdlg, rad1);
        HWND radio2Wind = GetDlgItem(hdlg, rad3);
        HWND copiesGroupWind = GetDlgItem(hdlg, grp2);
        HWND okWind = GetDlgItem(hdlg, IDOK);
        HWND cancelWind = GetDlgItem(hdlg, IDCANCEL);
        if (!printerGroupWind || !printerComboWind || !nameLabelWind || !statusLabelWind || !printRangeGroupWind || !radio1Wind || !radio2Wind || !copiesGroupWind || !okWind || !cancelWind)
            return 0;

        // Get the size and position of the above controls relative to the
        // print dialog window
        RECT printerGroupRect;
        RECT printerComboRect;
        RECT nameLabelRect;
        RECT statusLabelRect;
        RECT printRangeGroupRect;
        RECT radio1Rect, radio2Rect;
        RECT copiesGroupRect;
        RECT okRect, cancelRect;
        getLocalPos(printerGroupWind, hdlg, &printerGroupRect);
        getLocalPos(printerComboWind, hdlg, &printerComboRect);
        getLocalPos(nameLabelWind, hdlg, &nameLabelRect);
        getLocalPos(statusLabelWind, hdlg, &statusLabelRect);
        getLocalPos(printRangeGroupWind, hdlg, &printRangeGroupRect);
        getLocalPos(radio1Wind, hdlg, &radio1Rect);
        getLocalPos(radio2Wind, hdlg, &radio2Rect);
        getLocalPos(copiesGroupWind, hdlg, &copiesGroupRect);
        getLocalPos(okWind, hdlg, &okRect);
        getLocalPos(cancelWind, hdlg, &cancelRect);

        // Calc space required for new group
        int interGroupSpace = printRangeGroupRect.top - printerGroupRect.bottom;
        int groupHeight = statusLabelRect.top - printerGroupRect.top + printRangeGroupRect.bottom - radio1Rect.bottom;

        // Increase dialog size
        RECT dlgRect;
        GetWindowRect(hdlg, &dlgRect);
        SetWindowPos(hdlg, nullptr, dlgRect.left, dlgRect.top, dlgRect.right - dlgRect.left, dlgRect.bottom - dlgRect.top + interGroupSpace + groupHeight, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);

        // Add new group and controls
        HINSTANCE hinstance = (HINSTANCE)GetWindowLongPtr(hdlg, GWLP_HINSTANCE);
        RECT pdfGroupBoxRect;
        pdfGroupBoxRect.left = printRangeGroupRect.left;
        pdfGroupBoxRect.right = copiesGroupRect.right;
        pdfGroupBoxRect.top = printRangeGroupRect.bottom + interGroupSpace;
        pdfGroupBoxRect.bottom = pdfGroupBoxRect.top + groupHeight;
        createGroupBox(hdlg, hinstance, (HMENU)grp3, "PDF Print Options", &pdfGroupBoxRect);

        RECT textRect;
        textRect.left = nameLabelRect.left;
        textRect.right = static_cast<LONG>(nameLabelRect.left + 1.8 * (printerComboRect.left - nameLabelRect.left));
        textRect.top = pdfGroupBoxRect.top + nameLabelRect.top - printerGroupRect.top;
        textRect.bottom = textRect.top + nameLabelRect.bottom - nameLabelRect.top;
        createStaticText(hdlg, hinstance, (HMENU)stc1, "Page Scaling:", &textRect);

        RECT comboBoxRect;
        comboBoxRect.left = textRect.right;
        comboBoxRect.right = comboBoxRect.left + printerComboRect.right - printerComboRect.left;
        ;
        comboBoxRect.top = pdfGroupBoxRect.top + printerComboRect.top - printerGroupRect.top;
        comboBoxRect.bottom = textRect.top + 4 * (printerComboRect.bottom - printerComboRect.top);
        HWND comboBoxWind = createPageScaleComboBox(hdlg, hinstance, (HMENU)cmb1, &comboBoxRect);

        RECT checkBox1Rect;
        checkBox1Rect.left = radio1Rect.left;
        checkBox1Rect.right = pdfGroupBoxRect.right - 10;
        checkBox1Rect.top = pdfGroupBoxRect.top + statusLabelRect.top - printerGroupRect.top;
        checkBox1Rect.bottom = checkBox1Rect.top + radio1Rect.bottom - radio1Rect.top;
        HWND checkBox1Wind = createCheckBox(hdlg, hinstance, (HMENU)chx3, "Center", &checkBox1Rect);

        RECT checkBox2Rect;
        checkBox2Rect.left = radio1Rect.left;
        checkBox2Rect.right = pdfGroupBoxRect.right - 10;
        checkBox2Rect.top = checkBox1Rect.top + radio2Rect.top - radio1Rect.top;
        checkBox2Rect.bottom = checkBox2Rect.top + radio1Rect.bottom - radio1Rect.top;
        HWND checkBox2Wind = createCheckBox(hdlg, hinstance, (HMENU)chx4, "Select page size using document page size", &checkBox2Rect);

        // Move OK and Cancel buttons down ensuring they are last in the Z order
        // so that the tab order is correct.
        SetWindowPos(okWind, HWND_BOTTOM, okRect.left, okRect.top + interGroupSpace + groupHeight, 0, 0,
                     SWP_NOSIZE); // keep current size
        SetWindowPos(cancelWind, HWND_BOTTOM, cancelRect.left, cancelRect.top + interGroupSpace + groupHeight, 0, 0,
                     SWP_NOSIZE); // keep current size

        // Initialize control values
        ComboBox_SetCurSel(comboBoxWind, pageScale);
        Button_SetCheck(checkBox1Wind, centerPage ? BST_CHECKED : BST_UNCHECKED);
        Button_SetCheck(checkBox2Wind, useOrigPageSize ? BST_CHECKED : BST_UNCHECKED);

    } else if (uiMsg == WM_COMMAND) {
        // Save settings
        UINT id = LOWORD(wParam);
        if (id == cmb1)
            pageScale = (PageScale)ComboBox_GetCurSel(GetDlgItem(hdlg, cmb1));
        if (id == chx3)
            centerPage = IsDlgButtonChecked(hdlg, chx3);
        if (id == chx4)
            useOrigPageSize = IsDlgButtonChecked(hdlg, chx4);
    }
    return 0;
}

void win32SetupPrinter(GooString *printer, GooString *printOpt, bool duplex, bool setupdlg)
{
    if (printer->c_str()[0] == 0) {
        DWORD size = 0;
        GetDefaultPrinterA(nullptr, &size);
        printerName = (char *)gmalloc(size);
        GetDefaultPrinterA(printerName, &size);
    } else {
        printerName = copyString(printer->c_str(), printer->getLength());
    }

    // Query the size of the DEVMODE struct
    LONG szProp = DocumentPropertiesA(nullptr, nullptr, printerName, nullptr, nullptr, 0);
    if (szProp < 0) {
        fprintf(stderr, "Error: Printer \"%s\" not found\n", printerName);
        exit(99);
    }
    devmode = (DEVMODEA *)gmalloc(szProp);
    memset(devmode, 0, szProp);
    devmode->dmSize = sizeof(DEVMODEA);
    devmode->dmSpecVersion = DM_SPECVERSION;
    // Load the current default configuration for the printer into devmode
    if (DocumentPropertiesA(nullptr, nullptr, printerName, devmode, devmode, DM_OUT_BUFFER) < 0) {
        fprintf(stderr, "Error: Printer \"%s\" not found\n", printerName);
        exit(99);
    }

    // Update devmode with selected print options
    fillCommonPrinterOptions(duplex);
    fillPrinterOptions(duplex, printOpt);

    // Call DocumentProperties again so the driver can update its private data
    // with the modified print options. This will also display the printer
    // properties dialog if setupdlg is true.
    int ret;
    DWORD mode = DM_IN_BUFFER | DM_OUT_BUFFER;
    if (setupdlg)
        mode |= DM_IN_PROMPT;
    ret = DocumentPropertiesA(nullptr, nullptr, printerName, devmode, devmode, mode);
    if (ret < 0) {
        fprintf(stderr, "Error: Printer \"%s\" not found\n", printerName);
        exit(99);
    }
    if (setupdlg && ret == IDCANCEL)
        exit(0);

    hdc = CreateDCA(nullptr, printerName, nullptr, devmode);
    if (!hdc) {
        fprintf(stderr, "Error: Printer \"%s\" not found\n", printerName);
        exit(99);
    }
}

void win32ShowPrintDialog(bool *expand, bool *noShrink, bool *noCenter, bool *usePDFPageSize, bool *allPages, int *firstPage, int *lastPage, int maxPages)
{
    PRINTDLG pd;
    memset(&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize = sizeof(PRINTDLG);
    pd.Flags = PD_NOSELECTION | PD_ENABLEPRINTHOOK | PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDC;
    if (*allPages) {
        pd.nFromPage = 1;
        pd.nToPage = maxPages;
    } else {
        pd.Flags |= PD_PAGENUMS;
        pd.nFromPage = *firstPage;
        pd.nToPage = *lastPage;
    }
    pd.nCopies = 1;
    pd.nMinPage = 1;
    pd.nMaxPage = maxPages;
    pd.lpfnPrintHook = printDialogHookProc;
    if (!*expand && *noShrink)
        pageScale = NONE;
    else if (!*expand && !*noShrink)
        pageScale = SHRINK;
    else
        pageScale = FIT;
    centerPage = !*noCenter;
    useOrigPageSize = *usePDFPageSize;

    if (PrintDlgA(&pd)) {
        // Ok
        hDevnames = pd.hDevNames;
        DEVNAMES *devnames = (DEVNAMES *)GlobalLock(hDevnames);
        printerName = (char *)devnames + devnames->wDeviceOffset;
        hDevmode = pd.hDevMode;
        devmode = (DEVMODEA *)GlobalLock(hDevmode);
        hdc = pd.hDC;
        if (pd.Flags & PD_PAGENUMS) {
            *allPages = false;
            *firstPage = pd.nFromPage;
            *lastPage = pd.nToPage;
        } else {
            *allPages = true;
        }
        if (pageScale == NONE) {
            *expand = false;
            *noShrink = true;
        } else if (pageScale == SHRINK) {
            *expand = false;
            *noShrink = false;
        } else {
            *expand = true;
            *noShrink = false;
        }
        *noCenter = !centerPage;
        *usePDFPageSize = useOrigPageSize;
    } else {
        // Cancel
        exit(0);
    }
}

cairo_surface_t *win32BeginDocument(GooString *inputFileName, GooString *outputFileName)
{
    DOCINFOA docinfo;
    memset(&docinfo, 0, sizeof(docinfo));
    docinfo.cbSize = sizeof(docinfo);
    if (inputFileName->cmp("fd://0") == 0)
        docinfo.lpszDocName = "pdftocairo <stdin>";
    else
        docinfo.lpszDocName = inputFileName->c_str();
    if (outputFileName)
        docinfo.lpszOutput = outputFileName->c_str();
    if (StartDocA(hdc, &docinfo) <= 0) {
        fprintf(stderr, "Error: StartDoc failed\n");
        exit(99);
    }

    return cairo_win32_printing_surface_create(hdc);
}

void win32BeginPage(double *w, double *h, bool changePageSize, bool useFullPage)
{
    if (changePageSize)
        fillPagePrinterOptions(*w, *h);
    if (DocumentPropertiesA(nullptr, nullptr, printerName, devmode, devmode, DM_IN_BUFFER | DM_OUT_BUFFER) < 0) {
        fprintf(stderr, "Error: Printer \"%s\" not found\n", printerName);
        exit(99);
    }
    ResetDCA(hdc, devmode);

    // Get actual paper size or if useFullPage is false the printable area.
    // Transform the hdc scale to points to be consistent with other cairo backends
    int x_dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    int y_dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    int x_off = GetDeviceCaps(hdc, PHYSICALOFFSETX);
    int y_off = GetDeviceCaps(hdc, PHYSICALOFFSETY);
    if (useFullPage) {
        *w = GetDeviceCaps(hdc, PHYSICALWIDTH) * 72.0 / x_dpi;
        *h = GetDeviceCaps(hdc, PHYSICALHEIGHT) * 72.0 / y_dpi;
    } else {
        *w = GetDeviceCaps(hdc, HORZRES) * 72.0 / x_dpi;
        *h = GetDeviceCaps(hdc, VERTRES) * 72.0 / y_dpi;
    }
    XFORM xform;
    xform.eM11 = x_dpi / 72.0f;
    xform.eM12 = 0;
    xform.eM21 = 0;
    xform.eM22 = y_dpi / 72.0f;
    if (useFullPage) {
        xform.eDx = static_cast<FLOAT>(-x_off);
        xform.eDy = static_cast<FLOAT>(-y_off);
    } else {
        xform.eDx = 0;
        xform.eDy = 0;
    }
    SetGraphicsMode(hdc, GM_ADVANCED);
    SetWorldTransform(hdc, &xform);

    StartPage(hdc);
}

void win32EndPage(GooString *imageFileName)
{
    EndPage(hdc);
}

void win32EndDocument()
{
    EndDoc(hdc);
    DeleteDC(hdc);
    if (hDevmode) {
        GlobalUnlock(hDevmode);
        GlobalFree(hDevmode);
    } else {
        gfree(devmode);
    }
    if (hDevnames) {
        GlobalUnlock(hDevnames);
        GlobalFree(hDevnames);
    } else {
        gfree(printerName);
    }
}

#endif // CAIRO_HAS_WIN32_SURFACE
