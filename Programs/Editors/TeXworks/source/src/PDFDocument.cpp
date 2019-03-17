/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "PDFDocument.h"
#include "TeXDocument.h"
#include "TWApp.h"
#include "TWUtils.h"
#include "FindDialog.h"
#include "ClickableLabel.h"

#include <QDockWidget>
#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QPainter>
#include <QPaintEngine>
#include <QLabel>
#include <QScrollArea>
#include <QStyle>
#include <QDesktopWidget>
#include <QScrollBar>
#include <QRegion>
#include <QVector>
#include <QList>
#include <QStack>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QShortcut>
#include <QToolTip>
#include <QSignalMapper>
#if defined(MIKTEX_TODO_PRINT)
// see http://code.google.com/p/texworks/issues/detail?id=78#c1
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintEngine>
#include <QBuffer>
#include <QProgressDialog>
#endif

#include <math.h>

#define SYNCTEX_GZ_EXT	".synctex.gz"
#define SYNCTEX_EXT		".synctex"

#define ROUND(x) floor((x)+0.5)

// Possible sizes of the magnifying glass (in pixel)
const int magSizes[] = { 200, 300, 400 };

// duration of highlighting in PDF view (might make configurable?)
const int kPDFHighlightDuration = 2000;



#pragma mark === PDFDocument ===

// TODO: This is seemingly unused---verify && remove
QList<PDFDocument*> PDFDocument::docList;

PDFDocument::PDFDocument(const QString &fileName, TeXDocument *texDoc)
	: _syncHighlight(NULL), _synchronizer(NULL), openedManually(false), _fullScreenManager(NULL)
{
	init();

	if (texDoc == NULL)
		openedManually = true;

	loadFile(fileName);

	QMap<QString,QVariant> properties = TWApp::instance()->getFileProperties(curFile);
	if (properties.contains(QString::fromLatin1("geometry")))
		restoreGeometry(properties.value(QString::fromLatin1("geometry")).toByteArray());
	else
		TWUtils::zoomToHalfScreen(this, true);

	if (properties.contains(QString::fromLatin1("state")))
		restoreState(properties.value(QString::fromLatin1("state")).toByteArray(), kPDFWindowStateVersion);
	
	if (properties.contains(QString::fromLatin1("pdfPageMode")))
		setPageMode(properties.value(QString::fromLatin1("pdfPageMode"), -1).toInt());

	QTimer::singleShot(100, this, SLOT(setDefaultScale()));

	if (texDoc != NULL) {
		stackUnder((QWidget*)texDoc);
		actionSide_by_Side->setEnabled(true);
		actionGo_to_Source->setEnabled(true);
		sourceDocList.append(texDoc);
	}
}

PDFDocument::~PDFDocument()
{
	docList.removeAll(this);
}

void PDFDocument::init()
{
	docList.append(this);

	setupUi(this);
#if defined(Q_OS_WIN)
	TWApp::instance()->createMessageTarget(this);
#endif

	setAttribute(Qt::WA_DeleteOnClose, true);
	setAttribute(Qt::WA_MacNoClickThrough, true);

	QIcon winIcon;
#if defined(Q_OS_UNIX) && !defined(Q_OS_DARWIN)
	// The Compiz window manager doesn't seem to support icons larger than
	// 128x128, so we add a suitable one first
	winIcon.addFile(QString::fromLatin1(":/images/images/TeXworks-doc-128.png"));
#endif
	winIcon.addFile(QString::fromLatin1(":/images/images/TeXworks-doc.png"));
	setWindowIcon(winIcon);

	pdfWidget = new QtPDF::PDFDocumentWidget(this);
	pdfWidget->setSearchResultHighlightBrush(QBrush(Qt::transparent));
	pdfWidget->setCurrentSearchResultHighlightBrush(QBrush(Qt::transparent));
	pdfWidget->setAcceptDrops(false);
	_searchResultHighlightBrush = QColor(255, 255, 0, 63);
	setCentralWidget(pdfWidget);

	connect(pdfWidget, SIGNAL(changedPage(int)), this, SLOT(updateStatusBar()));
	connect(pdfWidget, SIGNAL(changedZoom(qreal)), this, SLOT(updateStatusBar()));
	connect(pdfWidget, SIGNAL(changedDocument(const QWeakPointer<QtPDF::Backend::Document>)), this, SLOT(changedDocument(const QWeakPointer<QtPDF::Backend::Document>)));
	connect(pdfWidget, SIGNAL(searchResultHighlighted(const int, const QList<QPolygonF>)), this, SLOT(searchResultHighlighted(const int, const QList<QPolygonF>)));
	connect(pdfWidget, SIGNAL(changedPageMode(QtPDF::PDFDocumentView::PageMode)), this, SLOT(updatePageMode(QtPDF::PDFDocumentView::PageMode)));
	connect(pdfWidget, SIGNAL(requestOpenPdf(QString,QtPDF::PDFDestination,bool)), this, SLOT(maybeOpenPdf(QString,QtPDF::PDFDestination,bool)));
	connect(pdfWidget, SIGNAL(requestOpenUrl(QUrl)), this, SLOT(maybeOpenUrl(QUrl)));
	connect(pdfWidget, SIGNAL(textSelectionChanged(bool)), this, SLOT(maybeEnableCopyCommand(bool)));

	toolButtonGroup = new QButtonGroup(toolBar);
	toolButtonGroup->addButton(qobject_cast<QAbstractButton*>(toolBar->widgetForAction(actionMagnify)), QtPDF::PDFDocumentView::MouseMode_MagnifyingGlass);
	toolButtonGroup->addButton(qobject_cast<QAbstractButton*>(toolBar->widgetForAction(actionScroll)), QtPDF::PDFDocumentView::MouseMode_Move);
	toolButtonGroup->addButton(qobject_cast<QAbstractButton*>(toolBar->widgetForAction(actionSelect_Text)), QtPDF::PDFDocumentView::MouseMode_Select);
//	toolButtonGroup->addButton(qobject_cast<QAbstractButton*>(toolBar->widgetForAction(actionSelect_Image)), kSelectImage);
	connect(toolButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(setMouseMode(int)));
	pdfWidget->setMouseModeMagnifyingGlass();

	scaleLabel = new ClickableLabel();
	statusBar()->addPermanentWidget(scaleLabel);
	scaleLabel->setFrameStyle(QFrame::StyledPanel);
	scaleLabel->setFont(statusBar()->font());
	connect(scaleLabel, SIGNAL(mouseLeftClick(QMouseEvent*)), this, SLOT(scaleLabelClick(QMouseEvent*)));
	
	pageLabel = new ClickableLabel();
	statusBar()->addPermanentWidget(pageLabel);
	pageLabel->setFrameStyle(QFrame::StyledPanel);
	pageLabel->setFont(statusBar()->font());
	connect(pageLabel, SIGNAL(mouseLeftClick(QMouseEvent*)), this, SLOT(doPageDialog()));

	connect(actionAbout_TW, SIGNAL(triggered()), qApp, SLOT(about()));
	connect(actionSettings_and_Resources, SIGNAL(triggered()), qApp, SLOT(doResourcesDialog()));
	connect(actionGoToHomePage, SIGNAL(triggered()), qApp, SLOT(goToHomePage()));
	connect(actionWriteToMailingList, SIGNAL(triggered()), qApp, SLOT(writeToMailingList()));

	connect(actionNew, SIGNAL(triggered()), qApp, SLOT(newFile()));
	connect(actionNew_from_Template, SIGNAL(triggered()), qApp, SLOT(newFromTemplate()));
	connect(actionOpen, SIGNAL(triggered()), qApp, SLOT(open()));
	connect(actionPrintPdf, SIGNAL(triggered()), this, SLOT(print()));

	connect(actionQuit_TeXworks, SIGNAL(triggered()), TWApp::instance(), SLOT(maybeQuit()));

	connect(actionCopy, SIGNAL(triggered(bool)), this, SLOT(copySelectedTextToClipboard()));

	connect(actionFind, SIGNAL(triggered()), this, SLOT(doFindDialog()));

	connect(actionFirst_Page, SIGNAL(triggered()), pdfWidget, SLOT(goFirst()));
	connect(actionPrevious_Page, SIGNAL(triggered()), pdfWidget, SLOT(goPrev()));
	connect(actionNext_Page, SIGNAL(triggered()), pdfWidget, SLOT(goNext()));
	connect(actionLast_Page, SIGNAL(triggered()), pdfWidget, SLOT(goLast()));
	connect(actionGo_to_Page, SIGNAL(triggered()), this, SLOT(doPageDialog()));
	addAction(actionPrevious_ViewRect);
	connect(actionPrevious_ViewRect, SIGNAL(triggered()), pdfWidget, SLOT(goPrevViewRect()));
	connect(pdfWidget, SIGNAL(changedPage(int)), this, SLOT(enablePageActions(int)));

	connect(actionActual_Size, SIGNAL(triggered()), pdfWidget, SLOT(zoom100()));
	connect(actionFit_to_Width, SIGNAL(triggered()), pdfWidget, SLOT(zoomFitWidth()));
	connect(actionFit_to_Window, SIGNAL(triggered()), pdfWidget, SLOT(zoomFitWindow()));
	connect(actionFit_to_Content_Width, SIGNAL(triggered()), pdfWidget, SLOT(zoomFitContentWidth()));
	connect(actionZoom_In, SIGNAL(triggered()), pdfWidget, SLOT(zoomIn()));
	connect(actionZoom_Out, SIGNAL(triggered()), pdfWidget, SLOT(zoomOut()));
	connect(actionFull_Screen, SIGNAL(triggered()), this, SLOT(toggleFullScreen()));
	connect(pdfWidget, SIGNAL(contextClick(int, const QPointF&)), this, SLOT(syncClick(int, const QPointF&)));
	pageModeSignalMapper.setMapping(actionPageMode_Single, QtPDF::PDFDocumentView::PageMode_SinglePage);
	pageModeSignalMapper.setMapping(actionPageMode_Continuous, QtPDF::PDFDocumentView::PageMode_OneColumnContinuous);
	pageModeSignalMapper.setMapping(actionPageMode_TwoPagesContinuous, QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous);
	connect(actionPageMode_Single, SIGNAL(triggered()), &pageModeSignalMapper, SLOT(map()));
	connect(actionPageMode_Continuous, SIGNAL(triggered()), &pageModeSignalMapper, SLOT(map()));
	connect(actionPageMode_TwoPagesContinuous, SIGNAL(triggered()), &pageModeSignalMapper, SLOT(map()));
	connect(&pageModeSignalMapper, SIGNAL(mapped(int)), this, SLOT(setPageMode(int)));

	if (actionZoom_In->shortcut() == QKeySequence(tr("Ctrl++")))
		new QShortcut(QKeySequence(tr("Ctrl+=")), pdfWidget, SLOT(zoomIn()));
	
	connect(actionTypeset, SIGNAL(triggered()), this, SLOT(retypeset()));
	
	connect(actionStack, SIGNAL(triggered()), qApp, SLOT(stackWindows()));
	connect(actionTile, SIGNAL(triggered()), qApp, SLOT(tileWindows()));
	connect(actionSide_by_Side, SIGNAL(triggered()), this, SLOT(sideBySide()));
	connect(actionPlace_on_Left, SIGNAL(triggered()), this, SLOT(placeOnLeft()));
	connect(actionPlace_on_Right, SIGNAL(triggered()), this, SLOT(placeOnRight()));
	connect(actionGo_to_Source, SIGNAL(triggered()), this, SLOT(goToSource()));
	
	connect(actionFind_Again, SIGNAL(triggered()), this, SLOT(doFindAgain()));

	updateRecentFileActions();
	connect(qApp, SIGNAL(recentFileActionsChanged()), this, SLOT(updateRecentFileActions()));
	connect(qApp, SIGNAL(windowListChanged()), this, SLOT(updateWindowMenu()));
	connect(actionClear_Recent_Files, SIGNAL(triggered()), TWApp::instance(), SLOT(clearRecentFiles()));

	connect(qApp, SIGNAL(hideFloatersExcept(QWidget*)), this, SLOT(hideFloatersUnlessThis(QWidget*)));
	connect(this, SIGNAL(activatedWindow(QWidget*)), qApp, SLOT(activatedWindow(QWidget*)));

	connect(actionPreferences, SIGNAL(triggered()), qApp, SLOT(preferences()));

	connect(this, SIGNAL(destroyed()), qApp, SLOT(updateWindowMenus()));

	connect(qApp, SIGNAL(syncPdf(const QString&, int, int, bool)), this, SLOT(syncFromSource(const QString&, int, int, bool)));

	_syncHighlightRemover.setSingleShot(true);
	connect(&_syncHighlightRemover, SIGNAL(timeout()), this, SLOT(clearSyncHighlight()));

	_searchResultHighlightRemover.setSingleShot(true);
	connect(&_searchResultHighlightRemover, SIGNAL(timeout()), this, SLOT(clearSearchResultHighlight()));

	menuShow->addAction(toolBar->toggleViewAction());
	menuShow->addSeparator();

	QDockWidget * dw = pdfWidget->dockWidget(QtPDF::PDFDocumentView::Dock_TableOfContents, this);
	dw->hide();
	addDockWidget(Qt::LeftDockWidgetArea, dw);
	menuShow->addAction(dw->toggleViewAction());

	dw = pdfWidget->dockWidget(QtPDF::PDFDocumentView::Dock_MetaData, this);
	dw->hide();
	addDockWidget(Qt::LeftDockWidgetArea, dw);
	menuShow->addAction(dw->toggleViewAction());

	dw = pdfWidget->dockWidget(QtPDF::PDFDocumentView::Dock_Fonts, this);
	dw->hide();
	addDockWidget(Qt::BottomDockWidgetArea, dw);
	menuShow->addAction(dw->toggleViewAction());

	dw = pdfWidget->dockWidget(QtPDF::PDFDocumentView::Dock_Permissions, this);
	dw->hide();
	addDockWidget(Qt::LeftDockWidgetArea, dw);
	menuShow->addAction(dw->toggleViewAction());

	dw = pdfWidget->dockWidget(QtPDF::PDFDocumentView::Dock_Annotations, this);
	dw->hide();
	addDockWidget(Qt::LeftDockWidgetArea, dw);
	menuShow->addAction(dw->toggleViewAction());
	
	QSETTINGS_OBJECT(settings);
	switch(settings.value(QString::fromLatin1("pdfPageMode"), kDefault_PDFPageMode).toInt()) {
		case 0:
			setPageMode(QtPDF::PDFDocumentView::PageMode_SinglePage);
			break;
		case 1:
			setPageMode(QtPDF::PDFDocumentView::PageMode_OneColumnContinuous);
			break;
		case 2:
			setPageMode(QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous);
			break;
		default:
			setPageMode(kDefault_PDFPageMode);
			break;
	}
	resetMagnifier();

	if (settings.contains(QString::fromLatin1("previewResolution")))
		pdfWidget->setResolution(settings.value(QString::fromLatin1("previewResolution"), QApplication::desktop()->logicalDpiX()).toInt());

	TWUtils::applyToolbarOptions(this, settings.value(QString::fromLatin1("toolBarIconSize"), 2).toInt(), settings.value(QString::fromLatin1("toolBarShowText"), false).toBool());

	TWApp::instance()->updateWindowMenus();
	
	initScriptable(menuScripts, actionAbout_Scripts, actionManage_Scripts,
				   actionUpdate_Scripts, actionShow_Scripts_Folder);
	
	TWUtils::insertHelpMenuItems(menuHelp);
	TWUtils::installCustomShortcuts(this);
#if defined(MIKTEX)
        actionAbout_MiKTeX = new QAction(this);
        actionAbout_MiKTeX->setIcon(QIcon(":/MiKTeX/miktex16x16.png"));
        actionAbout_MiKTeX->setObjectName(QString::fromUtf8("actionAbout_MiKTeX"));
        actionAbout_MiKTeX->setText(QApplication::translate("PDFDocument", "About MiKTeX..."));
        actionAbout_MiKTeX->setMenuRole(QAction::AboutRole);
        connect(actionAbout_MiKTeX, SIGNAL(triggered()), qApp, SLOT(aboutMiKTeX()));
#if 1
        menuHelp->addAction(actionAbout_MiKTeX);
#endif
#endif
}

void PDFDocument::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		QString title = windowTitle();
		retranslateUi(this);
		TWUtils::insertHelpMenuItems(menuHelp);
		setWindowTitle(title);
		updateStatusBar();
	}
	QMainWindow::changeEvent(event);
}

void PDFDocument::linkToSource(TeXDocument *texDoc)
{
	if (texDoc != NULL) {
		if (!sourceDocList.contains(texDoc))
			sourceDocList.append(texDoc);
		actionGo_to_Source->setEnabled(true);
	}
}

void PDFDocument::texClosed(QObject *obj)
{
	TeXDocument *texDoc = reinterpret_cast<TeXDocument*>(obj);
		// can't use qobject_cast here as the object's metadata is already gone!
	if (texDoc != 0) {
		sourceDocList.removeAll(texDoc);
		if (sourceDocList.count() == 0)
			close();
	}
}

void PDFDocument::texActivated(TeXDocument * texDoc)
{
	// A source file was activated. Make sure it is the first in the list of
	// source docs so that future "Goto Source" actions point there.
	if (sourceDocList.first() != texDoc) {
		sourceDocList.removeAll(texDoc);
		sourceDocList.prepend(texDoc);
	}
}

void PDFDocument::updateRecentFileActions()
{
	TWUtils::updateRecentFileActions(this, recentFileActions, menuOpen_Recent, actionClear_Recent_Files);
}

void PDFDocument::updateWindowMenu()
{
	TWUtils::updateWindowMenu(this, menuWindow);

	// If the window list changed, we might want to update our window title as
	// well to uniquely identify the current file among all others open in
	// TeXworks
	Q_FOREACH(QAction * action, menuWindow->actions()) {
		SelWinAction * selWinAction = qobject_cast<SelWinAction*>(action);
		// If this is not an action related to an open window, skip it
		if (!selWinAction)
			continue;
		// If this action corresponds to the current file, use it's label as
		// window text
		if (selWinAction->data().toString() == fileName())
			setWindowTitle(tr("%1[*] - %2").arg(selWinAction->text()).arg(tr(TEXWORKS_NAME)));
	}
}

void PDFDocument::sideBySide()
{
	if (sourceDocList.count() > 0) {
		TWUtils::sideBySide(sourceDocList.first(), this);
		sourceDocList.first()->selectWindow(false);
		selectWindow();
	}
	else
		placeOnRight();
}

bool PDFDocument::event(QEvent *event)
{
	switch (event->type()) {
		case QEvent::WindowActivate:
			showFloaters();
			emit activatedWindow(this);
			break;
		default:
			break;
	}
	return QMainWindow::event(event);
}

void PDFDocument::closeEvent(QCloseEvent *event)
{
	event->accept();
	if (openedManually) {
		saveRecentFileInfo();
	}
	deleteLater();
}

void PDFDocument::saveRecentFileInfo()
{
	QMap<QString,QVariant> fileProperties;
	fileProperties.insert(QString::fromLatin1("path"), curFile);
	fileProperties.insert(QString::fromLatin1("geometry"), saveGeometry());
	fileProperties.insert(QString::fromLatin1("state"), saveState(kPDFWindowStateVersion));
	fileProperties.insert(QString::fromLatin1("pdfPageMode"), pdfWidget->pageMode());
	TWApp::instance()->addToRecentFiles(fileProperties);
}

void PDFDocument::loadFile(const QString &fileName)
{
	setCurrentFile(fileName);
	QSETTINGS_OBJECT(settings);
	QFileInfo info(fileName);
	settings.setValue(QString::fromLatin1("openDialogDir"), info.canonicalPath());

	reload();
}

void PDFDocument::reload()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	clearSyncHighlight();
	if (pdfWidget->load(curFile)) {
		loadSyncData();
		emit reloaded();
	}
	else {
		statusBar()->showMessage(tr("Failed to load file \"%1\"; perhaps it is not a valid PDF document.").arg(TWUtils::strippedName(curFile)));
	}
	QApplication::restoreOverrideCursor();
}

void PDFDocument::loadSyncData()
{
	if (_synchronizer) {
		delete _synchronizer;
		_synchronizer = NULL;
	}
	_synchronizer = new TWSyncTeXSynchronizer(curFile);
	if (!_synchronizer)
		statusBar()->showMessage(tr("Error initializing SyncTeX"), kStatusMessageDuration);
	else if (!_synchronizer->isValid())
		statusBar()->showMessage(tr("No SyncTeX data available"), kStatusMessageDuration);
	else
		statusBar()->showMessage(tr("SyncTeX: \"%1\"").arg(_synchronizer->syncTeXFilename()), kStatusMessageDuration);
}

void PDFDocument::syncClick(int pageIndex, const QPointF& pos)
{
	QSETTINGS_OBJECT(settings);
	TWSynchronizer::Resolution res;
	switch (settings.value(QString::fromLatin1("syncResolutionToTeX"), TWSynchronizer::kDefault_Resolution_ToTeX).toInt()) {
		case 0:
			res = TWSynchronizer::CharacterResolution;
			break;
		case 1:
			res = TWSynchronizer::WordResolution;
			break;
		case 2:
			res = TWSynchronizer::LineResolution;
			break;
		default:
			res = TWSynchronizer::kDefault_Resolution_ToPDF;
	}

	syncRange(pageIndex, pos, pos, res);
}

void PDFDocument::syncRange(const int pageIndex, const QPointF & start, const QPointF & end, const TWSynchronizer::Resolution resolution)
{
	if (!_synchronizer)
		return;

	clearSyncHighlight();

	// NOTE: "start" and "end" are in PDF coordinates, which are upside down
	// (i.e., (0,0) is in the lower left), whereas SyncTeX expects TeX
	// coordinates (i.e., (0,0) in the upper left). Hence we need to convert the
	// coordinates.
	QSharedPointer<QtPDF::Backend::Document> doc = pdfWidget->document().toStrongRef();
	if (!doc)
		return;
	QSharedPointer<QtPDF::Backend::Page> page = doc->page(pageIndex).toStrongRef();
	if (!page)
		return;

	// Synchronize the point "start"
	TWSynchronizer::PDFSyncPoint srcStart;
	srcStart.filename = curFile;
	srcStart.page = pageIndex + 1;
	srcStart.rects.append(QRectF(start.x(), page->pageSizeF().height() - start.y(), 0, 0));
	TWSynchronizer::TeXSyncPoint destStart = _synchronizer->syncFromPDF(srcStart, resolution);

	// Syncronize the point "end"
	TWSynchronizer::TeXSyncPoint destEnd;
	if (end.isNull() || end == start)
		// If "end" was not provided or was the same as "start", just copy the
		// result
		destEnd = destStart;
	else {
		// Otherwise, perform the synchronization
		TWSynchronizer::PDFSyncPoint srcEnd;
		srcEnd.filename = curFile;
		srcEnd.page = pageIndex + 1;
		srcEnd.rects.append(QRectF(end.x(), page->pageSizeF().height() - end.y(), 0, 0));
		destEnd = _synchronizer->syncFromPDF(srcEnd, resolution);
	}

	// Check if (at least) "start" was properly synchronized; if not: bail out
	if (destStart.filename.isEmpty() || destStart.line < 0)
		return;

	// Open the destination document (for the point "start"), and put the cursor
	// on the right line (though not necessarily on the right column or with the
	// the right selection, yet, as that requires additional handling below)
	QDir curDir(QFileInfo(curFile).canonicalPath());
	TeXDocument * texDoc = TeXDocument::openDocument(QFileInfo(curDir, destStart.filename).canonicalFilePath(), true, true, destStart.line);
	if (!texDoc)
		return;

	// Get a text cursor in the correct position for "start" (if no valid column
	// was found, place it at the beginning of the correct line)
	QTextCursor curStart = texDoc->textCursor();
	curStart.setPosition(0);
	curStart.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, destStart.line - 1);
	if (destStart.col >= 0)
		curStart.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, destStart.col);

	// Get a text cursor in the correct position for "end" (if no valid column
	// was found, place it at the end of the correct line)
	QTextCursor curEnd = texDoc->textCursor();
	if (destEnd.filename == destStart.filename) {
		curEnd.setPosition(0);
		curEnd.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, destStart.line - 1);
		if (destEnd.col >= 0)
			curEnd.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, destEnd.col + qMax(1, destEnd.len));
		else
			curEnd.movePosition(QTextCursor::EndOfBlock);
	}
	else {
		// If we get here, the end point is in a different file than the
		// starting point. In that case, we assume the rest of the file
		// containing the starting point should be highlighted)
		curEnd.movePosition(QTextCursor::End);
	}

	// Properly highlight the destination document
	// NB: We use TeXDocument::openDocument here even though we already have a
	// pointer to the document as that is the only publicly available function
	// that does what we need (i.e., position the cursor and possibly change the
	// current selection).
	TeXDocument::openDocument(QFileInfo(curDir, destStart.filename).canonicalFilePath(), true, true, destStart.line, curStart.position() - curStart.block().position(), curEnd.position() - curStart.block().position());
}

void PDFDocument::syncFromSource(const QString& sourceFile, int lineNo, int col, bool activatePreview)
{
	if (!_synchronizer)
		return;

	QSETTINGS_OBJECT(settings);
	TWSynchronizer::Resolution res;
	switch (settings.value(QString::fromLatin1("syncResolutionToPDF"), TWSynchronizer::kDefault_Resolution_ToPDF).toInt()) {
		case 0:
			res = TWSynchronizer::CharacterResolution;
			break;
		case 1:
			res = TWSynchronizer::WordResolution;
			break;
		case 2:
			res = TWSynchronizer::LineResolution;
			break;
		default:
			res = TWSynchronizer::kDefault_Resolution_ToPDF;
	}

	TWSynchronizer::TeXSyncPoint src;
	src.filename = sourceFile;
	src.line = lineNo;
	src.col = col;

	// Get target point
	TWSynchronizer::PDFSyncPoint dest = _synchronizer->syncFromTeX(src, res);

	// Check target point
	if (dest.page < 1 || QFileInfo(curFile) != QFileInfo(dest.filename))
		return;

	// Display the result
	pdfWidget->goToPage(dest.page - 1);
	QPainterPath path;
	path.setFillRule(Qt::WindingFill);
	foreach(QRectF r, dest.rects)
		path.addRect(r);

	clearSyncHighlight();
	_syncHighlight = pdfWidget->addHighlightPath(dest.page - 1, path, QColor(255, 255, 0, 63));

	// Ensure that the synhronization point is displayed (in the center)
	pdfWidget->centerOn(_syncHighlight->mapToScene(_syncHighlight->boundingRect().center()));

	// Start the highlight removal timer (if applicable)
	if (kPDFHighlightDuration > 0)
		_syncHighlightRemover.start(kPDFHighlightDuration);

	// Update the view (and possibly bring it to the front)
	pdfWidget->update();
	if (activatePreview)
		selectWindow();
}

void PDFDocument::invalidateSyncHighlight()
{
	// This slot should be called when the graphics item pointed to by
	// _syncHighlight goes out of scope (e.g., because the PDF changed, all pages
	// were deleted, and, in the process, all subordinate graphics items as well).
	_syncHighlight = NULL;
	_syncHighlightRemover.stop();
}

void PDFDocument::clearSyncHighlight()
{
	if (_syncHighlight) {
		delete _syncHighlight;
		_syncHighlight = NULL;
	}
	_syncHighlightRemover.stop();
}

void PDFDocument::clearSearchResultHighlight()
{
	if (!widget())
		return;
	widget()->setCurrentSearchResultHighlightBrush(QBrush(Qt::transparent));
}

void PDFDocument::copySelectedTextToClipboard()
{
	if (!widget()) return;
	QString textToCopy = widget()->selectedText();
	if (textToCopy.isEmpty())
		return;
	Q_ASSERT(QApplication::clipboard());
	QApplication::clipboard()->setText(textToCopy);
}

void PDFDocument::maybeEnableCopyCommand(const bool isTextSelected)
{
  Q_ASSERT(actionCopy);
  if (!widget())
	return;
  QSharedPointer<QtPDF::Backend::Document> doc = widget()->document().toStrongRef();
  if (!doc)
	actionCopy->setEnabled(false);
  else
	actionCopy->setEnabled(isTextSelected && doc->permissions().testFlag(QtPDF::Backend::Document::Permission_Extract));
}

void PDFDocument::setCurrentFile(const QString &fileName)
{
	curFile = QFileInfo(fileName).canonicalFilePath();
	//: Format for the window title (ex. "file.pdf[*] - TeXworks")
	setWindowTitle(tr("%1[*] - %2").arg(TWUtils::strippedName(curFile)).arg(tr(TEXWORKS_NAME)));
	TWApp::instance()->updateWindowMenus();
}
 
PDFDocument *PDFDocument::findDocument(const QString &fileName)
{
	QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

	foreach (QWidget *widget, qApp->topLevelWidgets()) {
		PDFDocument *theDoc = qobject_cast<PDFDocument*>(widget);
		if (theDoc && theDoc->curFile == canonicalFilePath)
			return theDoc;
	}
	return NULL;
}

void PDFDocument::zoomToRight(QWidget *otherWindow)
{
	QDesktopWidget *desktop = QApplication::desktop();
	QRect screenRect = desktop->availableGeometry(otherWindow == NULL ? this : otherWindow);
	screenRect.setTop(screenRect.top() + 22);
	screenRect.setLeft((screenRect.left() + screenRect.right()) / 2 + 1);
	screenRect.setBottom(screenRect.bottom() - 1);
	screenRect.setRight(screenRect.right() - 1);
	setGeometry(screenRect);
}

void PDFDocument::showPage(int page)
{
	pageLabel->setText(tr("page %1 of %2").arg(page).arg(pdfWidget->lastPage()));
}

void PDFDocument::showScale(qreal scale)
{
	scaleLabel->setText(tr("%1%").arg(ROUND(scale * 10000.0) / 100.0));
}

void PDFDocument::retypeset()
{
	if (sourceDocList.count() > 0)
		sourceDocList.first()->typeset();
}

void PDFDocument::interrupt()
{
	if (sourceDocList.count() > 0)
		sourceDocList.first()->interrupt();
}

void PDFDocument::goToSource()
{
	if (sourceDocList.count() > 0)
		sourceDocList.first()->selectWindow();
	else
		// should not occur, the action is supposed to be disabled
		actionGo_to_Source->setEnabled(false);
}

void PDFDocument::changedDocument(const QWeakPointer<QtPDF::Backend::Document> newDoc) {
	updateStatusBar();
	invalidateSyncHighlight();
	enablePageActions(pdfWidget->currentPage());
}

void PDFDocument::enablePageActions(int pageIndex)
{
//#if !defined(Q_OS_DARWIN)
// On Mac OS X, disabling these leads to a crash if we hit the end of document while auto-repeating a key
// (seems like a Qt bug, but needs further investigation)
// 2008-09-07: seems to no longer be a problem, probably thanks to Qt 4.4 update
	actionFirst_Page->setEnabled(pageIndex > 0);
	actionPrevious_Page->setEnabled(pageIndex > 0);
	actionNext_Page->setEnabled(pageIndex < pdfWidget->lastPage() - 1);
	actionLast_Page->setEnabled(pageIndex < pdfWidget->lastPage() - 1);
//#endif
}

void PDFDocument::toggleFullScreen()
{
	Q_ASSERT(_fullScreenManager);
	_fullScreenManager->toggleFullscreen();
}

void PDFDocument::setPageMode(const int newMode)
{
	if (!pdfWidget)
		return;

	switch (newMode) {
		case QtPDF::PDFDocumentView::PageMode_SinglePage:
		case QtPDF::PDFDocumentView::PageMode_OneColumnContinuous:
		case QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous:
			pdfWidget->setPageMode((QtPDF::PDFDocumentView::PageMode)newMode);
			break;
		default:
			return;
	}
}

void PDFDocument::updatePageMode(const QtPDF::PDFDocumentView::PageMode newMode)
{
	// Mark proper menu item
	actionPageMode_Single->setChecked(newMode == QtPDF::PDFDocumentView::PageMode_SinglePage);
	actionPageMode_Continuous->setChecked(newMode== QtPDF::PDFDocumentView::PageMode_OneColumnContinuous);
	actionPageMode_TwoPagesContinuous->setChecked(newMode == QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous);
}

void PDFDocument::resetMagnifier()
{
	Q_ASSERT(pdfWidget != NULL);
	QSETTINGS_OBJECT(settings);

	if (settings.value(QString::fromLatin1("circularMagnifier"), kDefault_CircularMagnifier).toBool())
		pdfWidget->setMagnifierShape(QtPDF::DocumentTool::MagnifyingGlass::Magnifier_Circle);
	else
		pdfWidget->setMagnifierShape(QtPDF::DocumentTool::MagnifyingGlass::Magnifier_Rectangle);

	pdfWidget->setMagnifierSize(magSizes[qBound(0, settings.value(QString::fromLatin1("magnifierSize"), kDefault_MagnifierSize).toInt() - 1, 2)]);
}

void PDFDocument::setResolution(const double res)
{
	Q_ASSERT(pdfWidget != NULL);
	if (res > 0)
		pdfWidget->setResolution(res);
}

void PDFDocument::enableTypesetAction(bool enabled)
{
	actionTypeset->setEnabled(enabled);
}

void PDFDocument::updateTypesettingAction(bool processRunning)
{
	if (processRunning) {
		disconnect(actionTypeset, SIGNAL(triggered()), this, SLOT(retypeset()));
		actionTypeset->setIcon(QIcon(QString::fromLatin1(":/images/tango/process-stop.png")));
		actionTypeset->setText(tr("Abort typesetting"));
		connect(actionTypeset, SIGNAL(triggered()), this, SLOT(interrupt()));
		enableTypesetAction(true);
	}
	else {
		disconnect(actionTypeset, SIGNAL(triggered()), this, SLOT(interrupt()));
		actionTypeset->setIcon(QIcon(QString::fromLatin1(":/images/images/runtool.png")));
		actionTypeset->setText(tr("Typeset"));
		connect(actionTypeset, SIGNAL(triggered()), this, SLOT(retypeset()));
	}
}

void PDFDocument::dragEnterEvent(QDragEnterEvent *event)
{
	// Only accept files for now
	event->ignore();
	if (event->mimeData()->hasUrls()) {
		const QList<QUrl> urls = event->mimeData()->urls();
		foreach (const QUrl& url, urls) {
			if (url.scheme() == QLatin1String("file")) {
				event->acceptProposedAction();
				break;
			}
		}
	}
}

void PDFDocument::dropEvent(QDropEvent *event)
{
	event->ignore();
	if (event->mimeData()->hasUrls()) {
		const QList<QUrl> urls = event->mimeData()->urls();
		foreach (const QUrl& url, urls)
			if (url.scheme() == QLatin1String("file"))
				TWApp::instance()->openFile(url.toLocalFile());
		event->acceptProposedAction();
	}
}

void PDFDocument::contextMenuEvent(QContextMenuEvent *event)
{
	Q_ASSERT(pdfWidget != NULL);
	QMenu menu(this);

	// Disarm the active tool (if any) as the menu will be highjacking all
	// events. This will, e.g., close an open magnifier view, which would
	// otherwise not receive a proper mouseReleaseEvent
	pdfWidget->disarmTool();

	if (_synchronizer && _synchronizer->isValid()) {
		QAction *act = new QAction(tr("Jump to Source"), &menu);
		act->setData(QVariant(event->pos()));
		connect(act, SIGNAL(triggered()), this, SLOT(jumpToSource()));
		menu.addAction(act);
		menu.addSeparator();
	}

	menu.addAction(tr("Zoom In"), pdfWidget, SLOT(zoomIn()));
	menu.addAction(tr("Zoom Out"), pdfWidget, SLOT(zoomOut()));
	menu.addAction(tr("Actual Size"), pdfWidget, SLOT(zoom100()));
	menu.addAction(tr("Fit to Width"), pdfWidget, SLOT(zoomFitWidth()));
	menu.addAction(tr("Fit to Window"), pdfWidget, SLOT(zoomFitWindow()));

	menu.exec(event->globalPos());
}

void PDFDocument::mouseMoveEvent(QMouseEvent *event)
{
	if (_fullScreenManager) _fullScreenManager->mouseMoveEvent(event);
}

void PDFDocument::jumpToSource()
{
	QAction *act = qobject_cast<QAction*>(sender());
	if (!act || !pdfWidget)
		return;

	QPoint eventPos = act->data().toPoint();
	QPointF scenePos = pdfWidget->mapToScene(pdfWidget->mapFrom(this, eventPos));

	// Map to scene, then map to page
	QtPDF::PDFDocumentScene * scene = qobject_cast<QtPDF::PDFDocumentScene*>(pdfWidget->scene());
	if (!scene)
		return;
	QtPDF::PDFPageGraphicsItem * page = (QtPDF::PDFPageGraphicsItem*)(scene->pageAt(scenePos));
	if (!page)
		return;

	syncClick(scene->pageNumFor(page), page->mapToPage(page->mapFromScene(scenePos)));
}

void PDFDocument::doFindDialog()
{
	if (PDFFindDialog::doFindDialog(this) == QDialog::Accepted)
		doFindAgain(true);
}

void PDFDocument::doFindAgain(bool newSearch /* = false */)
{
	QSETTINGS_OBJECT(settings);

	QString	searchText = settings.value(QString::fromLatin1("searchText")).toString();
	if (searchText.isEmpty())
		return;

	QtPDF::Backend::SearchFlags searchFlags;
	QTextDocument::FindFlags flags = (QTextDocument::FindFlags)settings.value(QString::fromLatin1("searchFlags")).toInt();

	if ((flags & QTextDocument::FindCaseSensitively) == 0)
		searchFlags |= QtPDF::Backend::Search_CaseInsensitive;
	if ((flags & QTextDocument::FindBackward) != 0)
		searchFlags |= QtPDF::Backend::Search_Backwards;

	widget()->search(searchText, searchFlags);
}

void PDFDocument::searchResultHighlighted(const int pageNum, const QList<QPolygonF> pdfRegion)
{
	QSETTINGS_OBJECT(settings);

	widget()->setCurrentSearchResultHighlightBrush(_searchResultHighlightBrush);
	if (kPDFHighlightDuration > 0)
		_searchResultHighlightRemover.start(kPDFHighlightDuration);

	if (hasSyncData() && settings.value(QString::fromLatin1("searchPdfSync")).toBool() && !pdfRegion.isEmpty()) {
		// In order to properly synchronize the search result, we first obtain
		// the bounding boxes of the first and last matched characters and then
		// use those for synchronization

		QSharedPointer<QtPDF::Backend::Document> doc = widget()->document().toStrongRef();
		Q_ASSERT(doc);
		QSharedPointer<QtPDF::Backend::Page> page = doc->page(pageNum);
		Q_ASSERT(page);
		QMap<int, QRectF> charBoxes;

		// NOTE: pdfRegion is in PDF coordinates (i.e., (0,0) is in the lower
		// left), whereas QtPDF::Backend::Page::selectedText() expects TeX
		// coordinates (i.e., (0,0) in the upper left). Hence we need to convert
		// the coordinates
		QList<QPolygonF> region;
		foreach (QPolygonF pdfPolygon, pdfRegion) {
			QPolygonF polygon;
			foreach (QPointF p, pdfPolygon)
				polygon << QPointF(p.x(), page->pageSizeF().height() - p.y());
			region << polygon;
		}

		// Obtain the chracter bounding boxes of the search result
		page->selectedText(region, NULL, &charBoxes, true);
		Q_ASSERT(charBoxes.size() > 0);

		// Obtain the centers of the first and last character bounding boxes and
		// convert them to PDF coordinates (i.e., (0,0) in the lower left) as
		// required by syncRange()
		QPointF pt1 = charBoxes[0].center();
		QPointF pt2 = charBoxes[charBoxes.size() - 1].center();
		pt1.ry() = page->pageSizeF().height() - pt1.y();
		pt2.ry() = page->pageSizeF().height() - pt2.y();

		// Perform the synchronization
		syncRange(pageNum, pt1, pt2, TWSynchronizer::CharacterResolution);
	}
}

void PDFDocument::setDefaultScale() {
	QSETTINGS_OBJECT(settings);
	switch (settings.value(QString::fromLatin1("scaleOption"), kDefault_PreviewScaleOption).toInt()) {
		case 2:
			pdfWidget->zoomFitWidth();
			break;
		case 3:
			pdfWidget->zoomFitWindow();
			break;
		case 4:
		    pdfWidget->setZoomLevel(settings.value(QString::fromLatin1("previewScale"), kDefault_PreviewScale).toFloat() / 100.);
			break;
		default:
			pdfWidget->zoom100();
			break;
	}
}

void PDFDocument::maybeOpenUrl(const QUrl url)
{
	// Opening URLs could be a security risk, so ask the user (but make "yes,
	// proceed the default option - after all the user typically clicked on the
	// link deliberately)
	if (QMessageBox::question(this, tr("Open URL"), tr("You are in the process of opening the URL %1. Opening unknown or untrusted web adresses can be a security risk.\nDo you want to continue?").arg(url.toString()),
	                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
		QDesktopServices::openUrl(url);
}

void PDFDocument::maybeOpenPdf(QString filename, QtPDF::PDFDestination destination, bool newWindow)
{
	// Unlike in maybeOpenUrl, this function only works on local PDF files which
	// we assume are safe.
	// TODO: We currently ignore the value of newWindow and always open a new
	// window. This avoids the need to update/invalidate all pointers to this
	// PDFDocument (e.g., in the TeXDocument associated with it) to notify the
	// other parts of the code that a completely new and unrelated document is
	// loaded here now.
	PDFDocument * pdf = qobject_cast<PDFDocument*>(TWApp::instance()->openFile(filename));
	if (!pdf || !pdf->widget())
		return;
	pdf->widget()->goToPDFDestination(destination, false);
}


void PDFDocument::print()
{
#if defined(MIKTEX_TODO_PRINT)
  // see http://code.google.com/p/texworks/issues/detail?id=78#c1
  QPrinter printer(QPrinter::HighResolution);
  QPrintDialog printDlg(&printer, this);
  QPainter painter;
  QProgressDialog progressDlg(this);
  Poppler::Page * page;
  QImage pageImage;
  QRect viewport;
  int dpiX, dpiY;
  double dpiXScale = 1.0, dpiYScale = 1.0;
  double scale;
  unsigned int firstPage, lastPage, i;
  bool success = false;

  // check if there's a pdf document to print
  if (!document) return;

  // check if we have permission to print the document
  if (!document->okToPrint()) {
    QMessageBox::critical(this, tr("Printing denied"), tr("You are not permitted to print this document"));
    return;
  }

  // Set up some basic information about the document
  printer.setCreator(TEXWORKS_NAME);
  printer.setDocName(document->info("Title"));
  if (printer.docName().isEmpty()) printer.setDocName(QFileInfo(curFile).baseName());

  // do some setup for the print dialog
  printDlg.setMinMax(1, document->numPages());
  printDlg.setOption(QAbstractPrintDialog::PrintToFile, true);
  printDlg.setOption(QAbstractPrintDialog::PrintSelection, false);
  printDlg.setOption(QAbstractPrintDialog::PrintPageRange, true);
  printDlg.setOption(QAbstractPrintDialog::PrintCollateCopies, true);
  printDlg.setWindowTitle(tr("Print %1").arg(QFileInfo(curFile).fileName()));

  // show the print dialog to the user
  if (printDlg.exec() != QDialog::Accepted) return;

  // determine the print range
  switch (printDlg.printRange()) {
  case QAbstractPrintDialog::PageRange:
    firstPage = printDlg.fromPage();
    lastPage = printDlg.toPage();
    break;
  default:
    firstPage = 1;
    lastPage = document->numPages();
  }

  // On *nix-like platforms, try using lp for printing
#if defined(Q_WS_X11) || defined(Q_WS_MAC)
// Catch empty printer names (e.g. used by CUPS for "printing to pdf")
  if (!printer.printerName().isEmpty()) {
    QStringList arguments;

    arguments << "lp";

    arguments << QString("-d %1").arg(printer.printerName());
    arguments << QString("-n %1").arg(printer.numCopies());
    arguments << QString("-t \"%1\"").arg(printer.docName());
    arguments << QString("-P %1-%2").arg(firstPage).arg(lastPage);

    switch (printer.duplex()) {
    case QPrinter::DuplexNone:
      arguments << "-o sides=one-sided";
      break;
    case QPrinter::DuplexShortSide:
      arguments << "-o sides=two-sided-short-edge";
      break;
    case QPrinter::DuplexLongSide:
      arguments << "-o sides=two-sided-long-edge";
      break;
    default:
      break;
    }
    arguments << "--";

    arguments << QString("\"%1\"").arg(curFile);

    // passing arguments as QStringList didn't work for me - probably
    // because of improper quoting of spaces
    if (QProcess::execute(arguments.join(" ")) == 0) return;
  }
#endif // On *nix, try using lpr for printing

  // On Windows, try using OS native operation
#if defined(Q_WS_WIN)
  {
    // First try: directly passing postscript to the printer (if supported)
    // I'm not sure if this is supported anywhere
    // Note: QPrinter::getDC and QPrinter::releaseDC are undocumented
    HDC dc;
    Poppler::PSConverter * psConv;

    dc = printer.getDC();

    if (dc) {
      int nEscapeCode = POSTSCRIPT_PASSTHROUGH;
      if (ExtEscape(dc, QUERYESCSUPPORT, sizeof(int), (LPCSTR)&nEscapeCode, 0, NULL) > 0 && (psConv = document->psConverter())) {
        // Convert the pdf to postscript instructions
        QBuffer buffer;

        buffer.open(QBuffer::ReadWrite);
        psConv->setOutputDevice(&buffer);
        psConv->convert();

        // and send them to the printer
        success = (ExtEscape(dc, POSTSCRIPT_PASSTHROUGH, buffer.data().size(), buffer.data().data(), 0, NULL) > 0);

        buffer.close();
      }
      printer.releaseDC(dc);
      if (success) return;
    }
    // Second try: print by calling the system's standard printing program for pdf
    // Seems to only print to the default printer, hence disabled for now
/*
                wchar_t * filename;
                HRESULT coInit;
                filename = new wchar_t[curFile.size()];

                curFile.toWCharArray(filename);
                coInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                if(coInit == S_OK || coInit == S_FALSE) {
                        success = ((int)ShellExecute(NULL, L"print", filename, NULL, NULL, SW_NORMAL) > 32);
                        CoUninitialize();
                }
                delete[] filename;
                if(success) return;
*/
  }
#endif // On Windows, try using OS native operation

  // Fallback: use Qt for printing
  //
  // This means rendering the pages as images via poppler and printing those
  // To keep the rendering time and file size reasonable, resolution is
  // clamped to a maximum of 300dpi. If you need more, use another program ;).

  {
    // ignore printer margins while painting - the margins are included in the
    // pdf file
    printer.setFullPage(true);

    painter.begin(&printer);
    viewport = painter.viewport();

    dpiX = printer.printEngine()->metric(QPaintDevice::PdmDpiX);
    dpiY = printer.printEngine()->metric(QPaintDevice::PdmDpiY);

    // clamp the resolution to 300 dpi (postscript devices return 1200 dpi to
    // Qt by default) to reduce file size and render time
    dpiXScale = qMax(1, dpiX / 300);
    dpiYScale = qMax(1, dpiY / 300);
    dpiX = qMin(300, dpiX);
    dpiY = qMin(300, dpiY);

    progressDlg.setAutoClose(true);
    progressDlg.setRange(firstPage, lastPage + 1);
    progressDlg.setWindowModality(Qt::WindowModal);
    for (i = firstPage; i <= lastPage && !progressDlg.wasCanceled(); ++i) {
      progressDlg.setValue(i - firstPage);
      progressDlg.setLabelText(tr("Printing page %1 of %2").arg(i - firstPage + 1).arg(lastPage - firstPage + 1));

      page = document->page(i - 1);
      if (page) {
        pageImage = page->renderToImage(dpiX, dpiY);
        painter.save();

        // calculate the scale factor necessary to resize the page image to
        // the real page while retaining the aspect ratio
        scale = qMin((double)viewport.width() / (dpiXScale * pageImage.width()), (double)viewport.height() / (dpiYScale * pageImage.height()));

        // if we need to shrink the page, so be it
        if (scale < 1) {
          painter.scale(scale, scale);
        }
        // otherwise center the page image on the page
        // TODO: handle landscape etc.
        else {
          painter.translate((viewport.width() - dpiXScale * pageImage.width()) / 2, (viewport.height() - dpiYScale * pageImage.height()) / 2);
        }
        painter.scale(dpiXScale, dpiYScale);

        painter.drawImage(0, 0, pageImage);

        painter.restore();
      }

      if (i != lastPage) printer.newPage();
    }

    if (progressDlg.wasCanceled()) printer.abort();
    else painter.end();

    progressDlg.reset();
  }
#else
	// Currently, printing is not supported in a reliable, cross-platform way
	// Instead, offer to open the document in the system's default viewer
	
	QString msg = tr("Unfortunately, this version of %1 is unable to print Pdf documents due to various technical reasons.\n").arg(QString::fromLatin1(TEXWORKS_NAME));
	msg += tr("Do you want to open the file in the default viewer for printing instead?");
	msg += tr(" (remember to close it again to avoid access problems)");
	
	if(QMessageBox::information(this,
		tr("Print Pdf..."), msg,
		QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes
	) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(curFile));
	}
#endif
}

void PDFDocument::showScaleContextMenu(const QPoint pos)
{
	static QMenu * contextMenu = NULL;
	
	if (contextMenu == NULL) {
		contextMenu = new QMenu(this);
		static QSignalMapper * contextMenuMapper = new QSignalMapper(this);
		QAction * a;
		
		contextMenu->addAction(actionFit_to_Width);
		contextMenu->addAction(actionFit_to_Window);
		contextMenu->addSeparator();
		
		a = contextMenu->addAction(tr("Custom..."));
		connect(a, SIGNAL(triggered()), this, SLOT(doScaleDialog()));

		a = contextMenu->addAction(tr("200%"));
		connect(a, SIGNAL(triggered()), contextMenuMapper, SLOT(map()));
		contextMenuMapper->setMapping(a, QString::fromLatin1("2"));
		a = contextMenu->addAction(tr("150%"));
		connect(a, SIGNAL(triggered()), contextMenuMapper, SLOT(map()));
		contextMenuMapper->setMapping(a, QString::fromLatin1("1.5"));
		// "100%" corresponds to "Actual Size", but we keep the numeric value
		// here for consistency
		a = contextMenu->addAction(tr("100%"));
		a->setShortcut(actionActual_Size->shortcut());
		connect(a, SIGNAL(triggered()), contextMenuMapper, SLOT(map()));
		contextMenuMapper->setMapping(a, QString::fromLatin1("1"));
		a = contextMenu->addAction(tr("75%"));
		connect(a, SIGNAL(triggered()), contextMenuMapper, SLOT(map()));
		contextMenuMapper->setMapping(a, QString::fromLatin1(".75"));
		a = contextMenu->addAction(tr("50%"));
		connect(a, SIGNAL(triggered()), contextMenuMapper, SLOT(map()));
		contextMenuMapper->setMapping(a, QString::fromLatin1(".5"));
		
		connect(contextMenuMapper, SIGNAL(mapped(const QString&)), this, SLOT(setScaleFromContextMenu(const QString&)));
	}
	
	contextMenu->popup(scaleLabel->mapToGlobal(pos));
}

void PDFDocument::setScaleFromContextMenu(const QString & strZoom)
{
	bool conversionOK = false;
	float zoom = strZoom.toFloat(&conversionOK);
	// FIXME: This should actually use the point the context menu was opened at as
	// anchor for zooming. Currently, arbitrary coordinates are not supported yet
	// (and using QGraphicsView::AnchorUnderMouse would use the position of the
	// mouse cursor when the user clicks on the respective menu item - which will
	// be somewhere else
	if (pdfWidget && conversionOK)
		pdfWidget->setZoomLevel(zoom);
}

void PDFDocument::updateStatusBar()
{
	Q_ASSERT(pdfWidget != NULL);
	showPage(pdfWidget->currentPage() + 1);
	showScale(pdfWidget->zoomLevel());
}

void PDFDocument::setMouseMode(const int newMode)
{
	Q_ASSERT(pdfWidget != NULL);
	pdfWidget->setMouseMode((QtPDF::PDFDocumentView::MouseMode)newMode);
}

void PDFDocument::doPageDialog()
{
	bool ok;
	Q_ASSERT(pdfWidget != NULL);

#if QT_VERSION < 0x050000
	int pageNo = QInputDialog::getInteger(this, tr("Go to Page"),
									tr("Page number:"), pdfWidget->currentPage() + 1,
                  1, pdfWidget->lastPage(), 1, &ok);
#else
	int pageNo = QInputDialog::getInt(this, tr("Go to Page"),
									tr("Page number:"), pdfWidget->currentPage() + 1,
                  1, pdfWidget->lastPage(), 1, &ok);
#endif
	if (ok)
		pdfWidget->goToPage(pageNo - 1);
}

void PDFDocument::doScaleDialog()
{
	bool ok;
	Q_ASSERT(pdfWidget != NULL);

	double newScale = QInputDialog::getDouble(this, tr("Set Zoom"), tr("Zoom level:"), 100 * pdfWidget->zoomLevel(), 0, 2147483647, 0, &ok);
	if (ok)
		pdfWidget->setZoomLevel(newScale / 100);
}

FullscreenManager::FullscreenManager(QMainWindow * parent)
	: _parent(parent)
{
	addShortcut(Qt::Key_Escape, SLOT(toggleFullScreen()));
	_menuBarTimer.setSingleShot(true);
	_menuBarTimer.setInterval(500);
	connect(&_menuBarTimer, SIGNAL(timeout()), this, SLOT(showMenuBar()));
}

//virtual
FullscreenManager::~FullscreenManager()
{
	foreach (const shortcut_info & sci, _shortcuts) {
		if (sci.shortcut) delete sci.shortcut;
	}
}

void FullscreenManager::setFullscreen(const bool fullscreen /* = true */)
{
	if (!_parent) return;
	if (fullscreen == isFullscreen()) return;

	if (fullscreen) {
		// entering full-screen mode

		// store the visibilities of important widgets such as the menu bar
		// before hiding them (so we can restore them when exiting fullscreen
		// mode)
		_normalVisibility.clear();
		if (_parent->menuBar()) {
			_normalVisibility[_parent->menuBar()] = _parent->menuBar()->isVisible();
			hideMenuBar();
		}
		if (_parent->statusBar()) {
			_normalVisibility[_parent->statusBar()] = _parent->statusBar()->isVisible();
			_parent->statusBar()->hide();
		}
		foreach (QToolBar * tb, _parent->findChildren<QToolBar*>()) {
			_normalVisibility[tb] = tb->isVisible();
			tb->hide();
		}

		_parent->showFullScreen();

		// Enable custom shortcuts
		foreach (const shortcut_info & sci, _shortcuts) {
			// Skip shortcuts that are associated with a menu QAction; those are
			// enabled/disabled when the menubar is hidden/shown
			if (sci.action) continue;
			sci.shortcut->setEnabled(fullscreen);
		}
	}
	else if (!fullscreen) {
		// exiting full-screen mode
		// stop the timer, just in case
		_menuBarTimer.stop();

		_parent->showNormal();

		// restore visibilities
		foreach (QWidget * w, _normalVisibility.keys())
			w->setVisible(_normalVisibility[w]);

		// Disable custom shortcuts
		foreach (const shortcut_info & sci, _shortcuts)
			sci.shortcut->setEnabled(false);
	}

	// Enable/disable mouse tracking (which is required for getting mouse move
	// events when no mouse button is pressed) to be able to show/hide the
	// menu bar depending on the mouse position
	_parent->setMouseTracking(fullscreen);

	emit fullscreenChanged(fullscreen);
}

bool FullscreenManager::isFullscreen() const
{
	if (!_parent) return false;
	return _parent->windowState().testFlag(Qt::WindowFullScreen);
}

void FullscreenManager::toggleFullscreen()
{
	setFullscreen(!isFullscreen());
}

void FullscreenManager::mouseMoveEvent(QMouseEvent * event)
{
	if (!_parent || !_parent->menuBar() || !isFullscreen()) return;
	const int thresholdHeight = 10;

	// The menu bar is shown when the mouse stays within thresholdHeight
	// from the top of the screen for _menuBarTimer.interval()
	// When the mouse moves in(to) that area and the timer is not running,
	// start it
	if (!_parent->menuBar()->isVisible()) {
		if (event->pos().y() <= thresholdHeight && !_menuBarTimer.isActive()) _menuBarTimer.start();
		// When the mouse moves out(side) of that area and the timer is running,
		// stop it
		else if (event->pos().y() > thresholdHeight && _menuBarTimer.isActive()) _menuBarTimer.stop();
	}
	// The menu bar is hidden whenever the mouse moves off of the menu bar
	// (Note: when opening a menu, that menu intercepts all mouse events so we
	// don't need to worry about the menu bar disappearing while the user
	// browses through the menus.
	else if (event->pos().y() > _parent->menuBar()->height())
		hideMenuBar();
}

void FullscreenManager::addShortcut(QAction * action, const char * member)
{
	addShortcut(action->shortcut(), member, action);
}

void FullscreenManager::addShortcut(const QKeySequence & key, const char * member, QAction * action /* = NULL */)
{
	shortcut_info sci;
	sci.shortcut = new QShortcut(key, _parent, member);
	sci.shortcut->setEnabled(false);
	sci.action = action;
	if (action)
		connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDeleted(QObject*)));
	_shortcuts << sci;
}

void FullscreenManager::actionDeleted(QObject * obj)
{
	QAction * a = qobject_cast<QAction *>(obj);
	if (!a) return;
	for (unsigned int i = 0; i < _shortcuts.size(); ) {
		if (_shortcuts[i].action == a) {
			if (_shortcuts[i].shortcut)
				delete _shortcuts[i].shortcut;
			_shortcuts.removeAt(i);
		}
		else
			++i;
	}
}

void FullscreenManager::setMenuBarVisible(const bool visible /* = true */)
{
	if (!_parent || !_parent->menuBar()) return;
	if (visible == _parent->menuBar()->isVisible()) return;

	_parent->menuBar()->setVisible(visible);

	// Enable our shortcut overrides when the menubar is hidden (to ensure that
	// the most important shortcuts are available even when the corresponding
	// QActions are hidden and disabled)
	foreach (const shortcut_info & sci, _shortcuts) {
		// Skip shortcuts that are not associated with any menu QAction
		if (!sci.action) continue;

		// If the shortcut gets enabled and corresponds to a valid, named
		// QAction, update it to the QAction's key sequence (in case that
		// got changed in the meantime)
		if (!visible)
			sci.shortcut->setKey(sci.action->shortcut());
		sci.shortcut->setEnabled(!visible);
	}
}
