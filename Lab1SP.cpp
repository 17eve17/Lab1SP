#include <windows.h>
#include <vector>
#include <string>
#include <chrono>

int main() {
    HANDLE eventParent = CreateEventA(NULL, TRUE, FALSE, "ParentEvent");
    HANDLE eventChild = CreateEventA(NULL, TRUE, FALSE, "ChildEvent");

    std::vector<std::string> progs = { "notepad.exe", "mspaint.exe" };
    std::vector<PROCESS_INFORMATION> piList;

    for (auto& p : progs) {
        STARTUPINFOA si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};

        std::vector<char> buf(p.begin(), p.end());
        buf.push_back('\0');

        CreateProcessA(NULL, buf.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        piList.push_back(pi);
    }

    SetEvent(eventParent);

    auto start = std::chrono::steady_clock::now();

    for (auto& pi : piList) {
        while (true) {
            DWORD waitProc = WaitForSingleObject(pi.hProcess, 300);
            DWORD waitEvent = WaitForSingleObject(eventChild, 1);

            if (waitProc == WAIT_OBJECT_0) break;
            if (waitEvent == WAIT_OBJECT_0) break;

            auto now = std::chrono::steady_clock::now();
            auto t = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();

            if (t > 10) {
                TerminateProcess(pi.hProcess, 1);
                break;
            }
        }

        DWORD code;
        GetExitCodeProcess(pi.hProcess, &code);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    CloseHandle(eventParent);
    CloseHandle(eventChild);

    return 0;
}
