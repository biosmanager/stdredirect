/***********************************************************************************************************************
* stdredirect_example.c
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

#include "stdredirect.h"

#include <Windows.h>

#include <stdio.h>

int main() {
    printf("This stdout string is displayed on the console.\n");

    /* enable redirection only if debugger is present/attached */
    if (IsDebuggerPresent()) {
        if (STDREDIRECT_redirectAllToDebugger() != STDREDIRECT_ERROR_NO_ERROR) {
            getchar();
            return EXIT_FAILURE;
        }

        /* these two strings are being redirected to the Visual Studio output window */
        printf("This stdout string is displayed in the debugger.\n");
        fprintf(stderr, "This stderr string is displayed in the debugger.\n");

        /* this goes directly to the console window */
        STDREDIRECT_printToConsole("This string bypasses the redirection.\n");

        /* give the thread some time to read from the pipe before unredirecting
           if you unredirect or exit the process too soon after a write to the stream,
           it may be swallowed and will never appear
           may be longer on your system
         */
        Sleep(1);
        
        /* stdout/stderr are displayed on the console again */
        if (STDREDIRECT_unredirectAll() != STDREDIRECT_ERROR_NO_ERROR) {
            getchar();
            return EXIT_FAILURE;
        }
        printf("This stdout string is displayed on the console again.\n");
        
        /* duplication mode prints to the console but also redirects to the debugger */
        if (STDREDIRECT_duplicateStdoutToDebugger() != STDREDIRECT_ERROR_NO_ERROR) {
            getchar();
            return EXIT_FAILURE;
        }
        printf("This stdout string is displayed in the debugger and on the console.\n");
        
        /* this one only appears on the console because it is not redirected */
        fprintf(stderr, "This stderr string is only displayed on the console.\n");
    }

    getchar();
    return EXIT_SUCCESS;
}

