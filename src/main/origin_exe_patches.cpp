// Patches for Origin.exe

#include "pch.hpp"
#include "main.hpp"

decltype(&LoadLibraryW) LoadLibraryW_org;
HMODULE WINAPI LoadLibraryW_Hook(LPCWSTR lpLibFileName)
{
	auto ret = LoadLibraryW_org(lpLibFileName);

	static bool didDoOriginClientDllPatches = false;
	if (ret && !didDoOriginClientDllPatches && GetProcAddress(ret, "OriginApplicationStart"))
	{
		OriginClient = CModule("OriginClient.dll");
		OriginClientAdr = CMemory(OriginClient.GetModuleBase());

		// statically imported by OriginClient.dll
		Qt5Core = CModule("Qt5Core.dll");

		DoOriginClientDllPatches();

		MH_STATUS result;
		if ((result = MH_EnableHook(MH_ALL_HOOKS)) != MH_OK)
			MessageBoxA(nullptr, ("MH_EnableHook(MH_ALL_HOOKS) error: " + std::to_string(result)).c_str(), ERROR_MSGBOX_CAPTION, MB_ICONERROR);

		didDoOriginClientDllPatches = true;
	}

	return ret;
}

void DoOriginExePatches()
{
	// We hook LoadLibraryW to monitor for when OriginClient.dll is actually loaded, as it's loaded dynamically.
	// It is worth noting that it's not loaded right away, instead Origin.exe itself first starts the updater, which
	// then starts Origin.exe again, to then finally actually start up, making this solution necessary.
	CreateHookNamed("kernel32", "LoadLibraryW", LoadLibraryW_Hook, reinterpret_cast<LPVOID*>(&LoadLibraryW_org));
}
