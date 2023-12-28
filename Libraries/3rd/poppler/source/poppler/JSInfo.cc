//========================================================================
//
// JSInfo.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2017, 2020, 2021 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2020 Nelson Benítez León <nbenitezl@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <cstdio>
#include "Object.h"
#include "Dict.h"
#include "Annot.h"
#include "PDFDoc.h"
#include "JSInfo.h"
#include "Link.h"
#include "Form.h"
#include "UnicodeMap.h"
#include "UTF.h"
// #include "Win32Console.h"

JSInfo::JSInfo(PDFDoc *docA, int firstPage)
{
    doc = docA;
    currentPage = firstPage + 1;
}

JSInfo::~JSInfo() { }

void JSInfo::printJS(const GooString *js)
{
    Unicode *u = nullptr;
    char buf[8];
    int i, n, len;

    if (!js || !js->c_str()) {
        return;
    }

    len = TextStringToUCS4(js->toStr(), &u);
    for (i = 0; i < len; i++) {
        n = uniMap->mapUnicode(u[i], buf, sizeof(buf));
        fwrite(buf, 1, n, file);
    }
    gfree(u);
}

void JSInfo::scanLinkAction(LinkAction *link, const char *action)
{
    if (!link) {
        return;
    }

    if (link->getKind() == actionJavaScript) {
        hasJS = true;
        if (print) {
            LinkJavaScript *linkjs = static_cast<LinkJavaScript *>(link);
            if (linkjs->isOk()) {
                const std::string &s = linkjs->getScript();
                fprintf(file, "%s:\n", action);
                GooString gooS = GooString(s);
                printJS(&gooS);
                fputs("\n\n", file);
            }
        }
    }

    if (link->getKind() == actionRendition) {
        LinkRendition *linkr = static_cast<LinkRendition *>(link);
        if (!linkr->getScript().empty()) {
            hasJS = true;
            if (print) {
                fprintf(file, "%s (Rendition):\n", action);
                const GooString s(linkr->getScript());
                printJS(&s);
                fputs("\n\n", file);
            }
        }
    }
}

void JSInfo::scanJS(int nPages)
{
    print = false;
    file = nullptr;
    onlyFirstJS = false;
    scan(nPages);
}

void JSInfo::scanJS(int nPages, FILE *fout, const UnicodeMap *uMap)
{
    print = true;
    file = fout;
    uniMap = uMap;
    onlyFirstJS = false;
    scan(nPages);
}

void JSInfo::scanJS(int nPages, bool stopOnFirstJS)
{
    print = false;
    file = nullptr;
    onlyFirstJS = stopOnFirstJS;
    scan(nPages);
}

void JSInfo::scan(int nPages)
{
    Page *page;
    Annots *annots;
    int lastPage;

    hasJS = false;

    // Names
    int numNames = doc->getCatalog()->numJS();
    if (numNames > 0) {
        hasJS = true;
        if (onlyFirstJS) {
            return;
        }
        if (print) {
            for (int i = 0; i < numNames; i++) {
                fprintf(file, "Name Dictionary \"%s\":\n", doc->getCatalog()->getJSName(i)->c_str());
                GooString *js = doc->getCatalog()->getJS(i);
                printJS(js);
                delete js;
                fputs("\n\n", file);
            }
        }
    }

    // document actions
    scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionCloseDocument).get(), "Before Close Document");
    scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionSaveDocumentStart).get(), "Before Save Document");
    scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionSaveDocumentFinish).get(), "After Save Document");
    scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionPrintDocumentStart).get(), "Before Print Document");
    scanLinkAction(doc->getCatalog()->getAdditionalAction(Catalog::actionPrintDocumentFinish).get(), "After Print Document");

    if (onlyFirstJS && hasJS) {
        return;
    }
    // form field actions
    if (doc->getCatalog()->getFormType() == Catalog::AcroForm) {
        Form *form = doc->getCatalog()->getForm();
        for (int i = 0; i < form->getNumFields(); i++) {
            FormField *field = form->getRootField(i);
            for (int j = 0; j < field->getNumWidgets(); j++) {
                FormWidget *widget = field->getWidget(j);
                scanLinkAction(widget->getActivationAction(), "Field Activated");
                scanLinkAction(widget->getAdditionalAction(Annot::actionFieldModified).get(), "Field Modified");
                scanLinkAction(widget->getAdditionalAction(Annot::actionFormatField).get(), "Format Field");
                scanLinkAction(widget->getAdditionalAction(Annot::actionValidateField).get(), "Validate Field");
                scanLinkAction(widget->getAdditionalAction(Annot::actionCalculateField).get(), "Calculate Field");
                if (onlyFirstJS && hasJS) {
                    return;
                }
            }
        }
    }

    // scan pages

    if (currentPage > doc->getNumPages()) {
        return;
    }

    lastPage = currentPage + nPages;
    if (lastPage > doc->getNumPages() + 1) {
        lastPage = doc->getNumPages() + 1;
    }

    for (int pg = currentPage; pg < lastPage; ++pg) {
        page = doc->getPage(pg);
        if (!page) {
            continue;
        }

        // page actions (open, close)
        scanLinkAction(page->getAdditionalAction(Page::actionOpenPage).get(), "Page Open");
        scanLinkAction(page->getAdditionalAction(Page::actionClosePage).get(), "Page Close");

        if (onlyFirstJS && hasJS) {
            return;
        }
        // annotation actions (links, screen, widget)
        annots = page->getAnnots();
        for (Annot *a : annots->getAnnots()) {
            if (a->getType() == Annot::typeLink) {
                AnnotLink *annot = static_cast<AnnotLink *>(a);
                scanLinkAction(annot->getAction(), "Link Annotation Activated");
                if (onlyFirstJS && hasJS) {
                    return;
                }
            } else if (a->getType() == Annot::typeScreen) {
                AnnotScreen *annot = static_cast<AnnotScreen *>(a);
                scanLinkAction(annot->getAction(), "Screen Annotation Activated");
                scanLinkAction(annot->getAdditionalAction(Annot::actionCursorEntering).get(), "Screen Annotation Cursor Enter");
                scanLinkAction(annot->getAdditionalAction(Annot::actionCursorLeaving).get(), "Screen Annotation Cursor Leave");
                scanLinkAction(annot->getAdditionalAction(Annot::actionMousePressed).get(), "Screen Annotation Mouse Pressed");
                scanLinkAction(annot->getAdditionalAction(Annot::actionMouseReleased).get(), "Screen Annotation Mouse Released");
                scanLinkAction(annot->getAdditionalAction(Annot::actionFocusIn).get(), "Screen Annotation Focus In");
                scanLinkAction(annot->getAdditionalAction(Annot::actionFocusOut).get(), "Screen Annotation Focus Out");
                scanLinkAction(annot->getAdditionalAction(Annot::actionPageOpening).get(), "Screen Annotation Page Open");
                scanLinkAction(annot->getAdditionalAction(Annot::actionPageClosing).get(), "Screen Annotation Page Close");
                scanLinkAction(annot->getAdditionalAction(Annot::actionPageVisible).get(), "Screen Annotation Page Visible");
                scanLinkAction(annot->getAdditionalAction(Annot::actionPageInvisible).get(), "Screen Annotation Page Invisible");

                if (onlyFirstJS && hasJS) {
                    return;
                }
            } else if (a->getType() == Annot::typeWidget) {
                AnnotWidget *annot = static_cast<AnnotWidget *>(a);
                scanLinkAction(annot->getAction(), "Widget Annotation Activated");
                scanLinkAction(annot->getAdditionalAction(Annot::actionCursorEntering).get(), "Widget Annotation Cursor Enter");
                scanLinkAction(annot->getAdditionalAction(Annot::actionCursorLeaving).get(), "Widget Annotation Cursor Leave");
                scanLinkAction(annot->getAdditionalAction(Annot::actionMousePressed).get(), "Widget Annotation Mouse Pressed");
                scanLinkAction(annot->getAdditionalAction(Annot::actionMouseReleased).get(), "Widget Annotation Mouse Released");
                scanLinkAction(annot->getAdditionalAction(Annot::actionFocusIn).get(), "Widget Annotation Focus In");
                scanLinkAction(annot->getAdditionalAction(Annot::actionFocusOut).get(), "Widget Annotation Focus Out");
                scanLinkAction(annot->getAdditionalAction(Annot::actionPageOpening).get(), "Widget Annotation Page Open");
                scanLinkAction(annot->getAdditionalAction(Annot::actionPageClosing).get(), "Widget Annotation Page Close");
                scanLinkAction(annot->getAdditionalAction(Annot::actionPageVisible).get(), "Widget Annotation Page Visible");
                scanLinkAction(annot->getAdditionalAction(Annot::actionPageInvisible).get(), "Widget Annotation Page Invisible");
                if (onlyFirstJS && hasJS) {
                    return;
                }
            }
        }
    }

    currentPage = lastPage;
}

bool JSInfo::containsJS()
{
    return hasJS;
}
