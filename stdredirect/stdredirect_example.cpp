#include "stdredirect.h"

#include <iostream>

int main(int argc, char* argv[]) {
    // redirect stdout to debugger
    STDREDIRECT_REDIRECTOR stdoutRedirector;             
    stdoutRedirector.stream = STDOUT;
    stdoutRedirector.callback = &STDREDIRECT_debuggerCallback;

    // enable redirection
    STDREDIRECT_redirect(&stdoutRedirector);          
    std::cout << "This stdout string is displayed in the debugger." << std::endl;

    std::getchar();
    return EXIT_SUCCESS;
}