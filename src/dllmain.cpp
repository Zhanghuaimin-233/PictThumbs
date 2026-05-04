#include "dllmain.h"
#include "regsetup.h"
#include "ClassFactory.h"
#include "DllRef.h"
#include "codecsetup.h"
#include "orz/logger.h"
#include "orz/types.h"
#include <new>
#include <ShlObj.h>
#include <locale>
#include <filesystem>

extern HRESULT CPictusThumbnailProvider_CreateInstance(REFIID riid, void **ppv);
extern HRESULT CPreviewHandler_CreateInstance(REFIID riid, void **ppv);


// add classes supported by this module here
const CLASS_OBJECT_INIT c_rgClassObjectInit[] =
{
	{ &CLSID_PictusThumbnailProvider, CPictusThumbnailProvider_CreateInstance },
	{ &CLSID_PictPreviewHandler, CPreviewHandler_CreateInstance }
};



// Handle the the DLL's module
HINSTANCE g_hInst = NULL;

// Standard DLL functions
STDAPI_(BOOL) DllMain(_In_ HINSTANCE hInstance, _In_ DWORD dwReason, _In_opt_ void *)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hInst = hInstance;
		DisableThreadLibraryCalls(hInstance);

		// Set locale to system default
		std::locale::global(std::locale(""));

		// Initialize log file
		WCHAR szAppData[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szAppData)))
		{
			std::filesystem::path logDir = std::filesystem::path(szAppData) / L"PictThumbs";
			std::filesystem::create_directories(logDir);
			std::filesystem::path logFile = logDir / L"pictthumbs.log";
			Log.SetOutput(WStringToUTF8(logFile.wstring()));
		}
	}
	return TRUE;
}

STDAPI DllCanUnloadNow() {
	// Only allow the DLL to be unloaded after all outstanding references have been released
	return (DllRefCount() == 0) ? S_OK : S_FALSE;
}


_Check_return_ STDAPI DllGetClassObject(_In_ REFCLSID clsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv) {
	return CClassFactory::CreateInstance(clsid, c_rgClassObjectInit, ARRAYSIZE(c_rgClassObjectInit), riid, ppv);
}

STDAPI DllRegisterServer()
{
	Log << "(Thumb): Registering service ...\n";

	auto hr = RegisterInprocServer(SZ_CLSID_PICTTHUMBSPROVIDER, SZ_THUMBNAILPROVIDERNAME);
	if (FAILED(hr))
	{
		Log << "(Thumb): RegisterInProcServer failed, error code " << std::hex << hr << "\n";
		return hr;
	}

	hr = RegisterInprocServer(SZ_CLSID_PICTPREVIEWHANDLER, SZ_PREVIEWHANDLERNAME);
	if (FAILED(hr))
	{
		Log << "(Preview): RegisterInProcServer failed, error code " << std::hex << hr << "\n";
		return hr;
	}

	// Register AppID for preview handler to use prevhost.exe
	hr = RegisterPreviewHandlerAppID(SZ_CLSID_PICTPREVIEWHANDLER);
	if (FAILED(hr))
	{
		Log << "(Preview): RegisterPreviewHandlerAppID failed, error code " << std::hex << hr << "\n";
		return hr;
	}

	Img::CodecFactoryStore cfs;
	CodecManagerSetup(&cfs);

	const Img::CodecFactoryStore::InfoVector& v = cfs.CodecInfo();
	for (size_t i = 0; i < v.size(); i++)
	{
		const Img::CodecFactoryStore::Info& info = v[i];
		for (size_t j = 0; j < info.Extensions.size(); ++j)
		{
			hr = RegisterThumbnailProvider(SZ_CLSID_PICTTHUMBSPROVIDER, info.Extensions[j]);
			if (FAILED(hr))
			{
				Log << "(Thumb): RegisterThumbnailProvider failed, error code " << std::hex << hr << "\n";
				return hr;
			}

			hr = RegisterPreviewHandler(SZ_CLSID_PICTPREVIEWHANDLER, info.Extensions[j]);
			if (FAILED(hr))
			{
				Log << "(Preview): RegisterPreviewHandler failed, error code " << std::hex << hr << "\n";
				return hr;
			}
		}
	}

	// Clear thumbnail cache to force regeneration
	ClearThumbnailCache();

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	return hr;
}

STDAPI DllUnregisterServer()
{
	auto hr = UnregisterInprocServer(SZ_CLSID_PICTTHUMBSPROVIDER);
	if (FAILED(hr)) return hr;

	hr = UnregisterInprocServer(SZ_CLSID_PICTPREVIEWHANDLER);
	if (FAILED(hr)) return hr;

	Img::CodecFactoryStore cfs;
	CodecManagerSetup(&cfs);

	const Img::CodecFactoryStore::InfoVector& v = cfs.CodecInfo();
	for (size_t i = 0; i < v.size(); i++) {
		const Img::CodecFactoryStore::Info& info = v[i];
		for (size_t j = 0; j < info.Extensions.size(); ++j) {
			hr = UnRegisterThumbnailProvider(SZ_CLSID_PICTTHUMBSPROVIDER, info.Extensions[j]);
			if (FAILED(hr)) return hr;

			hr = UnRegisterPreviewHandler(SZ_CLSID_PICTPREVIEWHANDLER, info.Extensions[j]);
			if (FAILED(hr)) return hr;
		}
	}

	return hr;
}
