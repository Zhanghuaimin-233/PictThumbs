#include "regsetup.h"
#include "codecsetup.h"
#include "dllmain.h"
#include "regutils.h"
#include "orz/logger.h"
#include "orz/types.h"
#include <filesystem>
#include <ShlObj.h>

HRESULT RegisterInprocServer(const std::string& clsid, const std::string& friendlyName) {
	Log << "(Thumb): Register InprocServer\n";
	WCHAR szModuleName[MAX_PATH];

	if (!GetModuleFileNameW(g_hInst, szModuleName, ARRAYSIZE(szModuleName)))
	{
		Log << "(Thumb): GetModuleFileNameW failed, " << std::hex << GetLastError() << "\n";
		return HRESULT_FROM_WIN32(GetLastError());
	}

	Log << "(Thumb): GetModuleFileNameW OK\n";

	auto moduleName = WStringToUTF8(szModuleName);

	Log << "(Thumb): moduleName=" << moduleName << "\n";

	auto subKey = "CLSID\\" + clsid;
	auto servKey = subKey + "\\InProcServer32";
	HRESULT hr;
	Log << "(Thumb): Setting InprocServer friendly name\n";
	hr = SetHkcrRegistryKeyAndValue(subKey, 0, friendlyName.c_str());
	if (FAILED(hr))
	{
		Log << "(Thumb): Failed creating key/value for CLSID\n";
		return hr;
	}

	Log << "(Thumb): Setting InprocServer module name\n";
	hr = SetHkcrRegistryKeyAndValue(servKey, 0, moduleName.c_str());
	if (FAILED(hr))
	{
		Log << "(Thumb): Failed creating key/value for InProcServer32\n";
		return hr;
	}

	Log << "(Thumb): Setting InprocServer threading model\n";
	hr = SetHkcrRegistryKeyAndValue(servKey, "ThreadingModel", "Apartment");
	if (FAILED(hr))
	{
		Log << "(Thumb): Failed creating key/value for ThreadingModel\n";
		return hr;
	}

	return hr;
}

HRESULT RegisterPreviewHandlerAppID(const std::string& clsid)
{
	Log << "(Preview): Registering AppID for preview handler\n";

	// Set AppID for the CLSID to use prevhost.exe as surrogate
	auto clsidKey = "CLSID\\" + clsid;
	HRESULT hr = SetHkcrRegistryKeyAndValue(clsidKey, "AppID", "{6d2b5079-2f0b-48dd-ab7f-97cec514d30b}");
	if (FAILED(hr))
	{
		Log << "(Preview): Failed to set AppID for CLSID\n";
		return hr;
	}

	// Register the AppID with DllSurrogate
	auto appIDKey = "AppID\\{6d2b5079-2f0b-48dd-ab7f-97cec514d30b}";
	hr = SetHkcrRegistryKeyAndValue(appIDKey, 0, "Preview Handler Surrogate Host");
	if (FAILED(hr))
	{
		Log << "(Preview): Failed to create AppID key\n";
		return hr;
	}

	// Set DllSurrogate to prevhost.exe
	hr = SetHkcrRegistryKeyAndValue(appIDKey, "DllSurrogate", "%SystemRoot%\\system32\\prevhost.exe");
	if (FAILED(hr))
	{
		Log << "(Preview): Failed to set DllSurrogate\n";
		return hr;
	}

	// Disable process isolation for Windows 11 compatibility
	hr = SetHkcrRegistryKeyAndValue(clsidKey, "DisableProcessIsolation", "1");
	if (FAILED(hr))
	{
		Log << "(Preview): Failed to set DisableProcessIsolation\n";
		return hr;
	}

	Log << "(Preview): AppID registration succeeded\n";
	return S_OK;
}

HRESULT RegisterThumbnailProvider(const std::string& clsId, const std::string& extension)
{
	// We always register the shellex directly into the extension key.
	// This allows thumbnails to work for that format even when the user changes associations (progids).
	Log << "(Thumb): Registering thumbnail provider for " << extension << "\n";

	auto currentSubKey = "." + extension + "\\shellex\\{e357fccd-a995-4576-b01f-234630154e96}";
	return SetHkcrRegistryKeyAndValue(currentSubKey, 0, clsId.c_str());
}


HRESULT UnregisterInprocServer(const std::string& clsid)
{
	Log << "(Thumb): Unregistering InprocServer\n";
	auto subKey = "CLSID\\" + clsid;

	return HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CLASSES_ROOT, UTF8ToWString(subKey).c_str()));
}

HRESULT UnRegisterThumbnailProvider(const std::string& clsId, const std::string& extension)
{
	Log << "(Thumb): Unregistering thumbnail provider for " << extension << "\n";

	auto currentSubKey = "." + extension + "\\shellex\\{e357fccd-a995-4576-b01f-234630154e96}";

	auto ret = GetHkcrRegistryKeyAndValue(currentSubKey, 0);
	auto hr = std::get<0>(ret);
	auto currentClsId = std::get<1>(ret);
	
	// If the key doesn't exist, that's OK - nothing to unregister
	if (FAILED(hr)) {
		Log << "(Thumb:UnRegisterThumbnailProvider): Key not found for " << extension << ", skipping.\n";
		return S_OK;
	}

	if (currentClsId != clsId) {
		Log << "(Thumb:UnRegisterThumbnailProvider): CLSID mismatch for " << extension << ", expected " << clsId << " but got " << currentClsId << "\n";
		return S_OK; // Clsids didn't match. Not an error, but we shouldn't remove the key.
	}

	return HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CLASSES_ROOT, UTF8ToWString(currentSubKey).c_str()));
}

HRESULT RegisterPreviewHandler(const std::string& clsId, const std::string& extension)
{
	Log << "(Preview): Registering preview handler for " << extension << "\n";

	// Register as preview handler for this extension
	auto currentSubKey = "." + extension + "\\shellex\\{8895b1c6-b41f-4c1c-a562-0d564250836f}";
	HRESULT hr = SetHkcrRegistryKeyAndValue(currentSubKey, 0, clsId.c_str());
	if (FAILED(hr))
	{
		Log << "(Preview): Failed to register shellex for " << extension << "\n";
		return hr;
	}

	// Also register in the global PreviewHandlers list (HKEY_LOCAL_MACHINE)
	auto previewHandlersKey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers";
	HKEY hKey;
	LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, previewHandlersKey, 0, KEY_SET_VALUE, &hKey);
	if (result == ERROR_SUCCESS)
	{
		auto wideClsId = UTF8ToWString(clsId);
		auto wideName = UTF8ToWString("Pictus Preview Handler");
		result = RegSetValueExW(hKey, wideClsId.c_str(), 0, REG_SZ, 
			reinterpret_cast<const BYTE*>(wideName.c_str()), 
			(wideName.length() + 1) * sizeof(wchar_t));
		RegCloseKey(hKey);
		
		if (result == ERROR_SUCCESS)
		{
			Log << "(Preview): Registered in global PreviewHandlers\n";
		}
		else
		{
			Log << "(Preview): Failed to register in global PreviewHandlers, error " << result << "\n";
		}
	}
	else
	{
		Log << "(Preview): Failed to open PreviewHandlers key, error " << result << "\n";
	}

	return S_OK;
}

HRESULT UnRegisterPreviewHandler(const std::string& clsId, const std::string& extension)
{
	Log << "(Preview): Unregistering preview handler for " << extension << "\n";

	auto currentSubKey = "." + extension + "\\shellex\\{8895b1c6-b41f-4c1c-a562-0d564250836f}";

	auto ret = GetHkcrRegistryKeyAndValue(currentSubKey, 0);
	auto hr = std::get<0>(ret);
	auto currentClsId = std::get<1>(ret);

	// If the key doesn't exist, that's OK - nothing to unregister
	if (FAILED(hr)) {
		Log << "(Preview:UnRegisterPreviewHandler): Key not found for " << extension << ", skipping.\n";
		return S_OK;
	}

	if (currentClsId != clsId) {
		Log << "(Preview:UnRegisterPreviewHandler): CLSID mismatch for " << extension << ", expected " << clsId << " but got " << currentClsId << "\n";
		return S_OK;
	}

	hr = HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CLASSES_ROOT, UTF8ToWString(currentSubKey).c_str()));
	if (FAILED(hr)) return hr;

	// Remove from global PreviewHandlers list
	auto previewHandlersKey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\PreviewHandlers";
	HKEY hKey;
	LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, UTF8ToWString(previewHandlersKey).c_str(), 0, KEY_SET_VALUE, &hKey);
	if (result == ERROR_SUCCESS)
	{
		RegDeleteValueW(hKey, UTF8ToWString(clsId).c_str());
		RegCloseKey(hKey);
	}

	return S_OK;
}

void ClearThumbnailCache()
{
	Log << "(Cache): Clearing thumbnail cache...\n";

	WCHAR szAppData[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szAppData)))
	{
		std::filesystem::path explorerDir = std::filesystem::path(szAppData) / L"Microsoft" / L"Windows" / L"Explorer";
		
		if (std::filesystem::exists(explorerDir))
		{
			int deletedCount = 0;
			for (const auto& entry : std::filesystem::directory_iterator(explorerDir))
			{
				if (entry.is_regular_file())
				{
					std::wstring filename = entry.path().filename().wstring();
					if (filename.find(L"thumbcache_") == 0 && filename.find(L".db") != std::wstring::npos)
					{
						try
						{
							std::filesystem::remove(entry.path());
							deletedCount++;
							Log << "(Cache): Deleted " << WStringToUTF8(filename) << "\n";
						}
						catch (const std::exception& e)
						{
							Log << "(Cache): Failed to delete " << WStringToUTF8(filename) << ": " << e.what() << "\n";
						}
					}
				}
			}
			Log << "(Cache): Deleted " << deletedCount << " thumbnail cache files.\n";
		}
	}
}
