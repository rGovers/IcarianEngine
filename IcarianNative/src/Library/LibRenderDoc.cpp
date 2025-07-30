// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Rendering/LibRenderDoc.h"

#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC

static LibRenderDoc* Instance = nullptr;

#include <cstddef>

#ifdef WIN32
#include "Core/WindowsHeaders.h"
#else
#include <dlfcn.h>
#endif

#include "Core/IcarianDefer.h"
#endif

LibRenderDoc::LibRenderDoc()
{
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC
    m_module = NULL;
    m_api = NULL;
    m_shouldCapture = false;
    m_capturing = false;

#ifdef WIN32
    m_module = GetModuleHandleA("renderdoc.dll");
    if (m_module == NULL)
    {
        return;
    }

    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress((HMODULE)m_module, "RENDERDOC_GetAPI");
    if (RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&m_api) != 1)
    {
        m_module = NULL;
        m_api = NULL;
    }
#else
    m_module = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);
    if (m_module == NULL)
    {
        return;
    }

    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(m_module, "RENDERDOC_GetAPI");
    if (RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&m_api) != 1)
    {
        m_module = NULL;
        m_api = NULL;
    }
#endif
#endif
}
LibRenderDoc::~LibRenderDoc()
{
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC
    if (m_api != NULL)
    {
        m_api->RemoveHooks();
    }

    if (m_module != NULL)
    {
#ifndef WIN32
        dlclose(m_module);
#endif
}
#endif
}

void LibRenderDoc::Init()
{
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC
    if (Instance == nullptr)
    {
        Instance = new LibRenderDoc();
    }
#endif
}
void LibRenderDoc::Destroy()
{
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC
    if (Instance != nullptr)
    {
        delete Instance;
        Instance = nullptr;
    }
#endif
}

void LibRenderDoc::CaptureFrame()
{
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC
    if (Instance == nullptr)
    {
        return;
    }
    
    Instance->m_shouldCapture = true;
#endif
}

void LibRenderDoc::StartFrame()
{
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC
    if (Instance == nullptr)
    {
        return;
    }

    if (!Instance->m_shouldCapture)
    {
        return;
    }
    IDEFER(Instance->m_shouldCapture = false);

    if (Instance->m_module == NULL || Instance->m_api == NULL)
    {
        return;
    }

    Instance->m_api->StartFrameCapture(NULL, NULL);
    Instance->m_capturing = true;
#endif
}
void LibRenderDoc::EndFrame()
{
#ifdef ICARIANNATIVE_ENABLE_GRAPHICS_RENDERDOC
    if (Instance == nullptr)
    {
        return;
    }

    if (!Instance->m_capturing)
    {
        return;
    }
    IDEFER(Instance->m_capturing = false);

    if (Instance->m_module == NULL || Instance->m_api == NULL)
    {
        return;
    }

    Instance->m_api->EndFrameCapture(NULL, NULL);
#endif
}

// MIT License
// 
// Copyright (c) 2025 River Govers
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