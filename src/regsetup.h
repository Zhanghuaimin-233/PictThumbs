#ifndef PICTTHUMBS_REGSETUP_H
#define PICTTHUMBS_REGSETUP_H

#include "illa/core/codecmgr.h"

#include <string>

#include <windows.h>

HRESULT RegisterInprocServer(const std::string& clsid, const std::string& friendlyName);
HRESULT RegisterThumbnailProvider(const std::string& clsId, const std::string& extension);
HRESULT RegisterPreviewHandler(const std::string& clsId, const std::string& extension);
HRESULT RegisterPreviewHandlerAppID(const std::string& clsid);

HRESULT UnregisterInprocServer(const std::string& clsid);
HRESULT UnRegisterThumbnailProvider(const std::string& clsid, const std::string& extension);
HRESULT UnRegisterPreviewHandler(const std::string& clsid, const std::string& extension);

void ClearThumbnailCache();

#endif