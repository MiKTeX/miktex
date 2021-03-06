/*
  This is part of TeXworks, an environment for working with TeX documents
  Copyright (C) 2014-2020  Stefan Löffler, Jonathan Kew

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  For links to further information, or to contact the authors,
  see <http://www.tug.org/texworks/>.
*/

#ifndef TW_SYNCHRONIZER_H
#define TW_SYNCHRONIZER_H

#include "document/TeXDocument.h"
#include "../modules/QtPDF/src/PDFBackend.h"

#include <QList>
#include <QRectF>
#include <QString>
#include <functional>

namespace SyncTeX {
  #include <synctex_parser.h>
}

class TWSynchronizer
{
public:
  enum Resolution { CharacterResolution, WordResolution, LineResolution };

  static const Resolution kDefault_Resolution_ToTeX = CharacterResolution;
  static const Resolution kDefault_Resolution_ToPDF = WordResolution;

  struct TeXSyncPoint {
    QString filename;
    int line;
    int col;
    int len;

    bool operator==(const TeXSyncPoint & o) const {
      return (filename == o.filename && line == o.line && col == o.col && len == o.len);
    }
  };
  struct PDFSyncPoint {
    QString filename;
    int page;
    QList<QRectF> rects;

    bool operator==(const PDFSyncPoint & o) const {
      if (filename != o.filename || page != o.page || rects.size() != o.rects.size()) {
        return false;
      }
      // Explicitly use float qFuzzyCompare as SyncTeX internally uses float
      // which may not be converted to double in the same way on all platforms
      // (issues have occured with MXE compilations for Windows with GCC 5.5)
      for (int i = 0; i < rects.size(); ++i) {
        const QRectF & a = rects[i];
        const QRectF & b = o.rects[i];
        if (!qFuzzyCompare(static_cast<float>(a.top()), static_cast<float>(b.top()))) return false;
        if (!qFuzzyCompare(static_cast<float>(a.left()), static_cast<float>(b.left()))) return false;
        if (!qFuzzyCompare(static_cast<float>(a.width()), static_cast<float>(b.width()))) return false;
        if (!qFuzzyCompare(static_cast<float>(a.height()), static_cast<float>(b.height()))) return false;
      }
      return true;
    }
  };

  TWSynchronizer() = default;
  virtual ~TWSynchronizer() = default;
  virtual PDFSyncPoint syncFromTeX(const TeXSyncPoint & src, const Resolution resolution) const = 0;
  virtual TeXSyncPoint syncFromPDF(const PDFSyncPoint & src, const Resolution resolution) const = 0;
};


class TWSyncTeXSynchronizer : public TWSynchronizer
{
public:
  using TeXLoader = std::function<const Tw::Document::TeXDocument*(const QString &)>;
  using PDFLoader = std::function<const QSharedPointer<QtPDF::Backend::Document>(const QString &)>;

  explicit TWSyncTeXSynchronizer(const QString & filename, TeXLoader texLoader, PDFLoader pdfLoader);
  ~TWSyncTeXSynchronizer() override;

  bool isValid() const;

  QString syncTeXFilename() const;
  QString pdfFilename() const;

  PDFSyncPoint syncFromTeX(const TeXSyncPoint & src, const Resolution resolution) const override;
  TeXSyncPoint syncFromPDF(const PDFSyncPoint & src, const Resolution resolution) const override;

protected:
  void _syncFromTeXFine(const TeXSyncPoint & src, PDFSyncPoint & dest, const Resolution resolution) const;
  void _syncFromPDFFine(const PDFSyncPoint & src, TeXSyncPoint & dest, const Resolution resolution) const;

  static int _findCorrespondingPosition(const QString & srcContext, const QString & destContext, const int col, bool & unique);

  SyncTeX::synctex_scanner_p _scanner;
  TeXLoader m_TeXLoader;
  PDFLoader m_PDFLoader;
};

#endif // !defined(TW_SYNCHRONIZER_H)
