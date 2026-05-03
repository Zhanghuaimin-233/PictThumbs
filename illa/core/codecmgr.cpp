#include "orz/intl.h"
#include "orz/types.h"
#include "codecmgr.h"
#include "c_factory.h"
#include "config.h"

// Codecs - only include what we have
#include "../codecs/pcx/f_pcx.h"
#include "../codecs/tga/f_tga.h"
#include "../codecs/wbmp/f_wbmp.h"
#include "../codecs/webp/f_webp.h"
#include "../codecs/psp/f_psp.h"
#include "../codecs/psd/f_psd.h"
#include "../codecs/xyz/f_xyz.h"

#include <algorithm>

namespace Img {
	bool CodecFactoryStore::DoCodecExist(const char* ext) {
		std::string upper(ext);
		std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
		return (m_ext.find(upper) != m_ext.end());
	}

	AbstractCodec* CodecFactoryStore::CreateCodec(const std::string& ext) {
		std::string upper(ext);
		std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
		auto i = m_ext.find(upper);
		if (i != m_ext.end()) {
			return i->second->CreateCodec();
		}

		return nullptr;
	}

	AbstractCodec* CodecFactoryStore::CreateCodec(size_t index) {
		if (index >= m_factories.size()) {
			return nullptr;
		}

		return m_factories[index]->CreateCodec();
	}

	const CodecFactoryStore::InfoVector& CodecFactoryStore::CodecInfo() {
		return m_info;
	}

	bool CodecFactoryStore::AddCodecFactory(ICodecFactory* pCodecFactory) {
		FactoryPtr pFactory(pCodecFactory);

		for (auto i = 0u; i < m_factories.size(); i++) {
			if (m_factories[i].get() == pCodecFactory) {
				return true;
			}
		}

		m_factories.push_back(pFactory);

		Info info;
		info.Description = pCodecFactory->GetFormatName();

		auto& exts = pCodecFactory->SupportedExtensions();

		for(auto i = 0u; i < exts.size(); i++) {
			std::string upper(exts[i]);
			std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
			m_ext.insert(ExtPair(upper, pFactory));
			info.Extensions.push_back(exts[i]);
		}

		m_info.push_back(info);

		return true;
	}

	void CodecFactoryStore::ReleaseFactories() {
		m_info.clear();
		m_factories.clear();
		m_ext.clear();
	}

	void CodecFactoryStore::AddBuiltinCodecs() {
		AddCodecFactory(new Img::FactoryWebp());
		AddCodecFactory(new Img::FactoryPCX());
		AddCodecFactory(new Img::FactoryTGA());
		AddCodecFactory(new Img::FactoryWBMP());
		AddCodecFactory(new Img::FactoryPSP());
		AddCodecFactory(new Img::FactoryPSD());
		AddCodecFactory(new Img::FactoryXYZ());
	}

	CodecFactoryStore::~CodecFactoryStore() {
		m_info.clear();
		m_factories.clear();
		m_ext.clear();
	}
}
