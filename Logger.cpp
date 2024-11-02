#include "Logger.hpp"

#include <stdexcept>
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

void Logger::Log(string_view log_msg)
{
	cout << "[*] " << log_msg << endl;
}


void Logger::Error(string_view error_msg)
{
	throw std::runtime_error("[!] " + std::string(error_msg));
}