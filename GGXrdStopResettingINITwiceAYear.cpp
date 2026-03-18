#include "pch.h"
#include "GGXrdStopResettingINITwiceAYear.h"
#include "ConsoleEmulator.h"
#include "GGXrdStopResettingINITwiceAYearCommon.h"
#include "PatcherVersion.h"  // I included this file so that when it changes, it triggers the Pre-Build event and updates versions in the .rc file

bool forceAllowed = false;
UINT windowAppTitleResourceId = IDS_APP_TITLE;
UINT windowClassNameResourceId = IDC_GGXRDSTOPRESETTINGINITWICEAYEAR;
LPCWSTR windowIconId = MAKEINTRESOURCEW(IDI_GGXRDSTOPRESETTINGINITWICEAYEAR);
LPCWSTR windowMenuName = MAKEINTRESOURCEW(IDC_GGXRDSTOPRESETTINGINITWICEAYEAR);
LPCWSTR windowAcceleratorId = MAKEINTRESOURCEW(IDC_GGXRDSTOPRESETTINGINITWICEAYEAR);

bool parseArgs(int argc, LPWSTR* argv, int* exitCode) {
	for (int i = 0; i < argc; ++i) {
		if (_wcsicmp(argv[i], L"/?") == 0
				|| _wcsicmp(argv[i], L"--help") == 0
				|| _wcsicmp(argv[i], L"-help") == 0) {
			MessageBoxA(NULL,
				"Patcher for GGXrdAutomaticallyChangeAudioDevice for Guilty Gear Xrd Rev2 version 2211.",
				"GGXrdAutomaticallyChangeAudioDevicePatcher " PATCHER_VERSION,
				MB_OK);
			*exitCode = 0;
			return false;
		}
	}
	return true;
}

unsigned long __stdcall taskThreadProc(LPVOID unused) {
	unsigned long result = patcherMain();
	PostMessageW(mainWindow, WM_TASK_ENDED, 0, 0);
	return result;
}
