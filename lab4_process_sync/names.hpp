#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>

inline constexpr const wchar_t* kSemName = L"DownloadSlots";
inline constexpr const wchar_t* kMutexName = L"LogAccessMutex";
inline constexpr const wchar_t* kEvtName = L"BrowserClosingEvent";

inline std::wstring FindDownloaderExe() {
    wchar_t buf[MAX_PATH]{};
    GetModuleFileNameW(nullptr, buf, MAX_PATH);
    std::filesystem::path p(buf);
    return (p.parent_path() / L"Downloader.exe").wstring();
}

inline void LogLine(HANDLE hMutex, const std::string& line) {
    WaitForSingleObject(hMutex, INFINITE);
    std::cout << line << std::endl;
    ReleaseMutex(hMutex);
}

inline void WaitAllProcesses(const std::vector<HANDLE>& procs) {
    const size_t k = 64;
    for (size_t i = 0; i < procs.size();) {
        size_t chunk = std::min(k, procs.size() - i);
        WaitForMultipleObjects((DWORD)chunk, procs.data() + i, TRUE, INFINITE);
        i += chunk;
    }
}