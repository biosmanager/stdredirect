#ifdef _WIN32

#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <io.h>
#include <stdio.h>
#include <windows.h>


/* default stream file descriptors */
#define STDREDIRECT_STDOUT_FILENO 1
#define STDREDIRECT_STDERR_FILENO 2

/* standard output/error stream */
typedef enum STDREDIRECT_STREAM {
    STDREDIRECT_STREAM_STDOUT,
    STDREDIRECT_STREAM_STDERR
} STDREDIRECT_STREAM;

// function pointer to character-wise callback function
typedef void (*STDREDIRECT_CALLBACK)(char c);

/* redirection object for a given stream */
typedef struct STDREDIRECT_REDIRECTION {
    STDREDIRECT_STREAM   stream;
    STDREDIRECT_CALLBACK callback;

    /* DO NOT CHANGE */
    HANDLE stdHandle;
    HANDLE readablePipeEnd;
    HANDLE writablePipeEnd; 
    int writablePipeEndFileDescriptor;
    HANDLE thread;
} STDREDIRECT_REDIRECTION;


/* redirection objects */
static STDREDIRECT_REDIRECTION STDREDIRECT_stdoutRedirection;       
static STDREDIRECT_REDIRECTION STDREDIRECT_stderrRedirection;


/* forward declarations */
static BOOL STDREDIRECT_redirect(STDREDIRECT_REDIRECTION* redirection); 
static BOOL STDREDIRECT_redirectStdout(STDREDIRECT_CALLBACK stdoutCallback);
static BOOL STDREDIRECT_redirectStderr(STDREDIRECT_CALLBACK stderrCallback);
static BOOL STDREDIRECT_redirectStdoutToDebugger();
static BOOL STDREDIRECT_redirectStderrToDebugger();
static BOOL STDREDIRECT_unredirect(STDREDIRECT_REDIRECTION* redirection);
static BOOL STDREDIRECT_unredirectStdout();
static BOOL STDREDIRECT_unredirectStderr();
static void WINAPI STDREDIRECT_pipeReader(STDREDIRECT_REDIRECTION* redirection);
static void STDREDIRECT_debuggerCallback(char c);


/* redirect standard stream to character-wise callback */
static BOOL STDREDIRECT_redirect(STDREDIRECT_REDIRECTION* redirection) {
    /* create anonymous pipe */
    if (!CreatePipe(&redirection->readablePipeEnd, &redirection->writablePipeEnd, 0, 0)) {
        goto Error;
    }

    /* get current handle to selected stream */
    redirection->stdHandle = GetStdHandle(redirection->stream == STDREDIRECT_STREAM_STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    if (redirection->stdHandle == NULL) {
        goto Error;
    }
    /* set stream handle to writable pipe end */
    if (!SetStdHandle(redirection->stream == STDREDIRECT_STREAM_STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE, redirection->writablePipeEnd)) {
        goto Error;
    }

    /* assign OS handle to C-runtime file descriptor */
    redirection->writablePipeEndFileDescriptor = _open_osfhandle((intptr_t) redirection->writablePipeEnd, 0);
    if (redirection->writablePipeEndFileDescriptor == -1) {
        goto Error;
    }
    /* reassign standard stream file descriptor to writable pipe end */
    if (_dup2(redirection->writablePipeEndFileDescriptor, redirection->stream == STDREDIRECT_STREAM_STDOUT ? _fileno(stdout) : _fileno(stderr)) == -1) {
        goto Error;
    }

    /* run pipe reader in separate thread */
    redirection->thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) STDREDIRECT_pipeReader, redirection, 0, 0);
    if (redirection->thread == NULL) {
        goto Error;
    }

    return TRUE;

Error:
    /* TODO add cleanup */

    return FALSE;
}


/* redirect stdout to callback */
static BOOL STDREDIRECT_redirectStdout(STDREDIRECT_CALLBACK stdoutCallback) {
    STDREDIRECT_stdoutRedirection.stream = STDREDIRECT_STREAM_STDOUT;
    STDREDIRECT_stdoutRedirection.callback = stdoutCallback;

    return STDREDIRECT_redirect(&STDREDIRECT_stdoutRedirection);
}


/* redirect stderr to callback */
static BOOL STDREDIRECT_redirectStderr(STDREDIRECT_CALLBACK stderrCallback) {
    STDREDIRECT_stderrRedirection.stream = STDREDIRECT_STREAM_STDERR;
    STDREDIRECT_stderrRedirection.callback = stderrCallback;

    return STDREDIRECT_redirect(&STDREDIRECT_stderrRedirection);
}         


/* redirect stdout to debugger */                                                               
static BOOL STDREDIRECT_redirectStdoutToDebugger() {
    return STDREDIRECT_redirectStdout(&STDREDIRECT_debuggerCallback);
}


/* redirect stderr to debugger */
static BOOL STDREDIRECT_redirectStderrToDebugger() {
    return STDREDIRECT_redirectStderr(&STDREDIRECT_debuggerCallback);
}


/* undredirect stream */
static BOOL STDREDIRECT_unredirect(STDREDIRECT_REDIRECTION* redirection) {
    FILE* consoleFile;

    /* terminate pipe reader thread */
    if (!TerminateThread(redirection->thread, 0)) {
        goto Error;
    }

    /* re-open console */
    if (freopen_s(&consoleFile, "CONOUT$", "w", redirection->stream == STDREDIRECT_STREAM_STDOUT ? stdout : stderr) != 0) {
        goto Error;
    }

    /* reassign writable pipe end to standard stream file descriptor */
    if (_dup2(redirection->stream == STDREDIRECT_STREAM_STDOUT ? _fileno(stdout) : _fileno(stderr), redirection->writablePipeEndFileDescriptor) == -1) {
        goto Error;
    }
    /* restore std handle */
    if (!SetStdHandle(redirection->stream == STDREDIRECT_STREAM_STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE, redirection->stdHandle)) {
        goto Error;
    }

    /* close writable pipe end file descriptor */
    if (_close(redirection->writablePipeEndFileDescriptor) == -1) {
        goto Error;
    }
    /* close readable pipe end */
    if (!CloseHandle(redirection->readablePipeEnd)) {
        goto Error;
    }

    return TRUE;

Error:
    /* TODO add cleanup */

    return FALSE;
}


/* revert stdout redirection */
static BOOL STDREDIRECT_unredirectStdout() {
    return STDREDIRECT_unredirect(&STDREDIRECT_stdoutRedirection);
}


/* revert stderr redirection */
static BOOL STDREDIRECT_unredirectStderr() {
    return STDREDIRECT_unredirect(&STDREDIRECT_stderrRedirection);
}


/* pipe reader */
static void WINAPI STDREDIRECT_pipeReader(STDREDIRECT_REDIRECTION* redirection) {
    while (true) {
        char c;
        DWORD numBytesRead = 0;

        /* flush all streams so they become readable */
        if (fflush(NULL) == EOF) {
            /* TODO add error handling */
        }

        /* read from readable pipe end, blocks until input is available */
        if (!ReadFile(redirection->readablePipeEnd, (void*) &c, 1, &numBytesRead, 0)) {
            /* TODO add error handling */
        }
        if (numBytesRead > 0) {
            /* run callback */
            redirection->callback(c);
        }
    }
}


/* forward character as null-terminated string to debugger */
static void STDREDIRECT_debuggerCallback(char c) { 
    char str[2] = { c, '\0' };
    OutputDebugStringA(str);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIN32 */
