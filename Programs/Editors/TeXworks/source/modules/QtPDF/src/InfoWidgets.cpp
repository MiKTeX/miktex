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
#include "InfoWidgets.h"

#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QListView>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QtConcurrent>

#include "PaperSizes.h"
#include "PDFBackend.h"
#include "PDFDocumentView.h"

namespace QtPDF {

// PDFDocumentInfoWidget
// =====================

void PDFDocumentInfoWidget::setWindowTitle(const QString & windowTitle)
{
  QWidget::setWindowTitle(windowTitle);
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
  emit windowTitleChanged(windowTitle);
#endif
}

void PDFDocumentInfoWidget::changeEvent(QEvent * event)
{
  if (event && event->type() == QEvent::LanguageChange)
    retranslateUi();
  QWidget::changeEvent(event);
}


// PDFToCInfoWidget
// ============

PDFToCInfoWidget::PDFToCInfoWidget(QWidget * parent) :
    PDFDocumentInfoWidget(parent, PDFDocumentView::tr("Table of Contents"), QString::fromLatin1("QtPDF.ToCInfoWidget"))
{
  QVBoxLayout * layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  _tree = new QTreeWidget(this);
  _tree->setAlternatingRowColors(true);
  _tree->setHeaderHidden(true);
  _tree->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  connect(_tree, &QTreeWidget::itemSelectionChanged, this, &PDFToCInfoWidget::itemSelectionChanged);

  layout->addWidget(_tree);
  setLayout(layout);
}

void PDFToCInfoWidget::retranslateUi()
{
  setWindowTitle(PDFDocumentView::tr("Table of Contents"));
}

PDFToCInfoWidget::~PDFToCInfoWidget()
{
  clear();
}

void PDFToCInfoWidget::initFromDocument(const QWeakPointer<Backend::Document> newDoc)
{
  Q_ASSERT(_tree != nullptr);

  PDFDocumentInfoWidget::initFromDocument(newDoc);

  clear();
  QSharedPointer<Backend::Document> doc(newDoc.toStrongRef());
  if (doc) {
    const Backend::PDFToC tocData = doc->toc();
    recursiveAddTreeItems(tocData, _tree->invisibleRootItem());
  }
}

void PDFToCInfoWidget::clear()
{
  Q_ASSERT(_tree != nullptr);
  // make sure that no item is (and can be) selected while we clear the tree
  // (otherwise clearing it could trigger (numerous) itemSelectionChanged signals)
  _tree->setSelectionMode(QAbstractItemView::NoSelection);
  recursiveClearTreeItems(_tree->invisibleRootItem());
  _tree->setSelectionMode(QAbstractItemView::SingleSelection);
}

void PDFToCInfoWidget::itemSelectionChanged()
{
  Q_ASSERT(_tree != nullptr);
  // Since the ToC QTreeWidget is in single selection mode, we can only get zero
  // or one selected item(s)

  QList<QTreeWidgetItem *> selectedItems = _tree->selectedItems();
  if (selectedItems.count() == 0)
    return;
  QTreeWidgetItem * item = selectedItems.first();
  Q_ASSERT(item != nullptr);
  // TODO: It might be better to register PDFAction with the QMetaType framework
  // instead of doing casts with (void*).
  PDFAction * action = reinterpret_cast<PDFAction*>(item->data(0, Qt::UserRole).value<void*>());
  if (action)
    emit actionTriggered(action);
}

//static
void PDFToCInfoWidget::recursiveAddTreeItems(const QList<Backend::PDFToCItem> & tocItems, QTreeWidgetItem * parentTreeItem)
{
  foreach (const Backend::PDFToCItem & tocItem, tocItems) {
    QTreeWidgetItem * treeItem = new QTreeWidgetItem(parentTreeItem, QStringList(tocItem.label()));
    treeItem->setForeground(0, tocItem.color());
    if (tocItem.flags()) {
      QFont font = treeItem->font(0);
      font.setBold(tocItem.flags().testFlag(Backend::PDFToCItem::Flag_Bold));
      font.setItalic(tocItem.flags().testFlag(Backend::PDFToCItem::Flag_Bold));
      treeItem->setFont(0, font);
    }
    treeItem->setExpanded(tocItem.isOpen());
    // TODO: It might be better to register PDFAction via QMetaType to avoid
    // having to use (void*).
    if (tocItem.action())
      treeItem->setData(0, Qt::UserRole, QVariant::fromValue(reinterpret_cast<void*>(tocItem.action()->clone())));
    // TODO: maybe display page numbers in col 2?

    if (!tocItem.children().isEmpty())
      recursiveAddTreeItems(tocItem.children(), treeItem);
  }
}

//static
void PDFToCInfoWidget::recursiveClearTreeItems(QTreeWidgetItem * parent)
{
  Q_ASSERT(parent != nullptr);
  while (parent->childCount() > 0) {
    QTreeWidgetItem * item = parent->child(0);
    recursiveClearTreeItems(item);
    PDFAction * action = static_cast<PDFAction*>(item->data(0, Qt::UserRole).value<void*>());
    delete action;
    parent->removeChild(item);
    delete item;
  }
}


// PDFMetaDataInfoWidget
// ============
PDFMetaDataInfoWidget::PDFMetaDataInfoWidget(QWidget * parent) :
    PDFDocumentInfoWidget(parent, PDFDocumentView::tr("Meta Data"), QString::fromLatin1("QtPDF.MetaDataInfoWidget"))
{
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  // scrollArea ... the central widget of the QDockWidget
  // w ... the central widget of scrollArea
  // groupBox ... one (of many) group box in w
  // vLayout ... lays out the group boxes in w
  // layout ... lays out the actual data widgets in groupBox
  QVBoxLayout * vLayout = new QVBoxLayout(this);

  // We want the vLayout to set the size of w (which should encompass all child
  // widgets completely, since we in turn put it into scrollArea to handle
  // oversized children
  vLayout->setSizeConstraint(QLayout::SetFixedSize);
  // Set margins to 0 as space is very limited in the sidebar
  vLayout->setContentsMargins(0, 0, 0, 0);

  // NOTE: The labels are initialized in retranslteUi() below
  // The "Document" group box
  _documentGroup = new QGroupBox(this);
  QFormLayout * layout = new QFormLayout(_documentGroup);

  _titleLabel = new QLabel(_documentGroup);
  _title = new QLabel(_documentGroup);
  _title->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_titleLabel, _title);

  _authorLabel = new QLabel(_documentGroup);
  _author = new QLabel(_documentGroup);
  _author->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_authorLabel, _author);

  _subjectLabel = new QLabel(_documentGroup);
  _subject = new QLabel(_documentGroup);
  _subject->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_subjectLabel, _subject);

  _keywordsLabel = new QLabel(_documentGroup);
  _keywords = new QLabel(_documentGroup);
  _keywords->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_keywordsLabel, _keywords);

  _pageSizeLabel = new QLabel(_documentGroup);
  _pageSize = new QLabel(_documentGroup);
  _pageSize->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_pageSizeLabel, _pageSize);

  _fileSizeLabel = new QLabel(_documentGroup);
  _fileSize = new QLabel(_documentGroup);
  _fileSize->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_fileSizeLabel, _fileSize);

  _documentGroup->setLayout(layout);
  vLayout->addWidget(_documentGroup);

  // The "Processing" group box
  _processingGroup = new QGroupBox(PDFDocumentView::tr("Processing"), this);
  layout = new QFormLayout(_processingGroup);

  _creatorLabel = new QLabel(_processingGroup);
  _creator = new QLabel(_processingGroup);
  _creator->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_creatorLabel, _creator);

  _producerLabel = new QLabel(_processingGroup);
  _producer = new QLabel(_processingGroup);
  _producer->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_producerLabel, _producer);

  _creationDateLabel = new QLabel(_processingGroup);
  _creationDate = new QLabel(_processingGroup);
  _creationDate->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_creationDateLabel, _creationDate);

  _modDateLabel = new QLabel(_processingGroup);
  _modDate = new QLabel(_processingGroup);
  _modDate->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_modDateLabel, _modDate);

  _trappedLabel = new QLabel(_processingGroup);
  _trapped = new QLabel(_processingGroup);
  _trapped->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
  layout->addRow(_trappedLabel, _trapped);

  _processingGroup->setLayout(layout);
  vLayout->addWidget(_processingGroup);

  // The "Other" group box
  _otherGroup = new QGroupBox(PDFDocumentView::tr("Other"), this);
  layout = new QFormLayout(_otherGroup);
  // Hide the "Other" group box unless it has something to display
  _otherGroup->setVisible(false);
  // Note: Items are added to the "Other" box dynamically in
  // initFromDocument()

  _otherGroup->setLayout(layout);
  vLayout->addWidget(_otherGroup);

  setLayout(vLayout);
  retranslateUi();
}

void PDFMetaDataInfoWidget::initFromDocument(const QWeakPointer<Backend::Document> doc)
{
  PDFDocumentInfoWidget::initFromDocument(doc);
  reload();
}

void PDFMetaDataInfoWidget::reload()
{
  QLocale locale;
  QStringList sizeUnits;
  //: File size: bytes
  sizeUnits << PDFDocumentView::tr("B");
  //: File size: kilobytes
  sizeUnits << PDFDocumentView::tr("kB");
  //: File size: megabytes
  sizeUnits << PDFDocumentView::tr("MB");
  //: File size: gigabytes
  sizeUnits << PDFDocumentView::tr("GB");
  //: File size: terabytes
  sizeUnits << PDFDocumentView::tr("TB");

  QSharedPointer<Backend::Document> doc(_doc.toStrongRef());
  if (!doc) {
    clear();
    return;
  }
  _title->setText(doc->title());
  _author->setText(doc->author());
  _subject->setText(doc->subject());
  _keywords->setText(doc->keywords());

  // Convert the file size to human-readable form
  double fileSize = static_cast<double>(doc->fileSize());
  int iUnit{0};
  for (iUnit = 0; iUnit < sizeUnits.size() && fileSize >= 1000.; ++iUnit)
    fileSize /= 1000.;
  if (iUnit == 0)
    _fileSize->setText(QString::fromLatin1("%1 %2").arg(doc->fileSize()).arg(sizeUnits[0]));
  else
    _fileSize->setText(QString::fromLatin1("%1 %2").arg(fileSize, 0, 'f', 1).arg(sizeUnits[iUnit]));

  _pageSize->setText(doc->pageSize().isValid() ? PaperSize::findForPDFSize(doc->pageSize()).label() : QString());

  _creator->setText(doc->creator());
  _producer->setText(doc->producer());
  _creationDate->setText(locale.toString(doc->creationDate(), QLocale::LongFormat));
  _modDate->setText(locale.toString(doc->modDate(), QLocale::LongFormat));
  switch (doc->trapped()) {
  case Backend::Document::Trapped_True:
    _trapped->setText(PDFDocumentView::tr("Yes"));
    break;
  case Backend::Document::Trapped_False:
    _trapped->setText(PDFDocumentView::tr("No"));
    break;
  default:
    _trapped->setText(PDFDocumentView::tr("Unknown"));
    break;
  }
  QFormLayout * layout = qobject_cast<QFormLayout*>(_otherGroup->layout());
  Q_ASSERT(layout != nullptr);

  // Remove any items there may be
  while (layout->count() > 0) {
    QLayoutItem * child = layout->takeAt(0);
    if (child) {
      if (child->widget())
        child->widget()->deleteLater();
      delete child;
    }
  }

  QMap<QString, QString>::const_iterator it;
  for (it = doc->metaDataOther().constBegin(); it != doc->metaDataOther().constEnd(); ++it) {
    QLabel * l = new QLabel(it.value(), _otherGroup);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    layout->addRow(it.key(), l);
  }
  // Hide the "Other" group box unless it has something to display
  _otherGroup->setVisible(layout->count() > 0);
}

void PDFMetaDataInfoWidget::clear()
{
  _title->setText(QString());
  _author->setText(QString());
  _subject->setText(QString());
  _keywords->setText(QString());
  _fileSize->setText(QString());
  _creator->setText(QString());
  _producer->setText(QString());
  _creationDate->setText(QString());
  _modDate->setText(QString());
  _trapped->setText(PDFDocumentView::tr("Unknown"));
  QFormLayout * layout = qobject_cast<QFormLayout*>(_otherGroup->layout());
  Q_ASSERT(layout != nullptr);

  // Remove any items there may be
  while (layout->count() > 0) {
    QLayoutItem * child = layout->takeAt(0);
    delete child;
  }
}

void PDFMetaDataInfoWidget::retranslateUi()
{
  setWindowTitle(PDFDocumentView::tr("Meta Data"));

  _documentGroup->setTitle(PDFDocumentView::tr("Document"));
  _titleLabel->setText(PDFDocumentView::tr("Title:"));
  _authorLabel->setText(PDFDocumentView::tr("Author:"));
  _subjectLabel->setText(PDFDocumentView::tr("Subject:"));
  _keywordsLabel->setText(PDFDocumentView::tr("Keywords:"));
  _pageSizeLabel->setText(PDFDocumentView::tr("Page size:"));
  _fileSizeLabel->setText(PDFDocumentView::tr("File size:"));

  _processingGroup->setTitle(PDFDocumentView::tr("Processing"));
  _creatorLabel->setText(PDFDocumentView::tr("Creator:"));
  _producerLabel->setText(PDFDocumentView::tr("Producer:"));
  _creationDateLabel->setText(PDFDocumentView::tr("Creation date:"));
  _modDateLabel->setText(PDFDocumentView::tr("Modification date:"));
  _trappedLabel->setText(PDFDocumentView::tr("Trapped:"));

  _otherGroup->setTitle(PDFDocumentView::tr("Other"));

  reload();
}


// PDFFontsInfoWidget
// ============
PDFFontsInfoWidget::PDFFontsInfoWidget(QWidget * parent) :
    PDFDocumentInfoWidget(parent, PDFDocumentView::tr("Fonts"), QString::fromLatin1("QtPDF.FontsInfoWidget"))
{
  QVBoxLayout * layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  _table = new QTableWidget(this);

#if defined(Q_WS_MAC) || defined(Q_OS_MAC) /* don't do this on windows, as the font ends up too small */
  QFont f(_table->font());
  f.setPointSize(f.pointSize() - 2);
  _table->setFont(f);
#endif
  _table->setColumnCount(4);
  _table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  _table->setAlternatingRowColors(true);
  _table->setShowGrid(false);
  _table->setSelectionBehavior(QAbstractItemView::SelectRows);
  _table->verticalHeader()->hide();
  _table->horizontalHeader()->setStretchLastSection(true);
  _table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  layout->addWidget(_table);
  setLayout(layout);
  retranslateUi();
}

void PDFFontsInfoWidget::initFromDocument(const QWeakPointer<Backend::Document> doc)
{
  PDFDocumentInfoWidget::initFromDocument(doc);
  if (isVisible())
    reload();
}

void PDFFontsInfoWidget::reload()
{
  Q_ASSERT(_table != nullptr);

  clear();
  QSharedPointer<Backend::Document> doc(_doc.toStrongRef());
  if (!doc)
    return;

  QList<Backend::PDFFontInfo> fonts = doc->fonts();
  _table->setRowCount(static_cast<decltype(_table->rowCount())>(fonts.count()));

  int i = 0;
  foreach (Backend::PDFFontInfo font, fonts) {
    _table->setItem(i, 0, new QTableWidgetItem(font.descriptor().pureName()));
    switch (font.fontType()) {
    case Backend::PDFFontInfo::FontType_Type0:
      _table->setItem(i, 1, new QTableWidgetItem(PDFDocumentView::tr("Type 0")));
      break;
    case Backend::PDFFontInfo::FontType_Type1:
      _table->setItem(i, 1, new QTableWidgetItem(PDFDocumentView::tr("Type 1")));
      break;
    case Backend::PDFFontInfo::FontType_MMType1:
      _table->setItem(i, 1, new QTableWidgetItem(PDFDocumentView::tr("Type 1 (multiple master)")));
      break;
    case Backend::PDFFontInfo::FontType_Type3:
      _table->setItem(i, 1, new QTableWidgetItem(PDFDocumentView::tr("Type 3")));
      break;
    case Backend::PDFFontInfo::FontType_TrueType:
      _table->setItem(i, 1, new QTableWidgetItem(PDFDocumentView::tr("TrueType")));
      break;
    }
    _table->setItem(i, 2, new QTableWidgetItem(font.isSubset() ? PDFDocumentView::tr("yes") : PDFDocumentView::tr("no")));
    switch (font.source()) {
    case Backend::PDFFontInfo::Source_Embedded:
      _table->setItem(i, 3, new QTableWidgetItem(PDFDocumentView::tr("[embedded]")));
      break;
    case Backend::PDFFontInfo::Source_Builtin:
      _table->setItem(i, 3, new QTableWidgetItem(PDFDocumentView::tr("[builtin]")));
      break;
    case Backend::PDFFontInfo::Source_File:
      _table->setItem(i, 3, new QTableWidgetItem(font.fileName().canonicalFilePath()));
      break;
    }
    ++i;
  }
  _table->resizeColumnsToContents();
  _table->resizeRowsToContents();
  _table->sortItems(0);
}

void PDFFontsInfoWidget::clear()
{
  Q_ASSERT(_table != nullptr);
  _table->clearContents();
  _table->setRowCount(0);
}

void PDFFontsInfoWidget::retranslateUi()
{
  Q_ASSERT(_table != nullptr);
  setWindowTitle(PDFDocumentView::tr("Fonts"));
  _table->setHorizontalHeaderLabels(QStringList() << PDFDocumentView::tr("Name") << PDFDocumentView::tr("Type") << PDFDocumentView::tr("Subset") << PDFDocumentView::tr("Source"));
  reload();
}


// PDFPermissionsInfoWidget)
// ============
PDFPermissionsInfoWidget::PDFPermissionsInfoWidget(QWidget * parent) :
    PDFDocumentInfoWidget(parent, PDFDocumentView::tr("Permissions"), QString::fromLatin1("QtPDF.PermissionsInfoWidget"))
{
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  // layout ... lays out the widgets in w
  QFormLayout * layout = new QFormLayout(this);

  // We want the layout to set the size of w (which should encompass all child
  // widgets completely, since we in turn put it into scrollArea to handle
  // oversized children
  layout->setSizeConstraint(QLayout::SetFixedSize);

  _printLabel = new QLabel(this);
  _print = new QLabel(this);
  layout->addRow(_printLabel, _print);
  _modifyLabel = new QLabel(this);
  _modify = new QLabel(this);
  layout->addRow(_modifyLabel, _modify);
  _extractLabel = new QLabel(this);
  _extract = new QLabel(this);
  layout->addRow(_extractLabel, _extract);
  _addNotesLabel = new QLabel(this);
  _addNotes = new QLabel(this);
  layout->addRow(_addNotesLabel, _addNotes);
  _formLabel = new QLabel(this);
  _form = new QLabel(this);
  layout->addRow(_formLabel, _form);

  setLayout(layout);
  retranslateUi();
}

void PDFPermissionsInfoWidget::initFromDocument(const QWeakPointer<Backend::Document> doc)
{
  PDFDocumentInfoWidget::initFromDocument(doc);
  reload();
}

void PDFPermissionsInfoWidget::reload()
{
  QSharedPointer<Backend::Document> doc(_doc.toStrongRef());
  if (!doc) {
    clear();
    return;
  }

  Backend::Document::Permissions & perm = doc->permissions();

  if (perm.testFlag(Backend::Document::Permission_Print)) {
    if (perm.testFlag(Backend::Document::Permission_PrintHighRes))
      _print->setText(PDFDocumentView::tr("Allowed"));
    else
      _print->setText(PDFDocumentView::tr("Low resolution only"));
  }
  else
    _print->setText(PDFDocumentView::tr("Denied"));

  _modify->setToolTip(QString());
  if (perm.testFlag(Backend::Document::Permission_Change))
    _modify->setText(PDFDocumentView::tr("Allowed"));
  else if (perm.testFlag(Backend::Document::Permission_Assemble)) {
    _modify->setText(PDFDocumentView::tr("Assembling only"));
    _modify->setToolTip(PDFDocumentView::tr("Insert, rotate, or delete pages and create bookmarks or thumbnail images"));
  }
  else
    _modify->setText(PDFDocumentView::tr("Denied"));

  if (perm.testFlag(Backend::Document::Permission_Extract))
    _extract->setText(PDFDocumentView::tr("Allowed"));
  else if (perm.testFlag(Backend::Document::Permission_ExtractForAccessibility))
    _extract->setText(PDFDocumentView::tr("Accessibility support only"));
  else
    _extract->setText(PDFDocumentView::tr("Denied"));

  if (perm.testFlag(Backend::Document::Permission_Annotate))
    _addNotes->setText(PDFDocumentView::tr("Allowed"));
  else
    _addNotes->setText(PDFDocumentView::tr("Denied"));

  if (perm.testFlag(Backend::Document::Permission_FillForm))
    _form->setText(PDFDocumentView::tr("Allowed"));
  else
    _form->setText(PDFDocumentView::tr("Denied"));
}

void PDFPermissionsInfoWidget::clear()
{
  _print->setText(PDFDocumentView::tr("Denied"));
  _modify->setText(PDFDocumentView::tr("Denied"));
  _extract->setText(PDFDocumentView::tr("Denied"));
  _addNotes->setText(PDFDocumentView::tr("Denied"));
  _form->setText(PDFDocumentView::tr("Denied"));
}

void PDFPermissionsInfoWidget::retranslateUi()
{
  setWindowTitle(PDFDocumentView::tr("Permissions"));

  _printLabel->setText(PDFDocumentView::tr("Printing:"));
  _modifyLabel->setText(PDFDocumentView::tr("Modifications:"));
  _extractLabel->setText(PDFDocumentView::tr("Extraction:"));
  _addNotesLabel->setText(PDFDocumentView::tr("Annotation:"));
  _formLabel->setText(PDFDocumentView::tr("Filling forms:"));
  reload();
}


// PDFAnnotationsInfoWidget
// ============
PDFAnnotationsInfoWidget::PDFAnnotationsInfoWidget(QWidget * parent) :
    PDFDocumentInfoWidget(parent, PDFDocumentView::tr("Annotations"), QString::fromLatin1("QtPDF.AnnotationsInfoWidget"))
{
  QVBoxLayout * layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  _table = new QTableWidget(this);

#if defined(Q_WS_MAC) || defined(Q_OS_MAC) /* don't do this on windows, as the font ends up too small */
  QFont f(_table->font());
  f.setPointSize(f.pointSize() - 2);
  _table->setFont(f);
#endif
  _table->setColumnCount(4);
  _table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  _table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  _table->setAlternatingRowColors(true);
  _table->setShowGrid(false);
  _table->setSelectionBehavior(QAbstractItemView::SelectRows);
  _table->verticalHeader()->hide();
  _table->horizontalHeader()->setStretchLastSection(true);
  _table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  layout->addWidget(_table);
  setLayout(layout);

  connect(&_annotWatcher, &QFutureWatcher< QList< QSharedPointer<Annotation::AbstractAnnotation> > >::resultReadyAt, this, &PDFAnnotationsInfoWidget::annotationsReady);
  retranslateUi();
}

void PDFAnnotationsInfoWidget::initFromDocument(const QWeakPointer<Backend::Document> newDoc)
{
  QSharedPointer<Backend::Document> doc(newDoc.toStrongRef());
  if (!doc || !doc->isValid())
    return;

  QList< QWeakPointer<Backend::Page> > pages;
  for (int i = 0; i < doc->numPages(); ++i) {
    QWeakPointer<Backend::Page> page = doc->page(i);
    if (page)
      pages << page;
  }

  // If another search is still running, cancel it---after all, the user wants
  // to perform a new search
  if (!_annotWatcher.isFinished()) {
    _annotWatcher.cancel();
    _annotWatcher.waitForFinished();
  }

  clear();
  _annotWatcher.setFuture(QtConcurrent::mapped(pages, PDFAnnotationsInfoWidget::loadAnnotations));
}

//static
QList< QSharedPointer<Annotation::AbstractAnnotation> > PDFAnnotationsInfoWidget::loadAnnotations(QWeakPointer<Backend::Page> thePage)
{
  QSharedPointer<Backend::Page> page(thePage.toStrongRef());
  if (!page)
    return QList< QSharedPointer<Annotation::AbstractAnnotation> >();
  return page->loadAnnotations();
}

void PDFAnnotationsInfoWidget::annotationsReady(int index)
{
  Q_ASSERT(_table != nullptr);
  int i{_table->rowCount()};
  _table->setRowCount(static_cast<decltype(_table->rowCount())>(i + _annotWatcher.resultAt(index).count()));


  foreach(QSharedPointer<Annotation::AbstractAnnotation> pdfAnnot, _annotWatcher.resultAt(index)) {
    // we only use valid markup annotation here
    if (!pdfAnnot || !pdfAnnot->isMarkup())
      continue;
    Annotation::Markup * annot = dynamic_cast<Annotation::Markup*>(pdfAnnot.data());
    QSharedPointer<Backend::Page> page(annot->page().toStrongRef());
    if (page)
      _table->setItem(i, 0, new QTableWidgetItem(QString::number(page->pageNum() + 1)));
    _table->setItem(i, 1, new QTableWidgetItem(annot->subject()));
    _table->setItem(i, 2, new QTableWidgetItem(annot->author()));
    _table->setItem(i, 3, new QTableWidgetItem(annot->contents()));
    ++i;
  }
  _table->setRowCount(i);
}

void PDFAnnotationsInfoWidget::clear()
{
  _table->clearContents();
  _table->setRowCount(0);
}

void PDFAnnotationsInfoWidget::retranslateUi()
{
  setWindowTitle(PDFDocumentView::tr("Annotations"));
  _table->setHorizontalHeaderLabels(QStringList() << PDFDocumentView::tr("Page") << PDFDocumentView::tr("Subject") << PDFDocumentView::tr("Author") << PDFDocumentView::tr("Contents"));
}

} // namespace QtPDF
