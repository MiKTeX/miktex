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

#ifndef FindDialog_H
#define FindDialog_H

#include <QDialog>
#include <QDockWidget>
#include <QList>

#include "ui_Find.h"
#include "ui_Replace.h"
#include "ui_SearchResults.h"
#include "ui_PDFFind.h"

class TeXDocumentWindow;
class QTextEdit;
class PDFDocumentWindow;

class RecentStringsKeyFilter : public QObject
{
	Q_OBJECT
	
public:
	RecentStringsKeyFilter(QObject *parent, const QStringList& stringList)
		: QObject(parent), strings(stringList)
			{}
	
	~RecentStringsKeyFilter() override = default;
	
protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
	void setRecentString(QObject *obj, int dir);

	QStringList strings;
};

class FindDialog : public QDialog, private Ui::FindDialog
{
	Q_OBJECT

public:
	explicit FindDialog(QTextEdit * document);

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
	explicit PDFFindDialog(PDFDocumentWindow * document);

	static DialogCode doFindDialog(PDFDocumentWindow *document);

private slots:
//	void toggledFindAllOption(bool checked);
	void setSearchText();

private:
	void init(PDFDocumentWindow *document);
};


class ReplaceDialog : public QDialog, private Ui::ReplaceDialog
{
	Q_OBJECT

public:
	explicit ReplaceDialog(QTextEdit * parent);
	
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
	SearchResult(const TeXDocumentWindow* texdoc, int line, int start, int end)
		: doc(texdoc), lineNo(line), selStart(start), selEnd(end)
		{ }

	const TeXDocumentWindow* doc;
	int lineNo;
	int selStart;
	int selEnd;
};

class PDFSearchResult {
public:
	PDFSearchResult(const PDFDocumentWindow* pdfdoc = nullptr, int page = -1, QRectF rect = QRectF())
		: doc(pdfdoc), pageIdx(page), selRect(rect)
		{ }
		
	const PDFDocumentWindow* doc;
	int pageIdx;
	QRectF selRect;
};

class SearchResults : public QDockWidget, private Ui::SearchResults
{
	Q_OBJECT
	
public:
	static void presentResults(const QString& searchText, const QList<SearchResult>& results,
							   QMainWindow* parent, bool singleFile);
	
	explicit SearchResults(QWidget * parent);

private slots:
	void showSelectedEntry();
	void showEntry(QTableWidgetItem * item);
	void goToSource();
	void goToSourceAndClose();
};

#endif
