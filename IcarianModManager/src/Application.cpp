// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Application.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <sstream>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"

static void GLFWErrorCallback(int a_error, const char* a_description)
{
    printf("%s \n", a_description);
}

static void GLAPIENTRY GLMessageCallback
( 
    GLenum a_source,
    GLenum a_type,
    GLuint a_id,
    GLenum a_severity,
    GLsizei a_length,
    const GLchar* a_message,
    const void* a_userParam 
)
{
    std::stringstream stream;

    stream << "GL CALLBACK: ";

    switch (a_severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    {
        stream << "HIGH SEVERITY ";

        break;
    }
    case GL_DEBUG_SEVERITY_MEDIUM:
    {
        stream << "MEDIUM SEVERITY ";

        break;
    }
    case GL_DEBUG_SEVERITY_LOW:
    {
        stream << "LOW SEVERITY ";

        break;
    }
    default:
    {
        stream << std::hex << a_severity << " ";

        break;
    }
    }

    stream << a_message;

    switch (a_type)
    {
    case GL_DEBUG_TYPE_ERROR:
    case GL_DEBUG_TYPE_PERFORMANCE:
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    {
        const std::string s = stream.str();

        printf("%s \n", s.c_str());

        break;
    }
    default:
    {
        break;
    }
    }
}

static void SetImguiStyle()
{
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();

    style.FrameRounding = 2.0f;
    style.WindowRounding = 2.0f;
    style.ChildRounding = 2.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.FramePadding = ImVec2(6.0f, 3.0f);
    style.WindowMenuButtonPosition = ImGuiDir_Right;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.90f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.78f, 0.53f, 0.17f, 0.69f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.86f, 0.42f, 0.18f, 0.88f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.78f, 0.53f, 0.17f, 0.69f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.86f, 0.42f, 0.18f, 0.88f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.78f, 0.53f, 0.17f, 0.59f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.86f, 0.42f, 0.18f, 0.78f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.12f, 0.12f, 0.14f, 0.78f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.86f, 0.42f, 0.18f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.78f, 0.53f, 0.17f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.86f, 0.42f, 0.18f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.82f, 0.33f, 0.18f, 0.94f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.78f, 0.53f, 0.17f, 0.86f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.86f, 0.42f, 0.18f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.82f, 0.33f, 0.18f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.78f, 0.53f, 0.17f, 0.39f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.82f, 0.33f, 0.18f, 0.39f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.78f, 0.53f, 0.17f, 0.59f);
}

Application::Application()
{
    ICARIAN_ASSERT_R(glfwInit());

    glfwSetErrorCallback(GLFWErrorCallback);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_window = glfwCreateWindow(640, 480, "Icarian Mod Manager", NULL, NULL);

    glfwMakeContextCurrent(m_window);

    ICARIAN_ASSERT_R(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(GLMessageCallback, 0);
#endif

    ICARIAN_ASSERT_R(ImGui_ImplGlfw_InitForOpenGL(m_window, true));
    ICARIAN_ASSERT_R(ImGui_ImplOpenGL3_Init("#version 130"));

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = NULL;
    io.LogFilename = NULL;

    SetImguiStyle();
}
Application::~Application()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        IDEFER(glfwSwapBuffers(m_window));

        glfwMakeContextCurrent(m_window);

        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(640, 480));
        if (ImGui::Begin("##WIN", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
        {
            m_modManager.Update();
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
}

// MIT License
// 
// Copyright (c) 2024 River Govers
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