/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2007-2011  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef FindDialog_H
#define FindDialog_H

#include <QDialog>
#include <QDockWidget>
#include <QList>

#include "ui_Find.h"
#include "ui_Replace.h"
#include "ui_SearchResults.h"
#include "ui_PDFFind.h"

class TeXDocument;
class QTextEdit;
class PDFDocument;

class RecentStringsKeyFilter : public QObject
{
	Q_OBJECT
	
public:
	RecentStringsKeyFilter(QObject *parent, const QStringList& stringList)
		: QObject(parent), strings(stringList)
			{}
	
	virtual ~RecentStringsKeyFilter()
			{}
	
protected:
	bool eventFilter(QObject *obj, QEvent *event);
	void setRecentString(QObject *obj, int dir);

	QStringList strings;
};

class FindDialog : public QDialog, private Ui::FindDialog
{
	Q_OBJECT

public:
	FindDialog(QTextEdit *document);

	static DialogCode doFindDialog(QTextEdit *document);

private slots:
	void toggledAllFilesOption(bool checked);
	void toggledFindAllOption(bool checked);
	void toggledRegexOption(bool checked);
	void toggledSelectionOption(bool checked);
	void checkRegex(const QString& str);
	void setSearchText();

private:
	void init(QTextEdit *document);
};

class PDFFindDialog : public QDialog, private Ui::PDFFindDialog
{
	Q_OBJECT

public:
	PDFFindDialog(PDFDocument *document);

	static DialogCode doFindDialog(PDFDocument *document);

private slots:
//	void toggledFindAllOption(bool checked);
	void setSearchText();

private:
	void init(PDFDocument *document);
};


class ReplaceDialog : public QDialog, private Ui::ReplaceDialog
{
	Q_OBJECT

public:
	ReplaceDialog(QTextEdit *parent);
	
	typedef enum {
		Cancel,
		ReplaceOne,
		ReplaceAll
	} DialogCode;
	
	static DialogCode doReplaceDialog(QTextEdit *document);

private slots:
	void toggledAllFilesOption(bool checked);
	void toggledRegexOption(bool checked);
	void toggledSelectionOption(bool checked);
	void checkRegex(const QString& str);
	void clickedReplace();
	void clickedReplaceAll();
	void setSearchText();
	void setReplaceText();

private:
	void init(QTextEdit *document);
};


class SearchResult {
public:
	SearchResult(const TeXDocument* texdoc, int line, int start, int end)
		: doc(texdoc), lineNo(line), selStart(start), selEnd(end)
		{ }

	const TeXDocument* doc;
	int lineNo;
	int selStart;
	int selEnd;
};

class PDFSearchResult {
public:
	PDFSearchResult(const PDFDocument* pdfdoc = NULL, int page = -1, QRectF rect = QRectF())
		: doc(pdfdoc), pageIdx(page), selRect(rect)
		{ }
		
	const PDFDocument* doc;
	int pageIdx;
	QRectF selRect;
};

class SearchResults : public QDockWidget, private Ui::SearchResults
{
	Q_OBJECT
	
public:
	static void presentResults(const QString& searchText, const QList<SearchResult>& results,
							   QMainWindow* parent, bool singleFile);
	
	SearchResults(QWidget* parent);

protected slots:
	void focusChanged(QWidget * old, QWidget * now);

private slots:
	void showSelectedEntry();
	void showEntry(QTableWidgetItem * item);
	void goToSource();
	void goToSourceAndClose();

private:
	QPalette editorOriginalPalette, editorModifiedPalette;
};

#endif
