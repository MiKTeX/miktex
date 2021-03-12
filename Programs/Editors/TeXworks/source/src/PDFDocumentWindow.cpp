/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2021  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "PDFDocumentWindow.h"

#include "FindDialog.h"
#include "Settings.h"
#include "TWApp.h"
#include "TWUtils.h"
#include "TeXDocumentWindow.h"

#include <QCloseEvent>
#include <QDesktopServices>
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QDesktopWidget>
#endif
#include <QDockWidget>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QPaintEngine>
#include <QPainter>
#include <QRegion>
#include <QScrollArea>
#include <QScrollBar>
#include <QShortcut>
#include <QSignalMapper>
#include <QStack>
#include <QStatusBar>
#include <QStyle>
#include <QToolTip>
#include <QUrl>
#include <QVector>
#include <cmath>


#define SYNCTEX_GZ_EXT	".synctex.gz"
#define SYNCTEX_EXT		".synctex"

#define ROUND(x) floor((x)+0.5)

// Possible sizes of the magnifying glass (in pixel)
const int magSizes[] = { 200, 300, 400 };

// duration of highlighting in PDF view (might make configurable?)
const int kPDFHighlightDuration = 2000;



// TODO: This is seemingly unused---verify && remove
QList<PDFDocumentWindow*> PDFDocumentWindow::docList;

PDFDocumentWindow::PDFDocumentWindow(const QString &fileName, TeXDocumentWindow *texDoc)
	: pdfWidget(nullptr)
	, scrollArea(nullptr)
	, toolButtonGroup(nullptr)
	, pageLabel(nullptr)
	, scaleLabel(nullptr)
	, _fullScreenManager(nullptr)
	, _syncHighlight(nullptr)
	, openedManually(false)
	, _synchronizer(nullptr)
{
	init();

	if (!texDoc)
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

#if QT_VERSION < QT_VERSION_CHECK(5, 4, 0)
	QTimer::singleShot(100, this, SLOT(setDefaultScale()));
#else
	QTimer::singleShot(100, this, &PDFDocumentWindow::setDefaultScale);
#endif

	if (texDoc) {
		stackUnder(texDoc);
		actionSide_by_Side->setEnabled(true);
		actionGo_to_Source->setEnabled(true);
		sourceDocList.append(texDoc);
	}
}

PDFDocumentWindow::~PDFDocumentWindow()
{
	docList.removeAll(this);
#if defined(Q_OS_DARWIN)
	// Work around QTBUG-17941
	QTimer::singleShot(0, TWApp::instance(), &TWApp::recreateSpecialMenuItems);
#endif // defined(Q_OS_DARWIN)
}

void PDFDocumentWindow::init()
{
	docList.append(this);

	setupUi(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

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

	connect(pdfWidget, &QtPDF::PDFDocumentWidget::changedPage, this, &PDFDocumentWindow::updateStatusBar);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::changedZoom, this, &PDFDocumentWindow::updateStatusBar);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::changedDocument, this, &PDFDocumentWindow::changedDocument);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::searchResultHighlighted, this, &PDFDocumentWindow::searchResultHighlighted);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::changedPageMode, this, &PDFDocumentWindow::updatePageMode);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::requestOpenPdf, this, &PDFDocumentWindow::maybeOpenPdf);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::requestOpenUrl, this, &PDFDocumentWindow::maybeOpenUrl);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::textSelectionChanged, this, &PDFDocumentWindow::maybeEnableCopyCommand);

	toolButtonGroup = new QButtonGroup(toolBar);
	toolButtonGroup->addButton(qobject_cast<QAbstractButton*>(toolBar->widgetForAction(actionMagnify)), QtPDF::PDFDocumentView::MouseMode_MagnifyingGlass);
	toolButtonGroup->addButton(qobject_cast<QAbstractButton*>(toolBar->widgetForAction(actionScroll)), QtPDF::PDFDocumentView::MouseMode_Move);
	toolButtonGroup->addButton(qobject_cast<QAbstractButton*>(toolBar->widgetForAction(actionSelect_Text)), QtPDF::PDFDocumentView::MouseMode_Select);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	connect(toolButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &PDFDocumentWindow::setMouseMode);
#else
	connect(toolButtonGroup, &QButtonGroup::idClicked, this, &PDFDocumentWindow::setMouseMode);
#endif
	pdfWidget->setMouseModeMagnifyingGlass();

	scaleLabel = new Tw::UI::ClickableLabel();
	statusBar()->addPermanentWidget(scaleLabel);
	scaleLabel->setFrameStyle(QFrame::StyledPanel);
	scaleLabel->setFont(statusBar()->font());
	connect(scaleLabel, &Tw::UI::ClickableLabel::mouseLeftClick, this, &PDFDocumentWindow::scaleLabelClick);

	pageLabel = new Tw::UI::ClickableLabel();
	statusBar()->addPermanentWidget(pageLabel);
	pageLabel->setFrameStyle(QFrame::StyledPanel);
	pageLabel->setFont(statusBar()->font());
	connect(pageLabel, &Tw::UI::ClickableLabel::mouseLeftClick, this, &PDFDocumentWindow::doPageDialog);

	connect(actionAbout_TW, &QAction::triggered, TWApp::instance(), &TWApp::about);
	connect(actionSettings_and_Resources, &QAction::triggered, TWApp::instance(), &TWApp::doResourcesDialog);
	connect(actionGoToHomePage, &QAction::triggered, TWApp::instance(), &TWApp::goToHomePage);
	connect(actionWriteToMailingList, &QAction::triggered, TWApp::instance(), &TWApp::writeToMailingList);

	connect(actionNew, &QAction::triggered, TWApp::instance(), &TWApp::newFile);
	connect(actionNew_from_Template, &QAction::triggered, TWApp::instance(), &TWApp::newFromTemplate);
	connect(actionOpen, &QAction::triggered, []() { TWApp::instance()->open(TWUtils::chooseDefaultFilter(QStringLiteral("a.pdf"), *TWUtils::filterList())); });
	connect(actionPrintPdf, &QAction::triggered, this, &PDFDocumentWindow::print);

	connect(actionQuit_TeXworks, &QAction::triggered, TWApp::instance(), &TWApp::maybeQuit);

	connect(actionCopy, &QAction::triggered, this, &PDFDocumentWindow::copySelectedTextToClipboard);

	connect(actionFind, &QAction::triggered, this, &PDFDocumentWindow::doFindDialog);

	connect(actionFirst_Page, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::goFirst);
	connect(actionPrevious_Page, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::goPrev);
	connect(actionNext_Page, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::goNext);
	connect(actionLast_Page, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::goLast);
	connect(actionGo_to_Page, &QAction::triggered, this, &PDFDocumentWindow::doPageDialog);
	addAction(actionPrevious_ViewRect);
	connect(actionPrevious_ViewRect, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::goPrevViewRect);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::changedPage, this, &PDFDocumentWindow::enablePageActions);

	connect(actionActual_Size, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::zoom100);
	connect(actionFit_to_Width, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::zoomFitWidth);
	connect(actionFit_to_Window, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::zoomFitWindow);
	connect(actionFit_to_Content_Width, &QAction::triggered, pdfWidget, &QtPDF::PDFDocumentWidget::zoomFitContentWidth);
	connect(actionZoom_In, &QAction::triggered, pdfWidget, [=]() { pdfWidget->zoomIn(); });
	connect(actionZoom_Out, &QAction::triggered, pdfWidget, [=]() { pdfWidget->zoomOut(); });
	connect(actionFull_Screen, &QAction::triggered, this, &PDFDocumentWindow::toggleFullScreen);
	connect(pdfWidget, &QtPDF::PDFDocumentWidget::contextClick, this, &PDFDocumentWindow::syncClick);
	pageModeSignalMapper.setMapping(actionPageMode_Single, QtPDF::PDFDocumentView::PageMode_SinglePage);
	pageModeSignalMapper.setMapping(actionPageMode_Continuous, QtPDF::PDFDocumentView::PageMode_OneColumnContinuous);
	pageModeSignalMapper.setMapping(actionPageMode_TwoPagesContinuous, QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous);
	connect(actionPageMode_Single, &QAction::triggered, &pageModeSignalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
	connect(actionPageMode_Continuous, &QAction::triggered, &pageModeSignalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
	connect(actionPageMode_TwoPagesContinuous, &QAction::triggered, &pageModeSignalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	connect(&pageModeSignalMapper, static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped), this, &PDFDocumentWindow::setPageMode);
#else
	connect(&pageModeSignalMapper, &QSignalMapper::mappedInt, this, &PDFDocumentWindow::setPageMode);
#endif

	if (actionZoom_In->shortcut() == QKeySequence(tr("Ctrl++")))
		new QShortcut(QKeySequence(tr("Ctrl+=")), pdfWidget, SLOT(zoomIn()));

	connect(actionTypeset, &QAction::triggered, this, &PDFDocumentWindow::retypeset);

	connect(actionStack, &QAction::triggered, TWApp::instance(), &TWApp::stackWindows);
	connect(actionTile, &QAction::triggered, TWApp::instance(), &TWApp::tileWindows);
	connect(actionSide_by_Side, &QAction::triggered, this, &PDFDocumentWindow::sideBySide);
	connect(actionPlace_on_Left, &QAction::triggered, this, &PDFDocumentWindow::placeOnLeft);
	connect(actionPlace_on_Right, &QAction::triggered, this, &PDFDocumentWindow::placeOnRight);
	connect(actionGo_to_Source, &QAction::triggered, this, &PDFDocumentWindow::goToSource);

	connect(actionFind_Again, &QAction::triggered, this, &PDFDocumentWindow::doFindAgain);

	updateRecentFileActions();
	connect(TWApp::instance(), &TWApp::recentFileActionsChanged, this, &PDFDocumentWindow::updateRecentFileActions);
	connect(TWApp::instance(), &TWApp::windowListChanged, this, &PDFDocumentWindow::updateWindowMenu);
	connect(actionClear_Recent_Files, &QAction::triggered, TWApp::instance(), &TWApp::clearRecentFiles);

	connect(TWApp::instance(), &TWApp::hideFloatersExcept, this, &PDFDocumentWindow::hideFloatersUnlessThis);
	connect(this, &PDFDocumentWindow::activatedWindow, TWApp::instance(), &TWApp::activatedWindow);

	connect(actionPreferences, &QAction::triggered, TWApp::instance(), &TWApp::preferences);

	connect(this, &PDFDocumentWindow::destroyed, TWApp::instance(), &TWApp::updateWindowMenus);

	connect(TWApp::instance(), &TWApp::syncPdf, this, &PDFDocumentWindow::syncFromSource);

	_syncHighlightRemover.setSingleShot(true);
	connect(&_syncHighlightRemover, &QTimer::timeout, this, &PDFDocumentWindow::clearSyncHighlight);

	_searchResultHighlightRemover.setSingleShot(true);
	connect(&_searchResultHighlightRemover, &QTimer::timeout, this, &PDFDocumentWindow::clearSearchResultHighlight);

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

	Tw::Settings settings;
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

	if (settings.contains(QString::fromLatin1("previewResolution"))) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
		pdfWidget->setResolution(settings.value(QString::fromLatin1("previewResolution"), QApplication::desktop()->logicalDpiX()).toInt());
#else
		pdfWidget->setResolution(settings.value(QString::fromLatin1("previewResolution"), screen()->logicalDotsPerInch()).toInt());
#endif
	}

	TWUtils::applyToolbarOptions(this, settings.value(QString::fromLatin1("toolBarIconSize"), 2).toInt(), settings.value(QString::fromLatin1("toolBarShowText"), false).toBool());

	TWApp::instance()->updateWindowMenus();

	initScriptable(menuScripts, actionAbout_Scripts, actionManage_Scripts,
				   actionUpdate_Scripts, actionShow_Scripts_Folder);

	TWUtils::insertHelpMenuItems(menuHelp);
#if defined(MIKTEX)
        actionAbout_MiKTeX = new QAction(this);
        actionAbout_MiKTeX->setIcon(QIcon(QStringLiteral(":/MiKTeX/miktex16x16.png")));
        actionAbout_MiKTeX->setObjectName(QStringLiteral("actionAbout_MiKTeX"));
        actionAbout_MiKTeX->setText(QApplication::translate("PDFDocument", "Apropos MiKTeX..."));
        connect(actionAbout_MiKTeX, SIGNAL(triggered()), qApp, SLOT(aboutMiKTeX()));
#if 1
        menuHelp->addAction(actionAbout_MiKTeX);
#endif
#endif
	TWUtils::installCustomShortcuts(this);

	pdfWidget->setMouseTracking(true);
	_fullScreenManager = new Tw::Utils::FullscreenManager(this);
	_fullScreenManager->addShortcut(actionFull_Screen, SLOT(toggleFullScreen()));
	connect(_fullScreenManager, &Tw::Utils::FullscreenManager::fullscreenChanged, actionFull_Screen, &QAction::setChecked);
	connect(_fullScreenManager, &Tw::Utils::FullscreenManager::fullscreenChanged, this, &PDFDocumentWindow::maybeZoomToWindow, Qt::QueuedConnection);
}

void PDFDocumentWindow::changeEvent(QEvent *event)
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

void PDFDocumentWindow::linkToSource(TeXDocumentWindow *texDoc)
{
	if (texDoc) {
		if (!sourceDocList.contains(texDoc))
			sourceDocList.append(texDoc);
		actionGo_to_Source->setEnabled(true);
	}
}

void PDFDocumentWindow::texClosed(QObject *obj)
{
	TeXDocumentWindow *texDoc = reinterpret_cast<TeXDocumentWindow*>(obj);
	// can't use qobject_cast here as the object's metadata is already gone!
	if (texDoc) {
		sourceDocList.removeAll(texDoc);
		if (sourceDocList.empty())
			close();
	}
}

void PDFDocumentWindow::texActivated(TeXDocumentWindow * texDoc)
{
	// A source file was activated. Make sure it is the first in the list of
	// source docs so that future "Goto Source" actions point there.
	if (sourceDocList.first() != texDoc) {
		sourceDocList.removeAll(texDoc);
		sourceDocList.prepend(texDoc);
	}
}

void PDFDocumentWindow::updateRecentFileActions()
{
	TWUtils::updateRecentFileActions(this, recentFileActions, menuOpen_Recent, actionClear_Recent_Files);
}

void PDFDocumentWindow::updateWindowMenu()
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
			setWindowTitle(tr("%1[*] - %2").arg(selWinAction->text(), tr(TEXWORKS_NAME)));
	}
}

void PDFDocumentWindow::sideBySide()
{
	if (sourceDocList.count() > 0) {
		TWUtils::sideBySide(sourceDocList.first(), this);
		sourceDocList.first()->selectWindow(false);
		selectWindow();
	}
	else
		placeOnRight();
}

bool PDFDocumentWindow::event(QEvent *event)
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

void PDFDocumentWindow::closeEvent(QCloseEvent *event)
{
	event->accept();
	if (openedManually) {
		saveRecentFileInfo();
	}
	deleteLater();
}

void PDFDocumentWindow::saveRecentFileInfo()
{
	QMap<QString,QVariant> fileProperties;
	fileProperties.insert(QString::fromLatin1("path"), curFile);
	fileProperties.insert(QString::fromLatin1("geometry"), saveGeometry());
	fileProperties.insert(QString::fromLatin1("state"), saveState(kPDFWindowStateVersion));
	fileProperties.insert(QString::fromLatin1("pdfPageMode"), pdfWidget->pageMode());
	TWApp::instance()->addToRecentFiles(fileProperties);
}

void PDFDocumentWindow::loadFile(const QString &fileName)
{
	setCurrentFile(fileName);
	Tw::Settings settings;
	QFileInfo info(fileName);
	settings.setValue(QString::fromLatin1("openDialogDir"), info.canonicalPath());

	reload();
}

void PDFDocumentWindow::reload()
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

void PDFDocumentWindow::loadSyncData()
{
	if (_synchronizer) {
		delete _synchronizer;
		_synchronizer = nullptr;
	}
	_synchronizer = new TWSyncTeXSynchronizer(curFile, [](const QString & filename) {
			const TeXDocumentWindow * win = TeXDocumentWindow::openDocument(filename, false, false);
			return (win ? win->textDoc() : nullptr);
		}, [](const QString & filename) {
			PDFDocumentWindow * pdfWin = PDFDocumentWindow::findDocument(filename);
			return (pdfWin && pdfWin->widget() ? pdfWin->widget()->document().toStrongRef() : QSharedPointer<QtPDF::Backend::Document>());
		}
	);
	if (!_synchronizer)
		statusBar()->showMessage(tr("Error initializing SyncTeX"), kStatusMessageDuration);
	else if (!_synchronizer->isValid())
		statusBar()->showMessage(tr("No SyncTeX data available"), kStatusMessageDuration);
	else
		statusBar()->showMessage(tr("SyncTeX: \"%1\"").arg(_synchronizer->syncTeXFilename()), kStatusMessageDuration);
}

void PDFDocumentWindow::syncClick(int pageIndex, const QPointF& pos)
{
	Tw::Settings settings;
	TWSynchronizer::Resolution res{TWSynchronizer::kDefault_Resolution_ToPDF};
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
	}

	syncRange(pageIndex, pos, pos, res);
}

void PDFDocumentWindow::syncRange(const int pageIndex, const QPointF & start, const QPointF & end, const TWSynchronizer::Resolution resolution)
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
	TeXDocumentWindow * texDoc = TeXDocumentWindow::openDocument(QFileInfo(curDir, destStart.filename).canonicalFilePath(), true, true, destStart.line);
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
	TeXDocumentWindow::openDocument(QFileInfo(curDir, destStart.filename).canonicalFilePath(), true, true, destStart.line, curStart.position() - curStart.block().position(), curEnd.position() - curStart.block().position());
}

void PDFDocumentWindow::syncFromSource(const QString& sourceFile, int lineNo, int col, bool activatePreview)
{
	if (!_synchronizer)
		return;

	Tw::Settings settings;
	TWSynchronizer::Resolution res{TWSynchronizer::kDefault_Resolution_ToPDF};
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
	_syncHighlight = pdfWidget->addHighlightPath(static_cast<unsigned int>(dest.page - 1), path, QColor(255, 255, 0, 63));

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

void PDFDocumentWindow::invalidateSyncHighlight()
{
	// This slot should be called when the graphics item pointed to by
	// _syncHighlight goes out of scope (e.g., because the PDF changed, all pages
	// were deleted, and, in the process, all subordinate graphics items as well).
	_syncHighlight = nullptr;
	_syncHighlightRemover.stop();
}

void PDFDocumentWindow::clearSyncHighlight()
{
	if (_syncHighlight) {
		delete _syncHighlight;
		_syncHighlight = nullptr;
	}
	_syncHighlightRemover.stop();
}

void PDFDocumentWindow::clearSearchResultHighlight()
{
	if (!widget())
		return;
	widget()->setCurrentSearchResultHighlightBrush(QBrush(Qt::transparent));
}

void PDFDocumentWindow::copySelectedTextToClipboard()
{
	if (!widget()) return;
	QString textToCopy = widget()->selectedText();
	if (textToCopy.isEmpty())
		return;
	Q_ASSERT(QApplication::clipboard());
	QApplication::clipboard()->setText(textToCopy);
}

void PDFDocumentWindow::maybeEnableCopyCommand(const bool isTextSelected)
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

void PDFDocumentWindow::setCurrentFile(const QString &fileName)
{
	curFile = QFileInfo(fileName).canonicalFilePath();
	//: Format for the window title (ex. "file.pdf[*] - TeXworks")
	setWindowTitle(tr("%1[*] - %2").arg(TWUtils::strippedName(curFile), tr(TEXWORKS_NAME)));
	TWApp::instance()->updateWindowMenus();
}

PDFDocumentWindow *PDFDocumentWindow::findDocument(const QString &fileName)
{
	QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

	foreach (QWidget *widget, qApp->topLevelWidgets()) {
		PDFDocumentWindow *theDoc = qobject_cast<PDFDocumentWindow*>(widget);
		if (theDoc && theDoc->curFile == canonicalFilePath)
			return theDoc;
	}
	return nullptr;
}

void PDFDocumentWindow::showPage(int page)
{
	pageLabel->setText(tr("page %1 of %2").arg(page).arg(pdfWidget->lastPage()));
}

void PDFDocumentWindow::showScale(qreal scale)
{
	scaleLabel->setText(tr("%1%").arg(ROUND(scale * 10000.0) / 100.0));
}

void PDFDocumentWindow::retypeset()
{
	if (sourceDocList.count() > 0)
		sourceDocList.first()->typeset();
}

void PDFDocumentWindow::interrupt()
{
	if (sourceDocList.count() > 0)
		sourceDocList.first()->interrupt();
}

void PDFDocumentWindow::goToSource()
{
	if (sourceDocList.count() > 0)
		sourceDocList.first()->selectWindow();
	else
		// should not occur, the action is supposed to be disabled
		actionGo_to_Source->setEnabled(false);
}

void PDFDocumentWindow::changedDocument(const QWeakPointer<QtPDF::Backend::Document> & newDoc) {
	Q_UNUSED(newDoc)
	updateStatusBar();
	invalidateSyncHighlight();
	enablePageActions(pdfWidget->currentPage());
}

void PDFDocumentWindow::enablePageActions(int pageIndex)
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

void PDFDocumentWindow::toggleFullScreen()
{
	Q_ASSERT(_fullScreenManager);
	_fullScreenManager->toggleFullscreen();
}

void PDFDocumentWindow::setPageMode(const int newMode)
{
	if (!pdfWidget)
		return;

	switch (newMode) {
		case QtPDF::PDFDocumentView::PageMode_SinglePage:
		case QtPDF::PDFDocumentView::PageMode_OneColumnContinuous:
		case QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous:
			pdfWidget->setPageMode(static_cast<QtPDF::PDFDocumentView::PageMode>(newMode));
			break;
		default:
			return;
	}
}

void PDFDocumentWindow::updatePageMode(const QtPDF::PDFDocumentView::PageMode newMode)
{
	// Mark proper menu item
	actionPageMode_Single->setChecked(newMode == QtPDF::PDFDocumentView::PageMode_SinglePage);
	actionPageMode_Continuous->setChecked(newMode== QtPDF::PDFDocumentView::PageMode_OneColumnContinuous);
	actionPageMode_TwoPagesContinuous->setChecked(newMode == QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous);
}

void PDFDocumentWindow::resetMagnifier()
{
	Q_ASSERT(pdfWidget);
	Tw::Settings settings;

	if (settings.value(QString::fromLatin1("circularMagnifier"), kDefault_CircularMagnifier).toBool())
		pdfWidget->setMagnifierShape(QtPDF::DocumentTool::MagnifyingGlass::Magnifier_Circle);
	else
		pdfWidget->setMagnifierShape(QtPDF::DocumentTool::MagnifyingGlass::Magnifier_Rectangle);

	pdfWidget->setMagnifierSize(magSizes[qBound(0, settings.value(QString::fromLatin1("magnifierSize"), kDefault_MagnifierSize).toInt() - 1, 2)]);
}

void PDFDocumentWindow::setResolution(const double res)
{
	Q_ASSERT(pdfWidget);
	if (res > 0)
		pdfWidget->setResolution(res);
}

void PDFDocumentWindow::enableTypesetAction(bool enabled)
{
	actionTypeset->setEnabled(enabled);
}

void PDFDocumentWindow::updateTypesettingAction(bool processRunning)
{
	if (processRunning) {
		disconnect(actionTypeset, &QAction::triggered, this, &PDFDocumentWindow::retypeset);
		actionTypeset->setIcon(QIcon::fromTheme(QStringLiteral("process-stop")));
		actionTypeset->setText(tr("Abort typesetting"));
		connect(actionTypeset, &QAction::triggered, this, &PDFDocumentWindow::interrupt);
		enableTypesetAction(true);
	}
	else {
		disconnect(actionTypeset, &QAction::triggered, this, &PDFDocumentWindow::interrupt);
		actionTypeset->setIcon(QIcon::fromTheme(QStringLiteral("process-start")));
		actionTypeset->setText(tr("Typeset"));
		connect(actionTypeset, &QAction::triggered, this, &PDFDocumentWindow::retypeset);
	}
}

void PDFDocumentWindow::dragEnterEvent(QDragEnterEvent *event)
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

void PDFDocumentWindow::dropEvent(QDropEvent *event)
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

void PDFDocumentWindow::contextMenuEvent(QContextMenuEvent *event)
{
	Q_ASSERT(pdfWidget);

	// If initiated with the mouse, only show the context menu if the user
	// clicked inside the pdfWidget area
	if (event->reason() == QContextMenuEvent::Mouse && !pdfWidget->rect().contains(pdfWidget->mapFrom(this, event->pos()))) {
		TWScriptableWindow::contextMenuEvent(event);
		return;
	}

	QMenu menu(this);

	// Disarm the active tool (if any) as the menu will be highjacking all
	// events. This will, e.g., close an open magnifier view, which would
	// otherwise not receive a proper mouseReleaseEvent
	pdfWidget->disarmTool();

	if (_synchronizer && _synchronizer->isValid()) {
		QAction *act = new QAction(tr("Jump to Source"), &menu);
		act->setData(QVariant(event->pos()));
		connect(act, &QAction::triggered, this, &PDFDocumentWindow::jumpToSource);
		menu.addAction(act);
		menu.addSeparator();
	}

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
	menu.addAction(tr("Zoom In"), pdfWidget, SLOT(zoomIn()));
	menu.addAction(tr("Zoom Out"), pdfWidget, SLOT(zoomOut()));
	menu.addAction(tr("Actual Size"), pdfWidget, SLOT(zoom100()));
	menu.addAction(tr("Fit to Width"), pdfWidget, SLOT(zoomFitWidth()));
	menu.addAction(tr("Fit to Window"), pdfWidget, SLOT(zoomFitWindow()));
#else
	menu.addAction(tr("Zoom In"), pdfWidget, [=]() { pdfWidget->zoomIn(); });
	menu.addAction(tr("Zoom Out"), pdfWidget, [=]() { pdfWidget->zoomOut(); });
	menu.addAction(tr("Actual Size"), pdfWidget, &QtPDF::PDFDocumentWidget::zoom100);
	menu.addAction(tr("Fit to Width"), pdfWidget, &QtPDF::PDFDocumentWidget::zoomFitWidth);
	menu.addAction(tr("Fit to Window"), pdfWidget, &QtPDF::PDFDocumentWidget::zoomFitWindow);
#endif

	menu.exec(event->globalPos());
}

void PDFDocumentWindow::mouseMoveEvent(QMouseEvent *event)
{
	if (_fullScreenManager) _fullScreenManager->mouseMoveEvent(event);
}

void PDFDocumentWindow::jumpToSource()
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
	QtPDF::PDFPageGraphicsItem * page = dynamic_cast<QtPDF::PDFPageGraphicsItem*>(scene->pageAt(scenePos));
	if (!page)
		return;

	syncClick(scene->pageNumFor(page), page->mapToPage(page->mapFromScene(scenePos)));
}

void PDFDocumentWindow::doFindDialog()
{
	if (PDFFindDialog::doFindDialog(this) == QDialog::Accepted)
		doFindAgain(true);
}

void PDFDocumentWindow::doFindAgain(bool newSearch /* = false */)
{
	Q_UNUSED(newSearch)
	Tw::Settings settings;

	QString	searchText = settings.value(QString::fromLatin1("searchText")).toString();
	if (searchText.isEmpty())
		return;

	QtPDF::Backend::SearchFlags searchFlags;
	QTextDocument::FindFlags flags = static_cast<QTextDocument::FindFlags>(settings.value(QString::fromLatin1("searchFlags")).toInt());

	if ((flags & QTextDocument::FindCaseSensitively) == 0)
		searchFlags |= QtPDF::Backend::Search_CaseInsensitive;
	if ((flags & QTextDocument::FindBackward) != 0)
		searchFlags |= QtPDF::Backend::Search_Backwards;

	widget()->search(searchText, searchFlags);
}

void PDFDocumentWindow::searchResultHighlighted(const int pageNum, const QList<QPolygonF> & pdfRegion)
{
	Tw::Settings settings;

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
		page->selectedText(region, nullptr, &charBoxes, true);
		Q_ASSERT(!charBoxes.empty());

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

void PDFDocumentWindow::setDefaultScale() {
	Tw::Settings settings;
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

void PDFDocumentWindow::maybeOpenUrl(const QUrl & url)
{
	// Opening URLs could be a security risk, so ask the user (but make "yes,
	// proceed the default option - after all the user typically clicked on the
	// link deliberately)
	if (QMessageBox::question(this, tr("Open URL"), tr("You are in the process of opening the URL %1. Opening unknown or untrusted web adresses can be a security risk.\nDo you want to continue?").arg(url.toString()),
	                          QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
		QDesktopServices::openUrl(url);
}

void PDFDocumentWindow::maybeOpenPdf(const QString & filename, const QtPDF::PDFDestination & destination, const bool newWindow)
{
	Q_UNUSED(newWindow)
	// Unlike in maybeOpenUrl, this function only works on local PDF files which
	// we assume are safe.
	// TODO: We currently ignore the value of newWindow and always open a new
	// window. This avoids the need to update/invalidate all pointers to this
	// PDFDocument (e.g., in the TeXDocument associated with it) to notify the
	// other parts of the code that a completely new and unrelated document is
	// loaded here now.
	// NB: TWApp::openFile() requires an absolute filename. Therefore, we have
	// to make it absolute (using the current file's folder as base) if it isn't
	// already
	PDFDocumentWindow * pdf = qobject_cast<PDFDocumentWindow*>(TWApp::instance()->openFile([this] (const QString & filename) {
			const QFileInfo fi(filename);
			if (fi.isAbsolute()) {
				return filename;
			}
			return QFileInfo(this->fileName()).dir().filePath(filename);
		}(filename)
	));
	if (!pdf || !pdf->widget())
		return;
	pdf->widget()->goToPDFDestination(destination, false);
}


void PDFDocumentWindow::print()
{
	// Currently, printing is not supported in a reliable, cross-platform way
	// Instead, offer to open the document in the system's default viewer

	QString msg = tr("Unfortunately, this version of %1 is unable to print PDF documents due to various technical reasons.\n").arg(QString::fromLatin1(TEXWORKS_NAME));
	msg += tr("Do you want to open the file in the default viewer for printing instead?");
	msg += tr(" (remember to close it again to avoid access problems)");

	if(QMessageBox::information(this,
		tr("Print PDF..."), msg,
		QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes
	) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(curFile));
	}
}

void PDFDocumentWindow::showScaleContextMenu(const QPoint pos)
{
	static QMenu * contextMenu = nullptr;

	if (!contextMenu) {
		contextMenu = new QMenu(this);
		static QSignalMapper * contextMenuMapper = new QSignalMapper(this);
		QAction * a{nullptr};

		contextMenu->addAction(actionFit_to_Width);
		contextMenu->addAction(actionFit_to_Window);
		contextMenu->addSeparator();

		a = contextMenu->addAction(tr("Custom..."));
		connect(a, &QAction::triggered, this, &PDFDocumentWindow::doScaleDialog);

		a = contextMenu->addAction(tr("200%"));
		connect(a, &QAction::triggered, contextMenuMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
		contextMenuMapper->setMapping(a, QString::fromLatin1("2"));
		a = contextMenu->addAction(tr("150%"));
		connect(a, &QAction::triggered, contextMenuMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
		contextMenuMapper->setMapping(a, QString::fromLatin1("1.5"));
		// "100%" corresponds to "Actual Size", but we keep the numeric value
		// here for consistency
		a = contextMenu->addAction(tr("100%"));
		a->setShortcut(actionActual_Size->shortcut());
		connect(a, &QAction::triggered, contextMenuMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
		contextMenuMapper->setMapping(a, QString::fromLatin1("1"));
		a = contextMenu->addAction(tr("75%"));
		connect(a, &QAction::triggered, contextMenuMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
		contextMenuMapper->setMapping(a, QString::fromLatin1(".75"));
		a = contextMenu->addAction(tr("50%"));
		connect(a, &QAction::triggered, contextMenuMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
		contextMenuMapper->setMapping(a, QString::fromLatin1(".5"));

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
		connect(contextMenuMapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), this, &PDFDocumentWindow::setScaleFromContextMenu);
#else
		connect(contextMenuMapper, &QSignalMapper::mappedString, this, &PDFDocumentWindow::setScaleFromContextMenu);
#endif
	}

	contextMenu->popup(scaleLabel->mapToGlobal(pos));
}

void PDFDocumentWindow::setScaleFromContextMenu(const QString & strZoom)
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

void PDFDocumentWindow::updateStatusBar()
{
	Q_ASSERT(pdfWidget);
	showPage(pdfWidget->currentPage() + 1);
	showScale(pdfWidget->zoomLevel());
}

void PDFDocumentWindow::setMouseMode(const int newMode)
{
	Q_ASSERT(pdfWidget);
	pdfWidget->setMouseMode(static_cast<QtPDF::PDFDocumentView::MouseMode>(newMode));
}

void PDFDocumentWindow::doPageDialog()
{
	Q_ASSERT(pdfWidget);
	bool ok{false};

	int pageNo = QInputDialog::getInt(this, tr("Go to Page"),
									tr("Page number:"), pdfWidget->currentPage() + 1,
                  1, pdfWidget->lastPage(), 1, &ok);
	if (ok)
		pdfWidget->goToPage(pageNo - 1);
}

void PDFDocumentWindow::doScaleDialog()
{
	Q_ASSERT(pdfWidget);
	bool ok{false};

	double newScale = QInputDialog::getDouble(this, tr("Set Zoom"), tr("Zoom level:"), 100 * pdfWidget->zoomLevel(), 0, 2147483647, 0, &ok);
	if (ok)
		pdfWidget->setZoomLevel(newScale / 100);
}
