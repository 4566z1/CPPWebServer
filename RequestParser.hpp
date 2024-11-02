#pragma once

#include <string>
using std::string;

// 12
struct HttpRequest
{
	string method;
	string path;
	string http_version;
	string host;
	string user_agent;
	string accept;
	string accept_language;
	string accept_encoding;
	string referer;
	string dnt;
	string connection;
	string cookie;
};

class RequestParser
{
public:
	virtual ~RequestParser() = default;
	RequestParser() = default;

	bool process(const char* http_buffer, const int& http_buffer_size);
	
	[[nodiscard]] const HttpRequest& GetHttpRequest() const { return m_http_request; }
private:
	enum HTTP_PROCESS_STATUS
	{
		HTTP_HEADER1,
		HTTP_HEADER2,
		HTTP_DATA,
		HTTP_ERROR,
		HTTP_OK,
	};

	string m_saved_keyword;
	HttpRequest m_http_request;
	int m_status = HTTP_HEADER1;

	HTTP_PROCESS_STATUS process_header(const char* http_buffer, const int& http_buffer_size);
	//HTTP_PROCESS_STATUS process_body(const char* http_buffer, const int& http_buffer_size);
};