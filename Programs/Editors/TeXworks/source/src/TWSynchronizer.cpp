/*
  This is part of TeXworks, an environment for working with TeX documents
  Copyright (C) 2014-2019  Stefan Löffler, Jonathan Kew

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

#include "TWSynchronizer.h"
#include "TWApp.h"
#include "TWUtils.h"
#include "TeXDocumentWindow.h"
#include "PDFDocumentWindow.h"

#include <QFileInfo>
#include <QDir>

// TODO for fine-grained search:
// - Specially handle \commands (and possibly other TeX codes)
// - Allow to increase the context to neighboring lines (in case lines were
//   added/removed above since the last typesetting)
// - Implement some diff-like algorithm to make use of the positions of
//   substrings in the line (e.g., to properly sync lines like
//   "abc\footnote{abc}")

TWSyncTeXSynchronizer::TWSyncTeXSynchronizer(const QString & filename)
{
#if defined(MIKTEX_WINDOWS)
  _scanner = SyncTeX::synctex_scanner_new_with_output_file(filename.toUtf8().data(), nullptr, 1);
#else
  _scanner = SyncTeX::synctex_scanner_new_with_output_file(filename.toLocal8Bit().data(), nullptr, 1);
#endif
}

TWSyncTeXSynchronizer::~TWSyncTeXSynchronizer()
{
  if (_scanner)
    synctex_scanner_free(_scanner);
}

bool TWSyncTeXSynchronizer::isValid() const
{
  return (_scanner != nullptr);
}


QString TWSyncTeXSynchronizer::syncTeXFilename() const
{
  if (!_scanner)
    return QString();
#if defined(MIKTEX_WINDOWS)
  return QString::fromUtf8(SyncTeX::synctex_scanner_get_synctex(_scanner));
#else
  return QString::fromLocal8Bit(SyncTeX::synctex_scanner_get_synctex(_scanner));
#endif
}

QString TWSyncTeXSynchronizer::pdfFilename() const
{
  if (!_scanner)
    return QString();
#if defined(MIKTEX_WINDOWS)
  return QString::fromUtf8(SyncTeX::synctex_scanner_get_output(_scanner));
#else
  return QString::fromLocal8Bit(SyncTeX::synctex_scanner_get_output(_scanner));
#endif
}

//virtual
TWSynchronizer::PDFSyncPoint TWSyncTeXSynchronizer::syncFromTeX(const TWSynchronizer::TeXSyncPoint & src, const Resolution resolution) const
{
  PDFSyncPoint retVal;
  retVal.page = -1;

  // Find the name SyncTeX is using for this source file...
  const QFileInfo sourceFileInfo(src.filename);
  QDir curDir(QFileInfo(pdfFilename()).canonicalPath());
  SyncTeX::synctex_node_p node = SyncTeX::synctex_scanner_input(_scanner);
  QString name;
  bool found = false;
  while (node) {
#if defined(MIKTEX_WINDOWS)
    name = QString::fromUtf8(SyncTeX::synctex_scanner_get_name(_scanner, SyncTeX::synctex_node_tag(node)));
#else
    name = QString::fromLocal8Bit(SyncTeX::synctex_scanner_get_name(_scanner, SyncTeX::synctex_node_tag(node)));
#endif
    const QFileInfo fi(curDir, name);
    if (fi == sourceFileInfo) {
      found = true;
      break;
    }
    node = synctex_node_sibling(node);
  }
  if (!found)
    return retVal;

  retVal.filename = pdfFilename();

#if defined(MIKTEX_WINDOWS)
  if (SyncTeX::synctex_display_query(_scanner, name.toUtf8().data(), src.line, src.col, -1) > 0)
  {
#else
  if (SyncTeX::synctex_display_query(_scanner, name.toLocal8Bit().data(), src.line, src.col, -1) > 0) {
#endif
	while ((node = SyncTeX::synctex_scanner_next_result(_scanner))) {
      if (retVal.page < 0)
        retVal.page = SyncTeX::synctex_node_page(node);
      if (SyncTeX::synctex_node_page(node) != retVal.page)
        continue;
      QRectF nodeRect(synctex_node_box_visible_h(node),
                      synctex_node_box_visible_v(node) - synctex_node_box_visible_height(node),
                      synctex_node_box_visible_width(node),
                      synctex_node_box_visible_height(node) + synctex_node_box_visible_depth(node));
      retVal.rects.append(nodeRect);
    }
  }

  // Only perform fine synchronization if requested
  if (resolution != LineResolution)
    _syncFromTeXFine(src, retVal, resolution);

  return retVal;
}

//virtual
TWSynchronizer::TeXSyncPoint TWSyncTeXSynchronizer::syncFromPDF(const TWSynchronizer::PDFSyncPoint & src, const Resolution resolution) const
{
  TeXSyncPoint retVal;
  retVal.line = -1;
  retVal.col = -1;
  retVal.len = -1;

  if (src.rects.length() != 1)
    return retVal;

  if (SyncTeX::synctex_edit_query(_scanner, src.page, static_cast<float>(src.rects[0].left()), static_cast<float>(src.rects[0].top())) > 0) {
    SyncTeX::synctex_node_p node;
	while ((node = SyncTeX::synctex_scanner_next_result(_scanner))) {
#if defined(MIKTEX_WINDOWS)
      retVal.filename = QString::fromUtf8(SyncTeX::synctex_scanner_get_name(_scanner, SyncTeX::synctex_node_tag(node)));
#else
      retVal.filename = QString::fromLocal8Bit(SyncTeX::synctex_scanner_get_name(_scanner, SyncTeX::synctex_node_tag(node)));
#endif
      retVal.line = SyncTeX::synctex_node_line(node);
      if (retVal.line <= 0)
        continue;
      retVal.col = -1;
      retVal.len = -1;

      // If we only need to match lines, we are done
      if (resolution == LineResolution)
        break;

      _syncFromPDFFine(src, retVal, resolution);
      // If we found a (unique) match, we are done; otherwise, try other
      // synctex_edit_query results (if any)
      if (retVal.col > -1 && retVal.len > 0)
        break;
    }
  }

  return retVal;
}

void TWSyncTeXSynchronizer::_syncFromTeXFine(const TWSynchronizer::TeXSyncPoint & src, TWSynchronizer::PDFSyncPoint & dest, const Resolution resolution) const
{
  // FIXME: this does not work properly for text which is split across pages!

  // If we get no sensible column information, there's no point in continuing
  if (src.col < 0)
    return;

  QDir curDir(QFileInfo(src.filename).canonicalPath());
  TeXDocumentWindow * tex = TeXDocumentWindow::findDocument(src.filename);
  PDFDocumentWindow * pdf = PDFDocumentWindow::findDocument(QFileInfo(curDir, dest.filename).canonicalFilePath());
  if (!tex || !pdf || !pdf->widget())
    return;
  QSharedPointer<QtPDF::Backend::Document> pdfDoc = pdf->widget()->document().toStrongRef();
  if (!pdfDoc)
    return;
  QSharedPointer<QtPDF::Backend::Page> pdfPage = pdfDoc->page(dest.page - 1).toStrongRef();
  if (!pdfPage)
    return;

  // Get source context
  QString srcContext = tex->getLineText(src.line);
  if (srcContext.isEmpty())
    return;

  // Get destination context
  QList<QPolygonF> selection;
  foreach (QRectF r, dest.rects)
    selection.append(r);
  QMap<int, QRectF> wordBoxes, charBoxes;
  QString destContext = pdfPage->selectedText(selection, &wordBoxes, &charBoxes);
  // Normalize the destContext. selectedText() returns newline chars between
  // separate (output) lines that all correspond to the same input line
  // (different input lines are handled by SyncTeX). Here we replace those \n
  // to make destContext more comparable to srcContext.
  destContext.replace(QChar::fromLatin1('\n'), QChar::fromLatin1(' '));

  // FIXME: the string returned by selectedText() seems to twist the beginning
  // (and ends) of footnotes sometimes.

  // If the user clicked past the end of the line, start matching at the last
  // character
  int col = src.col;
  if (col >= srcContext.length())
    col = srcContext.length() - 1;

  // Perform the text matching
  bool unique = false;
  int destCol = _findCorrespondingPosition(srcContext, destContext, col, unique);

  // If we found no (unique) match bail out
  if (destCol < 0 || !unique)
    return;

  // Update the matching destination rectangles
  if (resolution == WordResolution) {
    dest.rects.clear();
    dest.rects.append(wordBoxes[destCol]);
  }
  else if (resolution == CharacterResolution){
    dest.rects.clear();
    dest.rects.append(charBoxes[destCol]);
  }
}

void TWSyncTeXSynchronizer::_syncFromPDFFine(const TWSynchronizer::PDFSyncPoint &src, TWSynchronizer::TeXSyncPoint &dest, const Resolution resolution) const
{
  if (dest.filename.isEmpty())
    return;
  QDir curDir(QFileInfo(src.filename).canonicalPath());
  TeXDocumentWindow * tex = TeXDocumentWindow::openDocument(QFileInfo(curDir, dest.filename).canonicalFilePath(), false, false, dest.line);
  PDFDocumentWindow * pdf = PDFDocumentWindow::findDocument(src.filename);
  if (!tex || !pdf || !pdf->widget())
    return;
  QSharedPointer<QtPDF::Backend::Document> pdfDoc = pdf->widget()->document().toStrongRef();
  if (!pdfDoc)
    return;
  QSharedPointer<QtPDF::Backend::Page> pdfPage = pdfDoc->page(src.page - 1).toStrongRef();
  if (!pdfPage)
    return;

  // Get source context
  // In order to get the full context corresponding to the whole input line,
  // we use a forward search from the source to the PDF (which may turn up more
  // than one PDF rect for multiline paragraphs).
  // Note: this still does not help for paragraphs broken across pages
  QList<QPolygonF> selection;
#if defined(MIKTEX_WINDOWS)
  if (SyncTeX::synctex_display_query(_scanner, dest.filename.toUtf8().data(), dest.line, -1, src.page) > 0)
  {
#else
  if (SyncTeX::synctex_display_query(_scanner, dest.filename.toLocal8Bit().data(), dest.line, -1, src.page) > 0) {
#endif
	SyncTeX::synctex_node_p node;
	while ((node = SyncTeX::synctex_scanner_next_result(_scanner))) {
      if (SyncTeX::synctex_node_page(node) != src.page)
        continue;
      QRectF nodeRect(synctex_node_box_visible_h(node),
                      synctex_node_box_visible_v(node) - synctex_node_box_visible_height(node),
                      synctex_node_box_visible_width(node),
                      synctex_node_box_visible_height(node) + synctex_node_box_visible_depth(node));
      selection.append(nodeRect);
    }
  }
  // Find the box the user clicked on
  QMap<int, QRectF> boxes;
  QString srcContext = pdfPage->selectedText(selection, nullptr, &boxes);
  // Normalize the srcContext. selectedText() returns newline chars between
  // separate (output) lines that all correspond to the same input line
  // (different input lines are handled by SyncTeX). Here we replace those \n
  // to make srcContext more comparable to destContext below.
  srcContext.replace(QChar::fromLatin1('\n'), QChar::fromLatin1(' '));

  int col;
  for (col = 0; col < boxes.count(); ++col) {
    if (boxes[col].contains(src.rects[0].center()))
      break;
  }
  // If no valid box was found, bail out
  if (col >= boxes.count())
    return;

  // Get destination context
  QString destContext = tex->getLineText(dest.line);
  if (destContext.isEmpty())
    return;

  // Perform the text matching
  bool unique = false;
  int destCol = _findCorrespondingPosition(srcContext, destContext, col, unique);

  // If we found no (unique) match bail out
  if (destCol < 0 || !unique)
    return;

  // cross-check in the other direction (i.e., from dest to src) to avoid false
  // positives in case there is some crazy command expansion going on. E.g.:
  // \newcommand{\A}{abc}
  // \A abc\A

  if (col != _findCorrespondingPosition(destContext, srcContext, destCol, unique) || !unique)
    return;

  if (resolution == CharacterResolution) {
    dest.col = destCol;
    dest.len = 1;
  }
  else if (resolution == WordResolution) {
    TWUtils::findNextWord(destContext, destCol, dest.col, dest.len);
    dest.len -= dest.col;
    // Always select at least one character
    if (dest.len <= 0)
      dest.len = 1;
  }
}

// static
int TWSyncTeXSynchronizer::_findCorrespondingPosition(const QString & srcContext, const QString & destContext, const int col, bool & unique)
{
  // Find the position in the destination corresponding to the one in the source
  // Do this by enlarging the search string until a unique match is found
  // Start by enlarging to the right (until a unique match is found, no match is
  // found anymore (e.g., because we stumble across some TeX code like a
  // \command or a math delimiter), or the end of the string is reached). Then,
  // repeat the same process to the left.
  int deltaFront = 0, deltaBack;
  bool found = false;
  unique = false;

  // Search to the right
  // FIXME: Possibly use some form of bisectioning
  for (deltaBack = 1; col + deltaBack <= srcContext.length(); ++deltaBack) {
    int c = destContext.count(srcContext.midRef(col - deltaFront, deltaBack + deltaFront));
    found = (c > 0);
    unique = (c == 1);
    if (!found || unique)
      break;
  }
  if (!found) {
    // If the string was not found in this round, it must have been found in the
    // previous one, which must not have been unique (otherwise we'd never have
    // gotten here
    --deltaBack;
    found = true;
    unique = false;
  }
  if (!unique) {
    // Search to the left
    // FIXME: Possibly use some form of bisectioning
    for (deltaFront = 1; deltaFront <= col; ++deltaFront) {
      int c = destContext.count(srcContext.midRef(col - deltaFront, deltaBack + deltaFront));
      found = (c > 0);
      unique = (c == 1);
      if (!found || unique)
        break;
    }
    if (!found) {
      // If the string was not found in this round, it must have been found in the
      // previous one, which must not have been unique (otherwise we'd never have
      // gotten here
      --deltaFront;
      found = true;
      unique = false;
    }
  }
  // If we did not find any match return -1
  if (!found || (deltaBack == 0 && deltaFront == 0))
    return -1;
  return destContext.indexOf(srcContext.midRef(col - deltaFront, deltaBack + deltaFront)) + deltaFront;
}
