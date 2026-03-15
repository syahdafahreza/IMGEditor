#pragma once
#include <string>
#include <vector>
#include <map>
#include "SimpleIni.h"

struct LanguageInfo {
    std::string FileName;
    std::string Name; // The display name from the INI file
};

class Language
{
private:
    static inline CSimpleIniA m_Ini;
    static inline std::string m_CurrentLangFile;
    static inline std::string m_LangDir;
    static inline std::vector<LanguageInfo> m_AvailableLanguages;
    static inline bool m_bIsEnglishTemplate = false;
    static inline bool m_bTemplateModified = false;

public:
    // Initialize the language system
    static void Init(const std::string& appPath, const std::string& langFileName);

    // Shutdown the language system, saving template if modified
    static void Shutdown();

    // Get a translated string
    static const char* Get(const char* section, const char* key, const char* defaultText);

    // Get the list of available languages
    static const std::vector<LanguageInfo>& GetAvailableLanguages();

    // Get the currently active language file name (e.g., "en.ini")
    static const std::string& GetCurrentLanguageFile();

    // Set the active language
    static void SetLanguage(const std::string& langFileName);

    // Load available languages from the lang/ directory
    static void DiscoverLanguages();
};
