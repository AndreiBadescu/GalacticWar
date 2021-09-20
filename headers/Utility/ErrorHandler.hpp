#pragma once
#include <fstream>
#include <string>
#include <cstdint>

#define LOG_MESSAGE(...)	err::Logger::WriteMessage(__VA_ARGS__)
#define LOG_WARNING(...)	err::Logger::WriteWarning(__VA_ARGS__)
#define LOG_ERROR(...)		err::Logger::WriteError(__VA_ARGS__)
#define LOG_FATAL(...)		err::Logger::WriteFatal(__VA_ARGS__)
#define LOG_IN_CONSOLE(...) err::Logger::PrintInConsole(__VA_ARGS__)
#define LOG_TO_FILE(...)	err::Logger::SetOutputFile(__VA_ARGS__)

namespace err {

	// Singleton
	class Logger {
	public:
		static void WriteMessage(const char* const message);
		static void WriteMessage(const char* const message, int error_code);
		static void WriteMessage(const char* const message, const char* const info);

		static void WriteWarning(const char* const message);
		static void WriteWarning(const char* const message, int error_code);
		static void WriteWarning(const char* const message, const char* const info);

		static void WriteError(const char* const message);
		static void WriteError(const char* const message, int error_code);
		static void WriteError(const char* const message, const char* const info);

		static void WriteFatal(const char* const message);
		static void WriteFatal(const char* const message, int error_code);
		static void WriteFatal(const char* const message, const char* const info);

		static void PrintInConsole(const bool turn_on);
		static void SetOutputFile(const char* const filename);
		static void ClearLogFiles();

	private:
		Logger();
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;
		~Logger();

		static inline Logger& Get();
		void OpenLogFiles();
		void CloseLogFiles();
		void GetTimeAndDate();
		void WriteLine(const char* const message, const char symbol, const bool both);

		int32_t m_messages;
		int32_t m_warnings;
		int32_t m_errors;
		bool m_console;
		std::string brief_filename;
		std::string extended_filename;
		std::ofstream brief_output_file;
		std::ofstream extended_output_file;
		char curr_time[32];
	};
}