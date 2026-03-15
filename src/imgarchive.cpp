#include "pch.h"
#include "imgarchive.h"
#include <filesystem>
#include "editor.h"
#include "parser/pc_v1.h"
#include "parser/pc_v2.h"
#include "parser/unknown.h"

IMGArchive::IMGArchive(std::wstring Path, bool CreateNew)
{
    if (CreateNew)
    {
        this->FileName = Path;
        this->bCreateNew = true;
        wchar_t logBuf[256];
        Utils::ConvertUtf8ToWide(Language::Get("Logs", "CreatedArchive", "Created archive"), logBuf, sizeof(logBuf));
        AddLogMessage(logBuf);
        Parser = ParserPCv1::Get();
    }
    else
    {
        this->Path = Path;
        SetVersion(GetVersion(Path));
    }

    if (Parser)
    {
        Parser->Open(this);
        UpdateSelectList(L"");
    }
} 

void IMGArchive::UpdateSelectList(const wchar_t *text)
{
    SelectedList.clear();
    
    std::wstring lowerText(text);
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(),
                   [](wchar_t c) { return towlower(c); });

    for (EntryInfo &e : EntryList)
    {
        std::wstring lowerFileName(e.FileName);
        std::transform(lowerFileName.begin(), lowerFileName.end(), lowerFileName.begin(),
                       [](wchar_t c) { return towlower(c); });

        if (lowerFileName.find(lowerText) != std::wstring::npos)
        {
            SelectedList.push_back(&e);
        }
    }
}


std::wstring IMGArchive::GetFileType(const wchar_t* name)
{
    if (wcsstr(name, L".dff")) return L"Model";
    if (wcsstr(name, L".txd")) return L"Texture";
    if (wcsstr(name, L".col")) return L"Collision";
    if (wcsstr(name, L".ifp")) return L"Animation";
    if (wcsstr(name, L".ipl")) return L"Placement";
    if (wcsstr(name, L".ide")) return L"Defination";
    if (wcsstr(name, L".dat")) return L"Data";

    return std::filesystem::path(name).extension().wstring() + L" file";
}

void IMGArchive::ExportEntry(EntryInfo *pEntry, std::wstring filePath, bool log)
{
    if (pEntry->bImported) {
        std::filesystem::copy_file(pEntry->Path, filePath, std::filesystem::copy_options::overwrite_existing);
    }
    else
    {
        if (Parser)
        {
            Parser->Export(this, pEntry, filePath, log);
        }
    }
}

void IMGArchive::ExportAll(ArchiveInfo *pInfo)
{
    pInfo->pArc->ProgressBar.bInUse = true;
    size_t total =  pInfo->pArc->EntryList.size();
    
    for (size_t i = 0; i < total; ++i)
    {
        std::wstring path = std::format(L"{}\\{}", pInfo->path, pInfo->pArc->EntryList[i].FileName);
        pInfo->pArc->ExportEntry(&pInfo->pArc->EntryList[i], path, false);
        pInfo->pArc->ProgressBar.Percentage = (static_cast<float>(i)+1)/ static_cast<float>(total);

        if (pInfo->pArc->ProgressBar.bCancel)
        {
            pInfo->pArc->ProgressBar.bCancel = false;
            break;
        }
    }
    wchar_t logBuf[256];
    Utils::ConvertUtf8ToWide(Language::Get("Logs", "ExportedArchive", "Exported archive"), logBuf, sizeof(logBuf));
    pInfo->pArc->AddLogMessage(logBuf);
    pInfo->pArc->ProgressBar.bInUse = false;
    delete pInfo;
}

void IMGArchive::ExportSelected(ArchiveInfo *pInfo)
{
    pInfo->pArc->ProgressBar.bInUse = true;
    size_t total =  pInfo->pArc->EntryList.size();
    
    for (size_t i = 0; i < total; ++i)
    {
        if (pInfo->pArc->EntryList[i].bSelected)
        {
            std::wstring path = std::format(L"{}\\{}", pInfo->path, pInfo->pArc->EntryList[i].FileName);
            pInfo->pArc->ExportEntry(&pInfo->pArc->EntryList[i], path, false);
            pInfo->pArc->ProgressBar.Percentage = (static_cast<float>(i)+1)/ static_cast<float>(total);

            if (pInfo->pArc->ProgressBar.bCancel)
            {
                pInfo->pArc->ProgressBar.bCancel = false;
                break;
            }
        }
    }
    wchar_t logBuf[256];
    Utils::ConvertUtf8ToWide(Language::Get("Logs", "ExportedEntries", "Exported entries"), logBuf, sizeof(logBuf));
    pInfo->pArc->AddLogMessage(logBuf);
    pInfo->pArc->ProgressBar.bInUse = false;
    delete pInfo;
}

void IMGArchive::ImportEntry(const std::wstring &path, bool replace)
{
    if (Parser)
    {
        Parser->Import(this, path, replace);
    }
}

void IMGArchive::ImportEntries(ArchiveInfo *pInfo)
{
    std::vector<std::wstring> list;
    std::wstring temp = L"";
    std::wstring rootDir = L"";
    for (size_t i = 0; i < pInfo->path.size(); ++i)
    {
        wchar_t c = pInfo->path[i];
        if (c != L'\0')
        {
            temp += c;
        }
        else
        {
            if (temp.empty())
            {
                break;
            }

            if (rootDir.empty() && std::filesystem::is_directory(temp)) // first item might be directory
            {
                rootDir = temp + L"\\";
            }
            else
            {
                if (rootDir.empty())
                {
                    // Case where only a single file is selected
                    list.push_back(temp);
                }
                else
                {
                    // Case where multiple files are selected within rootDir
                    list.push_back(rootDir + temp);
                }
            }
            temp.clear();

            // Double null terminator indicates the end of the list
            if (i + 1 < pInfo->path.size() && pInfo->path[i + 1] == L'\0')
            {
                break;
            }
        }
    }
    
    for (size_t i = 0; i < list.size(); ++i)
    {
        pInfo->pArc->ImportEntry(list[i], pInfo->removeExisting);
    }
    pInfo->pArc->AddLogMessage(std::format(L"Imported {} entries", list.size())); // Leaving this as is since it is formatted with size.
    pInfo->pArc->bUpdateSearch = true;
}

void IMGArchive::AddLogMessage(std::wstring &&message)
{
    std::lock_guard<std::mutex> lock(LogMutex);
    LogList.push_back(std::move(message));
}

eImgVer IMGArchive::GetVersion() {
    return ImageVersion;
}

void IMGArchive::SetVersion(eImgVer ver) {
    this->ImageVersion = ver;

    switch(this->ImageVersion)
    {
    case eImgVer::One:
        Parser = ParserPCv1::Get();
        break;
    case eImgVer::Two:
        Parser = ParserPCv2::Get();
        break;
    default: 
        Parser = UnknownFMT::Get();
        break;
    }
}

std::string IMGArchive::GetFormatText() 
{
    return reinterpret_cast<ParserPCv1*>(Parser)->GetVersionText();
}

eImgVer IMGArchive::GetVersion(const std::wstring &Path)
{
    if (ParserPCv1::Get()->IsValid(Path)) {
        return eImgVer::One;
    }

    if (ParserPCv2::Get()->IsValid(Path)) {
        return eImgVer::Two;
    }

    return eImgVer::Unknown;
}

void IMGArchive::Save(ArchiveInfo *pInfo)
{
    if (pInfo->pArc->Parser)
    {
        pInfo->pArc->Parser->Save(pInfo);
    }
}