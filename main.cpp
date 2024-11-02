#include "WebServer.hpp"
#include <iostream>
#include <exception>
using std::exception;

int main()
{
	WebServer webserver;
	try
	{
		webserver.start(80, 10, "\\");
	}catch(const exception& e){
		std::cout << e.what() << std::endl;
	}
	return 0;
}