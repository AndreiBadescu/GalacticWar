#include "Utility/ErrorHandler.hpp"
#include <iostream>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <cassert>

#define BRIEF_FILENAME "logs/ErrorLog.txt"
#define EXTENDED_FILENAME "logs/FullLog.txt"

#define MAX 100

namespace err {
	void Logger::WriteMessage(const char* const message) {
		Get().WriteLine(message, '+', false);
		++Get().m_messages;
	}

	void Logger::WriteMessage(const char* const message, int error_code) {
		assert(strlen(message) < 100 && "Logger: Length of the message exceeds 100");
		char new_msg[MAX];
		sprintf_s(new_msg, MAX, message, error_code);
		Get().WriteMessage(new_msg);
	}

	void Logger::WriteMessage(const char* const message, const char* const info) {
		assert(strlen(message) < 100 && "Logger: Length of the message exceeds 100");
		char new_msg[MAX];
		sprintf_s(new_msg, MAX, message, info);
		Get().WriteMessage(new_msg);
	}

	void Logger::WriteWarning(const char* const message) {
		Get().WriteLine(message, 'w', true);
		++Get().m_warnings;
	}

	void Logger::WriteWarning(const char* const message, int error_code) {
		assert(strlen(message) < 100 && "Logger: Length of the message exceeds 100");
		char new_msg[MAX];
		sprintf_s(new_msg, MAX, message, error_code);
		Get().WriteWarning(new_msg);
	}

	void Logger::WriteWarning(const char* const message, const char* const info) {
		assert(strlen(message) < 100 && "Logger: Length of the message exceeds 100");
		char new_msg[MAX];
		sprintf_s(new_msg, MAX, message, info);
		Get().WriteWarning(new_msg);
	}

	void Logger::WriteError(const char* const message) {
		Get().WriteLine(message, '-', true);
		++Get().m_errors;
	}

	void Logger::WriteError(const char* const message, int error_code) {
		assert(strlen(message) < 100 && "Logger: Length of the message exceeds 100");
		char new_msg[MAX];
		sprintf_s(new_msg, MAX, message, error_code);
		Get().WriteError(new_msg);
	}

	void Logger::WriteError(const char* const message, const char* const info) {
		assert(strlen(message) < 100 && "Logger: Length of the message exceeds 100");
		char new_msg[MAX];
		sprintf_s(new_msg, MAX, message, info);
		Get().WriteError(new_msg);
	}

	void Logger::WriteFatal(const char* const message) {
		Get().WriteLine(message, '!', true);
		++Get().m_errors;
		exit(EXIT_FAILURE);
	}

	void Logger::WriteFatal(const char* const message, int error_code) {
		assert(strlen(message) < 100 && "Logger: Length of the message exceeds 100");
		char new_msg[MAX];
		sprintf_s(new_msg, MAX, message, error_code);
		Get().WriteFatal(new_msg);
	}

	void Logger::WriteFatal(const char* const message, const char* const info) {
		assert(strlen(message) < 100 && "Logger: Length of the message exceeds 100");
		char new_msg[MAX];
		sprintf_s(new_msg, MAX, message, info);
		Get().WriteFatal(new_msg);
	}

	void Logger::PrintInConsole(const bool turn_on) {
		Get().m_console = turn_on;
	}

	void Logger::SetOutputFile(const char* const filename) {
		Get().extended_filename = filename;
	}

	void Logger::ClearLogFiles() {
		Get().CloseLogFiles();
		Get().brief_output_file.open(Get().brief_filename, std::fstream::out);
		Get().extended_output_file.open(Get().extended_filename, std::fstream::out);
		Get().CloseLogFiles();
	}

	void Logger::OpenLogFiles() {
		brief_output_file.open(brief_filename, std::fstream::out | std::fstream::app);
		if (!brief_output_file) {
			if (m_console) {
				std::cout << "[-] Can't open file: \"" << brief_filename << "\"\n";
			}
		}
		extended_output_file.open(extended_filename, std::fstream::out | std::fstream::app);
		if (!extended_output_file) {
			if (m_console) {
				std::cout << "[-] Can't open file: \"" << extended_filename << "\"\n";
			}
			brief_output_file << "[-] Can't open file: \"" << extended_filename << "\"\n";
		}
	}

	void Logger::CloseLogFiles() {
		brief_output_file.close();
		extended_output_file.close();
	}

	void Logger::GetTimeAndDate() {
		time_t t = time(NULL);
		tm tm;
		localtime_s(&tm, &t);
		sprintf_s(curr_time, sizeof(curr_time), "%02d/%02d/%04d %02d:%02d:%02d ",
				  tm.tm_mday, 1 + tm.tm_mon, 1900 + tm.tm_year,
				  tm.tm_hour, tm.tm_min, tm.tm_sec);
	}

	void Logger::WriteLine(const char* const message, const char symbol, const bool both) {
		GetTimeAndDate();
		if (m_console) { std::cout << curr_time << "[" << symbol << "] " << message << '\n'; }
		OpenLogFiles();
		extended_output_file << curr_time << "[" << symbol << "] " << message << '\n';
		if (both) { brief_output_file << curr_time << "[" << symbol << "] " << message << '\n'; }
		CloseLogFiles();
	}

	Logger& Logger::Get() {
		static Logger unique_instance;
		return unique_instance;
	}

	Logger::Logger() {
#ifdef NDEBUG
		m_console = false;
#else
		m_console = true;
#endif
		m_messages = 0;
		m_warnings = 0;
		m_errors = 0;

		brief_filename = BRIEF_FILENAME;
		extended_filename = EXTENDED_FILENAME;

		if (!std::filesystem::is_directory("logs")) {
			std::filesystem::create_directory("logs");
		}

		WriteLine("NEW SESSION STARTED", 'i', true);
	}

	Logger::~Logger() {
		OpenLogFiles();
		// extended
		extended_output_file << "TOTAL MESSAGES: " << m_messages << '\n';
		extended_output_file << "TOTAL WARNINGS: " << m_warnings << '\n';
		extended_output_file << "TOTAL ERRORS  : " << m_errors << "\n\n";
		// brief
		brief_output_file << "TOTAL WARNINGS: " << m_warnings << '\n';
		brief_output_file << "TOTAL ERRORS  : " << m_errors << "\n\n";
		CloseLogFiles();
	}
}