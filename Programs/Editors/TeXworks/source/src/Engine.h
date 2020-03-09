/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2018-2020  Jonathan Kew, Stefan Löffler

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
#ifndef ENGINE_H
#define ENGINE_H

#include <QProcess>
#include <QFileInfo>

// specification of an "engine" used to process files
class Engine
{
public:
	Engine() = default;
	Engine(const QString& name, const QString& program, const QStringList & arguments, bool showPdf);
	Engine(const Engine& orig);
	Engine& operator=(const Engine& rhs);

	const QString name() const;
	const QString program() const;
	const QStringList arguments() const;
	bool showPdf() const;

	void setName(const QString& name);
	void setProgram(const QString& program);
	void setArguments(const QStringList& arguments);
	void setShowPdf(bool showPdf);

	bool isAvailable() const;
	QProcess * run(const QFileInfo & input, QObject * parent = nullptr);

	static QStringList binPaths();

private:
	static QString programPath(const QString & prog);

	QString _name;
	QString _program;
	QStringList _arguments;
	bool _showPdf{false};
};



#endif // !defined(ENGINE_H)
