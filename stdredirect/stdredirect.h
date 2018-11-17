#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#pragma once

#include <io.h>
#include <stdio.h>
#include <windows.h>

#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// standard output/error stream
typedef enum {
    STDOUT,
    STDERR
} STDREDIRECT_STREAM;

// function pointer to character-wise callback function
typedef void (*STDREDIRECT_CALLBACK)(char c);

// redirector object for a given stream
typedef struct STDREDIRECT_REDIRECTOR {
    STDREDIRECT_STREAM   stream;
    STDREDIRECT_CALLBACK callback;

    HANDLE stdHandle;
    HANDLE readablePipeEnd;
    HANDLE writablePipeEnd;

    HANDLE thread;
} STDREDIRECT_REDIRECTOR;


void STDREDIRECT_redirect(STDREDIRECT_REDIRECTOR* redirector);
void STDREDIRECT_unredirect(STDREDIRECT_REDIRECTOR* redirector);
static void WINAPI STDREDIRECT_reader(STDREDIRECT_REDIRECTOR* redirector);
static void STDREDIRECT_debuggerCallback(char c);

// redirect standard stream to character-wise callback
void STDREDIRECT_redirect(STDREDIRECT_REDIRECTOR* redirector) {
    // TODO error checking

    CreatePipe(&redirector->readablePipeEnd, &redirector->writablePipeEnd, 0, 0);
    redirector->stdHandle = GetStdHandle(redirector->stream == STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    SetStdHandle(redirector->stream == STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE, redirector->writablePipeEnd);

    int writablePipeEndFileStream = _open_osfhandle((intptr_t) redirector->writablePipeEnd, 0);
    FILE* writablePipeEndFile = NULL;
    writablePipeEndFile = _fdopen(writablePipeEndFileStream, "wt");
    _dup2(_fileno(writablePipeEndFile), redirector->stream == STDOUT ? STDOUT_FILENO : STDERR_FILENO);

    redirector->thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) STDREDIRECT_reader, redirector, 0, 0);
}

// undredirect stream
void STDREDIRECT_unredirect(STDREDIRECT_REDIRECTOR* redirector) {
    // TODO implement unredirect
    CloseHandle(redirector->thread);

    CloseHandle(redirector->writablePipeEnd);
    CloseHandle(redirector->readablePipeEnd);

    SetStdHandle(redirector->stream == STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE, redirector->stdHandle);
};


// pipe reader
static void WINAPI STDREDIRECT_reader(STDREDIRECT_REDIRECTOR* redirector) {
    while (true) {
        char c;
        DWORD read;
        fflush(NULL); // force current stdout to become readable
        // TODO add error handling
        ReadFile(redirector->readablePipeEnd, (void*) &c, 1, &read, 0); // this blocks until input is available
        if (read == 1) {
            redirector->callback(c);
        }
    }
}


// forward character as null terminated string to debugger
static void STDREDIRECT_debuggerCallback(char c) {

    char str[2] = { c, '\0' };
    OutputDebugStringA(str);
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _WIN32
