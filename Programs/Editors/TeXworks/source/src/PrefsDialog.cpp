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

#include "PrefsDialog.h"

#include "CompletingEdit.h"
#include "DefaultPrefs.h"
#include "PDFDocumentWindow.h"
#include "Settings.h"
#include "TWApp.h"
#include "TWUtils.h"
#include "TeXHighlighter.h"
#include "document/SpellChecker.h"

#include <QFileDialog>
#include <QFontDatabase>
#include <QMainWindow>
#include <QMessageBox>
#include <QSet>
#include <QTextCodec>
#include <QToolBar>

PrefsDialog::PrefsDialog(QWidget *parent)
	: QDialog(parent)
	, pathsChanged(false)
	, toolsChanged(false)
{
	init();
}

void PrefsDialog::init()
{
	setupUi(this);

	connect(buttonBox, &QDialogButtonBox::clicked, this, &PrefsDialog::buttonClicked);

	connect(binPathList, &QListWidget::itemSelectionChanged, this, &PrefsDialog::updatePathButtons);
	connect(pathUp, &QToolButton::clicked, this, &PrefsDialog::movePathUp);
	connect(pathDown, &QToolButton::clicked, this, &PrefsDialog::movePathDown);
	connect(pathAdd, &QToolButton::clicked, this, &PrefsDialog::addPath);
	connect(pathRemove, &QToolButton::clicked, this, &PrefsDialog::removePath);

	connect(toolList, &QListWidget::itemSelectionChanged, this, &PrefsDialog::updateToolButtons);
	connect(toolList, &QListWidget::itemDoubleClicked, this, &PrefsDialog::editTool);
	connect(toolUp, &QToolButton::clicked, this, &PrefsDialog::moveToolUp);
	connect(toolDown, &QToolButton::clicked, this, &PrefsDialog::moveToolDown);
	connect(toolAdd, &QToolButton::clicked, this, &PrefsDialog::addTool);
	connect(toolRemove, &QToolButton::clicked, this, &PrefsDialog::removeTool);
	connect(toolEdit, &QToolButton::clicked, this, [=]() { this->editTool(); });

	connect(tabWidget, &QTabWidget::currentChanged, this, &PrefsDialog::changedTabPanel);

	pathsChanged = toolsChanged = false;
}

void PrefsDialog::changedTabPanel(int index)
{
	// this all feels a bit hacky, but seems to keep things tidy on Mac OS X at least
	QWidget *page = tabWidget->widget(index);
	page->clearFocus();
	switch (index) {
		case 0: // General
			if (page->focusWidget())
				page->focusWidget()->clearFocus();
			break;
		case 1: // Editor
			editorFont->setFocus();
			editorFont->lineEdit()->selectAll();
			break;
		case 2: // Preview
			scale->setFocus();
			scale->selectAll();
			break;
		case 3: // Typesetting
			binPathList->setFocus();
			break;
		case 4: // Script
			if (page->focusWidget())
				page->focusWidget()->clearFocus();
			break;
	}
}

void PrefsDialog::updatePathButtons()
{
	int selRow = -1;
	if (binPathList->selectedItems().count() > 0)
		selRow = binPathList->currentRow();
	pathRemove->setEnabled(selRow != -1);
	pathUp->setEnabled(selRow > 0);
	pathDown->setEnabled(selRow != -1 && selRow < binPathList->count() - 1);
}

void PrefsDialog::movePathUp()
{
	int selRow = -1;
	if (binPathList->selectedItems().count() > 0)
		selRow = binPathList->currentRow();
	if (selRow > 0) {
		QListWidgetItem *item = binPathList->takeItem(selRow);
		binPathList->insertItem(selRow - 1, item);
		binPathList->setCurrentItem(binPathList->item(selRow - 1));
		pathsChanged = true;
	}
}

void PrefsDialog::movePathDown()
{
	int selRow = -1;
	if (binPathList->selectedItems().count() > 0)
		selRow = binPathList->currentRow();
	if (selRow != -1 &&  selRow < binPathList->count() - 1) {
		QListWidgetItem *item = binPathList->takeItem(selRow);
		binPathList->insertItem(selRow + 1, item);
		binPathList->setCurrentItem(binPathList->item(selRow + 1));
		pathsChanged = true;
	}
}

void PrefsDialog::addPath()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"),
	                 QString::fromLatin1("/usr"), /*0*/ /*QFileDialog::DontUseNativeDialog*/
								QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	// remove the trailing / (if any)
	dir = QDir::fromNativeSeparators(dir);
#if defined(Q_OS_WIN)
	if (dir.length() > 2)
#else
	if (dir.length() > 1)
#endif
		if (dir.at(dir.length() - 1) == QChar::fromLatin1('/'))
			dir.chop(1);
	if (!dir.isEmpty()) {
		binPathList->addItem(dir);
		binPathList->setCurrentItem(binPathList->item(binPathList->count() - 1));
		pathsChanged = true;
	}
}

void PrefsDialog::removePath()
{
	if (binPathList->currentRow() > -1)
		if (binPathList->currentItem()->isSelected()) {
			binPathList->takeItem(binPathList->currentRow());
			pathsChanged = true;
		}
}

void PrefsDialog::updateToolButtons()
{
	int selRow = -1;
	if (toolList->selectedItems().count() > 0)
		selRow = toolList->currentRow();
	toolEdit->setEnabled(selRow != -1);
	toolRemove->setEnabled(selRow != -1);
	toolUp->setEnabled(selRow > 0);
	toolDown->setEnabled(selRow != -1 && selRow < toolList->count() - 1);
}

void PrefsDialog::moveToolUp()
{
	int selRow = -1;
	if (toolList->selectedItems().count() > 0)
		selRow = toolList->currentRow();
	if (selRow > 0) {
		QListWidgetItem *item = toolList->takeItem(selRow);
		toolList->insertItem(selRow - 1, item);
		toolList->setCurrentItem(toolList->item(selRow - 1));
		Engine e = engineList.takeAt(selRow);
		engineList.insert(selRow - 1, e);
		refreshDefaultTool();
		toolsChanged = true;
	}
}

void PrefsDialog::moveToolDown()
{
	int selRow = -1;
	if (toolList->selectedItems().count() > 0)
		selRow = toolList->currentRow();
	if (selRow != -1 &&  selRow < toolList->count() - 1) {
		QListWidgetItem *item = toolList->takeItem(selRow);
		toolList->insertItem(selRow + 1, item);
		toolList->setCurrentItem(toolList->item(selRow + 1));
		Engine e = engineList.takeAt(selRow);
		engineList.insert(selRow + 1, e);
		refreshDefaultTool();
		toolsChanged = true;
	}
}

void PrefsDialog::addTool()
{
	Engine e;
	e.setName(tr("New Tool"));
	e.setShowPdf(true);
	if (ToolConfig::doToolConfig(this, e) == QDialog::Accepted) {
		engineList.append(e);
		toolList->addItem(e.name());
		refreshDefaultTool();
		toolsChanged = true;
	}
}

void PrefsDialog::removeTool()
{
	if (toolList->currentRow() > -1)
		if (toolList->currentItem()->isSelected()) {
			engineList.removeAt(toolList->currentRow());
			toolList->takeItem(toolList->currentRow());
			refreshDefaultTool();
			toolsChanged = true;
		}
}

void PrefsDialog::editTool(QListWidgetItem* item)
{
	int row = -1;
	if (item)
		row = toolList->row(item);
	else if (toolList->currentRow() > -1 && toolList->currentItem()->isSelected())
		row = toolList->currentRow();
	if (row > -1) {
		Engine e = engineList[toolList->currentRow()];
		if (ToolConfig::doToolConfig(this, e) == QDialog::Accepted) {
			engineList[toolList->currentRow()] = e;
			toolList->currentItem()->setText(e.name());
			toolsChanged = true;
		}
	}
}

void PrefsDialog::refreshDefaultTool()
{
	QString val;
	int toolIndex = -1, defaultIndex = -1;

	if (defaultTool->count() > 0)
		val = defaultTool->currentText();
	defaultTool->clear();
	foreach (Engine e, engineList) {
		defaultTool->addItem(e.name());
		if (e.name() == val)
			toolIndex = defaultTool->count() - 1;
		if (e.name() == QString::fromUtf8(DEFAULT_ENGINE_NAME))
			defaultIndex = defaultTool->count() - 1;
	}

	if (toolIndex >= 0)
		defaultTool->setCurrentIndex(toolIndex);
	else if (defaultIndex >= 0)
		defaultTool->setCurrentIndex(defaultIndex);
}

void PrefsDialog::buttonClicked(QAbstractButton *whichButton)
{
	if (buttonBox->buttonRole(whichButton) == QDialogButtonBox::ResetRole)
		restoreDefaults();
}

void PrefsDialog::restoreDefaults()
{
	switch (tabWidget->currentIndex()) {
		case 0:
			// General
			switch (kDefault_ToolBarIcons) {
				case 1:
					smallIcons->setChecked(true);
					break;
				case 2:
					mediumIcons->setChecked(true);
					break;
				case 3:
					largeIcons->setChecked(true);
					break;
			}
			showText->setChecked(kDefault_ToolBarText);
			switch (kDefault_LaunchOption) {
				case 1:
				default:
					blankDocument->setChecked(true);
					break;
				case 2:
					templateDialog->setChecked(true);
					break;
				case 3:
					openDialog->setChecked(true);
					break;
			}
			localePopup->setCurrentIndex(kDefault_Locale);
			openPDFwithTeX->setChecked(kDefault_OpenPDFwithTeX);
			break;

		case 1:
			// Editor
			{
				QFont font;
				editorFont->setEditText(font.family());
				fontSize->setValue(font.pointSize());
			}
			tabWidth->setValue(kDefault_TabWidth);
			lineSpacing->setValue(kDefault_LineSpacing);
			lineNumbers->setChecked(kDefault_LineNumbers);
			wrapLines->setChecked(kDefault_WrapLines);
			syntaxColoring->setCurrentIndex(kDefault_SyntaxColoring);
			autoIndent->setCurrentIndex(kDefault_IndentMode);
			smartQuotes->setCurrentIndex(kDefault_QuotesMode);
			language->setCurrentIndex(kDefault_SpellcheckLanguage);
			encoding->setCurrentIndex(encoding->findText(QString::fromLatin1("UTF-8")));
			highlightCurrentLine->setChecked(kDefault_HighlightCurrentLine);
			cursorWidth->setValue(kDefault_CursorWidth);
			autocompleteEnabled->setChecked(kDefault_AutocompleteEnabled);
			autoFollowFocusEnabled->setChecked(kDefault_AutoFollowFocusEnabled);
			break;

		case 2:
			// Preview
			switch (kDefault_PreviewScaleOption) {
				default:
					actualSize->setChecked(true);
					break;
				case 2:
					fitWidth->setChecked(true);
					break;
				case 3:
					fitWindow->setChecked(true);
					break;
				case 4:
					fixedScale->setChecked(true);
					break;
				case 5:
					fitContentWidth->setChecked(true);
					break;
			}
			scale->setValue(kDefault_PreviewScale);
			switch (kDefault_PDFPageMode) {
				case QtPDF::PDFDocumentView::PageMode_SinglePage:
					pdfPageMode->setCurrentIndex(0);
					break;
				case QtPDF::PDFDocumentView::PageMode_OneColumnContinuous:
				default:
					pdfPageMode->setCurrentIndex(1);
					break;
				case QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous:
					pdfPageMode->setCurrentIndex(2);
				break;
			}

			pdfPaperColor->setColor(kDefault_PaperColor);

			pdfRulerUnits->setCurrentIndex(kDefault_PreviewRulerUnits);
			pdfRulerShow->setChecked(kDefault_PreviewRulerShow);

			resolution->setDpi(QApplication::screens().first()->physicalDotsPerInch());

			pdfPageCacheSizeMiB->setValue(kDefault_PDFPageCacheSizeMiB);

			switch (TWSynchronizer::kDefault_Resolution_ToTeX) {
				case TWSynchronizer::CharacterResolution:
					cbSyncToTeX->setCurrentIndex(0);
					break;
				case TWSynchronizer::WordResolution:
					cbSyncToTeX->setCurrentIndex(1);
					break;
				case TWSynchronizer::LineResolution:
					cbSyncToTeX->setCurrentIndex(2);
					break;
			}
			switch (TWSynchronizer::kDefault_Resolution_ToPDF) {
				case TWSynchronizer::CharacterResolution:
					cbSyncToPDF->setCurrentIndex(0);
					break;
				case TWSynchronizer::WordResolution:
					cbSyncToPDF->setCurrentIndex(1);
					break;
				case TWSynchronizer::LineResolution:
					cbSyncToPDF->setCurrentIndex(2);
					break;
			}

			switch (kDefault_MagnifierSize) {
				case 1:
					smallMag->setChecked(true);
					break;
				default:
					mediumMag->setChecked(true);
					break;
				case 3:
					largeMag->setChecked(true);
					break;
			}
			circularMag->setChecked(kDefault_CircularMagnifier);
			break;

		case 3:
			// Typesetting
			TWApp::instance()->setDefaultEngineList();
			TWApp::instance()->setDefaultPaths();
			initPathAndToolLists();
			autoHideOutput->setCurrentIndex(kDefault_HideConsole);
			pathsChanged = true;
			toolsChanged = true;
			break;

		case 4:
			// Scripts
			allowScriptFileReading->setChecked(kDefault_AllowScriptFileReading);
			allowScriptFileWriting->setChecked(kDefault_AllowScriptFileWriting);
			enableScriptingPlugins->setChecked(kDefault_EnableScriptingPlugins);
			allowSystemCommands->setChecked(kDefault_AllowSystemCommands);
			scriptDebugger->setChecked(kDefault_ScriptDebugger);
			break;
	}
}

void PrefsDialog::initPathAndToolLists()
{
	binPathList->clear();
	toolList->clear();
	binPathList->addItems(TWApp::instance()->getPrefsBinaryPaths());
	engineList = TWApp::instance()->getEngineList();
	foreach (Engine e, engineList) {
		toolList->addItem(e.name());
		defaultTool->addItem(e.name());
		if (e.name() == TWApp::instance()->getDefaultEngine().name())
			defaultTool->setCurrentIndex(defaultTool->count() - 1);
	}
	if (binPathList->count() > 0)
		binPathList->setCurrentItem(binPathList->item(0));
	if (toolList->count() > 0)
		toolList->setCurrentItem(toolList->item(0));
	updatePathButtons();
	updateToolButtons();
}

const int kSystemLocaleIndex = 0;
const int kEnglishLocaleIndex = 1;

typedef QPair<QString, QString> DictPair;

static bool dictPairLessThan(const DictPair& d1, const DictPair& d2)
{
	return d1.first.toLower() < d2.first.toLower();
}

QDialog::DialogCode PrefsDialog::doPrefsDialog(QWidget *parent)
{
	PrefsDialog dlg(nullptr);
	using index_type = decltype(QComboBox().currentIndex());

	QStringList nameList;
	foreach (QTextCodec *codec, *TWUtils::findCodecs())
		nameList.append(QString::fromUtf8(codec->name().constData()));
	dlg.encoding->addItems(nameList);

	QStringList syntaxOptions = TeXHighlighter::syntaxOptions();
	dlg.syntaxColoring->addItems(syntaxOptions);

	QStringList indentModes = CompletingEdit::autoIndentModes();
	dlg.autoIndent->addItems(indentModes);

	QStringList quotesModes = CompletingEdit::smartQuotesModes();
	dlg.smartQuotes->addItems(quotesModes);

	QList< DictPair > dictList;
	foreach (const QString& dictKey, Tw::Document::SpellChecker::getDictionaryList()->uniqueKeys()) {
		foreach (QString dict, Tw::Document::SpellChecker::getDictionaryList()->values(dictKey)) {
			const QString label{Tw::Document::SpellChecker::labelForDict(dict)};
			dictList << qMakePair(label, dict);
		}
	}
	std::sort(dictList.begin(), dictList.end(), dictPairLessThan);
	foreach (const DictPair& dict, dictList)
		dlg.language->addItem(dict.first, dict.second);

	Tw::Settings settings;
	// initialize controls based on the current settings

	// General
	int oldIconSize = settings.value(QString::fromLatin1("toolBarIconSize"), kDefault_ToolBarIcons).toInt();
	switch (oldIconSize) {
		case 1:
			dlg.smallIcons->setChecked(true);
			break;
		case 3:
			dlg.largeIcons->setChecked(true);
			break;
		default:
			dlg.mediumIcons->setChecked(true);
			break;
	}
	bool oldShowText = settings.value(QString::fromLatin1("toolBarShowText"), kDefault_ToolBarText).toBool();
	dlg.showText->setChecked(oldShowText);

	switch (settings.value(QString::fromLatin1("launchOption"), kDefault_LaunchOption).toInt()) {
		default:
			dlg.blankDocument->setChecked(true);
			break;
		case 2:
			dlg.templateDialog->setChecked(true);
			break;
		case 3:
			dlg.openDialog->setChecked(true);
			break;
	}
	dlg.openPDFwithTeX->setChecked(settings.value(QString::fromLatin1("openPDFwithTeX"), kDefault_OpenPDFwithTeX).toBool());

	QString oldLocale = settings.value(QString::fromLatin1("locale")).toString();
	// System and English are predefined at index 0 and 1 (see constants above)
	dlg.localePopup->addItem(tr("System default [%1]").arg(QLocale::languageToString(QLocale(QLocale::system().name()).language())));
	int oldLocaleIndex = oldLocale.isEmpty() ? kSystemLocaleIndex : kEnglishLocaleIndex;
	QList< DictPair > displayList;
	foreach(QString trans, TWApp::getTranslationList()) {
		QLocale loc(trans);
		QLocale::Language	language = loc.language();
		QString locName;
		if (language == QLocale::C)
			locName = trans;
		else {
			const QString languageString = QLocale::languageToString(language);
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
			const QString territoryString = (loc.country() != QLocale::AnyCountry ? QLocale::countryToString(loc.country()) : QString());
#else
			const QString territoryString = (loc.territory() != QLocale::AnyTerritory ? QLocale::territoryToString(loc.territory()) : QString());
#endif
			if (trans.contains(QChar::fromLatin1('_')) && !territoryString.isEmpty()) {
				//: Language (%1) and Country (%2) for TeXworks translations (ex. "Portuguese (Brazil)")
				locName = tr("%1 (%2)").arg(languageString, territoryString);
			}
			else {
				locName = languageString;
			}
		}
		displayList << qMakePair(locName, trans);
	}
	std::sort(displayList.begin(), displayList.end(), dictPairLessThan);

	Q_FOREACH(DictPair p, displayList) {
		dlg.localePopup->addItem(p.first, p.second);
		if (p.second == oldLocale)
			oldLocaleIndex = dlg.localePopup->count() - 1;
	}
	dlg.localePopup->setCurrentIndex(oldLocaleIndex);

	// Editor
	dlg.syntaxColoring->setCurrentIndex(static_cast<index_type>(settings.contains(QString::fromLatin1("syntaxColoring"))
	                        ? 1 + syntaxOptions.indexOf(settings.value(QString::fromLatin1("syntaxColoring")).toString())
							: 1 + kDefault_SyntaxColoring));
	dlg.autoIndent->setCurrentIndex(static_cast<index_type>(settings.contains(QString::fromLatin1("autoIndent"))
	                        ? 1 + indentModes.indexOf(settings.value(QString::fromLatin1("autoIndent")).toString())
							: 1 + kDefault_IndentMode));
	dlg.smartQuotes->setCurrentIndex(static_cast<index_type>(settings.contains(QString::fromLatin1("smartQuotes"))
	                        ? 1 + quotesModes.indexOf(settings.value(QString::fromLatin1("smartQuotes")).toString())
							: 1 + kDefault_QuotesMode));
	dlg.lineNumbers->setChecked(settings.value(QString::fromLatin1("lineNumbers"), kDefault_LineNumbers).toBool());
	dlg.wrapLines->setChecked(settings.value(QString::fromLatin1("wrapLines"), kDefault_WrapLines).toBool());
	dlg.tabWidth->setValue(settings.value(QString::fromLatin1("tabWidth"), kDefault_TabWidth).toInt());
	dlg.lineSpacing->setValue(settings.value(QStringLiteral("lineSpacing"), kDefault_LineSpacing).toInt());
	const QStringList fontFamilies = []() {
		#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		return QFontDatabase().families();
		#else
		return QFontDatabase::families();
		#endif
	}();
	dlg.editorFont->addItems(fontFamilies);
	QString fontString = settings.value(QString::fromLatin1("font")).toString();
	QFont font;
	if (!fontString.isEmpty())
		font.fromString(fontString);
	dlg.editorFont->setCurrentIndex(static_cast<index_type>(fontFamilies.indexOf(font.family())));
	dlg.fontSize->setValue(font.pointSize());
	dlg.encoding->setCurrentIndex(static_cast<index_type>(nameList.indexOf(QString::fromUtf8(TWApp::instance()->getDefaultCodec()->name().constData()))));
	dlg.highlightCurrentLine->setChecked(settings.value(QString::fromLatin1("highlightCurrentLine"), kDefault_HighlightCurrentLine).toBool());
	dlg.cursorWidth->setValue(settings.value(QStringLiteral("cursorWidth"), kDefault_CursorWidth).toInt());
	dlg.autocompleteEnabled->setChecked(settings.value(QString::fromLatin1("autocompleteEnabled"), kDefault_AutocompleteEnabled).toBool());
	dlg.autoFollowFocusEnabled->setChecked(settings.value(QStringLiteral("autoFollowFocusEnabled"), kDefault_AutoFollowFocusEnabled).toBool());

	QString defDict = settings.value(QString::fromLatin1("language"), QString::fromLatin1("None")).toString();
	int i = dlg.language->findData(defDict);
	if (i >= 0)
		dlg.language->setCurrentIndex(i);

	// Preview
	switch (settings.value(QString::fromLatin1("scaleOption"), kDefault_PreviewScaleOption).toInt()) {
		default:
			dlg.actualSize->setChecked(true);
			break;
		case 2:
			dlg.fitWidth->setChecked(true);
			break;
		case 3:
			dlg.fitWindow->setChecked(true);
			break;
		case 4:
			dlg.fixedScale->setChecked(true);
			break;
		case 5:
			dlg.fitContentWidth->setChecked(true);
			break;
	}
	dlg.scale->setValue(settings.value(QString::fromLatin1("previewScale"), kDefault_PreviewScale).toInt());
	switch (settings.value(QString::fromLatin1("pdfPageMode"), kDefault_PDFPageMode).toInt()) {
		case QtPDF::PDFDocumentView::PageMode_SinglePage:
			dlg.pdfPageMode->setCurrentIndex(0);
			break;
		case QtPDF::PDFDocumentView::PageMode_OneColumnContinuous:
		default:
			dlg.pdfPageMode->setCurrentIndex(1);
			break;
		case QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous:
			dlg.pdfPageMode->setCurrentIndex(2);
			break;
	}

	dlg.pdfPaperColor->setColor(settings.value(QStringLiteral("pdfPaperColor"), QVariant::fromValue<QColor>(kDefault_PaperColor)).value<QColor>());

	dlg.pdfRulerUnits->setCurrentIndex(settings.value(QStringLiteral("pdfRulerUnits"), kDefault_PreviewRulerUnits).toInt());
	dlg.pdfRulerShow->setChecked(settings.value(QStringLiteral("pdfRulerShow"), kDefault_PreviewRulerShow).toBool());

	double oldResolution = settings.value(QString::fromLatin1("previewResolution"), QApplication::screens().first()->physicalDotsPerInch()).toDouble();
	dlg.resolution->setDpi(oldResolution);

	const int oldPDFPageCacheSize = settings.value(QStringLiteral("pdfPageCacheSizeMiB"), kDefault_PDFPageCacheSizeMiB).toInt();
	dlg.pdfPageCacheSizeMiB->setValue(oldPDFPageCacheSize);

	int oldSyncToTeX = settings.value(QString::fromLatin1("syncResolutionToTeX"), TWSynchronizer::kDefault_Resolution_ToTeX).toInt();
	dlg.cbSyncToTeX->setCurrentIndex(oldSyncToTeX);

	int oldSyncToPDF = settings.value(QString::fromLatin1("syncResolutionToPDF"), TWSynchronizer::kDefault_Resolution_ToPDF).toInt();
	dlg.cbSyncToPDF->setCurrentIndex(oldSyncToPDF);

	int oldMagSize = settings.value(QString::fromLatin1("magnifierSize"), kDefault_MagnifierSize).toInt();
	switch (oldMagSize) {
		case 1:
			dlg.smallMag->setChecked(true);
			break;
		default:
			dlg.mediumMag->setChecked(true);
			break;
		case 3:
			dlg.largeMag->setChecked(true);
			break;
	}
	bool oldCircular = settings.value(QString::fromLatin1("circularMagnifier"), kDefault_CircularMagnifier).toBool();
	dlg.circularMag->setChecked(oldCircular);

	// Typesetting
	dlg.initPathAndToolLists();
	QVariant hideConsoleSetting = settings.value(QString::fromLatin1("autoHideConsole"), kDefault_HideConsole);
	// Backwards compatibility to Tw 0.4.0 and before
	if (hideConsoleSetting.toString() == QLatin1String("true") || hideConsoleSetting.toString() == QLatin1String("false"))
		hideConsoleSetting = (hideConsoleSetting.toBool() ? kDefault_HideConsole : 0);
	dlg.autoHideOutput->setCurrentIndex(hideConsoleSetting.toInt());

	// Scripts
	dlg.allowScriptFileReading->setChecked(settings.value(QString::fromLatin1("allowScriptFileReading"), kDefault_AllowScriptFileReading).toBool());
	dlg.allowScriptFileWriting->setChecked(settings.value(QString::fromLatin1("allowScriptFileWriting"), kDefault_AllowScriptFileWriting).toBool());
	dlg.allowSystemCommands->setChecked(settings.value(QString::fromLatin1("allowSystemCommands"), kDefault_AllowSystemCommands).toBool());
	dlg.enableScriptingPlugins->setChecked(settings.value(QString::fromLatin1("enableScriptingPlugins"), kDefault_EnableScriptingPlugins).toBool());
	// there is always at least one built-in ScriptInterface
	if (TWApp::instance()->getScriptManager()->languages().size() <= 1)
		dlg.enableScriptingPlugins->setEnabled(false);
	dlg.scriptDebugger->setChecked(settings.value(QString::fromLatin1("scriptDebugger"), kDefault_ScriptDebugger).toBool());

	// Decide which tab to select initially
	if (sCurrentTab == -1) {
		if (parent && parent->inherits("TeXDocument"))
			sCurrentTab = 1;
		else if (parent && parent->inherits("PDFDocument"))
			sCurrentTab = 2;
		else
			sCurrentTab = 0;
	}
	if (sCurrentTab != dlg.tabWidget->currentIndex())
		dlg.tabWidget->setCurrentIndex(sCurrentTab);

	dlg.show();

	DialogCode result = static_cast<DialogCode>(dlg.exec());

	if (result == Accepted) {
		sCurrentTab = dlg.tabWidget->currentIndex();

		// General
		int iconSize = 2;
		if (dlg.smallIcons->isChecked())
			iconSize = 1;
		else if (dlg.largeIcons->isChecked())
			iconSize = 3;
		bool showText = dlg.showText->isChecked();
		if (iconSize != oldIconSize || showText != oldShowText) {
			settings.setValue(QString::fromLatin1("toolBarIconSize"), iconSize);
			settings.setValue(QString::fromLatin1("toolBarShowText"), showText);
			foreach (QWidget *widget, qApp->topLevelWidgets()) {
				QMainWindow *theWindow = qobject_cast<QMainWindow*>(widget);
				if (theWindow)
					TWUtils::applyToolbarOptions(theWindow, iconSize, showText);
			}
		}

		int launchOption = 1;
		if (dlg.templateDialog->isChecked())
			launchOption = 2;
		else if (dlg.openDialog->isChecked())
			launchOption = 3;
		settings.setValue(QString::fromLatin1("launchOption"), launchOption);

		settings.setValue(QString::fromLatin1("openPDFwithTeX"), dlg.openPDFwithTeX->isChecked());

		if (dlg.localePopup->currentIndex() != oldLocaleIndex) {
			switch (dlg.localePopup->currentIndex()) {
				case kSystemLocaleIndex: // system locale
					{
						QString locale = QLocale::system().name();
						TWApp::instance()->applyTranslation(locale);
						settings.remove(QString::fromLatin1("locale"));
					}
					break;
				default:
					{
						QString locale = dlg.localePopup->itemData(dlg.localePopup->currentIndex()).toString();
						TWApp::instance()->applyTranslation(locale);
						settings.setValue(QString::fromLatin1("locale"), locale);
					}
					break;
			}
		}

		// Editor
		settings.setValue(QString::fromLatin1("syntaxColoring"), dlg.syntaxColoring->currentText());
		settings.setValue(QString::fromLatin1("autoIndent"), dlg.autoIndent->currentText());
		settings.setValue(QString::fromLatin1("smartQuotes"), dlg.smartQuotes->currentText());
		settings.setValue(QString::fromLatin1("lineNumbers"), dlg.lineNumbers->isChecked());
		settings.setValue(QString::fromLatin1("wrapLines"), dlg.wrapLines->isChecked());
		settings.setValue(QString::fromLatin1("tabWidth"), dlg.tabWidth->value());
		settings.setValue(QStringLiteral("lineSpacing"), dlg.lineSpacing->value());
		font = QFont(dlg.editorFont->currentText());
		font.setPointSize(dlg.fontSize->value());
		settings.setValue(QString::fromLatin1("font"), font.toString());
		TWApp::instance()->setDefaultCodec(QTextCodec::codecForName(dlg.encoding->currentText().toLatin1()));
		if (dlg.language->currentIndex() >= 0) {
			QVariant data = dlg.language->itemData(dlg.language->currentIndex());
			if (data.isValid())
				settings.setValue(QString::fromLatin1("language"), data);
			else
				settings.setValue(QString::fromLatin1("language"), QString::fromLatin1("None"));
		}
		else
			settings.setValue(QString::fromLatin1("language"), QString::fromLatin1("None"));
		bool highlightLine = dlg.highlightCurrentLine->isChecked();
		settings.setValue(QString::fromLatin1("highlightCurrentLine"), highlightLine);
		CompletingEdit::setHighlightCurrentLine(highlightLine);

		settings.setValue(QStringLiteral("cursorWidth"), dlg.cursorWidth->value());

		bool autocompleteEnabled = dlg.autocompleteEnabled->isChecked();
		settings.setValue(QString::fromLatin1("autocompleteEnabled"), autocompleteEnabled);
		CompletingEdit::setAutocompleteEnabled(autocompleteEnabled);

		settings.setValue(QStringLiteral("autoFollowFocusEnabled"), dlg.autoFollowFocusEnabled->isChecked());

		// Since the tab width can't be set by any other means, forcibly update
		// all windows now
		foreach (QWidget* widget, TWApp::instance()->allWidgets()) {
			QTextEdit* editor = qobject_cast<QTextEdit*>(widget);
			if (editor)
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
				editor->setTabStopWidth(dlg.tabWidth->value());
#else
				editor->setTabStopDistance(dlg.tabWidth->value());
#endif
		}

		// Preview
		int scaleOption = 1;
		if (dlg.fitWidth->isChecked())
			scaleOption = 2;
		else if (dlg.fitWindow->isChecked())
			scaleOption = 3;
		else if (dlg.fixedScale->isChecked())
			scaleOption = 4;
		else if (dlg.fitContentWidth->isChecked())
			scaleOption = 5;
		int scale = dlg.scale->value();
		settings.setValue(QString::fromLatin1("scaleOption"), scaleOption);
		settings.setValue(QString::fromLatin1("previewScale"), scale);
		switch (dlg.pdfPageMode->currentIndex()) {
			case 0:
			    settings.setValue(QString::fromLatin1("pdfPageMode"), QtPDF::PDFDocumentView::PageMode_SinglePage);
				break;
			case 1:
			    settings.setValue(QString::fromLatin1("pdfPageMode"), QtPDF::PDFDocumentView::PageMode_OneColumnContinuous);
				break;
			case 2:
			    settings.setValue(QString::fromLatin1("pdfPageMode"), QtPDF::PDFDocumentView::PageMode_TwoColumnContinuous);
				break;
		}

		settings.setValue(QStringLiteral("pdfPaperColor"), dlg.pdfPaperColor->color());

		settings.setValue(QStringLiteral("pdfRulerUnits"), dlg.pdfRulerUnits->currentIndex());
		settings.setValue(QStringLiteral("pdfRulerShow"), dlg.pdfRulerShow->isChecked());

		double resolution = dlg.resolution->dpi();
		if (resolution != oldResolution) {
			settings.setValue(QString::fromLatin1("previewResolution"), resolution);
			foreach (QWidget *widget, qApp->topLevelWidgets()) {
				PDFDocumentWindow *thePdfDoc = qobject_cast<PDFDocumentWindow*>(widget);
				if (thePdfDoc)
					thePdfDoc->setResolution(resolution);
			}
		}

		const int pdfPageCacheSize = dlg.pdfPageCacheSizeMiB->value();
		settings.setValue(QStringLiteral("pdfPageCacheSizeMiB"), pdfPageCacheSize);
		QtPDF::Backend::Document::pageCache().setMaxCost(pdfPageCacheSize * 1024 * 1024);

		int syncToTeX = dlg.cbSyncToTeX->currentIndex();
		if (syncToTeX != oldSyncToTeX)
			settings.setValue(QString::fromLatin1("syncResolutionToTeX"), syncToTeX);

		int syncToPDF = dlg.cbSyncToPDF->currentIndex();
		if (syncToPDF != oldSyncToPDF)
			settings.setValue(QString::fromLatin1("syncResolutionToPDF"), syncToPDF);

		int magSize = 2;
		if (dlg.smallMag->isChecked())
			magSize = 1;
		else if (dlg.largeMag->isChecked())
			magSize = 3;
		settings.setValue(QString::fromLatin1("magnifierSize"), magSize);
		bool circular = dlg.circularMag->isChecked();
		settings.setValue(QString::fromLatin1("circularMagnifier"), circular);
		if (oldMagSize != magSize || oldCircular != circular) {
			foreach (QWidget *widget, qApp->topLevelWidgets()) {
				PDFDocumentWindow *thePdfDoc = qobject_cast<PDFDocumentWindow*>(widget);
				if (thePdfDoc)
					thePdfDoc->resetMagnifier();
			}
		}

		// Typesetting
		if (dlg.pathsChanged) {
			QStringList paths;
			for (int i = 0; i < dlg.binPathList->count(); ++i)
				paths << dlg.binPathList->item(i)->text();
			TWApp::instance()->setBinaryPaths(paths);
		}
		if (dlg.toolsChanged)
			TWApp::instance()->setEngineList(dlg.engineList);
		TWApp::instance()->setDefaultEngine(dlg.defaultTool->currentText());
		settings.setValue(QString::fromLatin1("autoHideConsole"), dlg.autoHideOutput->currentIndex());

		// Scripts
		settings.setValue(QString::fromLatin1("allowScriptFileReading"), dlg.allowScriptFileReading->isChecked());
		settings.setValue(QString::fromLatin1("allowScriptFileWriting"), dlg.allowScriptFileWriting->isChecked());
		settings.setValue(QString::fromLatin1("allowSystemCommands"), dlg.allowSystemCommands->isChecked());
		settings.setValue(QString::fromLatin1("enableScriptingPlugins"), dlg.enableScriptingPlugins->isChecked());
		settings.setValue(QString::fromLatin1("scriptDebugger"), dlg.scriptDebugger->isChecked());
		// with changed settings, the availability of scripts may have changed
		// (e.g., because of enabling/disabling the use of scripting plugins)
		TWApp::instance()->updateScriptsList();
	}

	return result;
}

int PrefsDialog::sCurrentTab = -1;

ToolConfig::ToolConfig(QWidget *parent)
	: QDialog(parent)
{
	init();
}

void ToolConfig::init()
{
	setupUi(this);

	connect(arguments, &QListWidget::itemSelectionChanged, this, &ToolConfig::updateArgButtons);
	connect(argUp, &QToolButton::clicked, this, &ToolConfig::moveArgUp);
	connect(argDown, &QToolButton::clicked, this, &ToolConfig::moveArgDown);
	connect(argAdd, &QToolButton::clicked, this, &ToolConfig::addArg);
	connect(argRemove, &QToolButton::clicked, this, &ToolConfig::removeArg);
	connect(btnBrowseForProgram, &QPushButton::clicked, this, &ToolConfig::browseForProgram);
}

void ToolConfig::browseForProgram()
{
	QFileInfo info(program->text());
	QString str = QFileDialog::getOpenFileName(this, tr("Select program file"),
											   info.exists() ? info.canonicalFilePath() : QString());
	if (!str.isNull()) {
		info.setFile(str);
		if (!info.isExecutable()) {
			QMessageBox::warning(this, tr("Invalid program"),
								 tr("The file '%1' is not executable!").arg(info.fileName()));
			return;
		}
		program->setText(info.canonicalFilePath());
	}
}

void ToolConfig::updateArgButtons()
{
	int selRow = -1;
	if (arguments->selectedItems().count() > 0)
		selRow = arguments->currentRow();
	argRemove->setEnabled(selRow != -1);
	argUp->setEnabled(selRow > 0);
	argDown->setEnabled(selRow != -1 && selRow < arguments->count() - 1);
}

void ToolConfig::moveArgUp()
{
	int selRow = -1;
	if (arguments->selectedItems().count() > 0)
		selRow = arguments->currentRow();
	if (selRow > 0) {
		QListWidgetItem *item = arguments->takeItem(selRow);
		arguments->insertItem(selRow - 1, item);
		arguments->setCurrentItem(arguments->item(selRow - 1));
	}
}

void ToolConfig::moveArgDown()
{
	int selRow = -1;
	if (arguments->selectedItems().count() > 0)
		selRow = arguments->currentRow();
	if (selRow != -1 &&  selRow < arguments->count() - 1) {
		QListWidgetItem *item = arguments->takeItem(selRow);
		arguments->insertItem(selRow + 1, item);
		arguments->setCurrentItem(arguments->item(selRow + 1));
	}
}

void ToolConfig::addArg()
{
	arguments->addItem(tr("NewArgument"));
	QListWidgetItem* item = arguments->item(arguments->count() - 1);
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	arguments->setCurrentItem(item);
	arguments->editItem(item);
}

void ToolConfig::removeArg()
{
	if (arguments->currentRow() > -1)
		if (arguments->currentItem()->isSelected())
			arguments->takeItem(arguments->currentRow());
}

QDialog::DialogCode ToolConfig::doToolConfig(QWidget *parent, Engine &engine)
{
	ToolConfig dlg(parent);

	dlg.toolName->setText(engine.name());
	dlg.program->setText(engine.program());
	dlg.arguments->addItems(engine.arguments());
	for (int i = 0; i < dlg.arguments->count(); ++i) {
		QListWidgetItem *item = dlg.arguments->item(i);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
	}
	dlg.viewPdf->setChecked(engine.showPdf());

	dlg.show();

	DialogCode result = static_cast<DialogCode>(dlg.exec());
	if (result == Accepted) {
		dlg.arguments->setCurrentItem(nullptr); // ensure editing is terminated
		engine.setName(dlg.toolName->text());
		engine.setProgram(dlg.program->text());
		QStringList args;
		for (int i = 0; i < dlg.arguments->count(); ++i)
			args << dlg.arguments->item(i)->text();
		engine.setArguments(args);
		engine.setShowPdf(dlg.viewPdf->isChecked());
	}

	return result;
}
