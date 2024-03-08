/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2023  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "../modules/QtPDF/src/PDFDocumentWidget.h"
#include "FindDialog.h"
#include "TWScriptableWindow.h"
#include "TWSynchronizer.h"
#include "ui/ClickableLabel.h"
#include "ui_PDFDocumentWindow.h"
#include "utils/FullscreenManager.h"

#include <QButtonGroup>
#include <QCursor>
#include <QImage>
#include <QList>
#include <QMouseEvent>
#include <QPainterPath>
#include <QTimer>


const int kDefault_MagnifierSize = 2;
const bool kDefault_CircularMagnifier = true;
const int kDefault_PreviewScaleOption = 1;
const int kDefault_PreviewScale = 200;
const QtPDF::PDFDocumentView::PageMode kDefault_PDFPageMode = QtPDF::PDFDocumentView::PageMode_OneColumnContinuous;
const bool kDefault_PreviewRulerShow = false;
const int kDefault_PreviewRulerUnits = QtPDF::Physical::Length::Centimeters;
const QColor kDefault_PaperColor = Qt::white;

const int kPDFWindowStateVersion = 1;

class QAction;
class QMenu;
class QToolBar;
class QScrollArea;
class TeXDocumentWindow;
class QShortcut;

class PDFDocumentWindow : public TWScriptableWindow, private Ui::PDFDocumentWindow
{
	Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName)

public:
	using size_type = QtPDF::Backend::Document::size_type;

	PDFDocumentWindow(const QString &fileName, TeXDocumentWindow *sourceDoc = nullptr);
	~PDFDocumentWindow() override;

	static PDFDocumentWindow *findDocument(const QString &fileName);
	static QList<PDFDocumentWindow*> documentList()
		{
			return docList;
		}

	QString fileName() const
		{ return curFile; }

	void showScale(qreal scale);
	void showPage(size_type page);
	void setResolution(const double res);
	void resetMagnifier();
	void enableTypesetAction(bool enabled);
	void linkToSource(TeXDocumentWindow *texDoc);
	bool hasSyncData() const { return static_cast<bool>(_synchronizer); }

	QtPDF::PDFDocumentWidget * widget() { return pdfWidget; }

protected:
	void changeEvent(QEvent *event) override;
	bool event(QEvent *event) override;
	void closeEvent(QCloseEvent *event) override;
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dropEvent(QDropEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;

	QString scriptContext() const override { return QStringLiteral("PDFDocument"); }

public slots:
	void texActivated(TeXDocumentWindow * texDoc);
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
	void updateTypesettingAction();

private slots:
	void changedDocument(const QWeakPointer<QtPDF::Backend::Document> & newDoc);
	void updateRecentFileActions();
	void updateWindowMenu();
	void enablePageActions(PDFDocumentWindow::size_type);
	void syncClick(PDFDocumentWindow::size_type page, const QPointF& pos);
	void syncRange(const PDFDocumentWindow::size_type pageIndex, const QPointF & start, const QPointF & end, const TWSynchronizer::Resolution resolution);
	void invalidateSyncHighlight();
	void scaleLabelClick(QMouseEvent * event) { showScaleContextMenu(event->pos()); }
	void showScaleContextMenu(const QPoint pos);
	void setScaleFromContextMenu(const QString & strZoom);
	void updateStatusBar();
	void updatePageMode(const QtPDF::PDFDocumentView::PageMode newMode);
	void doPageDialog();
	void doScaleDialog();
	void jumpToSource();
	void searchResultHighlighted(const PDFDocumentWindow::size_type pageNum, const QList<QPolygonF> & pdfRegion);
	void setDefaultScale();
	void maybeOpenUrl(const QUrl & url);
	void maybeOpenPdf(const QString & filename, const QtPDF::PDFDestination & destination, const bool newWindow);
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

	QString getMainSourceFilename() const;
	TeXDocumentWindow * getFirstTeXDocumentWindow(const bool openIfNecessary);

	QString curFile;

	QtPDF::PDFDocumentWidget *pdfWidget;
	QScrollArea	*scrollArea;
	QButtonGroup	*toolButtonGroup;

	QList<TeXDocumentWindow*> sourceDocList;

	Tw::UI::ClickableLabel *pageLabel;
	Tw::UI::ClickableLabel *scaleLabel;
	QMenu * scaleContextMenu{nullptr};
	QList<QAction*> recentFileActions;
	Tw::Utils::FullscreenManager * _fullScreenManager;
	QSignalMapper pageModeSignalMapper;

	QGraphicsItem * _syncHighlight;
	QTimer _syncHighlightRemover;

	QBrush _searchResultHighlightBrush;
	QTimer _searchResultHighlightRemover;

	bool openedManually;

	static QList<PDFDocumentWindow*> docList;

	std::unique_ptr<TWSyncTeXSynchronizer> _synchronizer;
#if defined(MIKTEX)
        QAction* actionAbout_MiKTeX;
#endif
};

#endif
