/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2013-2019  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "TWSystemCmd.h"


TWSystemCmd::TWSystemCmd(QObject* parent, const bool isOutputWanted /* = true */, const bool runInBackground /* = false*/ )
: QProcess(parent)
, wantOutput(isOutputWanted)
, deleteOnFinish(runInBackground)
{
	connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(processOutput()));
	connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
	connect(this, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	finishedSuccessfully = false;
	finished = false;
}

bool TWSystemCmd::waitForStarted(int msecs /* = 30000 */)
{
	if (finished)
		return finishedSuccessfully;
	return QProcess::waitForStarted(msecs);
}

bool TWSystemCmd::waitForFinished(int msecs /* = 30000 */)
{
	if (finished)
		return finishedSuccessfully;
	return QProcess::waitForFinished(msecs);
}

void TWSystemCmd::processError(QProcess::ProcessError error)
{
	finished = true;
	if (wantOutput)
		result = tr("ERROR: failure code %1").arg(error);
	if (deleteOnFinish)
		deleteLater();
}

void TWSystemCmd::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	finished = true;
	finishedSuccessfully = (exitStatus == QProcess::NormalExit);
	if (wantOutput) {
		if (exitStatus == QProcess::NormalExit) {
			if (bytesAvailable() > 0) {
				QByteArray ba = readAllStandardOutput();
#if defined(MIKTEX_WINDOWS)
                                result += QString::fromUtf8(ba.constData());
#else
				result += QString::fromLocal8Bit(ba.constData());
#endif
			}
		}
		else {
			result = tr("ERROR: exit code %1").arg(exitCode);
		}
	}
	if (deleteOnFinish)
		deleteLater();
}

void TWSystemCmd::processOutput()
{
	if (wantOutput && bytesAvailable() > 0) {
		QByteArray ba = readAllStandardOutput();
#if defined(MIKTEX_WINDOWS)
		result += QString::fromUtf8(ba.constData());
#else
		result += QString::fromLocal8Bit(ba.constData());
#endif
	}
}

