/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "CrashHandler.h"

#ifndef _WIN32
namespace crash {
    void registerHandler(int (*getPluginId)()) {}
    void unregisterHandler() {}
    void registerThread() {}
    void unregisterThread() {}
}
#else

#include <set>
#include <atomic>
#include <thread>
#include <windows.h>
#include <DbgHelp.h>

namespace {
LPTOP_LEVEL_EXCEPTION_FILTER previousHandler = nullptr;

int (*getActivePluginId)();
int ourPluginId;
std::thread::id mainThreadId;
std::atomic_flag threadLock;
std::set<std::thread::id> knownThreads;
}

namespace crash {
LONG WINAPI handleException(EXCEPTION_POINTERS *ei);

void registerHandler(int (*getPluginId)()) {
    getActivePluginId = getPluginId;
    mainThreadId = std::this_thread::get_id();
    ourPluginId = getPluginId();

    HMODULE module = ::GetModuleHandleA("dbghelp.dll");

    if (!module) {
        ::LoadLibraryA("dbghelp.dll");
    }

    previousHandler = SetUnhandledExceptionFilter(handleException);
}

void unregisterHandler() {
    if (previousHandler) {
        SetUnhandledExceptionFilter(previousHandler);
        previousHandler = nullptr;
    }
}

void registerThread() {
    while (threadLock.test_and_set(std::memory_order_acquire)) {
        // wait for lock
    }
    knownThreads.insert(std::this_thread::get_id());
    threadLock.clear();
}

void unregisterThread() {
    while (threadLock.test_and_set(std::memory_order_acquire)) {
        // wait for lock
    }
    knownThreads.erase(std::this_thread::get_id());
    threadLock.clear();
}

bool isOurFault() {
    auto threadId = std::this_thread::get_id();

    if (threadId == mainThreadId) {
        return (ourPluginId == getActivePluginId());
    }

    if (threadLock.test_and_set(std::memory_order_acquire)) {
        return false;
    }

    bool isKnownThread = (knownThreads.find(threadId) != knownThreads.end());
    threadLock.clear();
    return isKnownThread;
}

typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

LONG WINAPI handleException(EXCEPTION_POINTERS *ei) {
    if (!isOurFault()) {
        if (previousHandler) {
            return previousHandler(ei);
        }

        return EXCEPTION_CONTINUE_SEARCH;
    }

    HMODULE module = ::GetModuleHandleA("dbghelp.dll");
    if (!module) {
        module = ::LoadLibraryA("dbghelp.dll");
    }

    if (!module) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    const MINIDUMPWRITEDUMP pDump = MINIDUMPWRITEDUMP(::GetProcAddress(module, "MiniDumpWriteDump"));
    if (!pDump) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    char name[MAX_PATH];
    SYSTEMTIME t;
    GetSystemTime(&t);
    wsprintfA(name,
        "AviTab-crash_%4d%02d%02d_%02d%02d%02d.dmp",
        t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

    const HANDLE handle = ::CreateFileA(name, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    MINIDUMP_EXCEPTION_INFORMATION exception_information = {};
    exception_information.ThreadId = ::GetCurrentThreadId();
    exception_information.ExceptionPointers = ei;
    exception_information.ClientPointers = false;

    pDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            handle,
            MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpWithHandleData),
            &exception_information,
            nullptr,
            nullptr
    );

    ::CloseHandle(handle);
    return EXCEPTION_CONTINUE_SEARCH;
}

} // namespace crash

#endif
