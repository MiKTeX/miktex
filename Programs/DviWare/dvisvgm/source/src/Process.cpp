/*************************************************************************
** Process.cpp                                                          **
**                                                                      **
** This file is part of dvisvgm -- a fast DVI to SVG converter          **
** Copyright (C) 2005-2023 Martin Gieseking <martin.gieseking@uos.de>   **
**                                                                      **
** This program is free software; you can redistribute it and/or        **
** modify it under the terms of the GNU General Public License as       **
** published by the Free Software Foundation; either version 3 of       **
** the License, or (at your option) any later version.                  **
**                                                                      **
** This program is distributed in the hope that it will be useful, but  **
** WITHOUT ANY WARRANTY; without even the implied warranty of           **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the         **
** GNU General Public License for more details.                         **
**                                                                      **
** You should have received a copy of the GNU General Public License    **
** along with this program; if not, see <http://www.gnu.org/licenses/>. **
*************************************************************************/

#if defined(MIKTEX)
#include <config.h>
#endif
#ifdef _WIN32
	#include "windows.hpp"
#else
	#include <csignal>
	#include <cstring>
	#include <fcntl.h>
	#include <sys/wait.h>
	#include <unistd.h>
#endif
#if defined(MIKTEX_WINDOWS)
#include <miktex/Util/PathNameUtil>
#define EXPATH_(x) MiKTeX::Util::PathNameUtil::ToLengthExtendedPathName(x)
#endif

#include <cstdlib>
#include <regex>
#include "FileSystem.hpp"
#include "Process.hpp"
#include "SignalHandler.hpp"

using namespace std;


/** Helper class that encapsulates the system-specific parts of
 *  running a subprocess and retrieving its terminal output. */
class Subprocess {
	public:
		enum class State {RUNNING, FINISHED, FAILED};

	public:
		Subprocess () : _rdbuf(4096) {}
		Subprocess (const Subprocess&) =delete;
		Subprocess (Subprocess&&) =delete;
		~Subprocess ();
		bool run (const string &cmd, string params, Process::PipeFlags flags);
		bool readFromPipe (string &out, const SearchPattern &pattern);
		State state ();

	protected:
		string read (const SearchPattern &pattern, bool *error=nullptr);
		size_t readFromPipe (char *buf, size_t size, bool *error=nullptr) const;

	private:
		vector<char> _rdbuf;
		size_t _bufstartpos = 0;
#ifdef _WIN32
		HANDLE _pipeReadHandle = nullptr;   ///< handle of read end of pipe
		HANDLE _childProcHandle = nullptr;  ///< handle of child process
#else
		int _readfd = -1; ///< file descriptor of read end of pipe
		pid_t _pid = -1;  ///< PID of the subprocess
#endif
};


string Subprocess::read (const SearchPattern &pattern, bool *error) {
	string result;
	const size_t bytesRead = readFromPipe(&_rdbuf[_bufstartpos], _rdbuf.size()-_bufstartpos, error);
	const size_t bufendpos = _bufstartpos + bytesRead;
	smatch matches;
	size_t matchendpos = bufendpos;  // buffer position after last match
	if (pattern.search.empty())
		result.assign(_rdbuf.data(), matchendpos);
	else {
		bool matched=false;
		regex re(pattern.search);
		for (auto it = cregex_iterator(&_rdbuf[0], &_rdbuf[0]+bufendpos, re); it != cregex_iterator(); ++it) {
			result += it->format(pattern.replace, regex_constants::format_no_copy);
			matchendpos = it->position() + it->length();
			matched = true;
		}
		const size_t MAX_OVERLAP=50;
		size_t overlappos;
		if (matched)
			overlappos = max(bufendpos - min(bufendpos, MAX_OVERLAP), matchendpos);
		else
			overlappos = matchendpos - min(MAX_OVERLAP, matchendpos);
		copy(_rdbuf.begin()+overlappos, _rdbuf.begin()+bufendpos, _rdbuf.begin());
		_bufstartpos = bufendpos-overlappos;
	}
	return result;
}


Process::Process (string cmd, string paramstr)
	: _cmd(std::move(cmd)), _paramstr(std::move(paramstr))
{
}


/** Runs the process and waits until it's finished.
 *  @param[out] out takes the output written to stdout by the executed subprocess
 *  @return true if process terminated properly
 *  @throw SignalException if CTRL-C was pressed during execution */

bool Process::run (string *out, PipeFlags flags) {
	return run(out, SearchPattern(), flags);
}


bool Process::run (string *out, const SearchPattern &pattern, PipeFlags flags) {
	Subprocess subprocess;
	if (!subprocess.run(_cmd, _paramstr, flags))
		return false;
	if (out)
		out->clear();
	for (;;) {
		if (out)
			subprocess.readFromPipe(*out, pattern);
		Subprocess::State state = subprocess.state();
		if (state != Subprocess::State::RUNNING)
			return state == Subprocess::State::FINISHED;
		SignalHandler::instance().check();
	}
}


/** Runs the process in the given working directory and waits until it's finished.
 *  @param[in] dir working directory
 *  @param[out] out takes the output written to stdout by the executed process
 *  @return true if process terminated properly
 *  @throw SignalException if CTRL-C was pressed during execution */
bool Process::run (const string &dir, string *out, PipeFlags flags) {
	bool ret = false;
	string cwd = FileSystem::getcwd();
	if (FileSystem::chdir(dir)) {
		ret = run(out, flags);
		ret &= FileSystem::chdir(cwd);
	}
	return ret;
}

// system-specific stuff

#ifdef _WIN32

static inline void close_and_zero_handle (HANDLE &handle) {
	CloseHandle(handle);
	handle = nullptr;
}


Subprocess::~Subprocess () {
	if (_pipeReadHandle != nullptr)
		CloseHandle(_pipeReadHandle);
	if (_childProcHandle != nullptr) {
		TerminateProcess(_childProcHandle, 1);
		CloseHandle(_childProcHandle);
	}
}


/** Retrieves output generated by the child process and optionally filters
 *  the lines by a regular expression and replaces them. If the search pattern
 *  is not empty and the line currently processed doesn't match, the line isn't
 *  appended to the output string.
 *  @param[out] out read output is appended to this string
 *  @param[in] searchPattern regex pattern applied to each line
 *  @param[in] replacePattern replacement for all lines matching the search pattern
 *  @returns false on errors */
bool Subprocess::readFromPipe (string &out, const SearchPattern &pattern) {
	if (!_pipeReadHandle)
		return false;
	bool finished=false;
	bool processExited=false;
	DWORD len=0;
	while (PeekNamedPipe(_pipeReadHandle, nullptr, 0, nullptr, &len, nullptr)) {  // prevent blocking
		if (len == 0) {
			if (processExited)
				break;
			// process still busy
			processExited = (!_childProcHandle || WaitForSingleObject(_childProcHandle, 100) != WAIT_TIMEOUT);
		}
		string buf = read(pattern, &finished);
		if (buf.empty())
			break;
		out.append(buf);
	}
	return !finished;
}


/** Reads a sequence of bytes from the pipe into a buffer. The function stops reading
 *  when either all bytes have been read from the pipe or the buffer is completely filled.
 *  @param[in] buf pointer to start of the buffer
 *  @param[in] size maximal number of bytes that can be stored in the buffer
 *  @return number of bytes read */
size_t Subprocess::readFromPipe (char *buf, size_t size, bool *error) const {
	DWORD bytesRead;
	bool success = ReadFile(_pipeReadHandle, buf, size, &bytesRead, nullptr);
	if (error)
		*error = !success;
	return success ? bytesRead : 0;
}


/** Starts a child process.
 *  @param[in] cmd name of command to execute
 *  @param[in] paramstr parameters required by command
 *  @returns true if child process started properly */
bool Subprocess::run (const string &cmd, string paramstr, Process::PipeFlags flags) {
	SECURITY_ATTRIBUTES securityAttribs;
	ZeroMemory(&securityAttribs, sizeof(SECURITY_ATTRIBUTES));
	securityAttribs.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttribs.bInheritHandle = true;

	HANDLE pipeWriteHandle; // write end of pipe
	if (CreatePipe(&_pipeReadHandle, &pipeWriteHandle, &securityAttribs, 0) == ERROR_INVALID_HANDLE)
		return false;

	SetHandleInformation(_pipeReadHandle, HANDLE_FLAG_INHERIT, 0);
#if defined(MIKTEX_WINDOWS)
	HANDLE nullFile = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &securityAttribs, OPEN_EXISTING, 0, nullptr);
#else
	HANDLE nullFile = CreateFile("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &securityAttribs, OPEN_EXISTING, 0, nullptr);
#endif
	bool success = false;
	if (nullFile != INVALID_HANDLE_VALUE) {
		STARTUPINFO startupInfo;
		ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
		startupInfo.cb = sizeof(STARTUPINFO);
		startupInfo.dwFlags = STARTF_USESTDHANDLES;
		startupInfo.hStdInput = nullFile;
		startupInfo.hStdOutput = (flags & Process::PF_STDOUT) ? pipeWriteHandle : nullFile;
		startupInfo.hStdError = (flags & Process::PF_STDERR) ? pipeWriteHandle : nullFile;

		PROCESS_INFORMATION processInfo;
		ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

		string cmdline = cmd + " " + paramstr;
		// put subprocess in separate process group to prevent its termination in case of CTRL-C
#if defined(MIKTEX_WINDOWS)
		success = CreateProcessW(nullptr, (LPWSTR)EXPATH_(cmdline).c_str(), nullptr, nullptr, TRUE, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &startupInfo, &processInfo);
#else
		success = CreateProcess(nullptr, (LPSTR)cmdline.c_str(), nullptr, nullptr, true, CREATE_NEW_PROCESS_GROUP, nullptr, nullptr, &startupInfo, &processInfo);
#endif
		if (success) {
			_childProcHandle = processInfo.hProcess;
			CloseHandle(processInfo.hThread);
		}
		CloseHandle(nullFile);
	}
	CloseHandle(pipeWriteHandle);  // must be closed before reading from pipe to prevent blocking
	if (!success)
		close_and_zero_handle(_pipeReadHandle);
	return success;
}


/** Returns the current state of the child process. */
Subprocess::State Subprocess::state () {
	DWORD status;
	if (!GetExitCodeProcess(_childProcHandle, &status))
		return State::FAILED;
	if (status == STILL_ACTIVE)
		return State::RUNNING;
	close_and_zero_handle(_childProcHandle);
	return status == 0 ? State::FINISHED : State::FAILED;
}

#else  // !_WIN32

Subprocess::~Subprocess () {
	if (_readfd >= 0)
		close(_readfd);
	if (_pid > 0)
		kill(_pid, SIGKILL);
}


/** Retrieves output generated by child process.
 *  @param[out] out read output is appended to this string
 *  @returns false on errors */
bool Subprocess::readFromPipe (string &out, const SearchPattern &pattern) {
	if (_readfd < 0 || _pid < 0)
		return false;
	bool finished=false;
	for (;;) {
		string buf = read(pattern, &finished);
		if (buf.empty())
			break;
		out.append(buf);
	}
	if (finished) {
		close(_readfd);
		_readfd = -1;
	}
	return !finished;
}


size_t Subprocess::readFromPipe (char *buf, size_t size, bool *error) const {
	auto len = ::read(_readfd, buf, size);
	if (error)
		*error = len < 0;
	return len >= 0 ? size_t(len) : 0;
}

/** Extracts whitespace-separated parameters from a string.
 *  @param[in,out] paramstr the parameter string
 *  @param[out] params vector holding pointers to the extracted parameters */
static void split_paramstr (string &paramstr, vector<const char*> &params) {
	size_t left=0, right=0;  // index of first and last character of current parameter
	char quote=0;            // current quote character, 0=none
	const size_t len = paramstr.length();
	while (left <= right && right < len) {
		while (left < len && isspace(paramstr[left]))
			++left;
		if (left < len && (paramstr[left] == '"' || paramstr[left] == '\''))
			quote = paramstr[left++];
		right = left;
		while (right < len && (quote || !isspace(paramstr[right]))) {
			if (quote && paramstr[right] == quote) {
				quote=0;
				break;
			}
			else
				++right;
		}
		if (right < len)
			paramstr[right]=0;
		if (left < len)
			params.push_back(&paramstr[left]);
		left = ++right;
	}
}


/** Starts a child process.
 *  @param[in] cmd name of command to execute or absolute path to executable
 *  @param[in] paramstr parameters required by the command
 *  @returns true if child process started properly */
bool Subprocess::run (const string &cmd, string paramstr, Process::PipeFlags flags) {
	int pipefd[2];
	if (cmd.empty() || pipe(pipefd) < 0)
		return false;

	_pid = fork();
	if (_pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return false;
	}
	if (_pid == 0) {   // child process
		int devnull = open("/dev/null", O_WRONLY);
		dup2((flags & Process::PF_STDOUT) ? pipefd[1] : devnull, STDOUT_FILENO);  // redirect stdout to the pipe
		dup2((flags & Process::PF_STDERR) ? pipefd[1] : devnull, STDERR_FILENO);  // redirect stdout to the pipe
		close(pipefd[0]);
		close(pipefd[1]);
		close(devnull);

		vector<const char*> params;
		params.push_back(cmd.c_str());
		split_paramstr(paramstr, params);
		params.push_back(nullptr);  // trailing null pointer marks end of parameter list
		signal(SIGINT, SIG_IGN);    // child process is supposed to ignore ctrl-c events
		if (params[0][0] == '/')    // absolute path to executable?
			params[0] = strrchr(params[0], '/')+1;  // filename of executable
		execvp(cmd.c_str(), const_cast<char* const*>(params.data()));
		exit(1);
	}
	_readfd = pipefd[0];
	close(pipefd[1]);  // close write end of pipe
	return true;
}


/** Returns the current state of the child process. */
Subprocess::State Subprocess::state () {
	int status;
	pid_t wpid = waitpid(_pid, &status, WNOHANG);
	if (wpid == 0)
		return State::RUNNING;  // still running
	_pid = -1;
	if (wpid > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return State::FINISHED;
	return State::FAILED;
}

#endif  // !_WIN32
