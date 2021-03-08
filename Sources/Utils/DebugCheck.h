#pragma once

#include <string>
#include <sstream>
#include <windows.h>
#include <intrin.h>
#include <comdef.h>

static bool ShowDebugMessageBox(const char* message, const char* info)
{
    std::stringstream strMessage;
    strMessage << "Failed:\n" << message;
    if (info)
    {
        strMessage << "\nInfo: " << info;
    }
    int msgboxID = MessageBox(NULL, strMessage.str().c_str(), "Debug message", MB_ICONWARNING | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2);

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
    if (ShowDebugMessageBox(#condition, ""))\
    {\
        __debugbreak();\
    }\
}

static const char* GetHrMessage(HRESULT hr)
{
    _com_error err(hr);
    return err.ErrorMessage();
}

static bool DebugCheckCom(HRESULT hr, const char* condition)
{
    if (!SUCCEEDED(hr))
    {
        return ShowDebugMessageBox(condition, GetHrMessage(hr));
    }
    return false;
}

#define DCHECK_COM(condition) \
if (DebugCheckCom(condition, #condition))\
{\
    __debugbreak();\
}
