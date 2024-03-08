/**
 * Copyright (C) 2023-2024  Stefan Löffler
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
#include "PDFDocumentScene.h"

#include "PDFDocumentView.h"

#include <QtConcurrent>

namespace QtPDF {

// PDFDocumentScene
// ================
//
// A large canvas that manages the layout of QGraphicsItem subclasses. The
// primary items we are concerned with are PDFPageGraphicsItem and
// PDFLinkGraphicsItem.
PDFDocumentScene::PDFDocumentScene(QSharedPointer<Backend::Document> a_doc, QObject *parent /* = nullptr */, const double dpiX /* = -1 */, const double dpiY /* = -1 */):
    Super(parent),
    _shownPageIdx(-2),
    _doc(a_doc),
    _lastPage(-1)
{
  Q_ASSERT(a_doc != nullptr);
  // We need to register a QList<PDFLinkGraphicsItem *> meta-type so we can
  // pass it through inter-thread (i.e., queued) connections
  qRegisterMetaType< QList<PDFLinkGraphicsItem *> >();

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  _dpiX = (dpiX > 0 ? dpiX : QApplication::desktop()->physicalDpiX());
  _dpiY = (dpiY > 0 ? dpiY : QApplication::desktop()->physicalDpiY());
#else
  // FIXME: The QGraphicsScene should be independent of the hardware it is shown
  // on
  const QList<QGraphicsView *> & v = views();
  if (dpiX > 0)
    _dpiX = dpiX;
  else if (!v.isEmpty())
    _dpiX = v.first()->screen()->physicalDotsPerInchX();
  else
    _dpiX = 72;

  if (dpiY > 0)
    _dpiY = dpiY;
  else if (!v.isEmpty())
    _dpiY = v.first()->screen()->physicalDotsPerInchY();
  else
    _dpiY = 72;
#endif

  connect(&_pageLayout, &PDFPageLayout::layoutChanged, this, static_cast<void (PDFDocumentScene::*)(const QRectF&)>(&PDFDocumentScene::pageLayoutChanged));

  // Initialize the unlock widget
  {
    _unlockWidget = new QWidget();
    QVBoxLayout * layout = new QVBoxLayout();

    _unlockWidgetLockIcon = new QLabel(_unlockWidget);
    _unlockWidgetLockIcon->setPixmap(QIcon::fromTheme(QStringLiteral("status-locked")).pixmap(48));
    _unlockWidgetLockText = new QLabel(_unlockWidget);
    _unlockWidgetUnlockButton = new QPushButton(_unlockWidget);

    connect(_unlockWidgetUnlockButton, &QPushButton::clicked, this, &PDFDocumentScene::doUnlockDialog);

    layout->addWidget(_unlockWidgetLockIcon);
    layout->addWidget(_unlockWidgetLockText);
    layout->addSpacing(20);
    layout->addWidget(_unlockWidgetUnlockButton);

    layout->setAlignment(_unlockWidgetLockIcon, Qt::AlignHCenter);
    layout->setAlignment(_unlockWidgetLockText, Qt::AlignHCenter);
    layout->setAlignment(_unlockWidgetUnlockButton, Qt::AlignHCenter);

    _unlockWidget->setLayout(layout);
    _unlockProxy = new QGraphicsProxyWidget();
    _unlockProxy->setWidget(_unlockWidget);
    retranslateUi();
  }

  // We must not respond to a QFileSystemWatcher::timeout() signal directly as
  // file operations need not be atomic. I.e., QFileSystemWatcher could fire
  // several times between the begging of a change to the file and its
  // completion. Hence, we use a timer to delay the call to reloadDocument(). If
  // the QFileSystemWatcher fires several times, the timer gets restarted every
  // time.
  _reloadTimer.setSingleShot(true);
  _reloadTimer.setInterval(500);
  connect(&_reloadTimer, &QTimer::timeout, this, &PDFDocumentScene::reloadDocument);
  connect(&_fileWatcher, &QFileSystemWatcher::fileChanged, &_reloadTimer, static_cast<void (QTimer::*)()>(&QTimer::start));
  setWatchForDocumentChangesOnDisk(true);

  reinitializeScene();
}

PDFDocumentScene::~PDFDocumentScene()
{
  // Destroy the _unlockProxy if it is not currently attached to the scene (in
  // which case it is destroyed automatically)
  if (!_unlockProxy->scene()) {
    delete _unlockProxy;
    _unlockProxy = nullptr;
  }
}

void PDFDocumentScene::handleActionEvent(const PDFActionEvent * action_event)
{
  if (!action_event || !action_event->action)
    return;

  switch (action_event->action->type() )
  {
  // Link types that we don't handle here but that may be of interest
  // elsewhere (note: ActionGoto will be handled by
  // PDFDocumentView::pdfActionTriggered)
  case PDFAction::ActionTypeGoTo:
  case PDFAction::ActionTypeURI:
  case PDFAction::ActionTypeLaunch:
  case PDFAction::ActionTypeSetOCGState:
    break;
  default:
    // All other link types are currently not supported
    return;
  }
  // Translate into a signal that can be handled by some other part of the
  // program, such as a `PDFDocumentView`.
  emit pdfActionTriggered(action_event->action);
}


// Accessors
// ---------

QWeakPointer<Backend::Document> PDFDocumentScene::document() { return _doc.toWeakRef(); }
QList<QGraphicsItem*> PDFDocumentScene::pages() { return _pages; }

// Overloaded method that returns all page objects inside a given rectangular
// area. First, `items` is used to grab all items inside the rectangle. This
// list is then filtered by item type so that it contains only references to
// `PDFPageGraphicsItem` objects.
QList<QGraphicsItem*> PDFDocumentScene::pages(const QPolygonF &polygon)
{
  QList<QGraphicsItem*> pageList = items(polygon);
  QtConcurrent::blockingFilter(pageList, isPageItem);

  return pageList;
}

// Convenience function to avoid moving the complete list of pages around
// between functions if only one page is needed
QGraphicsItem* PDFDocumentScene::pageAt(const size_type idx) const
{
  if (idx < 0 || idx >= _pages.size())
    return nullptr;
  return _pages[idx];
}

// Overloaded method that returns all page objects at a given point. First,
// `items` is used to grab all items at the point. This list is then filtered by
// item type so that it contains only references to `PDFPageGraphicsItem` objects.
QGraphicsItem* PDFDocumentScene::pageAt(const QPointF &pt) const
{
  QList<QGraphicsItem*> pageList = items(pt);
  QtConcurrent::blockingFilter(pageList, isPageItem);

  if (pageList.isEmpty())
    return nullptr;
  return pageList[0];
}

// This is a convenience function for returning the page number of the first
// page item inside a given area of the scene. If no page is in the specified
// area, -1 is returned.
PDFDocumentScene::size_type PDFDocumentScene::pageNumAt(const QPolygonF &polygon)
{
  QList<QGraphicsItem*> p(pages(polygon));
  if (p.isEmpty())
    return -1;
  return static_cast<int>(_pages.indexOf(p.first()));
}

// This is a convenience function for returning the page number of the first
// page item at a given point. If no page is in the specified area, -1 is returned.
PDFDocumentScene::size_type PDFDocumentScene::pageNumAt(const QPointF &pt)
{
  return static_cast<int>(_pages.indexOf(pageAt(pt)));
}

PDFDocumentScene::size_type PDFDocumentScene::pageNumFor(const PDFPageGraphicsItem * const graphicsItem) const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  // Note: since we store QGraphicsItem* in _pages, we need to remove the const
  // or else indexOf() complains during compilation. Since we don't do anything
  // with the pointer, this should be safe to do while still remaining the
  // const'ness of `graphicsItem`, however.
  return _pages.indexOf(const_cast<PDFPageGraphicsItem *>(graphicsItem));
#else
  return _pages.indexOf(graphicsItem);
#endif
}

PDFDocumentScene::size_type PDFDocumentScene::lastPage() { return _lastPage; }

// Event Handlers
// --------------

// We re-implement the main event handler for the scene so that we can
// translate events generated by child items into signals that can be sent out
// to the rest of the program.
bool PDFDocumentScene::event(QEvent *event)
{
  if ( event->type() == PDFActionEvent::ActionEvent )
  {
    event->accept();
    // Cast to a pointer for `PDFActionEvent` so that we can access the `pageNum`
    // field.
    const PDFActionEvent *action_event = dynamic_cast<const PDFActionEvent*>(event);
    handleActionEvent(action_event);
    return true;
  }

  return Super::event(event);
}

// Public Slots
// --------------
void PDFDocumentScene::doUnlockDialog()
{
  Q_ASSERT(!_doc.isNull());

  bool ok{false};
  // TODO: Maybe use some parent for QInputDialog (and QMessageBox below)
  // instead of nullptr?
  QString password = QInputDialog::getText(nullptr, tr("Unlock PDF"), tr("Please enter the password to unlock the PDF"), QLineEdit::Password, QString(), &ok);
  if (ok) {
    if (_doc->unlock(password)) {
      // FIXME: the program crashes in the QGraphicsView::mouseReleaseEvent
      // handler (presumably from clicking the "Unlock" button) when
      // reinitializeScene() is called immediately. To work around this, delay
      // it until control returns to the event queue. Problem: slots connected
      // to documentChanged() will receive the new doc, but the scene itself
      // will not have changed, yet.
#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
      QTimer::singleShot(1, this, SLOT(finishUnlock()));
#else
      QTimer::singleShot(1, this, &PDFDocumentScene::finishUnlock);
#endif
    }
    else
      QMessageBox::information(nullptr, tr("Incorrect password"), tr("The password you entered was incorrect."));
  }
}

void PDFDocumentScene::retranslateUi()
{
  _unlockWidgetLockText->setText(tr("This document is locked. You need a password to open it."));
  _unlockWidgetUnlockButton->setText(tr("Unlock"));

  foreach (QGraphicsItem * i, items()) {
    if (!i)
      continue;
    switch (i->type()) {
    case PDFLinkGraphicsItem::Type:
    {
      PDFLinkGraphicsItem * gi = dynamic_cast<PDFLinkGraphicsItem*>(i);
      gi->retranslateUi();
    }
    break;
    default:
      break;
    }
  }
}

// Protected Slots
// --------------
void PDFDocumentScene::pageLayoutChanged(const QRectF& sceneRect)
{
  setSceneRect(sceneRect);
  emit pageLayoutChanged();
}

void PDFDocumentScene::reinitializeScene()
{
  // Remove the _unlockProxy from the scene (if applicable) to avoid it being
  // destroyed automatically by the subsequent call to clear()
  if (_unlockProxy->scene() == this)
    removeItem(_unlockProxy);
  clear();
  _pages.clear();
  _pageLayout.clearPages();

  _lastPage = _doc->numPages();
  if (!_doc->isValid())
    return;
  if (_doc->isLocked()) {
    // FIXME: Deactivate "normal" user interaction, e.g., zooming, panning, etc.
    addItem(_unlockProxy);
    setSceneRect(QRectF());
  }
  else {
    // Create a `PDFPageGraphicsItem` for each page in the PDF document and let
    // them be layed out by a `PDFPageLayout` instance.
    if (_shownPageIdx >= _lastPage)
      _shownPageIdx = _lastPage - 1;

    for (size_type i = 0; i < _lastPage; ++i)
    {
      PDFPageGraphicsItem * pagePtr = new PDFPageGraphicsItem(_doc->page(i), _dpiX, _dpiY);
      pagePtr->setVisible(i == _shownPageIdx || _shownPageIdx == -2);
      _pages.append(pagePtr);
      addItem(pagePtr);
      _pageLayout.addPage(pagePtr);
    }
    _pageLayout.relayout();
  }
}

void PDFDocumentScene::finishUnlock()
{
  reinitializeScene();
  emit documentChanged(_doc);
}

void PDFDocumentScene::reloadDocument()
{
  // If the file referenced by the document no longer exists, do nothing
  if(!QFile::exists(_doc->fileName()))
    return;

  _doc->reload();
  reinitializeScene();
  emit documentChanged(_doc.toWeakRef());
}


// Other
// -----
void PDFDocumentScene::showOnePage(const size_type pageIdx)
{
  for (size_type i = 0; i < _pages.size(); ++i) {
    if (!isPageItem(_pages[i]))
      continue;
    if (i == pageIdx) {
      _pages[i]->setVisible(true);
      _shownPageIdx = pageIdx;
    }
    else
      _pages[i]->setVisible(false);
  }
}

void PDFDocumentScene::showOnePage(const PDFPageGraphicsItem * page)
{
  for (size_type i = 0; i < _pages.size(); ++i) {
    if (!isPageItem(_pages[i]))
      continue;
    _pages[i]->setVisible(_pages[i] == page);
    if (_pages[i] == page) {
      _pages[i]->setVisible(true);
      _shownPageIdx = i;
    }
    else
      _pages[i]->setVisible(false);
  }
}

void PDFDocumentScene::showAllPages()
{
  for (size_type i = 0; i < _pages.size(); ++i) {
    if (!isPageItem(_pages[i]))
      continue;
    _pages[i]->setVisible(true);
  }
  _shownPageIdx = -2;
}

void PDFDocumentScene::setWatchForDocumentChangesOnDisk(const bool doWatch /* = true */)
{
  if (!_fileWatcher.files().empty())
    _fileWatcher.removePaths(_fileWatcher.files());
  if (doWatch) {
    _fileWatcher.addPath(_doc->fileName());
#ifdef DEBUG
    qDebug() << "Watching" << _doc->fileName();
#endif
  }
}

void PDFDocumentScene::setResolution(const double dpiX, const double dpiY)
{
  if (dpiX > 0)
    _dpiX = dpiX;
  if (dpiY > 0)
    _dpiY = dpiY;

  // FIXME: reinitializing everything seems like overkill
  reinitializeScene();
}

bool PDFDocumentScene::isPageItem(const QGraphicsItem * item)
{
  return (item->type() == PDFPageGraphicsItem::Type);
}

} // namespace QtPDF
