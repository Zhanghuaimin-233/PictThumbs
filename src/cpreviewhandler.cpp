#include "cpreviewhandler.h"
#include "illa/core/surfacemgr.h"
#include "orz/Win32/stream_windows.h"
#include "orz/stream_file.h"
#include "illa/core/swsurface.h"
#include "illa/core/config.h"
#include "illa/core/render.h"
#include "illa/core/filter.h"
#include "codecsetup.h"

#include <Shlwapi.h>
#include <shellapi.h>
#include <algorithm>

using namespace Img;
using namespace Geom;

CPreviewHandler::CPreviewHandler() :
	m_cRef(1),
	m_hwndParent(NULL),
	m_hwndPreview(NULL),
	m_punkSite(NULL),
	m_fHasPreview(false)
{
	CodecManagerSetup(&m_cfs);
	m_rc = { 0, 0, 0, 0 };
}

CPreviewHandler::~CPreviewHandler()
{
	Unload();
	if (m_punkSite)
	{
		m_punkSite->Release();
		m_punkSite = NULL;
	}
}

IFACEMETHODIMP CPreviewHandler::QueryInterface(REFIID riid, __deref_out void **ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(CPreviewHandler, IPreviewHandler),
		QITABENT(CPreviewHandler, IInitializeWithStream),
		QITABENT(CPreviewHandler, IInitializeWithFile),
		QITABENT(CPreviewHandler, IPreviewHandlerVisuals),
		QITABENT(CPreviewHandler, IOleWindow),
		QITABENT(CPreviewHandler, IObjectWithSite),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CPreviewHandler::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) CPreviewHandler::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (!cRef)
	{
		delete this;
	}
	return cRef;
}

IFACEMETHODIMP CPreviewHandler::Initialize(_In_ IStream *pStream, _In_ DWORD grfMode)
{
	Log << "(Preview) Initialize(IStream) called.\n";
	if (m_reader)
		return E_UNEXPECTED;
	if (!pStream)
		return E_INVALIDARG;

	IStream* stream;
	HRESULT ret = pStream->QueryInterface(IID_PPV_ARGS(&stream));
	if (!SUCCEEDED(ret))
		return ret;

	IO::Stream::Ptr winStream(new IO::StreamWindows(stream));
	m_reader.reset(new IO::FileReader(winStream));

	Log << "(Preview) Initialize(IStream) succeeded.\n";
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::Initialize(_In_ LPCWSTR pszFilePath, _In_ DWORD grfMode)
{
	Log << "(Preview) Initialize(IFile) called: " << WStringToUTF8(pszFilePath) << "\n";
	if (m_reader)
		return E_UNEXPECTED;
	if (!pszFilePath)
		return E_INVALIDARG;

	m_filePath = pszFilePath;

	// Create file reader from path
	std::string filePathUtf8 = WStringToUTF8(pszFilePath);
	IO::Stream::Ptr fileStream(new IO::StreamFile(filePathUtf8));
	m_reader.reset(new IO::FileReader(fileStream));

	Log << "(Preview) Initialize(IFile) succeeded.\n";
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::SetWindow(HWND hwnd, const RECT *prc)
{
	Log << "(Preview) SetWindow called, hwnd=" << hwnd << "\n";
	m_hwndParent = hwnd;
	if (prc)
	{
		m_rc = *prc;
		Log << "(Preview) SetRect: left=" << prc->left << ", top=" << prc->top 
			<< ", right=" << prc->right << ", bottom=" << prc->bottom << "\n";
	}
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::SetFocus()
{
	if (m_hwndPreview)
	{
		::SetFocus(m_hwndPreview);
	}
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::QueryFocus(HWND *phwnd)
{
	if (!phwnd)
		return E_INVALIDARG;

	*phwnd = ::GetFocus();
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::TranslateAccelerator(MSG *pmsg)
{
	if (m_punkSite)
	{
		IPreviewHandlerFrame* pFrame = NULL;
		HRESULT hr = m_punkSite->QueryInterface(IID_PPV_ARGS(&pFrame));
		if (SUCCEEDED(hr))
		{
			hr = pFrame->TranslateAccelerator(pmsg);
			pFrame->Release();
			return hr;
		}
	}
	return S_FALSE;
}

IFACEMETHODIMP CPreviewHandler::SetRect(const RECT *prc)
{
	if (!prc)
		return E_INVALIDARG;

	m_rc = *prc;

	if (m_hwndPreview)
	{
		MoveWindow(m_hwndPreview, m_rc.left, m_rc.top,
			m_rc.right - m_rc.left, m_rc.bottom - m_rc.top, TRUE);
	}

	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::DoPreview()
{
	Log << "(Preview) DoPreview called.\n";
	if (m_fHasPreview)
	{
		Log << "(Preview) Already has preview, returning E_UNEXPECTED.\n";
		return E_UNEXPECTED;
	}

	if (!m_reader)
	{
		Log << "(Preview) No stream initialized.\n";
		return E_FAIL;
	}

	if (!m_hwndParent)
	{
		Log << "(Preview) No parent window.\n";
		return E_FAIL;
	}

	Log << "(Preview) Loading surface...\n";
	m_surface = LoadSurface();
	if (!m_surface)
	{
		Log << "(Preview) Failed to load image.\n";
		return E_FAIL;
	}

	Log << "(Preview) Surface loaded successfully.\n";
	m_fHasPreview = true;

	// Create a child window for rendering
	m_hwndPreview = CreateWindowEx(
		0,
		L"STATIC",
		NULL,
		WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
		m_rc.left, m_rc.top,
		m_rc.right - m_rc.left, m_rc.bottom - m_rc.top,
		m_hwndParent,
		NULL,
		NULL,
		this
	);

	if (!m_hwndPreview)
	{
		Log << "(Preview) Failed to create preview window.\n";
		return E_FAIL;
	}

	Log << "(Preview) Preview window created.\n";

	// Store this pointer for WM_PAINT
	SetWindowLongPtr(m_hwndPreview, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	RenderToWindow();

	Log << "(Preview) DoPreview completed.\n";
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::Unload()
{
	m_surface.reset();
	m_reader.reset();
	m_fHasPreview = false;

	if (m_hwndPreview)
	{
		DestroyWindow(m_hwndPreview);
		m_hwndPreview = NULL;
	}

	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::SetBackgroundColor(COLORREF clr)
{
	Log << "(Preview) SetBackgroundColor called.\n";
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::SetFont(const LOGFONTW *plf)
{
	Log << "(Preview) SetFont called.\n";
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::SetTextColor(COLORREF color)
{
	Log << "(Preview) SetTextColor called.\n";
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::GetWindow(HWND *phwnd)
{
	if (!phwnd)
		return E_INVALIDARG;

	*phwnd = m_hwndParent;
	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

IFACEMETHODIMP CPreviewHandler::SetSite(IUnknown *punkSite)
{
	if (m_punkSite)
	{
		m_punkSite->Release();
		m_punkSite = NULL;
	}

	m_punkSite = punkSite;

	if (m_punkSite)
	{
		m_punkSite->AddRef();
	}

	return S_OK;
}

IFACEMETHODIMP CPreviewHandler::GetSite(REFIID riid, void **ppvSite)
{
	if (!ppvSite)
		return E_INVALIDARG;

	if (!m_punkSite)
		return E_NOINTERFACE;

	return m_punkSite->QueryInterface(riid, ppvSite);
}

AbstractCodec* CPreviewHandler::FindCodec()
{
	const Img::CodecFactoryStore::InfoVector& iv = m_cfs.CodecInfo();

	for (size_t i = 0; i < iv.size(); ++i)
	{
		AbstractCodec* c = m_cfs.CreateCodec(i);
		if (c == 0)
		{
			continue;
		}

		m_reader->Seek(0, IO::SeekMethod::Begin);
		if (!c->CanDetectFormat() || !c->LoadHeader(m_reader))
		{
			delete c;
		}
		else
		{
			return c;
		}
	}
	Log << "(Preview) Could not find a valid decoder.\n";
	return 0;
}

Img::Surface::Ptr CPreviewHandler::LoadSurface()
{
	AbstractCodec* c = FindCodec();
	if (c == 0)
	{
		Log << "(Preview) No codec is available. Can't load image.\n";
		return Img::Surface::Ptr();
	}

	// Try to allocate with a reasonable size limit for preview
	if (c->Allocate() != AbstractCodec::AllocationStatus::Ok)
	{
		delete c;
		return Img::Surface::Ptr();
	}

	if (c->LoadImageData() == AbstractCodec::LoadStatus::Failed)
	{
		Log << "(Preview) LoadImageData failed.\n";
		delete c;
		return Img::Surface::Ptr();
	}

	Img::ImageComposer::Ptr cmp = c->RequestImageComposer();
	delete c;
	return cmp->RequestCurrentSurface();
}

void CPreviewHandler::RenderToWindow()
{
	if (!m_hwndPreview || !m_surface)
		return;

	RECT rcClient;
	GetClientRect(m_hwndPreview, &rcClient);
	int width = rcClient.right - rcClient.left;
	int height = rcClient.bottom - rcClient.top;

	if (width <= 0 || height <= 0)
		return;

	HDC hdc = GetDC(m_hwndPreview);
	HDC hdcMem = CreateCompatibleDC(hdc);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	uint8_t* pBits = NULL;
	HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&pBits), NULL, 0);
	if (!hBitmap)
	{
		ReleaseDC(m_hwndPreview, hdc);
		DeleteDC(hdcMem);
		return;
	}

	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBitmap);

	// Fill with white background
	int stride = width * 4;
	for (int y = 0; y < height; y++)
	{
		uint8_t* row = pBits + y * stride;
		for (int x = 0; x < width; x++)
		{
			row[x * 4 + 0] = 255; // B
			row[x * 4 + 1] = 255; // G
			row[x * 4 + 2] = 255; // R
			row[x * 4 + 3] = 255; // A
		}
	}

	// Calculate scaled dimensions to fit window
	SizeInt surfDims = m_surface->GetSize();
	float scaleX = (float)width / surfDims.Width;
	float scaleY = (float)height / surfDims.Height;
	float scale = std::min(scaleX, std::min(scaleY, 1.0f));

	int scaledWidth = (int)(surfDims.Width * scale);
	int scaledHeight = (int)(surfDims.Height * scale);

	// Center the image
	int offsetX = (width - scaledWidth) / 2;
	int offsetY = (height - scaledHeight) / 2;

	// Render image
	Filter::FilterBuffer dst(SizeInt(scaledWidth, scaledHeight), 4, pBits + offsetY * stride + offsetX * 4, stride);
	FilterBufferAndLock src = GenerateFilterBuffer(m_surface);

	Properties props;
	props.ResampleFilter = Filter::Mode::Lanczos3;
	props.Zoom = scale;
	props.RetainAlpha = true;

	RenderToBuffer(dst, src.filterBuffer, m_surface->GetFormat(), RectInt(PointInt(0, 0), surfDims), props);

	if (HasAlpha(m_surface->GetFormat()))
	{
		Filter::Alpha::PremultiplyAlphaBuffer(dst);
	}
	else
	{
		Filter::Alpha::SetAlpha(dst, 0xff);
	}

	// Blit to window
	BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hOldBmp);
	DeleteObject(hBitmap);
	DeleteDC(hdcMem);
	ReleaseDC(m_hwndPreview, hdc);
}

HRESULT CPreviewHandler_CreateInstance(REFIID riid, __deref_out void **ppv)
{
	try
	{
		auto *pNew = new CPreviewHandler();
		auto hr = pNew->QueryInterface(riid, ppv);
		pNew->Release();
		return hr;
	}
	catch (std::bad_alloc&)
	{
		return E_OUTOFMEMORY;
	}
}
