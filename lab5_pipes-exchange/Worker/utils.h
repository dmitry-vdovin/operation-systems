#pragma once
#include <windows.h>
#include <string>
#include <iostream>

inline std::string MakePipeIn(int id) { return "\\\\.\\pipe\\worker_in_" + std::to_string(id); }
inline std::string MakePipeOut(int id) { return "\\\\.\\pipe\\worker_out_" + std::to_string(id); }

inline void PrintLastError(const char* where)
{
    DWORD err = GetLastError();
    LPVOID msgBuf = nullptr;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msgBuf, 0, NULL
    );

    std::cerr << where << " failed. GetLastError=" << err
        << (msgBuf ? (std::string(": ") + (char*)msgBuf) : "") << "\n";

    if (msgBuf) LocalFree(msgBuf);
}