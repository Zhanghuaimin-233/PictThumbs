#ifndef PICTTHUMBS_DLLMAIN_H
#define PICTTHUMBS_DLLMAIN_H

#include <windows.h>

// {36FCD09A-A906-4cd0-8EC9-52EB6E097DFB}
// TODO: Don't let these be defines.
#define SZ_CLSID_PICTTHUMBSPROVIDER		"{36FCD09A-A906-4cd0-8EC9-52EB6E097DFB}"
#define SZ_THUMBNAILPROVIDERNAME		"Pictus Thumbnail Provider"

static const CLSID CLSID_PictusThumbnailProvider = { 0x36fcd09a, 0xa906, 0x4cd0, { 0x8e, 0xc9, 0x52, 0xeb, 0x6e, 0x9, 0x7d, 0xfb } };

// {5A7B3F5C-4E6D-4a8b-9C1E-2F3A4B5C6D7E}
#define SZ_CLSID_PICTPREVIEWHANDLER		"{5A7B3F5C-4E6D-4a8b-9C1E-2F3A4B5C6D7E}"
#define SZ_PREVIEWHANDLERNAME			"Pictus Preview Handler"

static const CLSID CLSID_PictPreviewHandler = { 0x5a7b3f5c, 0x4e6d, 0x4a8b, { 0x9c, 0x1e, 0x2f, 0x3a, 0x4b, 0x5c, 0x6d, 0x7e } };

extern HMODULE g_hInst;

#endif
