#include "stdredirect.h"

#include <windows.h>

#include <stdio.h>

int main() {
	printf("This stdout string is displayed on the console.\n");

	/* enable redirection only if debugger is present/attached */
	if (IsDebuggerPresent()) {
		if (STDREDIRECT_redirectStdoutToDebugger() != STDREDIRECT_ERROR_NO_ERROR) {
			return EXIT_FAILURE;
		}
		printf("This stdout string is displayed in the debugger.\n");

		/* give the thread some time to read from the pipe */
		/* if you unredirect or exit the process too soon after a write to the stream, it may be swallowed and will never appear */
		/* may be longer on your system */
		Sleep(1);

		if (STDREDIRECT_unredirectStdout() != STDREDIRECT_ERROR_NO_ERROR) {
			return EXIT_FAILURE;
		}
		printf("This stdout string is displayed on the console again.\n");
		
		if (STDREDIRECT_redirectStdoutToDebugger() != STDREDIRECT_ERROR_NO_ERROR) {
			return EXIT_FAILURE;
		}
		printf("This stdout string is displayed in the debugger again.\n");
	}

	getchar();
	return EXIT_SUCCESS;
}

