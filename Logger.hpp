#pragma once

#include <string>
using std::string_view;

namespace Logger
{
	void Log(string_view log_msg);
	void Error(string_view error_msg);
}