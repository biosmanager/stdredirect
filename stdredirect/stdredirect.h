/***********************************************************************************************************************
* stdredirect.h																			 
*																						 
* Redirect stdout/stderr to callback (e.g. attached debugger).							 
* https://github.com/biosmanager/stdredirect											 
*  																						 
*																						 
* MIT License																			 
* 																						 
* Copyright (c) 2018 Matthias Albrecht													 
* 																						 
* Permission is hereby granted, free of charge, to any person obtaining a copy			 
* of this software and associated documentation files (the "Software"), to deal			 
* in the Software without restriction, including without limitation the rights			 
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell				 
* copies of the Software, and to permit persons to whom the Software is					 
* furnished to do so, subject to the following conditions:								 
* 																						 
* The above copyright notice and this permission notice shall be included in all		 
* copies or substantial portions of the Software.										 
* 																						 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR			 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,				 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE			 
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER				 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,			 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE			 
* SOFTWARE.																				 
*																						 
***********************************************************************************************************************/

/**
 * @file stdredirect.h
 * @author Matthias Albrecht
 * @brief Redirect stdout/stderr to callback (e.g. attached debugger).	
 */

#ifndef STDREDIRECT_H
#define STDREDIRECT_H

#ifdef _WIN32

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <Windows.h>

#include <conio.h>
#include <io.h>
#include <stdio.h>


/** @brief Default buffered pipe reader buffer size. */
const size_t STDREDIRECT_BUFFER_SIZE = 81;


/** @brief Thread exit timeout in ms. */
const DWORD STDREDIRECT_THREAD_EXIT_TIMEOUT_MS = 5;


/** @brief Error types. */
typedef enum STDREDIRECT_ERROR {
    STDREDIRECT_ERROR_NO_ERROR,         /**< no error                             */
    STDREDIRECT_ERROR_REDIRECT,         /**< redirect failed                      */
    STDREDIRECT_ERROR_UNREDIRECT,       /**< unredirect failed                    */
    STDREDIRECT_ERROR_THREAD,           /**< error in pipe reader thread          */
    STDREDIRECT_ERROR_CREATE,           /**< allocating redirection object failed */
    STDREDIRECT_ERROR_NULLPTR           /**< null-pointer error                   */
                                             
} STDREDIRECT_ERROR;                         
       

/** @brief Standard output/error streams. */ 
typedef enum STDREDIRECT_STREAM {            
    STDREDIRECT_STREAM_STDOUT,          /**< stdout */
    STDREDIRECT_STREAM_STDERR           /**< stderr */
                                             
} STDREDIRECT_STREAM;                        
    

/** @brief Redirection behaviour */          
typedef enum STDREDIRECT_BEHAVIOUR {         
    STDREDIRECT_BEHAVIOUR_REDIRECT,     /**< redirect stdout/stderr to callback, output is not sent to attached console  */
    STDREDIRECT_BEHAVIOUR_DUPLICATE     /**< redirect stdout/stderr to callback and also send output to attached console */
} STDREDIRECT_BEHAVIOUR;


/** @brief Function pointer to callback function. */
typedef void (*STDREDIRECT_CALLBACK)(const char* str);


/** @brief Redirection object for a given stream. 
 * 
 *  Use STDREDIRECT_create() to create one. 
 */
typedef struct STDREDIRECT_REDIRECTION {             
    /** @name State 
     *  Use the these variables to check the state of the redirection. Read only!
     */
    /*@{*/
    BOOL                  isRedirected;     /**< [read] redirection is redirected   */
    BOOL                  isValid;          /**< [read] redirection is valid        */
    STDREDIRECT_ERROR     error;            /**< [read] redirection error           */
    /*@}*/                 

    /** @name Internal
     *  DO NOT CHANGE THESE VARIABLES AT RUNTIME!
     */
    /*@{*/
    STDREDIRECT_STREAM    stream;                               /**< redirected standard stream                          */
    STDREDIRECT_CALLBACK  callback;                             /**< output callback                                     */
    STDREDIRECT_BEHAVIOUR behaviour  ;                          /**< redirection behaviour                               */
    HANDLE                stdHandle;                            /**< console standard device handle                      */
    HANDLE                readablePipeEnd;                      /**< readable pipe end                                   */
    HANDLE                writablePipeEnd;                      /**< writable pipe end                                   */
    int                   writablePipeEndFileDescriptor;        /**< C-runtime file descriptor for writable pipe end     */
    HANDLE                thread;                               /**< pipe reader thread                                  */
    HANDLE                exitThreadEvent;                      /**< event to signal thread it should exit               */
    char*                 buffer;                               /**< pipe reader buffer                                  */
    size_t                bufferSize;                           /**< pipe reader buffer size                             */
    /*@}*/

} STDREDIRECT_REDIRECTION;


/** @brief Default stdout redirection object. */
static STDREDIRECT_REDIRECTION* STDREDIRECT_stdoutRedirection;       
/** @brief Default stderr redirection object. */
static STDREDIRECT_REDIRECTION* STDREDIRECT_stderrRedirection;


/* forward declarations */

static STDREDIRECT_REDIRECTION* STDREDIRECT_create(STDREDIRECT_STREAM stream, STDREDIRECT_CALLBACK callback, STDREDIRECT_BEHAVIOUR redirectionBehaviour);
static STDREDIRECT_ERROR        STDREDIRECT_destroy(STDREDIRECT_REDIRECTION* redirection);
static STDREDIRECT_ERROR        STDREDIRECT_redirect(STDREDIRECT_REDIRECTION* redirection); 
static STDREDIRECT_ERROR        STDREDIRECT_redirectStdout(STDREDIRECT_CALLBACK stdoutCallback, STDREDIRECT_BEHAVIOUR redirectionBehaviour);
static STDREDIRECT_ERROR        STDREDIRECT_redirectStderr(STDREDIRECT_CALLBACK stderrCallback, STDREDIRECT_BEHAVIOUR redirectionBehaviour);
static STDREDIRECT_ERROR        STDREDIRECT_redirectStdoutToDebugger();
static STDREDIRECT_ERROR        STDREDIRECT_redirectStderrToDebugger();
static STDREDIRECT_ERROR        STDREDIRECT_redirectAllToDebugger();
static STDREDIRECT_ERROR        STDREDIRECT_duplicateStdoutToDebugger();
static STDREDIRECT_ERROR        STDREDIRECT_duplicateStderrToDebugger();
static STDREDIRECT_ERROR        STDREDIRECT_unredirect(STDREDIRECT_REDIRECTION* redirection);
static STDREDIRECT_ERROR        STDREDIRECT_unredirectStdout();
static STDREDIRECT_ERROR        STDREDIRECT_unredirectStderr();
static STDREDIRECT_ERROR        STDREDIRECT_unredirectAll();
static void WINAPI              STDREDIRECT_bufferedPipeReader(STDREDIRECT_REDIRECTION* redirection);
static void                     STDREDIRECT_debuggerCallback(const char* str);
static int                      STDREDIRECT_printToConsole(const char* format, ...);


/**
 * @brief Allocate redirection object.
 *
 * @param stream Stream to redirect.
 * @param callback Pointer to callback function.
 * @return Pointer to allocated redirection object, NULL on error.
 */
static STDREDIRECT_REDIRECTION* STDREDIRECT_create(STDREDIRECT_STREAM stream, STDREDIRECT_CALLBACK callback, STDREDIRECT_BEHAVIOUR redirectionBehaviour) {
    STDREDIRECT_REDIRECTION* redirection = (STDREDIRECT_REDIRECTION*) malloc(sizeof(STDREDIRECT_REDIRECTION));
    if (!redirection) {
        return NULL;
    }

    redirection->stream                            = stream;
    redirection->callback                          = callback;
    redirection->behaviour                         = redirectionBehaviour;
                                                   
    redirection->isRedirected                      = FALSE;
    redirection->isValid                           = FALSE;
    redirection->error                             = STDREDIRECT_ERROR_NO_ERROR;
                                                   
    redirection->stdHandle                         = NULL;
    redirection->readablePipeEnd                   = NULL;
    redirection->writablePipeEnd                   = NULL;
    redirection->writablePipeEndFileDescriptor     = -1;
    redirection->thread                            = NULL;
    redirection->exitThreadEvent                   = NULL;
    redirection->buffer                            = NULL;
    redirection->bufferSize                        = STDREDIRECT_BUFFER_SIZE;

    return redirection;
}


/**
 * @brief Destroy redirection object.
 *
 * @param redirection Pointer to redirection object.
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_destroy(STDREDIRECT_REDIRECTION* redirection) {
    if (redirection) {
        STDREDIRECT_ERROR unredirectError = STDREDIRECT_unredirect(redirection);
        free(redirection);
        redirection = NULL;

        return unredirectError;
    }

    return STDREDIRECT_ERROR_NULLPTR;
}


/** 
 * @brief Redirect standard stream to callback.
 * 
 * Callback is called when pipe buffer is full, see STDREDIRECT_REDIRECTION::bufferSize (defaults to STDREDIRECT_BUFFER_SIZE) 
 *
 * @param redirection Pointer to redirection object.
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_redirect(STDREDIRECT_REDIRECTION* redirection) {
    /* unredirect if already redirected */
    if (redirection->isRedirected && !STDREDIRECT_unredirect(redirection)) {
        goto Error;
    }

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

    /* create exit thread event */
    redirection->exitThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (redirection->exitThreadEvent == NULL) {
        goto Error;
    }      

    /* run pipe reader in separate thread */
    redirection->thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) STDREDIRECT_bufferedPipeReader, redirection, 0, 0);
    if (redirection->thread == NULL) {
        goto Error;
    }

    redirection->isRedirected = TRUE;
    redirection->isValid = TRUE;

    return redirection->error = STDREDIRECT_ERROR_NO_ERROR;

Error:
    /* cleanup */
    STDREDIRECT_unredirect(redirection);

    return redirection->error = STDREDIRECT_ERROR_REDIRECT;
}


/**
 * @brief Redirect stdout to callback.
 *
 * Callback is called when pipe buffer is full, see STDREDIRECT_REDIRECTION::bufferSize (defaults to STDREDIRECT_BUFFER_SIZE)
 *
 * @param stdoutCallback Pointer to callback function.
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_redirectStdout(STDREDIRECT_CALLBACK stdoutCallback, STDREDIRECT_BEHAVIOUR redirectionBehaviour) {
    STDREDIRECT_stdoutRedirection = STDREDIRECT_create(STDREDIRECT_STREAM_STDOUT, stdoutCallback, redirectionBehaviour);
    if (STDREDIRECT_stdoutRedirection == NULL) {
        return STDREDIRECT_ERROR_CREATE;
    }

    return STDREDIRECT_redirect(STDREDIRECT_stdoutRedirection);
}


/**
 * @brief Redirect stderr to callback.
 *
 * Callback is called when pipe buffer is full, see STDREDIRECT_REDIRECTION::bufferSize (defaults to STDREDIRECT_BUFFER_SIZE)
 *
 * @param stderrCallback Pointer to callback function.
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_redirectStderr(STDREDIRECT_CALLBACK stderrCallback, STDREDIRECT_BEHAVIOUR redirectionBehaviour) {
    STDREDIRECT_stderrRedirection = STDREDIRECT_create(STDREDIRECT_STREAM_STDERR, stderrCallback, redirectionBehaviour); 
    if (STDREDIRECT_stderrRedirection == NULL) {
        return STDREDIRECT_ERROR_CREATE;
    }

    return STDREDIRECT_redirect(STDREDIRECT_stderrRedirection);
}         


/**
 * @brief Redirect stdout to debugger.
 *
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_redirectStdoutToDebugger() {
    return STDREDIRECT_redirectStdout(&STDREDIRECT_debuggerCallback, STDREDIRECT_BEHAVIOUR_REDIRECT);
}


/**
 * @brief Redirect stderr to debugger.
 *
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_redirectStderrToDebugger() {
    return STDREDIRECT_redirectStderr(&STDREDIRECT_debuggerCallback, STDREDIRECT_BEHAVIOUR_REDIRECT);
}


/**
 * @brief Redirect both stdout and stderr to debugger.
 *
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_redirectAllToDebugger() {

    STDREDIRECT_ERROR stdoutError = STDREDIRECT_redirectStdoutToDebugger();
    STDREDIRECT_ERROR stderrError = STDREDIRECT_redirectStderrToDebugger();

    /* TODO improve error handling */
    if (stdoutError != STDREDIRECT_ERROR_NO_ERROR) {
        return stdoutError;
    }
    if (stderrError != STDREDIRECT_ERROR_NO_ERROR) {
        return stderrError;
    }

    return STDREDIRECT_ERROR_NO_ERROR;
}


/**
 * @brief Send output that is written to stdout to debugger and console (duplicate).
 *
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_duplicateStdoutToDebugger() {
    return STDREDIRECT_redirectStdout(&STDREDIRECT_debuggerCallback, STDREDIRECT_BEHAVIOUR_DUPLICATE);
}


/**
 * @brief Send output that is written to stderr to debugger and console (duplicate).
 *
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_duplicateStderrToDebugger() {
    return STDREDIRECT_redirectStderr(&STDREDIRECT_debuggerCallback, STDREDIRECT_BEHAVIOUR_DUPLICATE);
}


/**
 * @brief Unredirect redirection back to console.
 *
 * @param redirection Pointer to redirection object.
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_unredirect(STDREDIRECT_REDIRECTION* redirection) {
    FILE* consoleFile;

    /* check if still redirected */
    if (!redirection->isRedirected) {
        return STDREDIRECT_ERROR_NO_ERROR;
    }

    /* stop pipe reader thread */
    if (redirection->thread) {
        /* signal thread to exit */
        if (!SetEvent(redirection->exitThreadEvent)) {
            goto Error;
        }

        /* give thread time to exit, terminate otherwise */
        if (WaitForSingleObject(redirection->thread, STDREDIRECT_THREAD_EXIT_TIMEOUT_MS) != WAIT_OBJECT_0 && !TerminateThread(redirection->thread, EXIT_SUCCESS)) {
            goto Error;
        }

        /* close thread handle*/
        if (!CloseHandle(redirection->thread)) {
            goto Error;
        }
        redirection->thread = NULL;
    }

    /* close exit thread event handle */
    if (redirection->exitThreadEvent && !CloseHandle(redirection->exitThreadEvent)) {
        goto Error;
    }
    redirection->exitThreadEvent = NULL;      

    /* free read buffer */
    if (redirection->buffer) {
        free(redirection->buffer);
        redirection->buffer = NULL;
        redirection->bufferSize = 0;
    }

    /* restore std handle */
    if (redirection->stdHandle && !SetStdHandle(redirection->stream == STDREDIRECT_STREAM_STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE, redirection->stdHandle)) {
        goto Error;
    }
    redirection->stdHandle = NULL;

    /* close writable pipe end file descriptor, underlying handle is closed automatically */
    if (redirection->writablePipeEndFileDescriptor != -1 && _close(redirection->writablePipeEndFileDescriptor) == -1) {
        goto Error;
    }
    redirection->writablePipeEnd = NULL;
    redirection->writablePipeEndFileDescriptor = -1;

    /* close readable pipe end */
    if (redirection->readablePipeEnd && !CloseHandle(redirection->readablePipeEnd)) {
        goto Error;
    }
    redirection->readablePipeEnd = NULL;

    /* re-open console */
    if (freopen_s(&consoleFile, "CONOUT$", "w", redirection->stream == STDREDIRECT_STREAM_STDOUT ? stdout : stderr) != 0) {
        goto Error;
    }

    redirection->isRedirected = FALSE;
    redirection->isValid = TRUE;
    
    return redirection->error = STDREDIRECT_ERROR_NO_ERROR;

Error:
    /* critical error: unredirect failed */
    _cprintf_s("STDREDIRECT CRITICAL ERROR: Could not un-redirect %s! Redirection is in invalid state.\n", redirection->stream == STDREDIRECT_STREAM_STDOUT ? "stdout" : "stderr");

    redirection->isValid = FALSE;
    
    return redirection->error = STDREDIRECT_ERROR_UNREDIRECT;
}


/**
 * @brief Unredirect stdout back to console.
 *
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_unredirectStdout() {
    return STDREDIRECT_destroy(STDREDIRECT_stdoutRedirection);
}


/**
 * @brief Unredirect stderr back to console.
 *
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_unredirectStderr() {
    return STDREDIRECT_destroy(STDREDIRECT_stderrRedirection);
}


/**
 * @brief Unredirect both stdout and stderr back to console.
 *
 * @return ::STDREDIRECT_ERROR
 */
static STDREDIRECT_ERROR STDREDIRECT_unredirectAll() {
    STDREDIRECT_ERROR stdoutError = STDREDIRECT_unredirectStdout();
    STDREDIRECT_ERROR stderrError = STDREDIRECT_unredirectStderr();

    if (stdoutError != STDREDIRECT_ERROR_NO_ERROR) {
        return stdoutError;
    }
    if (stderrError != STDREDIRECT_ERROR_NO_ERROR) {
        return stderrError;
    }

    return STDREDIRECT_ERROR_NO_ERROR;
}


/**
 * @brief Buffered pipe reader, runs in separate thread.
 *
 * @param redirection Pointer to redirection object.
 */
static void WINAPI STDREDIRECT_bufferedPipeReader(STDREDIRECT_REDIRECTION* redirection) {
    DWORD numBytesRead;

    /* allocate string buffer */
    redirection->buffer = (char*) calloc(STDREDIRECT_BUFFER_SIZE, sizeof(char));
    if (redirection->buffer) {
        redirection->bufferSize = STDREDIRECT_BUFFER_SIZE;
    }
    else {
        goto Error;
    }

    /* read from pipe until exit thread event signal is received */
    while (WaitForSingleObject(redirection->exitThreadEvent, 0) != WAIT_OBJECT_0) {
        /* flush all streams so they become readable */
        if (fflush(NULL) == EOF) {
            goto Error;
        }

        /* TODO improve timing, sometimes characters are dropped/intercepted by another string */

        /* read from readable pipe end, blocks until input is available */
        /* read 1 character less to leave space for string-terminating null-character */
        if (!ReadFile(redirection->readablePipeEnd, (void*) redirection->buffer, (DWORD) redirection->bufferSize - 1, &numBytesRead, NULL)) {
            goto Error;
        }
        if (numBytesRead > 0) {
            /* ensure string is null-terminated */
            redirection->buffer[redirection->bufferSize - 1] = '\0';

            /* duplicate output to console */
            if (redirection->behaviour == STDREDIRECT_BEHAVIOUR_DUPLICATE) {
                _cprintf_s(redirection->buffer);
            }

            /* run string callback */
            redirection->callback(redirection->buffer);
        }

        /* clear buffer */
        memset(redirection->buffer, 0, redirection->bufferSize);
    }

    ExitThread(EXIT_SUCCESS);

Error:
    /* cleanup */

    /* close handle and set thread handle to null before unredirect so it doesn't try to terminate this thread */
    CloseHandle(redirection->thread);
    redirection->thread = NULL;
    STDREDIRECT_unredirect(redirection);

    redirection->error = STDREDIRECT_ERROR_THREAD;

    ExitThread(EXIT_FAILURE);
}


/**
 * @brief Default debugger callback.
 *
 * Forwards string to Windows function OutputDebugString().
 *
 * @param str String.
 */
static void STDREDIRECT_debuggerCallback(const char* str) {
    OutputDebugString(str);
}


/**
 * @brief Print directly to console, bypassing redirection.
 *
 * Just calls _cprintf_s().
 *
 * @param format Formatted output string, see _cprintf_s().
 * @return See _cprintf_s().
 */
static int STDREDIRECT_printToConsole(const char* format, ...) {
    int result;
    
    va_list argptr;
    va_start(argptr, format);
    result = _vcprintf_s(format, argptr);
    va_end(argptr);

    return result;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIN32 */

#endif /* STDREDIRECT_H */
