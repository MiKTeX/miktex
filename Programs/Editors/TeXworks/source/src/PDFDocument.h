/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2018  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef PDFDocument_H
#define PDFDocument_H

#include "TWScriptable.h"

#include <QImage>
#include <QLabel>
#include <QList>
#include <QCursor>
#include <QButtonGroup>
#include <QPainterPath>
#include <QTimer>
#include <QMouseEvent>

#include "TWApp.h"
#include "FindDialog.h"
#include "../modules/QtPDF/src/PDFDocumentWidget.h"
#include "TWSynchronizer.h"

#include "ui_PDFDocument.h"

const int kDefault_MagnifierSize = 2;
const bool kDefault_CircularMagnifier = true;
const int kDefault_PreviewScaleOption = 1;
const int kDefault_PreviewScale = 200;
const QtPDF::PDFDocumentView::PageMode kDefault_PDFPageMode = QtPDF::PDFDocumentView::PageMode_OneColumnContinuous;

const int kPDFWindowStateVersion = 1;

class QAction;
class QMenu;
class QToolBar;
class QScrollArea;
class TeXDocument;
class QShortcut;

class FullscreenManager : public QObject
{
	Q_OBJECT
public:
	FullscreenManager(QMainWindow * parent);
	virtual ~FullscreenManager();

	void setFullscreen(const bool fullscreen = true);
	bool isFullscreen() const;
	void toggleFullscreen();
	void mouseMoveEvent(QMouseEvent * event);

	void addShortcut(QAction * action, const char * member);
	void addShortcut(const QKeySequence & key, const char * member, QAction * action = NULL);

signals:
	void fullscreenChanged(bool fullscreen);

private slots:
	void showMenuBar() { setMenuBarVisible(true); }
	void hideMenuBar() { setMenuBarVisible(false); }
	void actionDeleted(QObject * obj);

protected:
	void setMenuBarVisible(const bool visible = true);

	struct shortcut_info {
		QShortcut * shortcut;
		QAction * action;
	};

	QList<shortcut_info> _shortcuts;
	QMap<QWidget*, bool> _normalVisibility;
	QMainWindow * _parent;
	QTimer _menuBarTimer;
};

class PDFDocument : public TWScriptable, private Ui::PDFDocument
{
	Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName)

public:
	PDFDocument(const QString &fileName, TeXDocument *sourceDoc = NULL);
	virtual ~PDFDocument();

	static PDFDocument *findDocument(const QString &fileName);
	static QList<PDFDocument*> documentList()
		{
			return docList;
		}

	QString fileName() const
		{ return curFile; }

	void zoomToRight(QWidget *otherWindow);
	void showScale(qreal scale);
	void showPage(int page);
	void setResolution(const double res);
	void resetMagnifier();
	void enableTypesetAction(bool enabled);
	void updateTypesettingAction(bool processRunning);
	void linkToSource(TeXDocument *texDoc);
	bool hasSyncData() const { return _synchronizer != NULL; }

	QtPDF::PDFDocumentWidget * widget() { return pdfWidget; }

protected:
	virtual void changeEvent(QEvent *event);
	virtual bool event(QEvent *event);
	virtual void closeEvent(QCloseEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

public slots:
	void texActivated(TeXDocument * texDoc);
	void texClosed(QObject *obj);
	void reload();
	void retypeset();
	void interrupt();
	void sideBySide();
	void doFindDialog();
	void doFindAgain(bool newSearch = false);
	void goToSource();
	void toggleFullScreen();
	void syncFromSource(const QString& sourceFile, int lineNo, int col, bool activatePreview);
	void print();
	void setMouseMode(const int newMode);
	void setPageMode(const int newMode);
	void clearSyncHighlight();
	void clearSearchResultHighlight();
	void copySelectedTextToClipboard();

private slots:
	void changedDocument(const QWeakPointer<QtPDF::Backend::Document> newDoc);
	void updateRecentFileActions();
	void updateWindowMenu();
	void enablePageActions(int);
	void syncClick(int page, const QPointF& pos);
	void syncRange(const int pageIndex, const QPointF & start, const QPointF & end, const TWSynchronizer::Resolution resolution);
	void invalidateSyncHighlight();
	void scaleLabelClick(QMouseEvent * event) { showScaleContextMenu(event->pos()); }
	void showScaleContextMenu(const QPoint pos);
	void setScaleFromContextMenu(const QString & strZoom);
	void updateStatusBar();
	void updatePageMode(const QtPDF::PDFDocumentView::PageMode newMode);
	void doPageDialog();
	void doScaleDialog();
	void jumpToSource();
	void searchResultHighlighted(const int pageNum, const QList<QPolygonF> pdfRegion);
	void setDefaultScale();
	void maybeOpenUrl(const QUrl url);
	void maybeOpenPdf(QString filename, QtPDF::PDFDestination destination, bool newWindow);
	void maybeZoomToWindow(bool doZoom) { if (doZoom) pdfWidget->zoomFitWindow(); }
	void maybeEnableCopyCommand(const bool isTextSelected);

signals:
	void reloaded();
	void activatedWindow(QWidget*);

private:
	void init();
	void loadFile(const QString &fileName);
	void setCurrentFile(const QString &fileName);
	void loadSyncData();
	void saveRecentFileInfo();

	QString curFile;

	QtPDF::PDFDocumentWidget *pdfWidget;
	QScrollArea	*scrollArea;
	QButtonGroup	*toolButtonGroup;

	QLinkedList<TeXDocument*> sourceDocList;

	QLabel *pageLabel;
	QLabel *scaleLabel;
	QList<QAction*> recentFileActions;
	FullscreenManager * _fullScreenManager;
	QSignalMapper pageModeSignalMapper;

	QGraphicsItem * _syncHighlight;
	QTimer _syncHighlightRemover;

	QBrush _searchResultHighlightBrush;
	QTimer _searchResultHighlightRemover;

	bool openedManually;
	
	static QList<PDFDocument*> docList;

	TWSyncTeXSynchronizer * _synchronizer;
	
	PDFSearchResult lastSearchResult;
	// stores the page idx a search was started on
	// after wrapping the search will continue only up to this page
	int firstSearchPage;
#if defined(MIKTEX)
    QAction * actionAbout_MiKTeX;
#if defined(MIKTEX_TODO_PRINT)
    QAction * actionPrintPDF;
#endif
#endif
};

#endif
