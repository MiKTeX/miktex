/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2010-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#ifndef SystemCommand_H
#define SystemCommand_H

#include <QProcess>

namespace Tw {
namespace Utils {

class SystemCommand : public QProcess {
	Q_OBJECT

public:
	explicit SystemCommand(QObject * parent, const bool isOutputWanted = true, const bool runInBackground = false);
	~SystemCommand() override = default;

	QString getResult() { return result; }

	// replacement of QProcess::waitForStarted()
	// unlike the QProcess version, this returns true if the process has already
	// finished when the function is called
	bool waitForStarted(int msecs = 30000);

	// replacement of QProcess::waitForFinished()
	// unlike the QProcess version, this returns true if the process has already
	// finished when the function is called
	bool waitForFinished(int msecs = 30000);

private slots:
	void processError(QProcess::ProcessError error);
	void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void processOutput();

private:
	bool wantOutput;
	bool deleteOnFinish;
	bool hasFinished;
	bool hasFinishedSuccessfully;
	QString result;
};

} // namespace Utils
} // namespace Tw

#endif // !defined(SystemCommand_H)
