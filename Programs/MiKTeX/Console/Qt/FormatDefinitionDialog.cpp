/* FormatDefinitionDialog.cpp:

   Copyright (C) 2018 Christian Schenk

   This file is part of MiKTeX Console.

   MiKTeX Console is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   MiKTeX Console is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MiKTeX Console; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. */

#include <set>

#include "FormatDefinitionDialog.h"
#include "ui_FormatDefinitionDialog.h"

using namespace MiKTeX::Core;
using namespace std;

FormatInfo newFormat()
{
  FormatInfo format;
  return format;
}

FormatDefinitionDialog::FormatDefinitionDialog(QWidget* parent) :
  FormatDefinitionDialog(parent, newFormat())
{
}

FormatDefinitionDialog::FormatDefinitionDialog(QWidget* parent, const FormatInfo& format) :
  QDialog(parent),
  ui(new Ui::FormatDefinitionDialog)
{
  QRegularExpression azPlus("[a-z]+");
  QRegularExpression azStar("[a-z]*");
  QRegularExpression fnamePlus("[-.A-Za-z_]+");
  QRegularExpression fnameStar("[-.A-Za-z_]+");
  ui->setupUi(this);
  ui->leKey->setText(QString::fromUtf8(format.key.c_str()));
  ui->leKey->setValidator(new QRegularExpressionValidator(azPlus, this));
  if (ui->leKey->hasAcceptableInput())
  {
    ui->leKey->setReadOnly(true);
  }
  ui->leName->setText(QString::fromUtf8(format.name.c_str()));
  ui->leName->setValidator(new QRegularExpressionValidator(azStar, this));
  set<string> compilers;
  for (const FormatInfo& f : session->GetFormats())
  {
    compilers.insert(f.compiler);
    if (PathName::Compare(f.key, format.key) == 0)
    {
      continue;
    }
    ui->comboPreloadedFormat->addItem(QString::fromUtf8(f.key.c_str()));
  }
  ui->comboPreloadedFormat->setCurrentIndex(ui->comboPreloadedFormat->findText(QString::fromUtf8(format.preloaded.c_str())));
  for (const string& compiler : compilers)
  {
    ui->comboCompiler->addItem(QString::fromUtf8(compiler.c_str()));
  }
  ui->comboCompiler->setCurrentIndex(ui->comboCompiler->findText(QString::fromUtf8(format.compiler.c_str())));
  ui->leInputFileName->setText(QString::fromUtf8(format.inputFile.c_str()));
  ui->leInputFileName->setValidator(new QRegularExpressionValidator(fnamePlus, this));
  ui->leOutputFileName->setText(QString::fromUtf8(format.outputFile.c_str()));
  ui->leOutputFileName->setValidator(new QRegularExpressionValidator(fnameStar, this));
  ui->leDescription->setText(QString::fromUtf8(format.description.c_str()));
  ui->checkExclude->setChecked(format.exclude);
}

FormatDefinitionDialog::~FormatDefinitionDialog()
{
  delete ui;
}

void FormatDefinitionDialog::done(int r)
{
  if (r == QDialog::Accepted)
  {
    if (!ui->leKey->hasAcceptableInput())
    {
      ui->leKey->setFocus();
      return;
    }
    if (!ui->leInputFileName->hasAcceptableInput())
    {
      ui->leInputFileName->setFocus();
      return;
    }
    if (ui->comboCompiler->currentIndex() < 0)
    {
      ui->comboCompiler->setFocus();
      return;
    }
  }
  QDialog::done(r);
}

FormatInfo FormatDefinitionDialog::GetFormatInfo() const
{
  FormatInfo format;
  format.key = ui->leKey->text().toUtf8().constData();
  format.name = ui->leName->text().toUtf8().constData();
  format.compiler = ui->comboCompiler->currentText().toUtf8().constData();
  format.preloaded = ui->comboPreloadedFormat->currentText().toUtf8().constData();
  format.inputFile = ui->leInputFileName->text().toUtf8().constData();
  format.outputFile = ui->leOutputFileName->text().toUtf8().constData();
  format.description = ui->leDescription->text().toUtf8().constData();
  format.exclude = ui->checkExclude->isChecked();
  return format;
}