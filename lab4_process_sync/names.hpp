#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>

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