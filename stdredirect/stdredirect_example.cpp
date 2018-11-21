#include "stdredirect.h"

#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "This stdout string is displayed on the console." << std::endl;

    // enable redirection only if debugger is present/attached
    if (IsDebuggerPresent()) {
        if (!STDREDIRECT_redirectStdoutToDebugger()) {
            return EXIT_FAILURE;
        } 
        std::cout << "This stdout string is displayed in the debugger." << std::endl;

        // give the thread some time to read from the pipe
        Sleep(100);

        if (!STDREDIRECT_unredirectStdout()) {
            return EXIT_FAILURE;
        }
        std::cout << "This stdout string is displayed on the console again." << std::endl;
    }

    std::getchar();
    return EXIT_SUCCESS;
}