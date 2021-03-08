#pragma once

#include <windows.h>
#include <intrin.h>

static bool ShowDebugMessageBox(const char* message)
{
    int msgboxID = MessageBox(NULL, (LPCSTR)message, (LPCSTR)"Debug message", MB_ICONWARNING | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2);

    switch (msgboxID)
    {
    case IDABORT:
        exit(1);
        break;
    case IDRETRY:
        return true;
        break;
    case IDIGNORE:
        break;
    }
    return false;
}

#define DCHECK(condition) \
if (!(condition))\
{\
    if (ShowDebugMessageBox("Failed: \n" ## #condition))\
    {\
        __debugbreak();\
    }\
}

#define DCHECK_COM(hr) DCHECK(SUCCEEDED(hr))
