#ifndef CPREVIEWHANDLER_H
#define CPREVIEWHANDLER_H

#include "illa/core/codec.h"
#include "illa/core/codecmgr.h"
#include "orz/logger.h"

#include <shobjidl.h>
#include <shlwapi.h>
#include <string>

class CPreviewHandler :
	public IPreviewHandler,
	public IInitializeWithStream,
	public IInitializeWithFile,
	public IPreviewHandlerVisuals,
	public IOleWindow,
	public IObjectWithSite
{
public:
	CPreviewHandler();
	virtual ~CPreviewHandler();

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, __deref_out void **ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IInitializeWithStream
	IFACEMETHODIMP Initialize(_In_ IStream *pStream, _In_ DWORD grfMode);

	// IInitializeWithFile
	IFACEMETHODIMP Initialize(_In_ LPCWSTR pszFilePath, _In_ DWORD grfMode);

	// IPreviewHandler
	STDMETHODIMP SetWindow(HWND hwnd, const RECT *prc);
	STDMETHODIMP SetFocus();
	STDMETHODIMP QueryFocus(HWND *phwnd);
	STDMETHODIMP TranslateAccelerator(MSG *pmsg);
	STDMETHODIMP SetRect(const RECT *prc);
	STDMETHODIMP DoPreview();
	STDMETHODIMP Unload();

	// IPreviewHandlerVisuals
	STDMETHODIMP SetBackgroundColor(COLORREF clr);
	STDMETHODIMP SetFont(const LOGFONTW *plf);
	STDMETHODIMP SetTextColor(COLORREF color);

	// IOleWindow
	STDMETHODIMP GetWindow(HWND *phwnd);
	STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

	// IObjectWithSite
	STDMETHODIMP SetSite(IUnknown *punkSite);
	STDMETHODIMP GetSite(REFIID riid, void **ppvSite);

private:
	Img::Surface::Ptr LoadSurface();
	Img::AbstractCodec* FindCodec();
	void RenderToWindow();

	long m_cRef;
	IO::FileReader::Ptr m_reader;
	Img::CodecFactoryStore m_cfs;

	HWND m_hwndParent;
	HWND m_hwndPreview;
	RECT m_rc;
	IUnknown *m_punkSite;
	Img::Surface::Ptr m_surface;
	bool m_fHasPreview;
	std::wstring m_filePath;
};

HRESULT CPreviewHandler_CreateInstance(REFIID riid, __deref_out void **ppv);

#endif
