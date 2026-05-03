#include "logger.h"

#include "types.h"

#include <vector>
#include <sstream>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <debugapi.h>
#endif

namespace IO {
	void Logger::SetOutput(const std::string& filename) {
		m_filename = filename;
		m_wroteBOM = false;
		AttemptInitialize();
	}

	void Logger::Write(const std::string& message) {
		// TODO: Work better with the << operator. Currently a single message can get split into multiple rows.
		m_dbgCached += message;

		// Replace \r\n with \n
		std::string::size_type pos = 0;
		while ((pos = m_dbgCached.find("\r\n", pos)) != std::string::npos) {
			m_dbgCached.replace(pos, 2, "\n");
		}

		// Split by \n
		std::vector<std::string> lines;
		std::istringstream stream(m_dbgCached);
		std::string line;
		while (std::getline(stream, line)) {
			lines.push_back(line);
		}

		if (!lines.empty()) {
			m_dbgCached = lines.back();
			lines.pop_back();
			for (const auto& l : lines) {
				auto u8str = "Pictus: " + l + "\r\n";
#ifdef _WIN32
				OutputDebugStringW(UTF8ToWString(u8str.c_str()).c_str());
#endif
			}
		}

		AttemptInitialize();
		IO::FileWriter f;
		if (!f.Open(m_filename.c_str(), true)) {
			return;  // Prefer losing logging messages than raising an error.
		}
		f.Write(message.c_str(), 1, message.length());
	}

	void Logger::AttemptInitialize() {
		if (m_filename.empty()) {
			return;
		}

		if (m_wroteBOM) {
			return;
		}

		IO::FileWriter f;
		if (f.Open(m_filename.c_str()) == false) {
			return;
		}

		const unsigned char bof[3] = {0xef, 0xbb, 0xbf};
		f.Write(bof, 1, 3);
		m_wroteBOM = true;
	}

	Logger::~Logger() {
#ifdef _WIN32
		if (!m_dbgCached.empty())
		{
			OutputDebugStringW(UTF8ToWString(("Pictus: " + m_dbgCached).c_str()).c_str());
		}
#endif
	}

	std::string Internal::Cleanup(const std::string& input) {
		std::string output(input);

		size_t pos = 0;
		while((pos = output.find("\n", pos)) != std::string::npos) {
			output.insert(pos, "\r");
			pos += 2;
		}
		return output;
	}
}

IO::Logger Log;
