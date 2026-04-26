#include "displayserver.h"
#include "common/edidHelper.h"
#include "common/windows/registry.h"
#include "common/windows/unicode.h"

#include <windows.h>
#include <shellscalingapi.h>

// DISPLAYCONFIG types were introduced in Windows 7 SDK (_WIN32_WINNT >= 0x0601).
// Since we target XP (_WIN32_WINNT=0x0501), we define them ourselves.
#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0601

typedef enum {
    DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME = 1,
    DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME = 2,
    DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE = 4,
    DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO = 11,
    DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO_2 = 17,
} DISPLAYCONFIG_DEVICE_INFO_TYPE;

typedef enum {
    DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE = 1,
    DISPLAYCONFIG_MODE_INFO_TYPE_TARGET = 2,
    DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE = 3,
} DISPLAYCONFIG_MODE_INFO_TYPE;

typedef enum {
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER = -1,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HD15 = 0,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SVIDEO = 1,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPOSITE_VIDEO = 2,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_COMPONENT_VIDEO = 3,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DVI = 4,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_HDMI = 5,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_LVDS = 6,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_D_JPN = 8,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_SDI = 9,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EXTERNAL = 10,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED = 11,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EXTERNAL = 12,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED = 14,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_RESERVED = 15,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL = 0x80000000,
    DISPLAYCONFIG_OUTPUT_TECHNOLOGY_FORCE_UINT32 = 0x7FFFFFFF,
} DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY;

typedef enum {
    DISPLAYCONFIG_ROTATION_IDENTITY = 1,
    DISPLAYCONFIG_ROTATION_ROTATE90 = 2,
    DISPLAYCONFIG_ROTATION_ROTATE180 = 3,
    DISPLAYCONFIG_ROTATION_ROTATE270 = 4,
} DISPLAYCONFIG_ROTATION;

typedef enum {
    DISPLAYCONFIG_SCALING_IDENTITY = 1,
    DISPLAYCONFIG_SCALING_CENTERED = 2,
    DISPLAYCONFIG_SCALING_STRETCHED = 3,
    DISPLAYCONFIG_SCALING_ASPECTRATIOCENTEREDMAX = 4,
    DISPLAYCONFIG_SCALING_CUSTOM = 5,
    DISPLAYCONFIG_SCALING_PREFERRED = 128,
} DISPLAYCONFIG_SCALING;

typedef enum {
    DISPLAYCONFIG_PIXELFORMAT_8BPP = 1,
    DISPLAYCONFIG_PIXELFORMAT_16BPP = 2,
    DISPLAYCONFIG_PIXELFORMAT_24BPP = 3,
    DISPLAYCONFIG_PIXELFORMAT_32BPP = 4,
    DISPLAYCONFIG_PIXELFORMAT_NONGDI = 5,
} DISPLAYCONFIG_PIXELFORMAT;

typedef enum {
    DISPLAYCONFIG_SCANLINE_ORDERING_UNSPECIFIED = 0,
    DISPLAYCONFIG_SCANLINE_ORDERING_PROGRESSIVE = 1,
    DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED = 2,
    DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED_UPPERFIELDFIRST = 3,
    DISPLAYCONFIG_SCANLINE_ORDERING_INTERLACED_LOWERFIELDFIRST = 4,
} DISPLAYCONFIG_SCANLINE_ORDERING;

typedef struct {
    UINT32 cx;
    UINT32 cy;
} DISPLAYCONFIG_2DREGION;

typedef struct {
    UINT32 Numerator;
    UINT32 Denominator;
} DISPLAYCONFIG_RATIONAL;

typedef struct {
    UINT64 pixelRate;
    DISPLAYCONFIG_RATIONAL hSyncFreq;
    DISPLAYCONFIG_RATIONAL vSyncFreq;
    DISPLAYCONFIG_2DREGION activeSize;
    DISPLAYCONFIG_2DREGION totalSize;
    UINT32 videoStandard;
    DISPLAYCONFIG_SCANLINE_ORDERING scanLineOrdering;
} DISPLAYCONFIG_VIDEO_SIGNAL_INFO;

typedef struct {
    DISPLAYCONFIG_VIDEO_SIGNAL_INFO targetVideoSignalInfo;
} DISPLAYCONFIG_TARGET_MODE;

typedef struct {
    UINT32 width;
    UINT32 height;
    DISPLAYCONFIG_PIXELFORMAT pixelFormat;
    POINTL position;
} DISPLAYCONFIG_SOURCE_MODE;

typedef struct {
    POINTL PathSourceSize;
    RECTL DesktopImageRegion;
    RECTL DesktopImageClip;
} DISPLAYCONFIG_DESKTOP_IMAGE_INFO;

typedef struct {
    DISPLAYCONFIG_DEVICE_INFO_TYPE type;
    UINT32 size;
    LUID adapterId;
    UINT32 id;
} DISPLAYCONFIG_DEVICE_INFO_HEADER;

typedef struct {
    LUID adapterId;
    UINT32 id;
    UINT32 modeInfoIdx;
    UINT32 cloneGroupId;
} DISPLAYCONFIG_PATH_SOURCE_INFO;

typedef struct {
    LUID adapterId;
    UINT32 id;
    union {
        UINT32 modeInfoIdx;
        struct {
            UINT32 desktopModeInfoIdx;
            UINT32 targetModeInfoIdx;
        };
    };
    DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology;
    DISPLAYCONFIG_ROTATION rotation;
    DISPLAYCONFIG_SCALING scaling;
    DISPLAYCONFIG_RATIONAL refreshRate;
    DISPLAYCONFIG_SCANLINE_ORDERING scanLineOrdering;
    BOOL targetAvailable;
    UINT32 statusFlags;
} DISPLAYCONFIG_PATH_TARGET_INFO;

typedef struct {
    DISPLAYCONFIG_PATH_SOURCE_INFO sourceInfo;
    DISPLAYCONFIG_PATH_TARGET_INFO targetInfo;
    UINT32 flags;
} DISPLAYCONFIG_PATH_INFO;

typedef struct {
    DISPLAYCONFIG_MODE_INFO_TYPE infoType;
    LUID adapterId;
    UINT32 id;
    union {
        DISPLAYCONFIG_TARGET_MODE targetMode;
        DISPLAYCONFIG_SOURCE_MODE sourceMode;
        DISPLAYCONFIG_DESKTOP_IMAGE_INFO desktopImageInfo;
    };
} DISPLAYCONFIG_MODE_INFO;

typedef struct {
    UINT32 friendlyNameFromEdid : 1;
    UINT32 friendlyNameFromDevice : 1;
    UINT32 edidIdsValid : 1;
    UINT32 reserved : 29;
} DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS;

typedef struct {
    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
    DISPLAYCONFIG_TARGET_DEVICE_NAME_FLAGS flags;
    DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology;
    UINT16 edidManufactureId;
    UINT16 edidProductCodeId;
    UINT32 connectorInstance;
    WCHAR monitorFriendlyDeviceName[64];
    WCHAR monitorDevicePath[128];
} DISPLAYCONFIG_TARGET_DEVICE_NAME;

typedef struct {
    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
    UINT32 width;
    UINT32 height;
    DISPLAYCONFIG_TARGET_MODE targetMode;
} DISPLAYCONFIG_TARGET_PREFERRED_MODE;

typedef struct {
    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
    union {
        struct {
            UINT32 advancedColorSupported : 1;
            UINT32 advancedColorEnabled : 1;
            UINT32 wideColorEnforced : 1;
            UINT32 advancedColorSupportedVirtual : 1;
            UINT32 advancedColorEnabledVirtual : 1;
            UINT32 reserved : 27;
        };
        UINT32 value;
    };
    UINT32 colorDataFormat;
    UINT32 bitsPerColorChannel;
} DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO;

typedef struct {
    DISPLAYCONFIG_DEVICE_INFO_HEADER header;
    union {
        struct {
            UINT32 advancedColorSupported : 1;
            UINT32 advancedColorEnabled : 1;
            UINT32 wideColorEnforced : 1;
            UINT32 advancedColorSupportedVirtual : 1;
            UINT32 advancedColorEnabledVirtual : 1;
            UINT32 highDynamicRangeSupported : 1;
            UINT32 highDynamicRangeUserEnabled : 1;
            UINT32 reserved : 25;
        };
        UINT32 value;
    };
    UINT32 colorDataFormat;
    UINT32 bitsPerColorChannel;
    UINT16 sdrWhiteLevel;
} DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO_2;

#define QDC_ALL_PATHS 1
#define QDC_ONLY_ACTIVE_PATHS 2
#define QDC_DATABASE_CURRENT 4

#define DISPLAYCONFIG_PATH_BOOST_REFRESH_RATE 0x00040000

#endif

typedef LONG (WINAPI *QueryDisplayConfig_t)(UINT32, UINT32*, DISPLAYCONFIG_PATH_INFO*, UINT32*, DISPLAYCONFIG_MODE_INFO*, void*);
typedef LONG (WINAPI *DisplayConfigGetDeviceInfo_t)(DISPLAYCONFIG_DEVICE_INFO_HEADER*);

static QueryDisplayConfig_t ffQueryDisplayConfig = NULL;
static DisplayConfigGetDeviceInfo_t ffDisplayConfigGetDeviceInfo = NULL;

static void ffLoadDisplayConfig(void) {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    if (hUser32) {
        ffQueryDisplayConfig = (QueryDisplayConfig_t)GetProcAddress(hUser32, "QueryDisplayConfig");
        ffDisplayConfigGetDeviceInfo = (DisplayConfigGetDeviceInfo_t)GetProcAddress(hUser32, "DisplayConfigGetDeviceInfo");
    }
}

static inline void freeArgBuffer(FFArgBuffer* buffer) {
    if (buffer->data) {
        free(buffer->data);
    }
    buffer->data = NULL;
    buffer->length = 0;
}
#define FF_AUTO_FREE_ARG_BUFFER FF_A_CLEANUP(freeArgBuffer)

// http://undoc.airesoft.co.uk/user32.dll/IsThreadDesktopComposited.php
BOOL WINAPI IsThreadDesktopComposited();
BOOL WINAPI GetDpiForMonitorInternal(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT* dpiX, UINT* dpiY);

static void detectDisplays(FFDisplayServerResult* ds) {
    ffLoadDisplayConfig();
    if (!ffQueryDisplayConfig || !ffDisplayConfigGetDeviceInfo)
        return;

    DISPLAYCONFIG_PATH_INFO paths[128];
    uint32_t pathCount = ARRAY_SIZE(paths);
    DISPLAYCONFIG_MODE_INFO modes[256];
    uint32_t modeCount = ARRAY_SIZE(modes);

    if (ffQueryDisplayConfig(
            QDC_ONLY_ACTIVE_PATHS,
            &pathCount,
            paths,
            &modeCount,
            modes,
            NULL) == ERROR_SUCCESS) {
        for (uint32_t i = 0; i < pathCount; ++i) {
            const DISPLAYCONFIG_PATH_INFO* path = &paths[i];
            const DISPLAYCONFIG_SOURCE_MODE* sourceMode = &modes[path->sourceInfo.modeInfoIdx].sourceMode;

            FF_STRBUF_AUTO_DESTROY name = ffStrbufCreate();
            uint32_t physicalWidth = 0, physicalHeight = 0;

            DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME,
                    .size = sizeof(targetName),
                    .adapterId = path->targetInfo.adapterId,
                    .id = path->targetInfo.id,
                },
            };
            FF_AUTO_FREE_ARG_BUFFER FFArgBuffer edid = {};
            if (ffDisplayConfigGetDeviceInfo(&targetName.header) == ERROR_SUCCESS) {
                wchar_t regPath[256] = L"SYSTEM\\CurrentControlSet\\Enum";
                wchar_t* pRegPath = regPath + strlen("SYSTEM\\CurrentControlSet\\Enum");
                wchar_t* pDevPath = targetName.monitorDevicePath + strlen("\\\\?");
                while (*pDevPath && *pDevPath != L'{') {
                    if (*pDevPath == L'#') {
                        *pRegPath = L'\\';
                    } else {
                        *pRegPath = *pDevPath;
                    }
                    ++pRegPath;
                    ++pDevPath;
                    assert(pRegPath < regPath + ARRAY_SIZE(regPath) + strlen("Device Parameters"));
                }
                wcscpy(pRegPath, L"Device Parameters");

                FF_AUTO_CLOSE_FD HANDLE hKey = NULL;
                if (ffRegOpenKeyForRead(HKEY_LOCAL_MACHINE, regPath, &hKey, NULL) &&
                    ffRegReadData(hKey, L"EDID", &edid, NULL) &&
                    ffEdidIsValid(edid.data, edid.length)) {
                    ffEdidGetName(edid.data, &name);
                    ffEdidGetPhysicalSize(edid.data, &physicalWidth, &physicalHeight);
                } else {
                    edid.length = 0;
                    if (targetName.flags.friendlyNameFromEdid) {
                        ffStrbufSetWS(&name, targetName.monitorFriendlyDeviceName);
                    } else {
                        ffStrbufSetWS(&name, targetName.monitorDevicePath);
                        ffStrbufSubstrAfterFirstC(&name, '#');
                        ffStrbufSubstrBeforeFirstC(&name, '#');
                    }
                }
            }

            uint32_t width = sourceMode->width;
            uint32_t height = sourceMode->height;
            uint32_t rotation;
            switch (path->targetInfo.rotation) {
                case DISPLAYCONFIG_ROTATION_ROTATE90:
                    rotation = 90;
                    break;
                case DISPLAYCONFIG_ROTATION_ROTATE180:
                    rotation = 180;
                    break;
                case DISPLAYCONFIG_ROTATION_ROTATE270:
                    rotation = 270;
                    break;
                default:
                    rotation = 0;
                    break;
            }

            DISPLAYCONFIG_TARGET_PREFERRED_MODE preferredMode = {
                .header = {
                    .type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_PREFERRED_MODE,
                    .size = sizeof(preferredMode),
                    .adapterId = path->targetInfo.adapterId,
                    .id = path->targetInfo.id,
                }
            };
            double preferredRefreshRate = 0;
            if (ffDisplayConfigGetDeviceInfo(&preferredMode.header) == ERROR_SUCCESS) {
                DISPLAYCONFIG_RATIONAL freq = preferredMode.targetMode.targetVideoSignalInfo.vSyncFreq;
                preferredRefreshRate = freq.Numerator / (double) freq.Denominator;
            }

            uint32_t systemDpi = 0;
            HMONITOR hMonitor = MonitorFromPoint(*(POINT*) &sourceMode->position, MONITOR_DEFAULTTONULL);
            if (hMonitor) {
                UINT ignored;
                GetDpiForMonitorInternal(hMonitor, MDT_EFFECTIVE_DPI, &systemDpi, &ignored);
            }

            if (systemDpi == 0) {
                HDC hdc = GetDC(NULL);
                systemDpi = (uint32_t) GetDeviceCaps(hdc, LOGPIXELSX);
                if (systemDpi == 0) {
                    systemDpi = 96;
                }
                ReleaseDC(NULL, hdc);
            }

            if (path->targetInfo.rotation == DISPLAYCONFIG_ROTATION_ROTATE90 ||
                path->targetInfo.rotation == DISPLAYCONFIG_ROTATION_ROTATE270) {
                uint32_t temp = width;
                width = height;
                height = temp;
            }

            FFDisplayResult* display = ffdsAppendDisplay(ds,
                width,
                height,
                path->targetInfo.refreshRate.Numerator / (double) path->targetInfo.refreshRate.Denominator,
                systemDpi,
                preferredMode.width,
                preferredMode.height,
                preferredRefreshRate,
                rotation,
                &name,
                path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER ? FF_DISPLAY_TYPE_UNKNOWN : path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL || path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_DISPLAYPORT_EMBEDDED || path->targetInfo.outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_UDI_EMBEDDED ? FF_DISPLAY_TYPE_BUILTIN
                                                                                                                                                                                                                                                                                                                                                                                       : FF_DISPLAY_TYPE_EXTERNAL,
                sourceMode->position.x == 0 && sourceMode->position.y == 0,
                (uintptr_t) hMonitor,
                physicalWidth,
                physicalHeight,
                "GDI");

            if (display) {
                DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO_2 advColorInfo2 = {
                    .header = {
                        .type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO_2,
                        .size = sizeof(advColorInfo2),
                        .adapterId = path->targetInfo.adapterId,
                        .id = path->targetInfo.id,
                    }
                };
                if (ffDisplayConfigGetDeviceInfo(&advColorInfo2.header) == ERROR_SUCCESS) {
                    if (advColorInfo2.highDynamicRangeUserEnabled) {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
                    } else if (advColorInfo2.highDynamicRangeSupported) {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;
                    } else {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                    }
                    display->bitDepth = (uint8_t) advColorInfo2.bitsPerColorChannel;
                } else {
                    DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO advColorInfo = {
                        .header = {
                            .type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO,
                            .size = sizeof(advColorInfo),
                            .adapterId = path->targetInfo.adapterId,
                            .id = path->targetInfo.id,
                        }
                    };
                    if (ffDisplayConfigGetDeviceInfo(&advColorInfo.header) == ERROR_SUCCESS) {
                        if (advColorInfo.advancedColorEnabled) {
                            display->hdrStatus = FF_DISPLAY_HDR_STATUS_ENABLED;
                        } else if (advColorInfo.advancedColorSupported) {
                            display->hdrStatus = FF_DISPLAY_HDR_STATUS_SUPPORTED;
                        } else {
                            display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNSUPPORTED;
                        }
                        display->bitDepth = (uint8_t) advColorInfo.bitsPerColorChannel;
                    } else {
                        display->hdrStatus = FF_DISPLAY_HDR_STATUS_UNKNOWN;
                    }
                }
                if (edid.length > 0) {
                    ffEdidGetSerialAndManufactureDate(edid.data, &display->serial, &display->manufactureYear, &display->manufactureWeek);
                }
                display->drrStatus = path->flags & DISPLAYCONFIG_PATH_BOOST_REFRESH_RATE ? FF_DISPLAY_DRR_STATUS_ENABLED : FF_DISPLAY_DRR_STATUS_DISABLED;
            }
        }
    }
}

void ffConnectDisplayServerImpl(FFDisplayServerResult* ds) {
    if (IsThreadDesktopComposited()) {
        ffStrbufSetStatic(&ds->wmProcessName, "dwm.exe");
        ffStrbufSetStatic(&ds->wmPrettyName, "Desktop Window Manager");
    } else {
        // `explorer.exe` only provides a subset of WM functions, as well as the taskbar and desktop icons.
        // While a window itself is drawn by kernel (GDI). Killing `explorer.exe` won't affect how windows are displayed generally.
        ffStrbufSetStatic(&ds->wmProcessName, "explorer.exe");
        ffStrbufSetStatic(&ds->wmPrettyName, "Internal");
    }

    detectDisplays(ds);
}
