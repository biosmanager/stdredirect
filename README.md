Visual Studio lacks the ability to show output from C/C++ applications in the integrated "Debug" window.
This small header-only library allows redirecting stdout/stderr to a custom callback function.
The callback could call [OutputDebugString()](https://msdn.microsoft.com/en-us/library/windows/desktop/aa363362(v=vs.85).aspx)
to send it to any attached debugger (e.g. Visual Studio).

See stdredirect_example.c or stdredirect_example.cpp for an example.
