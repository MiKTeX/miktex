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
#ifndef InfoWidgets_H
#define InfoWidgets_H

#include <QFutureWatcher>
#include <QWidget>

class QGroupBox;
class QLabel;
class QListView;
class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace QtPDF {

namespace Annotation {
class AbstractAnnotation;
}

namespace Backend {
class Document;
class Page;
class PDFToCItem;
}

class PDFAction;

class PDFDocumentInfoWidget : public QWidget
{
  Q_OBJECT
  friend class PDFDocumentView;
public:
  PDFDocumentInfoWidget(QWidget * parent = nullptr, const QString & title = QString(), const QString & objectName = QString()) : QWidget(parent) { setObjectName(objectName); setWindowTitle(title); }
  ~PDFDocumentInfoWidget() override = default;
  // If the widget has a fixed size, it should not be resized (it can, e.g., be
  // put into a QScrollArea instead).
public slots:
  void setWindowTitle(const QString & windowTitle);
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
signals:
  void windowTitleChanged(const QString &);
#endif
protected slots:
  virtual void initFromDocument(const QWeakPointer<QtPDF::Backend::Document> doc) { _doc = doc; }
  virtual void retranslateUi() { }
  virtual void clear() = 0;
protected:
  void changeEvent(QEvent * event) override;
  // we need to keep a reference to the document to allow dynamic lookup of data
  // (e.g., when retranslating the widget)
  QWeakPointer<QtPDF::Backend::Document> _doc;
};

class PDFToCInfoWidget : public PDFDocumentInfoWidget
{
  Q_OBJECT
public:
  PDFToCInfoWidget(QWidget * parent);
  ~PDFToCInfoWidget() override;

protected slots:
  void initFromDocument(const QWeakPointer<QtPDF::Backend::Document> newDoc) override;
  void clear() final;
  void retranslateUi() override;
signals:
  void actionTriggered(const QtPDF::PDFAction*);
private slots:
  void itemSelectionChanged();
private:
  static void recursiveAddTreeItems(const QList<Backend::PDFToCItem> & tocItems, QTreeWidgetItem * parentTreeItem);
  static void recursiveClearTreeItems(QTreeWidgetItem * parent);
  QTreeWidget * _tree;
};

class PDFMetaDataInfoWidget : public PDFDocumentInfoWidget
{
  Q_OBJECT
public:
  PDFMetaDataInfoWidget(QWidget * parent);
  ~PDFMetaDataInfoWidget() override = default;

protected slots:
  void initFromDocument(const QWeakPointer<QtPDF::Backend::Document> doc) override;
  void clear() override;
  void retranslateUi() final;
  void reload();
private:
  QGroupBox * _documentGroup;
  QLabel * _title, * _titleLabel;
  QLabel * _author, * _authorLabel;
  QLabel * _subject, * _subjectLabel;
  QLabel * _keywords, * _keywordsLabel;
  QLabel * _pageSize, * _pageSizeLabel;
  QLabel * _fileSize, * _fileSizeLabel;
  QGroupBox * _processingGroup;
  QLabel * _creator, * _creatorLabel;
  QLabel * _producer, * _producerLabel;
  QLabel * _creationDate, * _creationDateLabel;
  QLabel * _modDate, * _modDateLabel;
  QLabel * _trapped, * _trappedLabel;
  QGroupBox * _otherGroup;
};

class PDFFontsInfoWidget : public PDFDocumentInfoWidget
{
  Q_OBJECT
public:
  PDFFontsInfoWidget(QWidget * parent);
  ~PDFFontsInfoWidget() override = default;

protected slots:
  void initFromDocument(const QWeakPointer<QtPDF::Backend::Document> doc) override;
  void clear() final;
  void retranslateUi() final;
  void reload();
protected:
  void showEvent(QShowEvent * event) override {
    Q_UNUSED(event)
    initFromDocument(_doc);
  }
private:
  QTableWidget * _table;
};

class PDFPermissionsInfoWidget : public PDFDocumentInfoWidget
{
  Q_OBJECT
public:
  PDFPermissionsInfoWidget(QWidget * parent);
  ~PDFPermissionsInfoWidget() override = default;

protected slots:
  void initFromDocument(const QWeakPointer<QtPDF::Backend::Document> doc) override;
  void clear() final;
  void retranslateUi() final;
  void reload();
private:
  QLabel * _print, * _printLabel;
  QLabel * _modify, * _modifyLabel;
  QLabel * _extract, * _extractLabel;
  QLabel * _addNotes, * _addNotesLabel;
  QLabel * _form, * _formLabel;
};

class PDFAnnotationsInfoWidget : public PDFDocumentInfoWidget
{
  Q_OBJECT

  QFutureWatcher< QList< QSharedPointer<Annotation::AbstractAnnotation> > > _annotWatcher;
  QTableWidget * _table;

  static QList< QSharedPointer<Annotation::AbstractAnnotation> > loadAnnotations(QWeakPointer<Backend::Page> thePage);

public:
  PDFAnnotationsInfoWidget(QWidget * parent);
  ~PDFAnnotationsInfoWidget() override = default;

protected slots:
  void initFromDocument(const QWeakPointer<QtPDF::Backend::Document> newDoc) override;
  void clear() override;
  void retranslateUi() final;
  void annotationsReady(int index);
};

class PDFOptionalContentInfoWidget : public PDFDocumentInfoWidget
{
  Q_OBJECT
  QListView * _list{nullptr};
public:
  PDFOptionalContentInfoWidget(QWidget * parent);
  ~PDFOptionalContentInfoWidget() override = default;

protected slots:
  void initFromDocument(const QWeakPointer<QtPDF::Backend::Document> newDoc) override;
  void clear() override;
  void retranslateUi() final;
};

} // namespace QtPDF

#endif // !defined(InfoWidgets_H)
