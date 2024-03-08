/**
 * Copyright (C) 2023  Stefan Löffler
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
#ifndef PDFDocumentScene_H
#define PDFDocumentScene_H

#include "PDFPageLayout.h"

#include <QFileSystemWatcher>
#include <QGraphicsScene>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

namespace QtPDF {

class PDFAction;
class PDFActionEvent;
class PDFPageGraphicsItem;

namespace Backend {
class Document;
} // namespace Backend

class PDFDocumentScene : public QGraphicsScene
{
  Q_OBJECT
  typedef QGraphicsScene Super;
public:
  using size_type = QList<QGraphicsItem*>::size_type;

  PDFDocumentScene(QSharedPointer<Backend::Document> a_doc, QObject *parent = nullptr, const double dpiX = -1, const double dpiY = -1);
  ~PDFDocumentScene() override;

  QWeakPointer<Backend::Document> document();
  QList<QGraphicsItem*> pages();
  QList<QGraphicsItem*> pages(const QPolygonF &polygon);
  QGraphicsItem* pageAt(const size_type idx) const;
  QGraphicsItem* pageAt(const QPointF &pt) const;
  size_type pageNumAt(const QPolygonF &polygon);
  size_type pageNumAt(const QPointF &pt);
  size_type pageNumFor(const PDFPageGraphicsItem * const graphicsItem) const;
  PDFPageLayout& pageLayout() { return _pageLayout; }

  void showOnePage(const size_type pageIdx);
  void showOnePage(const PDFPageGraphicsItem * page);
  void showAllPages();

  bool watchForDocumentChangesOnDisk() const { return _fileWatcher.files().size() > 0; }
  void setWatchForDocumentChangesOnDisk(const bool doWatch = true);

  size_type lastPage();

  const QWeakPointer<Backend::Document> document() const { return _doc.toWeakRef(); }

  void setResolution(const double dpiX, const double dpiY);

  static bool isPageItem(const QGraphicsItem * item);

signals:
  void pageChangeRequested(QtPDF::PDFDocumentScene::size_type pageNum);
  void pageLayoutChanged();
  void pdfActionTriggered(const QtPDF::PDFAction * action);
  void documentChanged(const QWeakPointer<QtPDF::Backend::Document> doc);

public slots:
  void doUnlockDialog();
  void retranslateUi();
  void reloadDocument();

protected slots:
  void pageLayoutChanged(const QRectF& sceneRect);
  void reinitializeScene();
  void finishUnlock();

protected:
  // Used in non-continuous mode to keep track of currently shown page across
  // reloads. -2 is used in continuous mode. -1 indicates an invalid value.
  size_type _shownPageIdx;
  bool event(QEvent * event) override;

  QWidget * _unlockWidget;
  QLabel * _unlockWidgetLockText, * _unlockWidgetLockIcon;
  QPushButton * _unlockWidgetUnlockButton;
  QGraphicsProxyWidget * _unlockProxy;

private:
  const QSharedPointer<Backend::Document> _doc;

  // This may change to a `QSet` in the future
  QList<QGraphicsItem*> _pages;
  size_type _lastPage;
  PDFPageLayout _pageLayout;
  QFileSystemWatcher _fileWatcher;
  QTimer _reloadTimer;
  double _dpiX, _dpiY;

  void handleActionEvent(const PDFActionEvent * action_event);

  // Parent has no copy constructor, so this class shouldn't either. Also, we
  // hold some information in an `auto_ptr` which does interesting things on
  // copy that C++ newbies may not expect.
  Q_DISABLE_COPY(PDFDocumentScene)
};

} // namespace QtPDF

#endif // !defined(PDFDocumentScene_H)
