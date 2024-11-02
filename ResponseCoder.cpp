#include "ResponseCoder.hpp"

#include <sstream>
using std::stringstream;

const string ResponseCoder::process(const HttpResponse& http_response)
{
	stringstream buf;
	buf << http_response.http_version << " " << http_response.http_code << "\n";
	buf << "Date" << ": " << http_response.date << "\n";
	buf << "\n";
	return buf.str();
}