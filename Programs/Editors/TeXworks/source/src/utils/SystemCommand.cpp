/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2013-2020  Jonathan Kew, Stefan Löffler, Charlie Sharpsteen

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

#include "SystemCommand.h"

namespace Tw {
namespace Utils {

SystemCommand::SystemCommand(QObject* parent, const bool isOutputWanted /* = true */, const bool runInBackground /* = false*/ )
: QProcess(parent)
, wantOutput(isOutputWanted)
, deleteOnFinish(runInBackground)
{
	connect(this, &SystemCommand::readyReadStandardOutput, this, &SystemCommand::processOutput);
	connect(this, static_cast<void (SystemCommand::*)(int, QProcess::ExitStatus)>(&SystemCommand::finished), this, &SystemCommand::processFinished);
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
	connect(this, static_cast<void (SystemCommand::*)(QProcess::ProcessError)>(&SystemCommand::error), this, &SystemCommand::processError);
#else
	connect(this, &SystemCommand::errorOccurred, this, &SystemCommand::processError);
#endif
	hasFinishedSuccessfully = false;
	hasFinished = false;
}

bool SystemCommand::waitForStarted(int msecs /* = 30000 */)
{
	if (hasFinished)
		return hasFinishedSuccessfully;
	return QProcess::waitForStarted(msecs);
}

bool SystemCommand::waitForFinished(int msecs /* = 30000 */)
{
	if (hasFinished)
		return hasFinishedSuccessfully;
	return QProcess::waitForFinished(msecs);
}

void SystemCommand::processError(QProcess::ProcessError error)
{
	hasFinished = true;
	if (wantOutput)
		result = tr("ERROR: failure code %1").arg(error);
	if (deleteOnFinish)
		deleteLater();
}

void SystemCommand::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	hasFinished = true;
	hasFinishedSuccessfully = (exitStatus == QProcess::NormalExit);
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

void SystemCommand::processOutput()
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

} // namespace Utils
} // namespace Tw
