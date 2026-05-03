#include "intl.h"
#include "exception.h"
#include "types.h"

#include <vector>

namespace Intl {
	// Simple callback mechanism
	static std::vector<LanguageChangedCallback> s_callbacks;

	auto m_currLang = Language::English;

	// TODO: Replace this garbage
	const TableEntry* m_pTable = 0;

	void LanguageTable(const TableEntry* pTable) {
		m_pTable	= pTable;
	}

	void CurrentLanguage(const Language& lang) {
		if (m_currLang != lang) {
			m_currLang = lang;
			NotifyLanguageChanged();
		}
	}

	Language CurrentLanguage() {
		return m_currLang;
	}

	Language SystemLanguage() {
		// Default to English since we don't have sysinfo.h
		return Language::English;
	}

	const char* GetString(int id) {
		return GetStringLang(id, m_currLang);
	}

	const char* GetStringLang(int id, Language lang) {
		if (lang >= Language::Undefined) {
			DO_THROW(Err::CriticalError, "Invalid language identifier requested.");
		}

		return m_pTable[id].langs[static_cast<size_t>(lang)];
	}

	void RegisterLanguageChangedCallback(LanguageChangedCallback callback) {
		s_callbacks.push_back(std::move(callback));
	}

	void NotifyLanguageChanged() {
		for (const auto& callback : s_callbacks) {
			if (callback) {
				callback();
			}
		}
	}
}
