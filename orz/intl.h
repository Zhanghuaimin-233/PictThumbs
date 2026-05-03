#ifndef ORZ_INTL_H
#define ORZ_INTL_H

#include "intl_table_entry.h"
#include <functional>
#include <vector>

namespace Intl {
	void LanguageTable(const TableEntry* pTable);
	void CurrentLanguage(const Language& lang);
	Language CurrentLanguage();
	Language SystemLanguage();
	const char* GetString(int id);
	const char* GetStringLang(int id, Language lang);

	// Simple callback mechanism to replace boost::signals2
	using LanguageChangedCallback = std::function<void()>;
	void RegisterLanguageChangedCallback(LanguageChangedCallback callback);
	void NotifyLanguageChanged();
}

#endif
