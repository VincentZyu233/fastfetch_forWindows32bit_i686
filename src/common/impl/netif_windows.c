#include "common/netif.h"

#include <ws2tcpip.h> // AF_INET6, IN6_IS_ADDR_UNSPECIFIED
#include <iphlpapi.h>

#ifdef FF_WINXP_COMPAT

bool ffNetifGetDefaultRouteImplV4(FFNetifDefaultRouteResult* result) {
    ULONG bufSize = 0;
    if (GetIpForwardTable(NULL, &bufSize, FALSE) != ERROR_INSUFFICIENT_BUFFER)
        return false;

    PMIB_IPFORWARDTABLE pIpForwardTable = (PMIB_IPFORWARDTABLE) malloc(bufSize);
    if (!pIpForwardTable)
        return false;

    if (GetIpForwardTable(pIpForwardTable, &bufSize, FALSE) != NO_ERROR) {
        free(pIpForwardTable);
        return false;
    }

    bool foundDefault = false;
    uint32_t smallestMetric = UINT32_MAX;

    for (ULONG i = 0; i < pIpForwardTable->dwNumEntries; ++i) {
        MIB_IPFORWARDROW* row = &pIpForwardTable->table[i];

        if (row->dwForwardDest == 0 && row->dwForwardMask == 0) {
            MIB_IFROW ifRow = { .dwIndex = row->dwForwardIfIndex };
            if (GetIfEntry(&ifRow) == NO_ERROR &&
                ifRow.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL)
            {
                uint32_t realMetric = row->dwForwardMetric1;
                if (realMetric < smallestMetric) {
                    smallestMetric = realMetric;
                    result->ifIndex = row->dwForwardIfIndex;
                    foundDefault = true;
                }
            }
        }
    }

    free(pIpForwardTable);
    return foundDefault;
}

bool ffNetifGetDefaultRouteImplV6(FFNetifDefaultRouteResult* result) {
    (void) result;
    return false;
}

#else

bool ffNetifGetDefaultRouteImplV4(FFNetifDefaultRouteResult* result) {
    PMIB_IPFORWARD_TABLE2 pIpForwardTable = NULL;

    if (!NETIO_SUCCESS(GetIpForwardTable2(AF_INET, &pIpForwardTable))) {
        return false;
    }

    bool foundDefault = false;
    uint32_t smallestMetric = UINT32_MAX;

    for (ULONG i = 0; i < pIpForwardTable->NumEntries; ++i) {
        MIB_IPFORWARD_ROW2* row = &pIpForwardTable->Table[i];

        if (row->DestinationPrefix.PrefixLength == 0 &&
            row->DestinationPrefix.Prefix.Ipv4.sin_addr.S_un.S_addr == 0) {
            MIB_IF_ROW2 ifRow = {
                .InterfaceIndex = row->InterfaceIndex,
            };
            if (NETIO_SUCCESS(GetIfEntry2(&ifRow)) && ifRow.OperStatus == IfOperStatusUp) {
                MIB_IPINTERFACE_ROW ipInterfaceRow = {
                    .Family = AF_INET,
                    .InterfaceIndex = row->InterfaceIndex,
                };

                uint32_t realMetric = row->Metric /* Metric offset */;

                if (NETIO_SUCCESS(GetIpInterfaceEntry(&ipInterfaceRow))) {
                    realMetric += ipInterfaceRow.Metric /* Interface metric */;
                }

                if (realMetric < smallestMetric) {
                    smallestMetric = realMetric;
                    result->ifIndex = row->InterfaceIndex;
                    foundDefault = true;
                }
            }
        }
    }

    FreeMibTable(pIpForwardTable);

    return foundDefault;
}

bool ffNetifGetDefaultRouteImplV6(FFNetifDefaultRouteResult* result) {
    PMIB_IPFORWARD_TABLE2 pIpForwardTable = NULL;

    if (!NETIO_SUCCESS(GetIpForwardTable2(AF_INET6, &pIpForwardTable))) {
        return false;
    }

    bool foundDefault = false;
    uint32_t smallestMetric = UINT32_MAX;

    for (ULONG i = 0; i < pIpForwardTable->NumEntries; ++i) {
        MIB_IPFORWARD_ROW2* row = &pIpForwardTable->Table[i];

        if (row->DestinationPrefix.PrefixLength == 0 &&
            IN6_IS_ADDR_UNSPECIFIED(&row->DestinationPrefix.Prefix.Ipv6.sin6_addr)) {
            MIB_IF_ROW2 ifRow = {
                .InterfaceIndex = row->InterfaceIndex,
            };
            if (NETIO_SUCCESS(GetIfEntry2(&ifRow)) && ifRow.OperStatus == IfOperStatusUp) {
                MIB_IPINTERFACE_ROW ipInterfaceRow = {
                    .Family = AF_INET6,
                    .InterfaceIndex = row->InterfaceIndex,
                };

                uint32_t realMetric = row->Metric /* Metric offset */;

                if (NETIO_SUCCESS(GetIpInterfaceEntry(&ipInterfaceRow))) {
                    realMetric += ipInterfaceRow.Metric /* Interface metric */;
                }

                if (realMetric < smallestMetric) {
                    smallestMetric = realMetric;
                    result->ifIndex = row->InterfaceIndex;
                    foundDefault = true;
                }
            }
        }
    }

    FreeMibTable(pIpForwardTable);

    return foundDefault;
}

#endif
