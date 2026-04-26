#include "detection/locale/locale.h"
#include "common/windows/unicode.h"

#include <windows.h>

const char* ffDetectLocale(FFstrbuf* result) {
#ifdef FF_WINXP_COMPAT
    wchar_t lang[LOCALE_NAME_MAX_LENGTH], ctry[LOCALE_NAME_MAX_LENGTH];
    if (GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, lang, LOCALE_NAME_MAX_LENGTH) &&
        GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, ctry, LOCALE_NAME_MAX_LENGTH)) {
        ffStrbufSetF(result, "%ls-%ls", lang, ctry);
        return NULL;
    }
    return "GetLocaleInfoW() failed";
#else
    wchar_t name[LOCALE_NAME_MAX_LENGTH];
    int size = GetUserDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH);
    if (size <= 1) { // including '\0'
        return "GetUserDefaultLocaleName() failed";
    }

    ffStrbufSetNWS(result, (uint32_t) size - 1, name);

    return NULL;
#endif
}
