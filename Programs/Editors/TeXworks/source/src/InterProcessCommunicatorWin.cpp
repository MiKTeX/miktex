/*
	This is part of TeXworks, an environment for working with TeX documents
	Copyright (C) 2019-2020  Stefan Löffler

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

#include "InterProcessCommunicator.h"
#include "TWVersion.h"

#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0500
	#define _WIN32_WINNT			0x0500	// for HWND_MESSAGE
#endif
#include <windows.h>

namespace Tw {

#define TW_MUTEX_NAME		"org.tug.texworks-" TEXWORKS_VERSION
#define TW_HIDDEN_WINDOW_CLASS	"TeXworks:MessageTarget"
#define TW_OPEN_FILE_MSG		(('T' << 8) + 'W')	// just a small sanity check for the receiver
#define TW_BRING_TO_FRONT_MSG		(('B' << 8) + 'F')	// just a small sanity check for the receiver

LRESULT CALLBACK TW_HiddenWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class InterProcessCommunicatorPrivate
{
	Q_DISABLE_COPY(InterProcessCommunicatorPrivate)
	Q_DECLARE_PUBLIC(InterProcessCommunicator)

public:
	virtual ~InterProcessCommunicatorPrivate() {
		if (msgTarget)
			DestroyWindow(msgTarget);
	}

private:
	explicit InterProcessCommunicatorPrivate(InterProcessCommunicator * q) : hMutex(NULL), msgTarget(NULL), q_ptr(q) {}

	friend LRESULT CALLBACK TW_HiddenWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void receivedMessage(const unsigned int msg, const QByteArray & data) {
		Q_Q(InterProcessCommunicator);

		switch (msg) {
			case TW_BRING_TO_FRONT_MSG:
				emit q->receivedBringToFront();
				break;
			case TW_OPEN_FILE_MSG:
			{
				QStringList sl = QString::fromUtf8(data).split(QChar::fromLatin1('\n'));
				if (sl.size() == 1)
					emit q->receivedOpenFile(sl[0], -1);
				else
					emit q->receivedOpenFile(sl[0], sl[1].toInt());
				break;
			}
			default:
				break;
		}
	}

	static HWND findMessageWindow() {
		for (int retry = 0; retry < 100; ++retry) {
			HWND hWnd = FindWindowExA(HWND_MESSAGE, NULL, TW_HIDDEN_WINDOW_CLASS, NULL);
			if (hWnd)
				return hWnd;

			// couldn't find the other instance; not ready yet?
			// sleep for 50ms and then retry
			Sleep(50);
		}
		return NULL;
	}

	void createMessageTarget()
	{
		HINSTANCE hInstance = static_cast<HINSTANCE>(GetModuleHandle(NULL));
		if (!hInstance)
			return;

		WNDCLASSA myWindowClass;
		myWindowClass.style = 0;
		myWindowClass.lpfnWndProc = &TW_HiddenWindowProc;
		myWindowClass.cbClsExtra = 0;
		myWindowClass.cbWndExtra = 0;
		myWindowClass.hInstance = hInstance;
		myWindowClass.hIcon = NULL;
		myWindowClass.hCursor = NULL;
		myWindowClass.hbrBackground = NULL;
		myWindowClass.lpszMenuName = NULL;
		myWindowClass.lpszClassName = TW_HIDDEN_WINDOW_CLASS;

		ATOM atom = RegisterClassA(&myWindowClass);
		if (atom == 0)
			return;

		msgTarget = CreateWindowA(TW_HIDDEN_WINDOW_CLASS, "TeXworks", WS_OVERLAPPEDWINDOW,
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
						HWND_MESSAGE, NULL, hInstance, NULL);
		SetWindowLongPtr(msgTarget, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	// The mutex is handled exclusively by the InterProcessCommunicator
	HANDLE hMutex;
	HWND msgTarget;
	InterProcessCommunicator * q_ptr;
};

LRESULT CALLBACK TW_HiddenWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_COPYDATA:
		{
			InterProcessCommunicatorPrivate * ipcp = reinterpret_cast<InterProcessCommunicatorPrivate *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			const COPYDATASTRUCT* pcds = reinterpret_cast<const COPYDATASTRUCT*>(lParam);
			ipcp->receivedMessage(pcds->dwData, QByteArray::fromRawData(reinterpret_cast<const char*>(pcds->lpData), pcds->cbData));
			return 0;
		}
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

InterProcessCommunicator::InterProcessCommunicator()
	: _private(new InterProcessCommunicatorPrivate(this))
{
	Q_D(InterProcessCommunicator);
	d->hMutex = CreateMutexA(NULL, FALSE, TW_MUTEX_NAME);
	if (d->hMutex == NULL)
		return;

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		_isFirstInstance = false;
		CloseHandle(d->hMutex);
		d->hMutex = NULL;
	}

	if (_isFirstInstance)
		d->createMessageTarget();
}

InterProcessCommunicator::~InterProcessCommunicator()
{
	Q_D(InterProcessCommunicator);
	if (d->hMutex) {
		CloseHandle(d->hMutex);
		d->hMutex = NULL;
	}
}

void InterProcessCommunicator::sendBringToFront()
{
	HWND hWnd = InterProcessCommunicatorPrivate::findMessageWindow();
	if (hWnd == NULL)
		return;

	COPYDATASTRUCT cds;
	cds.dwData = TW_BRING_TO_FRONT_MSG;
	cds.cbData = 0;
	cds.lpData = NULL;
	SendMessageA(hWnd, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&cds));
}

void InterProcessCommunicator::sendOpenFile(const QString & path, const int position)
{
	HWND hWnd = InterProcessCommunicatorPrivate::findMessageWindow();
	if (hWnd == NULL)
		return;

	QByteArray ba = path.toUtf8() + "\n" + QByteArray::number(position);
	COPYDATASTRUCT cds;
	cds.dwData = TW_OPEN_FILE_MSG;
	cds.cbData = ba.length();
	cds.lpData = ba.data();
	SendMessageA(hWnd, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&cds));
}

} // namespace Tw
