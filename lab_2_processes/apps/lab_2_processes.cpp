#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include "lab_2_processes.h"

void fail(const char* where) {
    DWORD e = GetLastError();
    std::cerr << "[!] " << where << " failed, GetLastError=" << e << "\n";
    ExitProcess(1);
}

bool write_all(HANDLE h, const void* buf, DWORD bytes) {
    const BYTE* p = static_cast<const BYTE*>(buf);
    DWORD done = 0;
    while (done < bytes) {
        DWORD w = 0;
        if (!WriteFile(h, p + done, bytes - done, &w, nullptr)) return false;
        done += w;
    }
    return true;
}

bool read_exact(HANDLE h, void* buf, DWORD bytes) {
    BYTE* p = static_cast<BYTE*>(buf);
    DWORD done = 0;
    while (done < bytes) {
        DWORD r = 0;
        if (!ReadFile(h, p + done, bytes - done, &r, nullptr)) return false; // EOF/ошибка
        if (r == 0) return false; // EOF
        done += r;
    }
    return true;
}

int child_main() {
    // stdin/stdout у ребёнка уже перенаправлены родителем (через STARTF_USESTDHANDLES) (03-Процессы.pdf)
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD x = 0;
    if (!read_exact(hin, &x, sizeof(x))) return 0;
    write_all(hout, &x, sizeof(x)); // эхо-ответ
    return 0;
}

int parent_main() {
    // 1) Два анонимных канала: stdin ребёнка (Rd<- / Wr->) и stdout ребёнка (Rd<- / Wr->)
    SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE }; // наследуемые дескрипторы
    HANDLE cInRd = nullptr, cInWr = nullptr, cOutRd = nullptr, cOutWr = nullptr;
    if (!CreatePipe(&cInRd, &cInWr, &sa, 0)) fail("CreatePipe(stdin)");
    if (!CreatePipe(&cOutRd, &cOutWr, &sa, 0)) fail("CreatePipe(stdout)");

    // 2) Запрещаем наследование у тех концов, которые должны остаться только у родителя
    SetHandleInformation(cInWr, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(cOutRd, HANDLE_FLAG_INHERIT, 0);

    // 3) Готовим STARTUPINFO: перенаправляем stdin/stdout ребёнка в концы наших пайпов
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = cInRd;     // child stdin  <- чтение из Pipe1
    si.hStdOutput = cOutWr;    // child stdout -> запись в Pipe2
    si.hStdError = cOutWr;

    PROCESS_INFORMATION pi{};
    char exe[MAX_PATH]; GetModuleFileNameA(nullptr, exe, MAX_PATH);
    std::string cl = std::string("\"") + exe + "\" child";

    // ВАЖНО: bInheritHandles=TRUE — иначе перенаправленные std-handles не унаследуются. (03-Процессы.pdf)
    if (!CreateProcessA(nullptr, cl.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
        fail("CreateProcess(child)");
    }

    // 4) Родителю больше не нужны "детские" концы:
    CloseHandle(cInRd);
    CloseHandle(cOutWr);

    // 5) "Пингуем" ребёнка и закрываем запись => для него наступит EOF
    DWORD ping = 0xDEADBEEF;
    if (!write_all(cInWr, &ping, sizeof(ping))) fail("write(ping)");
    CloseHandle(cInWr); // важный момент, чтобы не зависнуть на чтении

    // 6) Ждём ответ и закрываем чтение
    DWORD pong = 0;
    if (!read_exact(cOutRd, &pong, sizeof(pong))) fail("read(pong)");
    CloseHandle(cOutRd);

    std::cout << "Ping=0x" << std::hex << ping << " Pong=0x" << pong << std::dec << "\n";

    // 7) Ждём завершения процесса-ребёнка и закрываем его дескрипторы (требование методички). (03-Процессы.pdf)
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;
}

int main(int argc, char** argv) {
    // Чтобы русские буквы печатались нормально:
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (argc > 1 && std::string(argv[1]) == "child") return child_main();
    return parent_main();
}