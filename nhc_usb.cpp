#include "nhc_usb.h"

#ifdef _WIN32

#include <windows.h>
#include <setupapi.h>
#include "winusb.h"

static HANDLE h_usb;
static WINUSB_INTERFACE_HANDLE h_winusb;
static OVERLAPPED ov;

uint32_t nhc_usb::open(uint32_t vid, uint32_t pid)
{
    HANDLE hDevInfoSet;
    GUID hidGuid = {0xdee824ef, 0x729b, 0x4a0e, 0x9c, 0x14, 0xb7, 0x11, 0x7d, 0x33, 0xa8, 0x17};

    conn = 0;

    hDevInfoSet = SetupDiGetClassDevs(&hidGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (!hDevInfoSet)
    {
        return 0;
    }

    DWORD dwRes;
    DWORD dwStatus;
    DWORD dwInterfaceIndex = 0;
    SP_DEVICE_INTERFACE_DATA devInterfaceData;
    SP_DEVICE_INTERFACE_DETAIL_DATA detail;
    devInterfaceData.cbSize = sizeof(devInterfaceData);
    char *pBuff;
    SP_DEVINFO_DATA devInfoData;
    DWORD dwRegType, dwRegSize;
    DWORD dwFound = 0;

    while (1)
    {
        dwRes = SetupDiEnumDeviceInterfaces(hDevInfoSet, NULL, &hidGuid, dwInterfaceIndex, &devInterfaceData);
        if (dwRes)
        {
            dwStatus = GetLastError();
        }
        else
        {
            return 0;
        }
        if (dwStatus == ERROR_NO_MORE_ITEMS)
        {
            return 0;
        }

        devInfoData.cbSize = sizeof(devInfoData);
        SetupDiEnumDeviceInfo(hDevInfoSet, dwInterfaceIndex, &devInfoData);

        SetupDiGetDeviceRegistryProperty(hDevInfoSet, &devInfoData, SPDRP_HARDWAREID, &dwRegType, NULL, 0, &dwRegSize);

        pBuff = (char *)malloc(dwRegSize);
        if (!pBuff)
        {
            return 0;
        }
        SetupDiGetDeviceRegistryProperty(hDevInfoSet, &devInfoData, SPDRP_HARDWAREID, &dwRegType, (unsigned char *)pBuff, dwRegSize, NULL);

        char szVid[100];
        char szPid[100];
        char *p;
        wsprintf(szPid, "pid_%04x", pid);
        wsprintf(szVid, "vid_%04x&", vid);
        lstrcat(szVid, szPid);
        DWORD dwLen, dwJ;
        dwLen = lstrlen(pBuff);
        for (dwJ = 0; dwJ < dwLen; ++dwJ)
        {
            if (pBuff[dwJ] >= 'A' && pBuff[dwJ] <= 'Z')
                pBuff[dwJ] += 0x20;
        }
        p = strstr(pBuff, szVid);
        if (p)
        {
            dwFound = 1;
            break;
        }
        ++dwInterfaceIndex;
    }
    if (dwFound)
    {
    }
    else
    {
        return 0;
    }

    memset(&detail, 0, sizeof(detail));
    detail.cbSize = sizeof(detail);
    DWORD dwSize = 0;
    SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &devInterfaceData, NULL, 0, &dwSize, NULL);
    PSP_DEVICE_INTERFACE_DETAIL_DATA ktNew;
    ktNew = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwSize);
    if (!ktNew)
    {
        return 0;
    }
    ktNew->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
    SetupDiGetDeviceInterfaceDetail(hDevInfoSet, &devInterfaceData, ktNew, dwSize, NULL, NULL);

    h_usb = CreateFile(ktNew->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    if (!h_usb)
    {
        return 0;
    }
    if (!WinUsb_Initialize(h_usb, &h_winusb))
    {
        return 0;
    }

    conn = 1;

    return 1;
}

void nhc_usb::close(void)
{

    if (conn)
    {
        CloseHandle(h_usb);
        WinUsb_Free(h_winusb);
        conn = 0;
    }
}

uint32_t nhc_usb::write(uint8_t *p_data, uint32_t len, uint8_t ep)
{
    uint32_t w;

    WinUsb_WritePipe(h_winusb, ep, p_data, len, (PULONG)&w, NULL);
    return 1;
}

uint32_t nhc_usb::read(uint8_t *p_data, uint32_t len, uint8_t ep)
{
    uint32_t r;
    uint32_t timeout = 5000;
    uint32_t t;

    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, "KT_WinUsb");

    WinUsb_SetPipePolicy(h_winusb, ep | 0x80, PIPE_TRANSFER_TIMEOUT, sizeof(ULONG), &timeout);

    WinUsb_ReadPipe(h_winusb, ep | 0x80, p_data, len, (PULONG)&r, &ov);

    WinUsb_GetOverlappedResult(h_winusb, &ov, (LPDWORD)&t, TRUE);
    if (t != len)
    {
        CloseHandle(ov.hEvent);
        return 0;
    }

    CloseHandle(ov.hEvent);
    return 1;
}

#else

#include <libusb-1.0/libusb.h>

static libusb_device_handle *h;

uint32_t nhc_usb::open(uint32_t vid, uint32_t pid)
{

    conn = 0;
    libusb_init(NULL);
    h = libusb_open_device_with_vid_pid(NULL, vid, pid);

    if (h)
    {
        conn = 1;
    }
    
    return h != NULL;
}

void nhc_usb::close(void)
{

    if (conn)
    {
        libusb_close(h);
        libusb_exit(NULL);
        conn = 0;
    }
}

uint32_t nhc_usb::write(uint8_t *p_data, uint32_t len, uint8_t ep)
{
    uint32_t tmp;

    tmp = len;
    if (libusb_bulk_transfer(h, ep, p_data, len, (int *)&tmp, 5000) != 0)
    {
        return 0;
    }

    return 1;
}

uint32_t nhc_usb::read(uint8_t *p_data, uint32_t len, uint8_t ep)
{
    uint32_t tmp;

    tmp = tmp;
    if (libusb_bulk_transfer(h, ep | 0x80, p_data, len, (int *)&tmp, 5000) != 0)
    {
        return 0;
    }

    return 1;
}

#endif
