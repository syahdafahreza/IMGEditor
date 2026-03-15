#include "pch.h"
#include "language.h"
#include <filesystem>
#include <iostream>

void Language::Init(const std::string& appPath, const std::string& langFileName)
{
    // Determine lang directory
    std::filesystem::path path(appPath);
    m_LangDir = (path.parent_path() / "lang").string();

    // Ensure lang directory exists
    if (!std::filesystem::exists(m_LangDir))
    {
        std::filesystem::create_directory(m_LangDir);
    }

    m_Ini.SetUnicode();
    SetLanguage(langFileName);
    DiscoverLanguages();
}

void Language::DiscoverLanguages()
{
    m_AvailableLanguages.clear();
    if (!std::filesystem::exists(m_LangDir)) return;

    for (const auto& entry : std::filesystem::directory_iterator(m_LangDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".ini")
        {
            CSimpleIniA tempIni;
            tempIni.SetUnicode();
            if (tempIni.LoadFile(entry.path().string().c_str()) == SI_OK)
            {
                std::string name = tempIni.GetValue("Info", "Name", entry.path().stem().string().c_str());
                m_AvailableLanguages.push_back({ entry.path().filename().string(), name });
            }
        }
    }
}

void Language::SetLanguage(const std::string& langFileName)
{
    std::string fullPath = m_LangDir + "/" + langFileName;
    m_CurrentLangFile = langFileName;

    // Save previous template if needed
    if (m_bIsEnglishTemplate && m_bTemplateModified)
    {
        m_Ini.SaveFile((m_LangDir + "/en.ini").c_str());
        m_bTemplateModified = false;
    }

    m_Ini.Reset();
    if (std::filesystem::exists(fullPath))
    {
        m_Ini.LoadFile(fullPath.c_str());
    }
    else if (langFileName == "en.ini")
    {
        m_Ini.SetValue("Info", "Name", "English");
        m_Ini.SaveFile(fullPath.c_str());
        m_bTemplateModified = false;
    }

    m_bIsEnglishTemplate = (langFileName == "en.ini");
}

void Language::Shutdown()
{
    if (m_bIsEnglishTemplate && m_bTemplateModified)
    {
        m_Ini.SaveFile((m_LangDir + "/en.ini").c_str());
        m_bTemplateModified = false;
    }
}

const char* Language::Get(const char* section, const char* key, const char* defaultText)
{
    const char* value = m_Ini.GetValue(section, key, nullptr);
    if (value)
    {
        return value;
    }

    // If key doesn't exist, we add it if it's the English template
    if (m_bIsEnglishTemplate)
    {
        m_Ini.SetValue(section, key, defaultText);
        m_bTemplateModified = true;
    }

    // Return the default text anyway
    return defaultText;
}

const std::vector<LanguageInfo>& Language::GetAvailableLanguages()
{
    return m_AvailableLanguages;
}

const std::string& Language::GetCurrentLanguageFile()
{
    return m_CurrentLangFile;
}
