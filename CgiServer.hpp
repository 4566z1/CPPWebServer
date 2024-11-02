#pragma once

#include "RequestParser.hpp"
#include "ResponseCoder.hpp"

#include <string>
using std::string_view;

class CgiServer
{
public:
	virtual ~CgiServer() = default;
	CgiServer() = default;

	static string execute(string_view file_path, const HttpRequest& http_request, const HttpResponse& http_response);
};