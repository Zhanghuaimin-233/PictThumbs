#include "cthumbprovider.h"
#include "illa/core/codecmgr.h"
#include "illa/core/surfacemgr.h"
#include "orz/Win32/stream_windows.h"
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

CPictusThumbnailProvider::CPictusThumbnailProvider():m_cRef(1) {
	CodecManagerSetup(&m_cfs);
}

CPictusThumbnailProvider::~CPictusThumbnailProvider() {}

IFACEMETHODIMP CPictusThumbnailProvider::QueryInterface(REFIID riid, __deref_out void **ppv) {
	static const QITAB qit[] =  {
		QITABENT(CPictusThumbnailProvider, IInitializeWithStream),
		QITABENT(CPictusThumbnailProvider, IThumbnailProvider),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG) CPictusThumbnailProvider::AddRef() {
	return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) CPictusThumbnailProvider::Release() {
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (!cRef) {
		delete this;
	}

	return cRef;
}

IFACEMETHODIMP CPictusThumbnailProvider::Initialize(_In_ IStream *pStream, _In_ DWORD grfMode) {
	if(m_reader)
		return E_UNEXPECTED;
	if(!pStream)
		return E_INVALIDARG;

	IStream* stream;
	HRESULT ret = pStream->QueryInterface(IID_PPV_ARGS(&stream));
	if(!SUCCEEDED(ret))
		return ret;

	IO::Stream::Ptr winStream(new IO::StreamWindows(stream));
	m_reader.reset(new IO::FileReader(winStream));

	// Try to detect the file extension from the stream
	// We'll use the codec detection to determine the format
	m_extension = "";

	return S_OK;
}

struct DimData {
	SizeInt sz;
	float scale;
};

DimData DetermineDimensions(UINT cx, Geom::SizeInt surfDims) {
	DimData d;
	Geom::SizeFloat factors = (float)cx / surfDims.StaticCast<float>();
	d.scale = std::min(factors.Width, std::min(factors.Height, 1.0f));
	d.sz = (surfDims * d.scale).StaticCast<int>();
	return d;
}

void CPictusThumbnailProvider::OverlayFileTypeIcon(HBITMAP hBitmap, UINT cx) {
	if (m_extension.empty()) {
		return;
	}

	std::wstring extWithDot = L"." + std::wstring(m_extension.begin(), m_extension.end());
	
	SHFILEINFOW sfi = {};
	HRESULT hr = SHGetFileInfoW(
		extWithDot.c_str(),
		FILE_ATTRIBUTE_NORMAL,
		&sfi,
		sizeof(sfi),
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES
	);

	if (FAILED(hr) || !sfi.hIcon) {
		return;
	}

	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBitmap);

	int iconSize = std::max(16, (int)(cx / 4));
	int iconX = (int)cx - iconSize - 2;
	int iconY = (int)cx - iconSize - 2;

	// Semi-transparent background
	HDC hdcAlpha = CreateCompatibleDC(hdcScreen);
	BITMAPINFO bmiAlpha = {};
	bmiAlpha.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiAlpha.bmiHeader.biWidth = iconSize;
	bmiAlpha.bmiHeader.biHeight = iconSize;
	bmiAlpha.bmiHeader.biPlanes = 1;
	bmiAlpha.bmiHeader.biBitCount = 32;
	bmiAlpha.bmiHeader.biCompression = BI_RGB;
	
	void* pAlphaBits = nullptr;
	HBITMAP hAlphaBmp = CreateDIBSection(hdcScreen, &bmiAlpha, DIB_RGB_COLORS, &pAlphaBits, NULL, 0);
	if (hAlphaBmp) {
		HBITMAP hOldAlpha = (HBITMAP)SelectObject(hdcAlpha, hAlphaBmp);
		RECT rcFill = { 0, 0, iconSize, iconSize };
		HBRUSH hFillBrush = CreateSolidBrush(RGB(32, 32, 32));
		FillRect(hdcAlpha, &rcFill, hFillBrush);
		DeleteObject(hFillBrush);
		
		BLENDFUNCTION blend = {};
		blend.BlendOp = AC_SRC_OVER;
		blend.SourceConstantAlpha = 180;
		AlphaBlend(hdcMem, iconX, iconY, iconSize, iconSize, hdcAlpha, 0, 0, iconSize, iconSize, blend);
		
		SelectObject(hdcAlpha, hOldAlpha);
		DeleteObject(hAlphaBmp);
	}
	DeleteDC(hdcAlpha);

	DrawIconEx(hdcMem, iconX, iconY, sfi.hIcon, iconSize, iconSize, 0, NULL, DI_NORMAL);

	SelectObject(hdcMem, hOldBmp);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);
	DestroyIcon(sfi.hIcon);
}

// TODO: Figure out why XYZ doesn't work. Doesn't SEEM to be registry related, so that leaves GetThumbnail.
// IThumbnailProvider
_Use_decl_annotations_ IFACEMETHODIMP CPictusThumbnailProvider::GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha) {
	try {
		if (cx == 0) {
			Log << "(Thumb) Argument cx invalid.\n";
			return E_INVALIDARG;
		}

		if (phbmp == 0) {
			Log << "(Thumb) Argument phbmp invalid.\n";
			return E_POINTER;
		}

		if (pdwAlpha != 0) {
			*pdwAlpha = WTSAT_UNKNOWN;
		}

		if (!m_reader) {
			Log << "(Thumb) Invalid state, stream not yet initialized.";
			return E_ILLEGAL_METHOD_CALL;
		}

		Img::SurfaceFactory(new FactorySurfaceSoftware);
		Img::CodecFactoryStore cfs;
		CodecManagerSetup(&cfs);

		// Detect the format and store extension
		m_reader->Seek(0, IO::SeekMethod::Begin);
		AbstractCodec* detectedCodec = nullptr;
		const Img::CodecFactoryStore::InfoVector& iv = cfs.CodecInfo();
		for (size_t i = 0; i < iv.size(); ++i) {
			AbstractCodec* c = cfs.CreateCodec(i);
			if (c == 0) continue;
			
			m_reader->Seek(0, IO::SeekMethod::Begin);
			if (c->CanDetectFormat() && c->LoadHeader(m_reader)) {
				detectedCodec = c;
				// Get the extension from the codec info
				if (!iv[i].Extensions.empty()) {
					m_extension = iv[i].Extensions[0];
				}
				delete c;
				break;
			}
			delete c;
		}

		Img::Surface::Ptr s = LoadSurface(cx);
		if(!s) {
			Log << "(Thumb) Failed to load image.\n";
			return E_NOTIMPL;
		}

		DimData outDims = DetermineDimensions(cx, s->GetSize());
		if(!IsPositive(s->GetSize()) || !IsPositive(outDims.sz)) {
			Log << "(Thumb) Image or thumbnail had non-positive image dimensions.\n";
			return E_NOTIMPL;
		}
		Log << "(Thumb) Loaded image, dims:" << s->GetSize() << "\n";

		// Shadow border size
		const int borderSize = 4;
		int totalWidth = outDims.sz.Width + borderSize * 2;
		int totalHeight = outDims.sz.Height + borderSize * 2;

		// Ensure we don't exceed cx
		if (totalWidth > (int)cx) totalWidth = (int)cx;
		if (totalHeight > (int)cx) totalHeight = (int)cx;

		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
		bmi.bmiHeader.biWidth = totalWidth;
		bmi.bmiHeader.biHeight = -static_cast<LONG>(totalHeight);
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		uint8_t* pBits;
		*phbmp = CreateDIBSection(0, &bmi, DIB_RGB_COLORS, reinterpret_cast<void **>(&pBits), NULL, 0);
		if(*phbmp == 0) {
			Log << "PictThumb: Failed to create DIB section.";
			return E_OUTOFMEMORY;
		}
		if (*phbmp == reinterpret_cast<HBITMAP>(ERROR_INVALID_PARAMETER)) {
			*phbmp = 0;
			Log << "(Thumb) Failed to create DIB section.\n";
			return E_UNEXPECTED;
		}

		// Fill with white background (shadow effect)
		int stride = totalWidth * 4;
		for (int y = 0; y < totalHeight; y++) {
			uint8_t* row = pBits + y * stride;
			for (int x = 0; x < totalWidth; x++) {
				row[x * 4 + 0] = 255; // B
				row[x * 4 + 1] = 255; // G
				row[x * 4 + 2] = 255; // R
				row[x * 4 + 3] = 255; // A
			}
		}

		// Render image centered on the white background
		uint8_t* imageStart = pBits + borderSize * stride + borderSize * 4;
		Filter::FilterBuffer dst(outDims.sz, 4, imageStart, stride);

		Img::FilterBufferAndLock src = GenerateFilterBuffer(s);

		Img::Properties props;
		props.ResampleFilter = Filter::Mode::Lanczos3;
		props.Zoom = outDims.scale; 
		props.RetainAlpha = true;

		Img::RenderToBuffer(dst, src.filterBuffer, s->GetFormat(), RectInt(PointInt(0, 0), s->GetSize()), props);
		if (HasAlpha(s->GetFormat())) {
			Filter::Alpha::PremultiplyAlphaBuffer(dst);
		}
		else {
			Filter::Alpha::SetAlpha(dst, 0xff);
		}

		// Overlay the file type icon in the bottom-right corner
		OverlayFileTypeIcon(*phbmp, totalWidth);

		*pdwAlpha = WTSAT_RGB;

		return S_OK;
	}
	catch(std::exception& e) {
		Log << "PictThumb: " << e.what() << "\n";
	}
	catch (...) { 
		Log << "Unknown exception encountered.\n";
	}
	return E_UNEXPECTED;
}

AbstractCodec* CPictusThumbnailProvider::FindCodec() {
	const Img::CodecFactoryStore::InfoVector& iv = m_cfs.CodecInfo();

	for(size_t i = 0; i < iv.size(); ++i) {
		AbstractCodec* c = m_cfs.CreateCodec(i);
		if (c == 0) {
			continue;
		}

		m_reader->Seek(0, IO::SeekMethod::Begin);
		if (!c->CanDetectFormat() || !c->LoadHeader(m_reader)) {
			delete c;
		}
		else {
			return c;
		}
	}
	Log << "(Thumb) Could not find a valid decoder.\n";
	return 0;
}

Img::Surface::Ptr CPictusThumbnailProvider::LoadSurface(UINT cx) {
	AbstractCodec* c = FindCodec();
	if (c == 0) {
		Log << "(Thumb) No codec is available. Can't load image.\n";
		return Img::Surface::Ptr();
	}
	if (c->Allocate(SizeInt(cx, cx)) == Img::AbstractCodec::AllocationStatus::NotSupported) {
		Log << "(Thumb) Specified image size not supported, will attempt full-size instead.\n";
		if (c->Allocate() != Img::AbstractCodec::AllocationStatus::Ok) {
			return Img::Surface::Ptr();
		}
	}

	if (c->LoadImageData() == AbstractCodec::LoadStatus::Failed) {
		Log << "(Thumb) LoadImageData failed.\n";
		return Img::Surface::Ptr();
	}

	Img::ImageComposer::Ptr cmp = c->RequestImageComposer();
	return cmp->RequestCurrentSurface();
}


HRESULT CPictusThumbnailProvider_CreateInstance(REFIID riid, __deref_out void **ppv) {
	try {
		auto *pNew = new CPictusThumbnailProvider();
		auto hr = pNew->QueryInterface(riid, ppv);
		pNew->Release();

		return hr;
	}
	catch (std::bad_alloc&) {
		return E_OUTOFMEMORY;
	}
}
