//========================================================================
//
// FileSpec.cc
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2008-2009 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2012, 2017-2021 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2012 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
// Copyright (C) 2024 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

//========================================================================
//
// Most of the code from Link.cc and PSOutputDev.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <config.h>

#include "FileSpec.h"
#include "XRef.h"
#include "goo/gfile.h"

EmbFile::EmbFile(Object &&efStream)
{
    m_size = -1;
    m_createDate = nullptr;
    m_modDate = nullptr;
    m_checksum = nullptr;
    m_mimetype = nullptr;

    m_objStr = std::move(efStream);

    if (m_objStr.isStream()) {
        // dataDict corresponds to Table 3.41 in the PDF1.6 spec.
        Dict *dataDict = m_objStr.streamGetDict();

        // subtype is normally the mimetype
        Object subtypeName = dataDict->lookup("Subtype");
        if (subtypeName.isName()) {
            m_mimetype = new GooString(subtypeName.getName());
        }

        // paramDict corresponds to Table 3.42 in the PDF1.6 spec
        Object paramDict = dataDict->lookup("Params");
        if (paramDict.isDict()) {
            Object paramObj = paramDict.dictLookup("ModDate");
            if (paramObj.isString()) {
                m_modDate = new GooString(paramObj.getString());
            }

            paramObj = paramDict.dictLookup("CreationDate");
            if (paramObj.isString()) {
                m_createDate = new GooString(paramObj.getString());
            }

            paramObj = paramDict.dictLookup("Size");
            if (paramObj.isInt()) {
                m_size = paramObj.getInt();
            }

            paramObj = paramDict.dictLookup("CheckSum");
            if (paramObj.isString()) {
                m_checksum = new GooString(paramObj.getString());
            }
        }
    }
}

EmbFile::~EmbFile()
{
    delete m_createDate;
    delete m_modDate;
    delete m_checksum;
    delete m_mimetype;
}

bool EmbFile::save(const std::string &path)
{
    FILE *f;
    bool ret;

    if (!(f = openFile(path.c_str(), "wb"))) {
        return false;
    }
    ret = save2(f);
    fclose(f);
    return ret;
}

bool EmbFile::save2(FILE *f)
{
    int c;

    if (unlikely(!m_objStr.isStream())) {
        return false;
    }

    m_objStr.streamReset();
    while ((c = m_objStr.streamGetChar()) != EOF) {
        fputc(c, f);
    }
    return true;
}

FileSpec::FileSpec(const Object *fileSpecA)
{
    ok = true;
    fileName = nullptr;
    platformFileName = nullptr;
    embFile = nullptr;
    desc = nullptr;
    fileSpec = fileSpecA->copy();

    Object obj1 = getFileSpecName(fileSpecA);
    if (!obj1.isString()) {
        ok = false;
        error(errSyntaxError, -1, "Invalid FileSpec");
        return;
    }

    fileName = obj1.getString()->copy();

    if (fileSpec.isDict()) {
        obj1 = fileSpec.dictLookup("EF");
        if (obj1.isDict()) {
            fileStream = obj1.dictLookupNF("F").copy();
            if (!fileStream.isRef()) {
                ok = false;
                fileStream.setToNull();
                error(errSyntaxError, -1, "Invalid FileSpec: Embedded file stream is not an indirect reference");
                return;
            }
        }

        obj1 = fileSpec.dictLookup("Desc");
        if (obj1.isString()) {
            desc = obj1.getString()->copy();
        }
    }
}

FileSpec::~FileSpec()
{
    delete fileName;
    delete platformFileName;
    delete embFile;
    delete desc;
}

EmbFile *FileSpec::getEmbeddedFile()
{
    if (!ok || !fileSpec.isDict()) {
        return nullptr;
    }

    if (embFile) {
        return embFile;
    }

    XRef *xref = fileSpec.getDict()->getXRef();
    embFile = new EmbFile(fileStream.fetch(xref));

    return embFile;
}

Object FileSpec::newFileSpecObject(XRef *xref, GooFile *file, const std::string &fileName)
{
    Object paramsDict = Object(new Dict(xref));
    paramsDict.dictSet("Size", Object(file->size()));

    // No Subtype in the embedded file stream dictionary for now
    Object streamDict = Object(new Dict(xref));
    streamDict.dictSet("Length", Object(file->size()));
    streamDict.dictSet("Params", std::move(paramsDict));

    FileStream *fStream = new FileStream(file, 0, false, file->size(), std::move(streamDict));
    fStream->setNeedsEncryptionOnSave(true);
    Stream *stream = fStream;
    const Ref streamRef = xref->addIndirectObject(Object(stream));

    Dict *efDict = new Dict(xref);
    efDict->set("F", Object(streamRef));

    Dict *fsDict = new Dict(xref);
    fsDict->set("Type", Object(objName, "Filespec"));
    fsDict->set("UF", Object(new GooString(fileName)));
    fsDict->set("EF", Object(efDict));

    return Object(fsDict);
}

GooString *FileSpec::getFileNameForPlatform()
{
    if (platformFileName) {
        return platformFileName;
    }

    Object obj1 = getFileSpecNameForPlatform(&fileSpec);
    if (obj1.isString()) {
        platformFileName = obj1.getString()->copy();
    }

    return platformFileName;
}

Object getFileSpecName(const Object *fileSpec)
{
    if (fileSpec->isString()) {
        return fileSpec->copy();
    }

    if (fileSpec->isDict()) {
        Object fileName = fileSpec->dictLookup("UF");
        if (fileName.isString()) {
            return fileName;
        }
        fileName = fileSpec->dictLookup("F");
        if (fileName.isString()) {
            return fileName;
        }
        fileName = fileSpec->dictLookup("DOS");
        if (fileName.isString()) {
            return fileName;
        }
        fileName = fileSpec->dictLookup("Mac");
        if (fileName.isString()) {
            return fileName;
        }
        fileName = fileSpec->dictLookup("Unix");
        if (fileName.isString()) {
            return fileName;
        }
    }
    return Object();
}

Object getFileSpecNameForPlatform(const Object *fileSpec)
{
    if (fileSpec->isString()) {
        return fileSpec->copy();
    }

    Object fileName;
    if (fileSpec->isDict()) {
        fileName = fileSpec->dictLookup("UF");
        if (!fileName.isString()) {
            fileName = fileSpec->dictLookup("F");
            if (!fileName.isString()) {
#ifdef _WIN32
                const char *platform = "DOS";
#else
                const char *platform = "Unix";
#endif
                fileName = fileSpec->dictLookup(platform);
                if (!fileName.isString()) {
                    error(errSyntaxError, -1, "Illegal file spec");
                    return Object();
                }
            }
        }
    } else {
        error(errSyntaxError, -1, "Illegal file spec");
        return Object();
    }

    // system-dependent path manipulation
#ifdef _WIN32
    int i, j;
    GooString *name = fileName.getString()->copy();
    // "//...."             --> "\...."
    // "/x/...."            --> "x:\...."
    // "/server/share/...." --> "\\server\share\...."
    // convert escaped slashes to slashes and unescaped slashes to backslashes
    i = 0;
    if (name->getChar(0) == '/') {
        if (name->getLength() >= 2 && name->getChar(1) == '/') {
            name->del(0);
            i = 0;
        } else if (name->getLength() >= 2 && ((name->getChar(1) >= 'a' && name->getChar(1) <= 'z') || (name->getChar(1) >= 'A' && name->getChar(1) <= 'Z')) && (name->getLength() == 2 || name->getChar(2) == '/')) {
            name->setChar(0, name->getChar(1));
            name->setChar(1, ':');
            i = 2;
        } else {
            for (j = 2; j < name->getLength(); ++j) {
                if (name->getChar(j - 1) != '\\' && name->getChar(j) == '/') {
                    break;
                }
            }
            if (j < name->getLength()) {
                name->setChar(0, '\\');
                name->insert(0, '\\');
                i = 2;
            }
        }
    }
    for (; i < name->getLength(); ++i) {
        if (name->getChar(i) == '/') {
            name->setChar(i, '\\');
        } else if (name->getChar(i) == '\\' && i + 1 < name->getLength() && name->getChar(i + 1) == '/') {
            name->del(i);
        }
    }
    fileName = Object(name);
#endif /* _WIN32 */

    return fileName;
}
