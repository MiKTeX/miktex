/**
 * Copyright (C) 2013-2020  Stefan Löffler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
#ifndef PDFDOCUMENTWIDGET_H
#define PDFDOCUMENTWIDGET_H

#include "PDFDocumentView.h"

namespace QtPDF {

class PDFDocumentWidget : public PDFDocumentView
{
	Q_OBJECT
public:
  PDFDocumentWidget(QWidget * parent = nullptr, const double dpi = -1);

  bool load(const QString & filename);

  QWeakPointer<Backend::Document> document() const;

  bool watchForDocumentChangesOnDisk() const {
    if (_scene) return _scene->watchForDocumentChangesOnDisk();
    else return false;
  }
  void setWatchForDocumentChangesOnDisk(const bool doWatch = true) { if (_scene) _scene->setWatchForDocumentChangesOnDisk(doWatch); }

  void setResolution(const double dpi);

protected:
  QSharedPointer<QtPDF::PDFDocumentScene> _scene;
  double _dpi;
};

} // namespace QtPDF

#endif // PDFDOCUMENTWIDGET_H
