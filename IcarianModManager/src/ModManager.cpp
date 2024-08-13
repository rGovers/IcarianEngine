// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "ModManager.h"

#include <filesystem>
#include <imgui.h>
#include <imgui_internal.h>
#include <tinyxml2.h>

#include "Core/IcarianDefer.h"
#include "ModInfo.h"

static bool Contains(uint32_t a_val, const std::vector<uint32_t>& a_vec)
{
    for (const uint32_t iter : a_vec)
    {
        if (iter == a_val)
        {
            return true;
        }
    }

    return false;
}

ModManager::ModManager()
{
    m_selectedMod = -1;

    const std::filesystem::path p = std::filesystem::path("Mod");

    if (!std::filesystem::exists(p))
    {
        std::filesystem::create_directories(p);
    }

    for (const auto& iter : std::filesystem::directory_iterator(p, std::filesystem::directory_options::skip_permission_denied))
    {
        const std::filesystem::path path = iter.path();

        if (iter.is_directory())
        {
            ModInfo* mod = ModInfo::LoadMod(path);

            if (mod != nullptr)
            {
                m_info.emplace_back(mod);
            }
        }
    }

    const uint32_t modCount = (uint32_t)m_info.size();

    const std::filesystem::path modListPath = p / "ModList.xml";
    if (std::filesystem::exists(modListPath))
    {
        const std::string modListPathStr = modListPath.string();

        tinyxml2::XMLDocument doc;  
        if (doc.LoadFile(modListPathStr.c_str()) == tinyxml2::XML_SUCCESS)
        {
            tinyxml2::XMLElement* root = doc.FirstChildElement("ModList");
            if (root != nullptr)
            {
                for (tinyxml2::XMLElement* iter = root->FirstChildElement(); iter != nullptr; iter = iter->NextSiblingElement())
                {
                    const std::string_view name = iter->Name();

                    if (name == "Mod")
                    {
                        const std::string_view id = iter->GetText();

                        for (uint32_t i = 0; i < modCount; ++i)
                        {
                            if (m_info[i]->GetID() == id)
                            {
                                m_loaded.emplace_back(i);

                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    for (uint32_t i = 0; i < modCount; ++i)
    {
        if (!Contains(i, m_loaded))
        {
            m_unloaded.emplace_back(i);
        }
    }
}
ModManager::~ModManager()
{
    for (const ModInfo* info : m_info)
    {
        delete info;
    }
}

static bool DisableButton(const char* a_label, bool a_state)
{
    if (!a_state)
    {
        const ImGuiStyle& style = ImGui::GetStyle();

        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, style.Alpha * 0.5f);
    }
    IDEFER(
    if (!a_state)
    {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
    });

    return ImGui::Button(a_label);
}

static void Remove(uint32_t a_val, std::vector<uint32_t>* a_vec)
{
    for (auto iter = a_vec->begin(); iter != a_vec->end(); ++iter)
    {
        if (*iter == a_val)
        {
            a_vec->erase(iter);

            return;
        }
    }
}

void ModManager::Update()
{
    if (ImGui::Button("Save"))
    {
        if (!std::filesystem::exists("Mod"))
        {
            std::filesystem::create_directories("Mod");
        }

        tinyxml2::XMLDocument doc;

        tinyxml2::XMLDeclaration* dec = doc.NewDeclaration();
        doc.InsertEndChild(dec);

        tinyxml2::XMLElement* root = doc.NewElement("ModList");
        doc.InsertEndChild(root);

        for (const uint32_t iter : m_loaded)
        {
            tinyxml2::XMLElement* element = doc.NewElement("Mod");
            root->InsertEndChild(element);

            const std::string_view id = m_info[iter]->GetID();
            element->SetText(id.data());
        }

        doc.SaveFile("Mod/ModList.xml");
    }

    {
        ImGui::BeginChild("LoadedModList", ImVec2(100.0f, 0.0f));
        IDEFER(ImGui::EndChild());

        for (const uint32_t iter : m_loaded)
        {
            const std::string_view name = m_info[iter]->GetName();

            const bool selected = m_selectedMod == iter;
            if (ImGui::Selectable(name.data(), selected))
            {
                m_selectedMod = iter;
            }
        }
    }

    ImGui::SameLine();

    {
        ImGui::BeginGroup();
        IDEFER(ImGui::EndGroup());

        const bool loaded = m_selectedMod != -1 && Contains(m_selectedMod, m_loaded);
        const bool unloaded = m_selectedMod != -1 && !loaded;
        if (DisableButton("<", unloaded))
        {
            m_loaded.emplace_back(m_selectedMod);

            Remove(m_selectedMod, &m_unloaded);
        }
        if (DisableButton(">", loaded))
        {
            m_unloaded.emplace_back(m_selectedMod);

            Remove(m_selectedMod, &m_loaded);
        }

        ImGui::Separator();
        
        if (DisableButton("<<", !m_unloaded.empty()))
        {
            m_loaded.insert(m_loaded.end(), m_unloaded.begin(), m_unloaded.end());

            m_unloaded.clear();
        }
        if (DisableButton(">>", !m_loaded.empty()))
        {
            m_unloaded.insert(m_unloaded.end(), m_loaded.begin(), m_loaded.end());

            m_loaded.clear();
        }
    }

    ImGui::SameLine();

    {
        ImGui::BeginChild("ModList", ImVec2(100.0f, 0.0f));
        IDEFER(ImGui::EndChild());

        for (const uint32_t iter : m_unloaded)
        {
            const std::string_view name = m_info[iter]->GetName();

            const bool selected = m_selectedMod == iter;
            if (ImGui::Selectable(name.data(), selected))
            {
                m_selectedMod = iter;
            }
        }
    }

    ImGui::SameLine();

    {
        ImGui::BeginChild("Info");
        IDEFER(ImGui::EndChild());

        if (m_selectedMod != -1)
        {
            const ModInfo* info = m_info[m_selectedMod];

            const std::string_view name = info->GetName();
            const std::string_view description = info->GetDescription();

            ImGui::Text("%s", name.data());

            ImGui::Separator();

            ImGui::TextWrapped("%s", description.data());
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