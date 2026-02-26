// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "MessageDialog.h"

#ifdef WIN32
#include "WindowsHeaders.h"
#endif

#include "Core/IcarianLambda.h"
#include "IcarianError.h"

[[maybe_unused]] constexpr const char* DialogStrings[] =
{
    "Message",
    "Warning",
    "Error",
};

void MessageDialog(e_MessageDialogType a_type, const std::string& a_str)
{
    switch (a_type) 
    {
    case MessageDialogType_Message:
    {
        Logger::Message(a_str);

        break;
    }
    case MessageDialogType_Warning:
    {
        Logger::Warning(a_str);

        break;
    }
    case MessageDialogType_Error:
    {
        Logger::Error(a_str);

        break;
    }
    default:
    {
        IERROR("Invalid message box type");

        break;
    }
    }

#ifdef WIN32
    const UINT type = ILAMBDA(
    {
        switch (a_type)
        {
        case MessageDialogType_Message:
        {
            ILRETURN MB_OK | MB_ICONINFORMATION;
        }
        case MessageDialogType_Warning:
        {
            ILRETURN MB_OK | MB_ICONWARNING;
        }
        case MessageDialogType_Error:
        {
            ILRETURN MB_OK | MB_ICONERROR;
        }
        default:
        {
            break;
        }
        }

        IERROR("Invalid message box type");

        ILRETURN 0;
    });

    const char* dialogStr = DialogStrings[a_type];

    MessageBoxA(NULL, a_str.c_str(), dialogStr, type);
#endif
}

// MIT License
// 
// Copyright (c) 2026 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.