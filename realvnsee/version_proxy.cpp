#include "pch.h"
#ifdef VNCVERSION_6_7_2
#include <windows.h>

/**
 * DLL Proxy Code for version.dll
 * Functions Proxied:
 *      - GetFileVersionInfoW
 *      - GetFileVersionInfoSizeW
 *      - VerQueryValueW
 */


// Module handle and function pointers for the real version.dll functions
HMODULE hRealVersionModule = NULL;

// GetFileVersionInfoW
typedef BOOL (WINAPI *GFVIW) (
	LPCWSTR lptstrFilename,
	DWORD   dwHandle,
	DWORD   dwLen,
	LPVOID  lpData
);
GFVIW RealGetFileVersionInfoW = NULL;

// GetFileVersionInfoSizeW
typedef DWORD (WINAPI *GFVISW) (
	LPCSTR  lptstrFilename,
	LPDWORD lpdwHandle
);
GFVISW RealGetFileVersionInfoSizeW = NULL;

// VerQueryValueW
typedef BOOL (WINAPI *VQVW)(
	LPCVOID pBlock,
	LPCWSTR lpSubBlock,
	LPVOID* lplpBuffer,
	PUINT   puLen
);
VQVW RealVerQueryValueW = NULL;

/**
 * LoadRealVersionDll
 * Load the real version.dll and get function pointers.
 */
BOOL LoadRealVersionDll(void) {
    // Load real version.dll
    hRealVersionModule = LoadLibraryW(L"C:\\Windows\\system32\\version.dll");
    if (hRealVersionModule == NULL) {
        OutputDebugStringW(L"SKYLIGHT: Failed to load C:\\Windows\\system32\\version.dll");
        return FALSE;
    }
    // Get function pointers
    RealGetFileVersionInfoW = (GFVIW) GetProcAddress(hRealVersionModule, "GetFileVersionInfoW");
    RealGetFileVersionInfoSizeW = (GFVISW) GetProcAddress(hRealVersionModule, "GetFileVersionInfoSizeW");
    RealVerQueryValueW = (VQVW) GetProcAddress(hRealVersionModule, "VerQueryValueW");
    return TRUE;
}

/************** Proxy Version Functions ***************/

BOOL ProxyGetFileVersionInfoW(
    LPCWSTR lptstrFilename,
    DWORD   dwHandle,
    DWORD   dwLen,
    LPVOID  lpData
) {
    // Load real version.dll if not already loaded
    if (hRealVersionModule == NULL) {
        LoadRealVersionDll();
    }
    if (RealGetFileVersionInfoW != NULL) {
        return RealGetFileVersionInfoW(lptstrFilename, dwHandle, dwLen, lpData);
    }
    return FALSE;
}

DWORD ProxyGetFileVersionInfoSizeW(
    LPCSTR  lptstrFilename,
    LPDWORD lpdwHandle
) {
    // Load real version.dll if not already loaded
    if (hRealVersionModule == NULL) {
        LoadRealVersionDll();
    }
    if (RealGetFileVersionInfoSizeW != NULL) {
        return RealGetFileVersionInfoSizeW(lptstrFilename, lpdwHandle);
    }
    return 0;
}

BOOL ProxyVerQueryValueW(
    LPCVOID pBlock,
    LPCWSTR lpSubBlock,
    LPVOID* lplpBuffer,
    PUINT   puLen
) {
    // Load real version.dll if not already loaded
    if (hRealVersionModule == NULL) {
        LoadRealVersionDll();
    }
    if (RealVerQueryValueW != NULL) {
        return RealVerQueryValueW(pBlock, lpSubBlock, lplpBuffer, puLen);
    }
    return FALSE;
}
#endif