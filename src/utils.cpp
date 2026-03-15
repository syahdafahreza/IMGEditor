#include "pch.h"
#include "utils.h"
#include <assert.h>
#include <windows.h>

void Utils::ConvertUtf8ToWide(const char* utf8Str, wchar_t* wideStr, size_t wideStrSize) {
    if (utf8Str == nullptr || wideStr == nullptr || wideStrSize == 0) {
        return;
    }

    // Pass cbMultiByte as the actual length of the string to avoid ERROR_INSUFFICIENT_BUFFER
    // if the buffer isn't large enough to hold the full string + null terminator.
    int strLen = static_cast<int>(strlen(utf8Str));
    int result = MultiByteToWideChar(CP_UTF8, 0, utf8Str, strLen, wideStr, static_cast<int>(wideStrSize - 1));

    // Always null-terminate manually since we might have truncated the string.
    if (result >= 0 && result < wideStrSize) {
        wideStr[result] = L'\0';
    } else {
        wideStr[wideStrSize - 1] = L'\0';
    }
}

void Utils::ConvertWideToUtf8(const wchar_t* wideStr, char* utf8Str, size_t utf8StrSize) {
    if (wideStr == nullptr || utf8Str == nullptr || utf8StrSize == 0) {
        return;
    }

    int strLen = static_cast<int>(wcslen(wideStr));
    int result = WideCharToMultiByte(CP_UTF8, 0, wideStr, strLen, utf8Str, static_cast<int>(utf8StrSize - 1), nullptr, nullptr);

    if (result >= 0 && result < utf8StrSize) {
        utf8Str[result] = '\0';
    } else {
        utf8Str[utf8StrSize - 1] = '\0';
    }
}

void Utils::ToLowerCase(wchar_t* wideStr) {
    if (wideStr == nullptr) return;

    for (size_t i = 0; wideStr[i] != L'\0'; ++i) {
        if (wideStr[i] >= L'A' && wideStr[i] <= L'Z') {
            wideStr[i] = wideStr[i] + (L'a' - L'A'); 
        }
    }
}
