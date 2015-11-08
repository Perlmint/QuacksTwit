#pragma once
#include <sstream>
#include <chrono>
#include <iomanip>

namespace Quacks
{
	namespace Twit
	{
		inline void ParseTime(std::chrono::system_clock::time_point &t, const std::string &str) {
			std::stringstream ss(str);
			std::tm tm;
			// sample : Sat Oct 24 12:02:30 +0000 2015
			ss >> std::get_time(&tm, "%a %b %d %H:%M:%S %Z %Y");
			t = std::chrono::system_clock::from_time_t(std::mktime(&tm));
		}
	}
}
