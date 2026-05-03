#include "fileops.h"
#include "file_reader.h"

#include <filesystem>
#include <mutex>

#ifdef _WIN32
#include <shellapi.h>
#endif

namespace fs = std::filesystem;

namespace IO {
	using std::mutex;

	mutex g_mutexSHFileOperation;

#ifdef _WIN32
	struct HANDLETOMAPPINGS {
		UINT              uNumberOfMappings;  // Number of mappings in the array.
		LPSHNAMEMAPPINGW  lpSHNameMapping;    // Pointer to the array of mappings.
	};
#endif

	bool DoFileExist(const std::string& file) {
		if (file.empty()) {
			return false;
		}

		IO::FileReader r(file);
		return r.Open();
	}

	bool DoPathExist(const std::string& file) {
		std::error_code ec;
		return fs::is_directory(UTF8ToWString(file), ec);
	}

#ifdef _WIN32
	static std::vector<wchar_t> ToTerminatedWcharArray(const std::string& utf8) {
		auto in = UTF8ToWString(utf8);
		size_t len = in.length() + 2; // Two extra due to _two_ terminating chars.
		std::vector<wchar_t> pFromWString(len);

		// Copy source-file into the struct and apply the extra terminator.
		errno_t copyRet = wcscpy_s(&pFromWString[0], len, in.c_str());
		if (copyRet != 0) DO_THROW(Err::CriticalError, "Couldn't copy string to buffer.");

		pFromWString[len - 1] = 0;	

		return pFromWString;
	}

	bool performDeleteRecycle(const std::string& file, bool doRecycle, HWND hwnd) {
		SHFILEOPSTRUCTW sfop;
		ZeroMemory(&sfop, sizeof(sfop));

		auto pFromWString = ToTerminatedWcharArray(file);
		sfop.pFrom = &pFromWString[0];
		sfop.wFunc = FO_DELETE;
		sfop.fFlags = FOF_SILENT;

		if(hwnd == 0) {
			sfop.fFlags |= FOF_NOCONFIRMATION;
		}

		if (doRecycle) {
			sfop.fFlags |= FOF_ALLOWUNDO;
		}

		sfop.hwnd = hwnd;

		std::lock_guard<std::mutex> l(g_mutexSHFileOperation);
		return (SHFileOperationW(&sfop) == 0) && (sfop.fAnyOperationsAborted == false);
	}

	bool FileDelete(const std::string& file, HWND hwnd) {
		return performDeleteRecycle(file, false, hwnd);
	}

	bool FileRecycle(const std::string& file, HWND hwnd) {
		return performDeleteRecycle(file, true, hwnd);
	}

	std::string Rename(const std::string& old_name, const std::string& new_name, HWND hwnd) {
		SHFILEOPSTRUCTW sfop;
		ZeroMemory(&sfop, sizeof(sfop));

		auto pOldName = ToTerminatedWcharArray(old_name);
		auto pNewName = ToTerminatedWcharArray(new_name);
		sfop.pFrom = &pOldName[0];
		sfop.pTo = &pNewName[0];
		sfop.wFunc = FO_RENAME;
		sfop.fFlags = FOF_SILENT | FOF_ALLOWUNDO | FOF_WANTMAPPINGHANDLE;
		sfop.hwnd = hwnd;

		if(hwnd == 0)
			sfop.fFlags |= FOF_NOCONFIRMATION;

		//		int ticket = pause_folders(GetPath(old_name));

		bool toRet;
		{
			std::lock_guard<std::mutex> l(g_mutexSHFileOperation);
			toRet = (SHFileOperationW(&sfop) == 0) && (sfop.fAnyOperationsAborted == false);
		}

		auto resulting_name = new_name;

		HANDLETOMAPPINGS* s = reinterpret_cast<HANDLETOMAPPINGS*>(sfop.hNameMappings);
		if(s && s->uNumberOfMappings == 1) {
			resulting_name = WStringToUTF8(s->lpSHNameMapping[0].pszNewPath);
		}

		SHFreeNameMappings(sfop.hNameMappings);

		if (!toRet) {
			return "";
		}

		return resulting_name;
	}

	bool SupportRecycle() {
		return true;
	}
#endif

	std::string GetPath(const std::string& s) {
#ifdef _WIN32
		auto wpath = UTF8ToWString(s);
		auto parent = fs::path(wpath).parent_path().wstring();
		if (!parent.empty() && parent.back() != L'\\') {
			parent += L'\\';
		}
		return WStringToUTF8(parent);
#else
		return fs::path(s).parent_path().string() + "/";
#endif
	}

	std::string GetFile(const std::string& s) {
		return WStringToUTF8(fs::path(UTF8ToWString(s)).filename().wstring());
	}

	std::string GetTitle(const std::string& s) {
		return WStringToUTF8(fs::path(UTF8ToWString(s)).stem().wstring());
	}

	std::string GetExtension(const std::string& s) {
		auto ext = WStringToUTF8(fs::path(UTF8ToWString(s)).extension().wstring());
		if (ext.length() > 0 && ext[0] == '.') {
			return ext.substr(1);
		}
		return ext;
	}
}
