#include <Windows.h>


WCHAR MainMonitorName[64] = { 0 };
DWORD currentW = 0;
DWORD currentH = 0;
DWORD currentHz = 0;

int getMainMonitorInfo()
{
	BOOL ret = FALSE;
	DWORD iDevNum = 0;

	do {
		DISPLAY_DEVICE displayDevice;
		ZeroMemory(&displayDevice, sizeof(DISPLAY_DEVICE));
		displayDevice.cb = sizeof(displayDevice);
		ret = EnumDisplayDevicesW(NULL, iDevNum, &displayDevice, 0);

		//qDebug("StateFlags 0x%X",displayDevice.StateFlags);
		if (ret && (displayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
			if (displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
				lstrcpyW(MainMonitorName, displayDevice.DeviceName);

				// get current resolution
				DEVMODE dm = { 0 };
				dm.dmSize = sizeof(dm);
				EnumDisplaySettingsW(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dm);
				currentW = dm.dmPelsWidth;
				currentH = dm.dmPelsHeight;
				currentHz = dm.dmDisplayFrequency;

				return 0;
			}
		}
		iDevNum++;
	} while (ret);

	return -1;
}

int changeResolution(const WCHAR* monitorName, DWORD width, DWORD height, DWORD hz) {
	DEVMODEW dm = { 0 };
	dm.dmSize = sizeof(dm);
	dm.dmPelsWidth = width;
	dm.dmPelsHeight = height;
	dm.dmDisplayFrequency = hz;
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
	DWORD dwFlags = CDS_UPDATEREGISTRY | CDS_GLOBAL;
	LONG ret = ChangeDisplaySettingsExW(monitorName, &dm, NULL, dwFlags, NULL);
	if (ret == 0) {
		// commit change
		ret = ChangeDisplaySettingsExW(NULL, NULL, NULL, 0, NULL);
	}

	return ret;
}

DWORD runProcess(const WCHAR* path) {
	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);
	si.dwFlags |= STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	PROCESS_INFORMATION pi;
	DWORD dwCreationFlags = 0;
	BOOL bRet = CreateProcessW(NULL, path, NULL, NULL, FALSE, dwCreationFlags, NULL, NULL, &si, &pi);
	if (bRet) {
		// wait for process exit
		WaitForSingleObject(pi.hProcess, INFINITE);
		DWORD dwExit;
		GetExitCodeProcess(pi.hProcess, &dwExit);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return dwExit;
	}
	return 0xFFFFFFFF;
}


int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine,
				   int nCmdShow)
{
	if (getMainMonitorInfo() == 0) {
		changeResolution(MainMonitorName, 1024, 768, currentHz);
		WCHAR llk[1024] = L"Á¬Á¬¿´.exe";
		runProcess(llk);
		changeResolution(MainMonitorName, currentW, currentH, currentHz);
	}

	return 0;
}

