/**
 * Copyright (C) 2011-2012  Charlie Sharpsteen, Stefan Löffler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

// NOTE: `MuPDFBackend.h` is included via `PDFBackend.h`
#include <PDFBackend.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif // defined(HAVE_LOCALE_H)

namespace QtPDF {

namespace Backend {

namespace MuPDF {

QRectF toRectF(const fz_rect r)
{
  return QRectF(QPointF(r.x0, r.y0), QPointF(r.x1, r.y1));
}

QRectF toRectF(const fz_bbox r)
{
  return QRectF(QPointF(r.x0, r.y0), QPointF(r.x1, r.y1));
}

QRectF toRectF(fz_obj * o)
{
  if (!fz_is_array(o) || fz_array_len(o) != 4)
    return QRectF();
  
  return QRectF(QPointF(fz_to_real(fz_array_get(o, 0)), fz_to_real(fz_array_get(o, 1))), \
                QPointF(fz_to_real(fz_array_get(o, 2)), fz_to_real(fz_array_get(o, 3))));
}

QColor toColor(fz_obj * o)
{
  if (!fz_is_array(o))
    return QColor();

  // No color = transparent
  if (fz_array_len(o) == 0)
    return QColor(0, 0, 0, 0);

  // DeviceGray
  if (fz_array_len(o) == 1)
    return QColor::fromRgbF(fz_to_real(fz_array_get(o, 0)), fz_to_real(fz_array_get(o, 0)), fz_to_real(fz_array_get(o, 0)));

  // DeviceRGB
  if (fz_array_len(o) == 3)
    return QColor::fromRgbF(fz_to_real(fz_array_get(o, 0)), fz_to_real(fz_array_get(o, 1)), fz_to_real(fz_array_get(o, 2)));

  // DeviceCMYK
  if (fz_array_len(o) == 4)
    return QColor::fromCmykF(fz_to_real(fz_array_get(o, 0)), fz_to_real(fz_array_get(o, 1)), fz_to_real(fz_array_get(o, 2)), fz_to_real(fz_array_get(o, 3)));

  return QColor();
}

// TODO: Find a better place to put this
PDFDestination toPDFDestination(pdf_xref * xref, fz_obj * dest)
{
  PDFDestination retVal;

  if (!dest)
    return PDFDestination();
  if (fz_is_name(dest))
    return PDFDestination(QString::fromAscii(fz_to_name(dest)));
  if (fz_is_string(dest))
    return PDFDestination(QString::fromAscii(fz_to_str_buf(dest)));

  if (fz_is_array(dest)) {
    // Structure of pdf_link->dest:
    // array w/ >= 2 entries:
    //  - dict which can be passed to pdf_find_page_number
    //  - name: /XYZ, /FIT, ...
    //  - numbers or nulls as required by /XYZ, /FIT, ...
    // TODO: How do external links look like? Or are remote gotos not
    // included in pdf_link structures? Do we need to go through all
    // annotations, then?
    // TODO: Check other types than /XYZ
    if (fz_array_len(dest) < 2)
      return PDFDestination();

    fz_obj * obj;

    obj = fz_array_get(dest, 0);
    if (fz_is_int(obj))
      retVal.setPage(fz_to_int(obj));
    else if (fz_is_dict(obj))
      retVal.setPage(pdf_find_page_number(xref, obj));
    else
      return PDFDestination();

    if (!fz_is_name(fz_array_get(dest, 1)))
      return PDFDestination();
    QString type = QString::fromAscii(fz_to_name(fz_array_get(dest, 1)));
    float left, top, bottom, right, zoom;

    // /XYZ left top zoom 
    if (type == QString::fromUtf8("XYZ")) {
      if (fz_array_len(dest) != 5)
        return PDFDestination();
      obj = fz_array_get(dest, 2);
      if (fz_is_real(obj) || fz_is_int(obj))
        left = fz_to_real(obj);
      else if (fz_is_null(obj))
        left = -1;
      else
        return PDFDestination();
      obj = fz_array_get(dest, 3);
      if (fz_is_real(obj) || fz_is_int(obj))
        top = fz_to_real(obj);
      else if (fz_is_null(obj))
        top = -1;
      else
        return PDFDestination();
      obj = fz_array_get(dest, 4);
      if (fz_is_real(obj) || fz_is_int(obj))
        zoom = fz_to_real(obj);
      else if (fz_is_null(obj))
        zoom = -1;
      else
        return PDFDestination();
      retVal.setType(PDFDestination::Destination_XYZ);
      retVal.setRect(QRectF(QPointF(left, top), QPointF(-1, -1)));
      retVal.setZoom(zoom);
    }
    // /Fit
    else if (type == QString::fromUtf8("Fit")) {
      if (fz_array_len(dest) != 2)
        return PDFDestination();
      retVal.setType(PDFDestination::Destination_Fit);
    }
    // /FitH top
    else if (type == QString::fromUtf8("FitH")) {
      if (fz_array_len(dest) != 3)
        return PDFDestination();
      obj = fz_array_get(dest, 2);
      if (fz_is_real(obj) || fz_is_int(obj))
        top = fz_to_real(obj);
      else if (fz_is_null(obj))
        top = -1;
      else
        return PDFDestination();
      retVal.setType(PDFDestination::Destination_FitH);
      retVal.setRect(QRectF(QPointF(-1, top), QPointF(-1, -1)));
    }
    // /FitV left
    else if (type == QString::fromUtf8("FitV")) {
      if (fz_array_len(dest) != 3)
        return PDFDestination();
      obj = fz_array_get(dest, 2);
      if (fz_is_real(obj) || fz_is_int(obj))
        left = fz_to_real(obj);
      else if (fz_is_null(obj))
        left = -1;
      else
        return PDFDestination();
      retVal.setType(PDFDestination::Destination_FitV);
      retVal.setRect(QRectF(QPointF(left, -1), QPointF(-1, -1)));
    }
    // /FitR left bottom right top
    if (type == QString::fromUtf8("FitR")) {
      if (fz_array_len(dest) != 6)
        return PDFDestination();
      obj = fz_array_get(dest, 2);
      if (fz_is_real(obj) || fz_is_int(obj))
        left = fz_to_real(obj);
      else if (fz_is_null(obj))
        left = -1;
      else
        return PDFDestination();
      obj = fz_array_get(dest, 3);
      if (fz_is_real(obj) || fz_is_int(obj))
        bottom = fz_to_real(obj);
      else if (fz_is_null(obj))
        bottom = -1;
      else
        return PDFDestination();
      obj = fz_array_get(dest, 4);
      if (fz_is_real(obj) || fz_is_int(obj))
        right = fz_to_real(obj);
      else if (fz_is_null(obj))
        right = -1;
      else
        return PDFDestination();
      obj = fz_array_get(dest, 5);
      if (fz_is_real(obj) || fz_is_int(obj))
        top = fz_to_real(obj);
      else if (fz_is_null(obj))
        top = -1;
      else
        return PDFDestination();
      retVal.setType(PDFDestination::Destination_FitR);
      retVal.setRect(QRectF(QPointF(left, top), QPointF(right, bottom)));
      retVal.setZoom(zoom);
    }
    // /FitB
    else if (type == QString::fromUtf8("FitB")) {
      if (fz_array_len(dest) != 2)
        return PDFDestination();
      retVal.setType(PDFDestination::Destination_FitB);
    }
    // /FitBH top
    else if (type == QString::fromUtf8("FitBH")) {
      if (fz_array_len(dest) != 3)
        return PDFDestination();
      obj = fz_array_get(dest, 2);
      if (fz_is_real(obj) || fz_is_int(obj))
        top = fz_to_real(obj);
      else if (fz_is_null(obj))
        top = -1;
      else
        return PDFDestination();
      retVal.setType(PDFDestination::Destination_FitBH);
      retVal.setRect(QRectF(QPointF(-1, top), QPointF(-1, -1)));
    }
    // /FitBV left
    else if (type == QString::fromUtf8("FitBV")) {
      if (fz_array_len(dest) != 3)
        return PDFDestination();
      obj = fz_array_get(dest, 2);
      if (fz_is_real(obj) || fz_is_int(obj))
        left = fz_to_real(obj);
      else if (fz_is_null(obj))
        left = -1;
      else
        return PDFDestination();
      retVal.setType(PDFDestination::Destination_FitBV);
      retVal.setRect(QRectF(QPointF(left, -1), QPointF(-1, -1)));
    }

    return retVal;
  }

  return PDFDestination();
}

#ifdef DEBUG
  const char * fz_type(fz_obj *obj) {
    if (!obj)
      return "(NULL)";
    if (fz_is_null(obj))
      return "null";
    if (fz_is_bool(obj))
      return "bool";
    if (fz_is_int(obj))
      return "int";
    if (fz_is_real(obj))
      return "real";
    if (fz_is_name(obj))
      return "name";
    if (fz_is_string(obj))
      return "string";
    if (fz_is_array(obj))
      return "array";
    if (fz_is_dict(obj))
      return "dict";
    if (fz_is_indirect(obj))
      return "reference";
    return "(unknown)";
  }
#endif


// TODO: Find a better place to put this
void initPDFAnnotation(Annotation::AbstractAnnotation * annot, QWeakPointer<Backend::Page> page, fz_obj * src)
{
  static char keyRect[] = "Rect";
  static char keyContents[] = "Contents";
  static char keyNM[] = "NM";
  static char keyM[] = "M";
  static char keyC[] = "C";
  static char keyF[] = "F";

  if (!annot)
    return;

  annot->setPage(page);

  if (!fz_is_dict(src))
    return;
  
  annot->setRect(toRectF(fz_dict_gets(src, keyRect)));
  annot->setContents(QString::fromUtf8(fz_to_str_buf(fz_dict_gets(src, keyContents))));
  annot->setName(QString::fromUtf8(fz_to_str_buf(fz_dict_gets(src, keyNM))));
  annot->setLastModified(fromPDFDate(QString::fromUtf8(fz_to_str_buf(fz_dict_gets(src, keyM)))));
  annot->setColor(toColor(fz_dict_gets(src, keyC)));
  annot->flags() = QFlags<Annotation::AbstractAnnotation::AnnotationFlags>(fz_to_int(fz_dict_gets(src, keyF)));
}

Annotation::Popup * toPDFPopupAnnotation(Annotation::Markup * parent, fz_obj * src)
{
  static char keyOpen[] = "Open";

  if (!fz_is_dict(src))
    return NULL;

  Annotation::Popup * retVal = new Annotation::Popup();

  initPDFAnnotation(retVal, parent->page(), src);
  retVal->setParent(parent);
  retVal->setOpen(fz_to_bool(fz_dict_gets(src, keyOpen)));
  return retVal;
}

void initPDFMarkupAnnotation(Annotation::Markup * annot, QWeakPointer<Backend::Page> page, fz_obj * src)
{
  static char keyT[] = "T";
  static char keyRC[] = "RC";
  static char keyCreationDate[] = "CreationDate";
  static char keySubj[] = "Subj";
  static char keyPopup[] = "Popup";

  if (!annot)
    return;

  initPDFAnnotation(annot, page, src);

  if (!fz_is_dict(src))
    return;

  annot->setTitle(QString::fromUtf8(fz_to_str_buf(fz_dict_gets(src, keyT))));
  annot->setRichContents(QString::fromUtf8(fz_to_str_buf(fz_dict_gets(src, keyRC))));
  annot->setCreationDate(fromPDFDate(QString::fromUtf8(fz_to_str_buf(fz_dict_gets(src, keyCreationDate)))));
  annot->setSubject(QString::fromUtf8(fz_to_str_buf(fz_dict_gets(src, keySubj))));

  annot->setPopup(toPDFPopupAnnotation(annot, fz_dict_gets(src, keyPopup)));
}


// Modeled after the idea behind QMutexLocker, i.e., the class sets LC_NUMERIC
// to "C" in the contructor and resets the original setting in the destructor
#ifdef HAVE_LOCALE_H
class MuPDFLocaleResetter
{
  char * _locale;
  static QMutex * _lock;
public:
  MuPDFLocaleResetter() { _lock->lock(); _locale = setlocale(LC_NUMERIC, NULL); setlocale(LC_NUMERIC, "C"); }
  ~MuPDFLocaleResetter() { _lock->unlock(); setlocale(LC_NUMERIC, _locale); }
};
QMutex * MuPDFLocaleResetter::_lock = new QMutex(QMutex::Recursive);
#else
class MuPDFLocaleResetter
{
public:
  MuPDFLocaleResetter() { }
};
#endif // defined(HAVE_LOCALE_H)


// Document Class
// ==============
Document::Document(QString fileName):
  Super(fileName),
  _mupdf_data(NULL),
  _glyph_cache(fz_new_glyph_cache())
{
#ifdef DEBUG
//  qDebug() << "MuPDF::Document::Document(" << fileName << ")";
#endif
  reload();
}

Document::~Document()
{
#ifdef DEBUG
//  qDebug() << "MuPDF::Document::~Document()";
#endif

  QWriteLocker docLocker(_docLock.data());

  clearPages();

  if( _mupdf_data ){
    pdf_free_xref(_mupdf_data);
    _mupdf_data = NULL;
  }

  fz_free_glyph_cache(_glyph_cache);
}

void Document::reload()
{
  // Clear the processing thread
  // NB: Do this before acquiring _docLock. See clearWorkStack() documentation.
  // This should not cause any problems as we are supposed to currently be in
  // the main (GUI) thread, and only this thread is supposed to add items to the
  // work stack.
  _processingThread.clearWorkStack();

  QWriteLocker docLocker(_docLock.data());
  MuPDFLocaleResetter lr;

  clearPages();
  _pageCache.clear();

  if (_mupdf_data) {
    pdf_free_xref(_mupdf_data);
    _mupdf_data = NULL;
  }

  fz_stream *pdf_file = fz_open_file(_fileName.toLocal8Bit().data());
  if (!pdf_file)
    return;
  pdf_open_xref_with_stream(&_mupdf_data, pdf_file, NULL);
  fz_close(pdf_file);

  if (!_mupdf_data)
    return;

  // Permission level determination works as follows:
  // 1) If there is no `crypt` dictionary, there is no security set, and
  //    consequently we have full permissions (i.e., owner level)
  // 2) If there is a `crypt` dictionary and we have a password to try, do that.
  //    Note: MuPDF currently doesn't provide any way to distinguish user from
  //    owner password, so to be on the safe side we always assume the lower
  //    (i.e. user) permission level
  // 3) If we have `crypt` but no password, see if that is enough for user level
  // 4) Otherwise, the document is locked
  if (!_mupdf_data->crypt)
    _permissionLevel = PermissionLevel_Owner;
  else if (!_password.isEmpty()) {
    // TODO: Check if toUtf8 makes sense
    // TODO: Revisit this once MuPDF tells us the permission level we have
    if (pdf_authenticate_password(_mupdf_data, _password.toUtf8().data()))
      _permissionLevel = PermissionLevel_User;
    else
      _permissionLevel = PermissionLevel_Locked;
  }
  else if (!pdf_needs_password(_mupdf_data))
    _permissionLevel = PermissionLevel_User;
  else
    _permissionLevel = PermissionLevel_Locked;

  // Permissions
  // - at `Locked` level, we have no permissions
  // - at `User` level, we take the permissions from MuPDF
  // - at `Owner` level, we assume full permissions (note that pdf_has_permission
  //   doesn't distinguish between user and owner level)
  switch (_permissionLevel) {
    case PermissionLevel_Locked:
      _permissions = QFlags<Permissions>();
      break;
    case PermissionLevel_User:
      _permissions = QFlags<Permissions>();
      if (pdf_has_permission(_mupdf_data, PDF_PERM_PRINT))
        _permissions |= Permission_Print;
      if (pdf_has_permission(_mupdf_data, PDF_PERM_CHANGE))
        _permissions |= Permission_Change;
      if (pdf_has_permission(_mupdf_data, PDF_PERM_COPY))
        _permissions |= Permission_Extract;
      if (pdf_has_permission(_mupdf_data, PDF_PERM_NOTES))
        _permissions |= Permission_Annotate;
      if (pdf_has_permission(_mupdf_data, PDF_PERM_FILL_FORM))
        _permissions |= Permission_FillForm;
      if (pdf_has_permission(_mupdf_data, PDF_PERM_ACCESSIBILITY))
        _permissions |= Permission_ExtractForAccessibility;
      if (pdf_has_permission(_mupdf_data, PDF_PERM_ASSEMBLE))
        _permissions |= Permission_Assemble;
      if (pdf_has_permission(_mupdf_data, PDF_PERM_HIGH_RES_PRINT))
        _permissions |= Permission_PrintHighRes;
      break;
    case PermissionLevel_Owner:
      _permissions = QFlags<Permissions>(Permission_Print | \
                                         Permission_Change | \
                                         Permission_Extract | \
                                         Permission_Annotate | \
                                         Permission_FillForm | \
                                         Permission_ExtractForAccessibility | \
                                         Permission_Assemble | \
                                         Permission_PrintHighRes);
      break;
  }

  // NOTE: This can also fail.
  pdf_load_page_tree(_mupdf_data);
  _numPages = pdf_count_pages(_mupdf_data);
  loadMetaData();
}

QWeakPointer<Backend::Page> Document::page(int at)
{
  {
    QReadLocker docLocker(_docLock.data());

    if (at < 0 || at >= _numPages)
      return QWeakPointer<Backend::Page>();

    if (at < _pages.size() && !_pages[at].isNull())
      return QWeakPointer<Backend::Page>(_pages[at]);;

    // if we get here, the page is not in the array
  }

  QWriteLocker docLocker(_docLock.data());

  // recheck everything that could have changed before we got the write lock
  if (at >= _numPages)
    return QWeakPointer<Backend::Page>();
  if (at < _pages.size() && !_pages[at].isNull())
    return _pages[at].toWeakRef();

  if( _pages.isEmpty() )
    _pages.resize(_numPages);

  _pages[at] = QSharedPointer<Backend::Page>(new Page(this, at, _docLock));
  return _pages[at].toWeakRef();
}

QWeakPointer<Backend::Page> Document::page(int at) const
{
  QReadLocker docLocker(_docLock.data());

  if (at < 0 || at >= _numPages || at >= _pages.size())
    return QWeakPointer<Backend::Page>();

  return _pages[at];
}

void Document::loadMetaData()
{
  char infoName[] = "Info"; // required because fz_dict_gets is not prototyped to take const char *

  QWriteLocker docLocker(_docLock.data());
  MuPDFLocaleResetter lr;

  if (_isLocked())
    return;

  // TODO: Handle encrypted meta data
  // Note: fz_is_dict(NULL)===0, i.e., it doesn't crash
  if (!fz_is_dict(_mupdf_data->trailer))
    return;

  clearMetaData();
  fz_obj * info = fz_dict_gets(_mupdf_data->trailer, infoName);
  if (fz_is_dict(info)) { // the `Info` entry is optional
    for (int i = 0; i < fz_dict_len(info); ++i) {
      // TODO: Check if fromAscii always gives correct results (the pdf specs
      // allow for arbitrary 8bit characters - fromAscii can handle them, but
      // can be codec dependent (see Qt docs)
      // Note: fz_to_name returns an internal pointer that must not be freed
      QString key = QString::fromAscii(fz_to_name(fz_dict_get_key(info, i)));
      // Handle standard keys
      if (key == QString::fromUtf8("Trapped")) {
        // TODO: Check if fromAscii always gives correct results (the pdf specs
        // allow for arbitrary 8bit characters - fromAscii can handle them, but
        // can be codec dependent (see Qt docs)
        QString val = QString::fromAscii(fz_to_name(fz_dict_get_val(info, i)));
        if (val == QString::fromUtf8("True"))
          _meta_trapped = Trapped_True;
        else if (val == QString::fromUtf8("False"))
          _meta_trapped = Trapped_False;
        else
          _meta_trapped = Trapped_Unknown;
      }
      else {
        // TODO: Check if fromAscii always gives correct results (the pdf specs
        // allow for arbitrary 8bit characters - fromAscii can handle them, but
        // can be codec dependent (see Qt docs)
        QString val = QString::fromAscii(fz_to_str_buf(fz_dict_get_val(info, i)));
        if (key == QString::fromUtf8("Title"))
          _meta_title = val;
        else if (key == QString::fromUtf8("Author"))
          _meta_author = val;
        else if (key == QString::fromUtf8("Subject"))
          _meta_subject = val;
        else if (key == QString::fromUtf8("Keywords"))
          _meta_keywords = val;
        else if (key == QString::fromUtf8("Creator"))
          _meta_creator = val;
        else if (key == QString::fromUtf8("Producer"))
          _meta_producer = val;
        else if (key == QString::fromUtf8("CreationDate"))
          _meta_creationDate = fromPDFDate(val);
        else if (key == QString::fromUtf8("ModDate"))
          _meta_modDate = fromPDFDate(val);
        else
          _meta_other[key] = val;
      }
    }
  }
  // TODO: Implement metadata stream handling (which should probably override
  // the data in the `Info` dictionary
}

PDFDestination Document::resolveDestination(const PDFDestination & namedDestination) const
{
  QReadLocker docLocker(_docLock.data());
  MuPDFLocaleResetter lr;

  Q_ASSERT(_mupdf_data != NULL);
  
  // TODO: Test this method

  // If namedDestination is not a named destination at all, simply return a copy
  if (namedDestination.isExplicit())
    return namedDestination;

  fz_obj * name = fz_new_name(namedDestination.destinationName().toUtf8().data());
  // Note: Ideally, we would use resolve_dest, but that is not declared
  // officially in mupdf.h, only in <mupdf>/pdf/pdf_annot.c
  //fz_obj * dest = resolve_dest(_mupdf_data, name)
  fz_obj * dest = pdf_lookup_dest(_mupdf_data, name);
  fz_drop_obj(name);
  return toPDFDestination(_mupdf_data, dest);
}

QList<PDFFontInfo> Document::fonts() const
{
  QReadLocker docLocker(_docLock.data());
  MuPDFLocaleResetter lr;

  int i;
  char typeKey[] = "Type";
  char subtypeKey[] = "Subtype";
  char descriptorKey[] = "FontDescriptor";
  char basefontKey[] = "BaseFont";
  char nameKey[] = "Name";
  char fontnameKey[] = "FontName";
  char fontfileKey[] = "FontFile";
  char fontfile2Key[] = "FontFile2";
  char fontfile3Key[] = "FontFile3";
  QList<PDFFontInfo> retVal;

  if (!_mupdf_data)
    return retVal;

#ifdef DEBUG
  QTime timer;
  timer.start();
#endif

  // Iterate over all objects
  for (i = 0; i < _mupdf_data->len; ++i) {
    switch (_mupdf_data->table[i].type) {
      case 'o':
      case 'n':
      {
        if (!fz_is_dict(_mupdf_data->table[i].obj))
          continue;
        if (QString::fromAscii(fz_to_name(fz_dict_gets(_mupdf_data->table[i].obj, typeKey))) != QString::fromUtf8("Font"))
          continue;

        QString subtype = QString::fromUtf8(fz_to_name(fz_dict_gets(_mupdf_data->table[i].obj, subtypeKey)));

        // Type0 fonts have no info we need right now---all relevant data is in
        // its descendant, which again is a dict of type /Font
        if (subtype == QString::fromUtf8("Type0"))
          continue;

        PDFFontDescriptor descriptor;
        PDFFontInfo fi;
        
        // Parse the /FontDescriptor dictionary (if it exists)
        // If not, try to derive the font name from /BaseFont (if it exists) or
        // /Name
        fz_obj * desc = fz_dict_gets(_mupdf_data->table[i].obj, descriptorKey);
        if (fz_is_dict(desc)) {
          descriptor.setName(QString::fromAscii(fz_to_name(fz_dict_gets(desc, fontnameKey))));
          fz_obj * ff = fz_dict_gets(desc, fontfileKey);
          if (fz_is_dict(ff)) {
            fi.setFontProgramType(PDFFontInfo::ProgramType_Type1);
            fi.setSource(PDFFontInfo::Source_Embedded);
          }
          else {
            fz_obj * ff = fz_dict_gets(desc, fontfile2Key);
            if (fz_is_dict(ff)) {
              fi.setFontProgramType(PDFFontInfo::ProgramType_TrueType);
              fi.setSource(PDFFontInfo::Source_Embedded);
            }
            else {
              fz_obj * ff = fz_dict_gets(desc, fontfile3Key);
              if (fz_is_dict(ff)) {
                QString ffSubtype = QString::fromUtf8(fz_to_name(fz_dict_gets(ff, subtypeKey)));
                if (ffSubtype == QString::fromUtf8("Type1C"))
                  fi.setFontProgramType(PDFFontInfo::ProgramType_Type1CFF);
                else if (ffSubtype == QString::fromUtf8("CIDFontType0C"))
                  fi.setFontProgramType(PDFFontInfo::ProgramType_CIDCFF);
                else if (ffSubtype == QString::fromUtf8("OpenType"))
                  fi.setFontProgramType(PDFFontInfo::ProgramType_OpenType);
                fi.setSource(PDFFontInfo::Source_Embedded);
              }
              else {
                // Note: It seems MuPDF handles embedded fonts, and for everything
                // else uses its own, built-in base14 fonts as substitution (i.e., it
                // doesn't use fonts from the file system)
                fi.setFontProgramType(PDFFontInfo::ProgramType_None);
                fi.setSource(PDFFontInfo::Source_Builtin);
              }
            }
          }
          // TODO: Parse other entries in /FontDescriptor if ever we need them
        }
        else {
          // Note: It seems MuPDF handles embedded fonts, and for everything
          // else uses its own, built-in base14 fonts as substitution (i.e., it
          // doesn't use fonts from the file system)
          fi.setFontProgramType(PDFFontInfo::ProgramType_None);
          fi.setSource(PDFFontInfo::Source_Builtin);

          fz_obj * basefont = fz_dict_gets(_mupdf_data->table[i].obj, basefontKey);
          if (fz_is_name(basefont))
            descriptor.setName(QString::fromAscii(fz_to_name(basefont)));
          else
            descriptor.setName(QString::fromAscii(fz_to_name(fz_dict_gets(_mupdf_data->table[i].obj, nameKey))));
        }
        fi.setDescriptor(descriptor);

        // Set the font type
        if (subtype == QString::fromUtf8("Type1")) {
          fi.setFontType(PDFFontInfo::FontType_Type1);
          fi.setCIDType(PDFFontInfo::CIDFont_None);
        }
        else if (subtype == QString::fromUtf8("MMType1")) {
          fi.setFontType(PDFFontInfo::FontType_MMType1);
          fi.setCIDType(PDFFontInfo::CIDFont_None);
        }
        else if (subtype == QString::fromUtf8("Type3")) {
          fi.setFontType(PDFFontInfo::FontType_Type3);
          fi.setCIDType(PDFFontInfo::CIDFont_None);
        }
        else if (subtype == QString::fromUtf8("TrueType")) {
          fi.setFontType(PDFFontInfo::FontType_TrueType);
          fi.setCIDType(PDFFontInfo::CIDFont_None);
        }
        else if (subtype == QString::fromUtf8("CIDFontType0")) {
          fi.setFontType(PDFFontInfo::FontType_Type0);
          fi.setCIDType(PDFFontInfo::CIDFont_Type0);
        }
        else if (subtype == QString::fromUtf8("CIDFontType2")) {
          fi.setFontType(PDFFontInfo::FontType_Type0);
          fi.setCIDType(PDFFontInfo::CIDFont_Type2);
        }

        retVal << fi;
        break;
      }
      case 0:
      case 'f':
      default:
        continue;
    }
  }

#ifdef DEBUG
  qDebug() << "loaded fonts in" << timer.elapsed() << "ms";
#endif

  return retVal;
}

void Document::recursiveConvertToC(QList<PDFToCItem> & items, pdf_outline * node) const
{
  while (node && node->title) {
    // TODO: It seems that this works, at least for pdfs produced with pdflatex
    // using either utf8 or latin1 encoding (and the approrpriate inputenc
    // package). Is this valid generally?
    PDFToCItem item(QString::fromUtf8(node->title));
    item.setOpen(node->count > 0);

    if (node->link && (node->link->kind == PDF_LINK_GOTO || node->link->kind == PDF_LINK_NAMED)) {
      Q_ASSERT(_mupdf_data != NULL);
      item.setAction(new PDFGotoAction(toPDFDestination(_mupdf_data, node->link->dest)));
    }

    recursiveConvertToC(item.children(), node->child);

    // NOTE: pdf_outline doesn't include color or flags; we could go through the
    // pdf ourselves to get to them, but for now we simply don't support them
    items << item;
    node = node->next;
  }
}

PDFToC Document::toc() const
{
  QReadLocker docLocker(_docLock.data());
  MuPDFLocaleResetter lr;

  PDFToC retVal;

  if (!_mupdf_data)
    return retVal;

  pdf_outline * outline = pdf_load_outline(_mupdf_data);
  if (outline) {
    recursiveConvertToC(retVal, outline);
    pdf_free_outline(outline);
  }
  return retVal;
}

bool Document::unlock(const QString password)
{
  QWriteLocker docLocker(_docLock.data());

  if (!_mupdf_data)
    return false;

  // Note: we try unlocking regardless of what isLocked() returns as the user
  // might want to unlock a document with the owner's password when user level
  // access is already granted.
  // TODO: Check if toUtf8 makes sense
  bool success = pdf_authenticate_password(_mupdf_data, password.toUtf8().data());

  if (success)
    _password = password;
  // Note: Reload in any case as pdf_authenticate_password can even relock the
  // document (which reload() undoes)
  reload();
  return success;
}


// Page Class
// ==========
Page::Page(Document *parent, int at, QSharedPointer<QReadWriteLock> docLock):
  Super(parent, at, docLock),
  _annotationsLoaded(false),
  _linksLoaded(false)
{
  MuPDFLocaleResetter lr;
  QWriteLocker pageLocker(_pageLock);

  pdf_page *page_data;
  pdf_load_page(&page_data, parent->_mupdf_data, _n);

  _bbox = page_data->mediabox;
  _size = QSizeF(qreal(_bbox.x1 - _bbox.x0), qreal(_bbox.y1 - _bbox.y0));
  // MuPDF's page_data->mediabox is rounded to integers. _size should be exact,
  // though.
  if (parent->_mupdf_data && at >= 0 && at < parent->_mupdf_data->page_len){
    fz_obj * pageobj = parent->_mupdf_data->page_objs[at];
    if (pageobj) {
      static char keyMediaBox[] = "MediaBox";
      QRectF r(toRectF(fz_dict_gets(pageobj, keyMediaBox)));
      if (!r.isEmpty())
        _size = r.size();
    }
  }
  _rotate = qreal(page_data->rotate);

  // This is also time-intensive. It takes Poppler ~500 ms to create page
  // objects for the entire PGF Manual. MuPDF takes ~1000 ms, but only ~200 if
  // this step is omitted. May be useful to move this into a render function
  // but it will require us to keep page_data around.
  _mupdf_page = fz_new_display_list();
  fz_device *dev = fz_new_list_device(_mupdf_page);
  pdf_run_page(parent->_mupdf_data, page_data, dev, fz_identity);

  fz_free_device(dev);
  pdf_free_page(page_data);
  
  loadTransitionData();
}

Page::~Page()
{
  QWriteLocker pageLocker(_pageLock);
  if( _mupdf_page )
    fz_free_display_list(_mupdf_page);
  _mupdf_page = NULL;
}

QSizeF Page::pageSizeF() const { QReadLocker pageLocker(_pageLock); return _size; }

QImage Page::renderToImage(double xres, double yres, QRect render_box, bool cache)
{
  QReadLocker docLocker(_docLock.data());
  QReadLocker pageLocker(_pageLock);
  if (!_parent)
    return QImage();

  // Set up the transformation matrix for the page. Really, we just start with
  // an identity matrix and scale it using the xres, yres inputs.
  fz_matrix render_trans = fz_identity;
  render_trans = fz_concat(render_trans, fz_translate(0, -_bbox.y1));
  render_trans = fz_concat(render_trans, fz_scale(xres/72.0, -yres/72.0));
  render_trans = fz_concat(render_trans, fz_rotate(_rotate));

#ifdef DEBUG
  qDebug() << "Page bbox is: (" << _bbox.x0 << "," << _bbox.y0 << "|" << _bbox.x1 << "," << _bbox.y1 << ")";
#endif

  fz_bbox render_bbox;
  if ( not render_box.isNull() ) {
    render_bbox.x0 = render_box.left();
    render_bbox.y0 = render_box.top();
    render_bbox.x1 = render_box.right();
    render_bbox.y1 = render_box.bottom();
  } else {
    render_bbox = fz_round_rect(fz_transform_rect(render_trans, _bbox));
  }

#ifdef DEBUG
  qDebug() << "Render bbox is: (" << render_bbox.x0 << "," << render_bbox.y0 << "|" << render_bbox.x1 << "," << render_bbox.y1 << ")";
#endif

  // NOTE: Using fz_device_bgr or fz_device_rbg may depend on platform endianness.
  fz_pixmap *mu_image = fz_new_pixmap_with_rect(fz_device_bgr, render_bbox);
  // Flush to white.
  fz_clear_pixmap_with_color(mu_image, 255);
  fz_device *renderer = fz_new_draw_device(static_cast<Document *>(_parent)->_glyph_cache, mu_image);

  // Actually render the page.
  fz_execute_display_list(_mupdf_page, renderer, render_trans, render_bbox);

  // Create a QImage that shares data with the fz_pixmap.
  QImage tmp_image(mu_image->samples, mu_image->w, mu_image->h, QImage::Format_ARGB32);
  // Now create a copy with its own data that can exist outside this function
  // call.
  QImage renderedPage = tmp_image.copy();

  // Dispose of unneeded items.
  fz_free_device(renderer);
  fz_drop_pixmap(mu_image);

  if( cache ) {
    PDFPageTile key(xres, yres, render_box, _n);
    QImage * img = new QImage(renderedPage.copy());
    if (img != _parent->pageCache().setImage(key, img))
      delete img;
  }

  return renderedPage;
}

QList< QSharedPointer<Annotation::Link> > Page::loadLinks()
{
  {
    QReadLocker pageLocker(_pageLock);
    if (_linksLoaded || !_parent)
      return _links;
  }

  QReadLocker docLocker(_docLock.data());
  QWriteLocker pageLocker(_pageLock);

  // Check if the links were loaded in another thread in the meantime
  if (_linksLoaded || !_parent)
    return _links;

  MuPDFLocaleResetter lr;

  pdf_xref * xref = static_cast<Document*>(_parent)->_mupdf_data;
  Q_ASSERT(xref != NULL);

  _linksLoaded = true;
  pdf_page * page;
  if (pdf_load_page(&page, xref, _n) != fz_okay)
    return _links;

  if (!page)
    return _links;
  pdf_link * mupdfLink = page->links;

  while (mupdfLink) {
    QSharedPointer<Annotation::Link> link(new Annotation::Link);
    link->setRect(toRectF(mupdfLink->rect));
    link->setPage(_parent->page(_n));
    // TODO: Initialize all other properties of PDFLinkAnnotation, such as
    // border, color, quadPoints, etc.

    switch (mupdfLink->kind) {
      case PDF_LINK_NAMED:
      case PDF_LINK_GOTO:
        link->setActionOnActivation(new PDFGotoAction(toPDFDestination(xref, mupdfLink->dest)));
        break;
      case PDF_LINK_URI:
        // TODO: Check if MuPDF indeed always returns properly encoded URLs
        link->setActionOnActivation(new PDFURIAction(QUrl::fromEncoded(fz_to_str_buf(mupdfLink->dest))));
        break;
      case PDF_LINK_LAUNCH:
        // TODO: Check if fromLocal8Bit works in all cases. The pdf specs say
        // that "[file specs] are stored as bytes and are passed to the
        // operating system without interpretation or conversion of any sort."
        // Since we have no influence on how strings are treated in process
        // creation, we cannot fully comply with this. However, it seems that
        // (at least in Qt 4.7.2):
        //  - on Unix/Mac, QFile::encodeName is used; Qt docs say that "this
        //    function converts fileName to the local 8-bit encoding determined
        //    by the user's locale".
        //  - on Win, full utf16 is used
        link->setActionOnActivation(new PDFLaunchAction(QString::fromLocal8Bit(fz_to_str_buf(mupdfLink->dest))));
        break;
      case PDF_LINK_ACTION:
        // we don't handle this yet
        link.clear();
        break;
    }
    if (link)
      _links << link;
    mupdfLink = mupdfLink->next;
  }

  pdf_free_page(page);
  return _links;
}

QList< QSharedPointer<Annotation::AbstractAnnotation> > Page::loadAnnotations()
{
  {
    QReadLocker pageLocker(_pageLock);
    if (_annotationsLoaded || !_parent)
      return _annotations;
  }

  QReadLocker docLocker(_docLock.data());
  QWriteLocker pageLocker(_pageLock);

  // Check if the annotations were loaded in another thread in the meantime
  if (_annotationsLoaded || !_parent)
    return _annotations;

  MuPDFLocaleResetter lr;
  static char keyType[] = "Type";
  static char keySubtype[] = "Subtype";

  pdf_xref * xref = static_cast<Document*>(_parent)->_mupdf_data;
  Q_ASSERT(xref != NULL);

  _annotationsLoaded = true;
  pdf_page * page;
  if (pdf_load_page(&page, xref, _n) != fz_okay)
    return _annotations;

  if (!page)
    return _annotations;
  pdf_annot * mupdfAnnot = page->annots;
  
  while (mupdfAnnot) {
    if (!fz_is_dict(mupdfAnnot->obj) || QString::fromAscii(fz_to_name(fz_dict_gets(mupdfAnnot->obj, keyType))) != QString::fromAscii("Annot")) {
      mupdfAnnot = mupdfAnnot->next;
      continue;
    }

    QString subtype = QString::fromAscii(fz_to_name(fz_dict_gets(mupdfAnnot->obj, keySubtype)));
    if (subtype == QString::fromAscii("Text")) {
      Annotation::Text * annot = new Annotation::Text();
      initPDFMarkupAnnotation(annot, _parent->page(_n), mupdfAnnot->obj);
      _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
    }
    else if (subtype == QString::fromAscii("FreeText")) {
      Annotation::FreeText * annot = new Annotation::FreeText();
      initPDFMarkupAnnotation(annot, _parent->page(_n), mupdfAnnot->obj);
      _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
    }
    else if (subtype == QString::fromAscii("Caret")) {
      Annotation::Caret * annot = new Annotation::Caret();
      initPDFMarkupAnnotation(annot, _parent->page(_n), mupdfAnnot->obj);
      _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
    }
    else if (subtype == QString::fromAscii("Highlight")) {
      Annotation::Highlight * annot = new Annotation::Highlight();
      initPDFMarkupAnnotation(annot, _parent->page(_n), mupdfAnnot->obj);
      _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
    }
    else if (subtype == QString::fromAscii("Underline")) {
      Annotation::Underline * annot = new Annotation::Underline();
      initPDFMarkupAnnotation(annot, _parent->page(_n), mupdfAnnot->obj);
      _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
    }
    else if (subtype == QString::fromAscii("Squiggly")) {
      Annotation::Squiggly * annot = new Annotation::Squiggly();
      initPDFMarkupAnnotation(annot, _parent->page(_n), mupdfAnnot->obj);
      _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
    }
    else if (subtype == QString::fromAscii("StrikeOut")) {
      Annotation::StrikeOut * annot = new Annotation::StrikeOut();
      initPDFMarkupAnnotation(annot, _parent->page(_n), mupdfAnnot->obj);
      _annotations << QSharedPointer<Annotation::AbstractAnnotation>(annot);
    }
    // TODO: Other annotation types (do we need Link annotations here?)
    mupdfAnnot = mupdfAnnot->next;
  }
  return _annotations;
}

QList<SearchResult> Page::search(QString searchText, SearchFlags flags)
{
  QList<SearchResult> results;
  fz_text_span * page_text, * span;
  fz_device * dev;
  QString text;
  int i, j, spanStart;
  Qt::CaseSensitivity caseSensitivity = (flags & Search_CaseInsensitive ? Qt::CaseInsensitive : Qt::CaseSensitive);

  QReadLocker pageLocker(_pageLock);

  // Use MuPDF transformations to get the text box coordinates right already
  // during fz_execute_display_list().
  fz_matrix render_trans = fz_identity;
  render_trans = fz_concat(render_trans, fz_translate(0, -_bbox.y1));
  render_trans = fz_concat(render_trans, fz_scale(1, -1));
  render_trans = fz_concat(render_trans, fz_rotate(_rotate));

  if (!_mupdf_page)
    return results;

  // Extract text from page
  page_text = fz_new_text_span();
  dev = fz_new_text_device(page_text);
  fz_execute_display_list(_mupdf_page, dev, render_trans, fz_infinite_bbox);
  fz_free_device(dev);

  // Convert fz_text_spans to QString
  // TODO: Decide what to do about the space MuPDF prepends and appends to each
  // line (at least for the base14-fonts.pdf test case).
  for (span = page_text; span != NULL; span = span->next) {
    for (i = 0; i < span->len; ++i)
      text.append(span->text[i].c);
    if (span->eol)
      text.append(QChar::fromAscii('\n'));
  }

  // Perform the actual search and extract box information
  i = 0;
  spanStart = 0;
  span = page_text;
  while ((i = text.indexOf(searchText, i, caseSensitivity)) >= 0) {
    // Search for the text span(s) the string is coming from. Note: Because we
    // are doing a forward search only, we don't need to reset `span` or
    // `spanStart`.
    for (; span != NULL; span = span->next) {
      if (i < spanStart + span->len)
        break;
      spanStart += span->len;
      if (span->eol)
        ++spanStart;
    }
    if (!span)
      break;

    SearchResult result;
    result.pageNum = pageNum();
    for (j = 0; j < searchText.length(); ++j) {
      while (span && i + j >= spanStart + span->len) {
        spanStart += span->len;
        if (span->eol)
          ++spanStart;
        span = span->next;
      }
      result.bbox |= toRectF(span->text[i + j - spanStart].bbox);
    }

    if (flags & ::QtPDF::Backend::Search_Backwards)
      results.prepend(result);
    else
      results << result;

    // Offset `i` so we don't find the same match over and over again    
    i += searchText.length();
  }

  fz_free_text_span(page_text);
  return results;
}

void Page::loadTransitionData()
{
  static char keyTrans[] = "Trans";
  static char keyS[] = "S";
  static char keyD[] = "D";
  static char keyDm[] = "Dm";
  static char keyM[] = "M";
  static char keyDi[] = "Di";
  // TODO: SS, B keys
  fz_obj * trans, * tmp;

  if (!_parent)
    return;

  // NOTE: That data is not available in the pdf_page struct - we need to parse
  // the fz_obj ourselves
  pdf_xref * xref = static_cast<Document*>(_parent)->_mupdf_data;
  Q_ASSERT(xref != NULL);
  Q_ASSERT(xref->page_len >= _n);
  
  fz_obj * page = xref->page_objs[_n];
  Q_ASSERT(page != NULL);
  
  if (_transition) {
    delete _transition;
    _transition = NULL;
  }
  
  trans = fz_dict_gets(page, keyTrans);
  if (!trans)
    return;
  tmp = fz_dict_gets(trans, keyS);
  if (!tmp)
    return;
  QString style = QString::fromAscii(fz_to_name(tmp));
  if (style == QString::fromAscii("Split"))
    _transition = new Transition::Split();
  else if (style == QString::fromAscii("Blinds"))
    _transition = new Transition::Blinds();
  else if (style == QString::fromAscii("Box"))
    _transition = new Transition::Box();
  else if (style == QString::fromAscii("Wipe"))
    _transition = new Transition::Wipe();
  else if (style == QString::fromAscii("Dissolve"))
    _transition = new Transition::Dissolve();
  else if (style == QString::fromAscii("Glitter"))
    _transition = new Transition::Glitter();
  else if (style == QString::fromAscii("R"))
    _transition = new Transition::Replace();
  else if (style == QString::fromAscii("Fly"))
    _transition = new Transition::Fly();
  else if (style == QString::fromAscii("Push"))
    _transition = new Transition::Push();
  else if (style == QString::fromAscii("Cover"))
    _transition = new Transition::Cover();
  else if (style == QString::fromAscii("Uncover"))
    _transition = new Transition::Uncover();
  else if (style == QString::fromAscii("Fade"))
    _transition = new Transition::Fade();
  if (!_transition)
    return;
  
  tmp = fz_dict_gets(trans, keyD);
  if (tmp)
    _transition->setDuration(fz_to_real(tmp));
  tmp = fz_dict_gets(trans, keyDm);
  if (tmp && (style == QString::fromAscii("Split") || style == QString::fromAscii("Blinds"))) {
    if (QString::fromAscii(fz_to_name(tmp)) == QString::fromAscii("V"))
      _transition->setDirection(90);
    else
      _transition->setDirection(0);
  }
  tmp = fz_dict_gets(trans, keyM);
  if (tmp && (style == QString::fromAscii("Split") || style == QString::fromAscii("Box") || style == QString::fromAscii("Fly"))) {
    if (QString::fromAscii(fz_to_name(tmp)) == QString::fromAscii("O"))
      _transition->setMotion(Transition::AbstractTransition::Motion_Outward);
    else
      _transition->setMotion(Transition::AbstractTransition::Motion_Inward);
  }
  tmp = fz_dict_gets(trans, keyDi);
  if (tmp && (style == QString::fromAscii("Wipe") || style == QString::fromAscii("Glitter") || 
              style == QString::fromAscii("Fly") || style == QString::fromAscii("Cover") || 
              style == QString::fromAscii("Uncover") || style == QString::fromAscii("Push"))) {
    if (fz_is_name(tmp)) {
      // TODO: Di == /None
    }
    else
      _transition->setDirection(fz_to_int(tmp));
  }
}

QList<Backend::Page::Box> Page::boxes()
{
  QReadLocker pageLocker(_pageLock);

  QList<Backend::Page::Box> retVal;
  if (!_mupdf_page)
    return retVal;
  
  fz_text_span * textSpan = fz_new_text_span();
  if (!textSpan)
    return retVal;
  
  // Use MuPDF transformations to get the text box coordinates right already
  // during fz_execute_display_list().
  fz_matrix render_trans = fz_identity;
  render_trans = fz_concat(render_trans, fz_translate(0, -_bbox.y1));
  render_trans = fz_concat(render_trans, fz_scale(1, -1));
  render_trans = fz_concat(render_trans, fz_rotate(_rotate));

  fz_device * textDevice = fz_new_text_device(textSpan);
  if (!textDevice) {
    fz_free_text_span(textSpan);
    return retVal;
  }
  fz_execute_display_list(_mupdf_page, textDevice, render_trans, fz_infinite_bbox);
  fz_free_device(textDevice);

  fz_text_span * span = textSpan;
  Backend::Page::Box b;
  while (span) {
    for (int i = 0; i < span->len; ++i) {
      Backend::Page::Box sb;
      sb.boundingBox = toRectF(span->text[i].bbox);
      b.subBoxes << sb;
    }
    if (span->eol) {
      b.boundingBox = QRectF();
      foreach (Backend::Page::Box sb, b.subBoxes)
        b.boundingBox |= sb.boundingBox;
      retVal << b;
      b.subBoxes.clear();
    }
    span = span->next;
  }

  fz_free_text_span(textSpan);
  
  return retVal;
}

inline bool polygonContains(const QPolygonF & poly, const QRectF & rect)
{
  QRectF r = poly.intersected(rect).boundingRect();
  
  return (qAbs(r.width() * r.height() - rect.width() * rect.height()) < 1e-6);
}

QString Page::selectedText(const QList<QPolygonF> & selection, QMap<int, QRectF> * wordBoxes /* = NULL */, QMap<int, QRectF> * charBoxes /* = NULL */)
{
  // FIXME: Implement wordBoxes and charBoxes
  QReadLocker pageLocker(_pageLock);

  QString retVal;
  if (!_mupdf_page)
    return retVal;
  
  fz_text_span * textSpan = fz_new_text_span();
  if (!textSpan)
    return retVal;
  
  fz_device * textDevice = fz_new_text_device(textSpan);
  if (!textDevice) {
    fz_free_text_span(textSpan);
    return retVal;
  }

  // Use MuPDF transformations to get the text box coordinates right already
  // during fz_execute_display_list().
  fz_matrix render_trans = fz_identity;
  render_trans = fz_concat(render_trans, fz_translate(0, -_bbox.y1));
  render_trans = fz_concat(render_trans, fz_scale(1, -1));
  render_trans = fz_concat(render_trans, fz_rotate(_rotate));

  fz_execute_display_list(_mupdf_page, textDevice, render_trans, fz_infinite_bbox);
  fz_free_device(textDevice);

  fz_text_span * span = textSpan;
  Backend::Page::Box b;
  
  while (span) {
    for (int i = 0; i < span->len; ++i) {
      QRectF charRect = toRectF(span->text[i].bbox);
      foreach (QPolygonF poly, selection) {
        if (polygonContains(poly, charRect)) {
          retVal.append(span->text[i].c);
          break;
        }
      }
    }
    if (span->eol)
      retVal.append(QChar::fromAscii('\n'));
    span = span->next;
  }

  fz_free_text_span(textSpan);
  
  return retVal.trimmed();
}

} // namespace MuPDF

} // namespace Backend

} // namespace QtPDF

// vim: set sw=2 ts=2 et

