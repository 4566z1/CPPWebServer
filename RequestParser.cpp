#include "RequestParser.hpp"
#include "Logger.hpp"

RequestParser::HTTP_PROCESS_STATUS RequestParser::process_header(const char* http_buffer, const int& http_buffer_size)
{
	string key_word;
	const char* p = http_buffer;
	int k = http_buffer_size;

	// Process the http header part1
	if(m_status == HTTP_HEADER1){
		for(; k >= 0; --k,++p)
		{
			if(*p == ' ' || *p == '\n')
			{
				if(key_word == "GET")
					m_http_request.method = "GET";
				else if(key_word == "POST")
					m_http_request.method = "POST";
				else if(key_word.find("HTTP") != key_word.npos)
					m_http_request.http_version = key_word;
				else {
					// maybe needs some update
					if(key_word == "/") key_word = "index.html";
					m_http_request.path = key_word;
				}

				key_word.clear();
				if(*p == '\n') break;
				continue;
			}
			key_word.push_back(*p);
		}

		// Check the result
		if (m_http_request.method.empty() || m_http_request.http_version.empty() || m_http_request.path.empty())
			return HTTP_ERROR;

		// Skip "\n" symbol
		++p;
	}

	// Process the http header part2
	m_status = HTTP_HEADER2;
	// restore key_word
	key_word = std::move(m_saved_keyword);
	for(; k >= 0; --k,++p)
	{
		if(*p == '\n')
		{
			const unsigned long pos = key_word.find(":");
			if(pos == key_word.npos) return HTTP_DATA;

			if(key_word.substr(0, pos) == "Host")
			{
				m_http_request.host = key_word.substr(pos + 1, key_word.size() - pos);
			}
			else if(key_word.substr(0, pos) == "User-Agent")
			{
				m_http_request.user_agent = key_word.substr(pos + 1, key_word.size() - pos);
			}
			else if(key_word.substr(0, pos) == "Accept")
			{
				m_http_request.accept = key_word.substr(pos + 1, key_word.size() - pos);
			}
			else if(key_word.substr(0, pos) == "Accept-Language")
			{
				m_http_request.accept_language = key_word.substr(pos + 1, key_word.size() - pos);
			}
			else if(key_word.substr(0, pos) == "Accept-Encoding")
			{
				m_http_request.accept_encoding = key_word.substr(pos + 1, key_word.size() - pos);
			}
			else if(key_word.substr(0, pos) == "Referer")
			{
				m_http_request.referer = key_word.substr(pos + 1, key_word.size() - pos);
			}
			else if(key_word.substr(0, pos) == "DNT")
			{
				m_http_request.dnt = key_word.substr(pos + 1, key_word.size() - pos);
			}
			else if(key_word.substr(0, pos) == "Connection")
			{
				m_http_request.connection = key_word.substr(pos + 1, key_word.size() - pos);
			}
			else if(key_word.substr(0, pos) == "Cookie")
			{
				m_http_request.cookie = key_word.substr(pos + 1, key_word.size() - pos);
			}

			key_word.clear();
			continue;
		}

		// Filter special symbols
		if (*p == '\377' || *p == '\177') continue;
		key_word.push_back(*p);
	}

	m_saved_keyword = std::move(key_word);
	return HTTP_HEADER2;
}

bool RequestParser::process(const char* http_buffer, const int& http_buffer_size)
{
	if(m_status == HTTP_HEADER1 || m_status == HTTP_HEADER2)
	{
		m_status = process_header(http_buffer, http_buffer_size);
		if (m_status == HTTP_ERROR) return false;
		if(m_status == HTTP_DATA && m_http_request.method != "POST") m_status = HTTP_OK;
	}
	else if(m_status == HTTP_DATA)
	{
		// There is post data parser
		//m_status = process_body(http_buffer, http_buffer_size);
	}
	return true;
}
