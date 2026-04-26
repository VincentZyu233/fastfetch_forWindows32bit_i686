#include "users.h"
#include "common/windows/unicode.h"
#include "common/time.h"

#include <windows.h>
#include <wtsapi32.h>
#include <ws2tcpip.h>

const char* ffDetectUsers(FFUsersOptions* options, FFlist* users) {
#ifdef FF_WINXP_COMPAT
    WTS_SESSION_INFOW* sessionInfo;
    DWORD sessionCount;
    if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &sessionInfo, &sessionCount))
        return "WTSEnumerateSessionsW() failed";

    for (DWORD i = 0; i < sessionCount; i++) {
        WTS_SESSION_INFOW* session = &sessionInfo[i];
        if (session->State != WTSActive)
            continue;

        LPWSTR pBuf = NULL;
        DWORD bufLen = 0;
        if (!WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, session->SessionId, WTSUserName, &pBuf, &bufLen))
            continue;
        FF_STRBUF_AUTO_DESTROY userName = ffStrbufCreateWS(pBuf);
        WTSFreeMemory(pBuf);

        if (options->myselfOnly && !ffStrbufEqual(&instance.state.platform.userName, &userName))
            continue;

        FFUserResult* user = FF_LIST_ADD(FFUserResult, *users);
        ffStrbufInitMove(&user->name, &userName);

        if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, session->SessionId, WTSClientName, &pBuf, &bufLen)) {
            ffStrbufInitWS(&user->hostName, pBuf);
            WTSFreeMemory(pBuf);
        } else
            ffStrbufInit(&user->hostName);

        if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, session->SessionId, WTSWinStationName, &pBuf, &bufLen)) {
            ffStrbufInitWS(&user->sessionName, pBuf);
            WTSFreeMemory(pBuf);
        } else
            ffStrbufInit(&user->sessionName);

        ffStrbufInit(&user->clientIp);
        user->loginTime = 0;

        PWTS_CLIENT_ADDRESS address = NULL;
        if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, session->SessionId, WTSClientAddress, (LPWSTR*) &address, &bufLen)) {
            if (address->AddressFamily == AF_INET) {
                ffStrbufSetF(&user->clientIp, "%u.%u.%u.%u", address->Address[2], address->Address[3], address->Address[4], address->Address[5]);
            } else if (address->AddressFamily == AF_INET6) {
                char ipStr[INET6_ADDRSTRLEN];
                SOCKADDR_IN6 tmp = {};
                tmp.sin6_family = AF_INET6;
                memcpy(&tmp.sin6_addr, address->Address, sizeof(tmp.sin6_addr));
                DWORD ipLen = sizeof(ipStr);
                if (WSAAddressToStringA((LPSOCKADDR) &tmp, sizeof(tmp), NULL, ipStr, &ipLen) == 0)
                    ffStrbufSetNS(&user->clientIp, ipLen, ipStr);
            }
            WTSFreeMemory(address);
        }
    }

    WTSFreeMemory(sessionInfo);
    return NULL;
#else
    WTS_SESSION_INFO_1W* sessionInfo;
    DWORD sessionCount;
    DWORD level = 1;

    if (!WTSEnumerateSessionsExW(WTS_CURRENT_SERVER_HANDLE, &level, 0, &sessionInfo, &sessionCount)) {
        return "WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE) failed";
    }

    for (DWORD i = 0; i < sessionCount; i++) {
        WTS_SESSION_INFO_1W* session = &sessionInfo[i];
        if (session->State != WTSActive) {
            continue;
        }

        FF_STRBUF_AUTO_DESTROY userName = ffStrbufCreateWS(session->pUserName);

        if (options->myselfOnly && !ffStrbufEqual(&instance.state.platform.userName, &userName)) {
            continue;
        }

        FFUserResult* user = FF_LIST_ADD(FFUserResult, *users);
        ffStrbufInitMove(&user->name, &userName);
        ffStrbufInitWS(&user->hostName, session->pHostName);
        ffStrbufInitWS(&user->sessionName, session->pSessionName);
        ffStrbufInit(&user->clientIp);
        user->loginTime = 0;

        DWORD bytes = 0;
        PWTS_CLIENT_ADDRESS address = NULL;
        if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, session->SessionId, WTSClientAddress, (LPWSTR*) &address, &bytes)) {
            if (address->AddressFamily == AF_INET) {
                ffStrbufSetF(&user->clientIp, "%u.%u.%u.%u", address->Address[2], address->Address[3], address->Address[4], address->Address[5]);
            } else if (address->AddressFamily == AF_INET6) {
                char ipStr[INET6_ADDRSTRLEN];
                SOCKADDR_IN6 tmp = {};
                tmp.sin6_family = AF_INET6;
                memcpy(&tmp.sin6_addr, address->Address, sizeof(tmp.sin6_addr));
                DWORD ipLen = sizeof(ipStr);
                if (WSAAddressToStringA((LPSOCKADDR) &tmp, sizeof(tmp), NULL, ipStr, &ipLen) == 0)
                    ffStrbufSetNS(&user->clientIp, ipLen, ipStr);
            }
            WTSFreeMemory(address);
        }

        bytes = 0;
        PWTSINFOW wtsInfo = NULL;
        if (WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, session->SessionId, WTSSessionInfo, (LPWSTR*) &wtsInfo, &bytes)) {
            user->loginTime = ffFileTimeToUnixMs((uint64_t) wtsInfo->LogonTime.QuadPart);
            WTSFreeMemory(wtsInfo);
        }
    }

    WTSFreeMemoryExW(WTSTypeSessionInfoLevel1, sessionInfo, 1);

    return NULL;
#endif
}
