#pragma once

#include <string>
using std::string;
using std::string_view;

struct HttpResponse
{
	string http_version;
	string http_code;
	string server;
	string content_type;
	string content_length;
	string date;
};

class ResponseCoder
{
public:
	virtual  ~ResponseCoder() = default;
	ResponseCoder() = default;

	const static string process(const HttpResponse& http_response);
};