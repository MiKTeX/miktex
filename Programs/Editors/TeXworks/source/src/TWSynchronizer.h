/*
  This is part of TeXworks, an environment for working with TeX documents
  Copyright (C) 2014-2018  Stefan Löffler, Jonathan Kew

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

#include <QString>
#include <QList>
#include <QRectF>


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
  };
  struct PDFSyncPoint {
    QString filename;
    int page;
    QList<QRectF> rects;
  };

  TWSynchronizer() { }
  virtual ~TWSynchronizer() { }
  virtual PDFSyncPoint syncFromTeX(const TeXSyncPoint & src, const Resolution resolution) const = 0;
  virtual TeXSyncPoint syncFromPDF(const PDFSyncPoint & src, const Resolution resolution) const = 0;
};


class TWSyncTeXSynchronizer : public TWSynchronizer
{
public:
  TWSyncTeXSynchronizer(const QString & filename);
  virtual ~TWSyncTeXSynchronizer();

  bool isValid() const;

  QString syncTeXFilename() const;
  QString pdfFilename() const;

  virtual PDFSyncPoint syncFromTeX(const TeXSyncPoint & src, const Resolution resolution) const;
  virtual TeXSyncPoint syncFromPDF(const PDFSyncPoint & src, const Resolution resolution) const;

protected:
  void _syncFromTeXFine(const TeXSyncPoint & src, PDFSyncPoint & dest, const Resolution resolution) const;
  void _syncFromPDFFine(const PDFSyncPoint & src, TeXSyncPoint & dest, const Resolution resolution) const;

  static int _findCorrespondingPosition(const QString & srcContext, const QString & destContext, const int col, bool & unique);

  SyncTeX::synctex_scanner_p _scanner;
};

#endif // !defined(TW_SYNCHRONIZER_H)
