#include "CgiServer.hpp"
#include "Utility.hpp"

#include <vector>

using std::vector;

string CgiServer::execute(string_view file_path, const HttpRequest& http_request, const HttpResponse& http_response)
{
	// Construct str vector
	vector<string> envp_buf;
	envp_buf.push_back("REQUEST_METHOD=" + http_request.method);

	// Construct the envp data
	char **envp= new char*[envp_buf.size()];
	for (int i = 0;i < envp_buf.size();++i) envp[i] = envp_buf[i].data();

	// Read output
	string result;
	char result_buf[128];
	FILE* file = Utility::popen(const_cast<char*>(file_path.data()), "r", envp);
	while(fgets(result_buf, 128, file)) result.append(result_buf);

	pclose(file);
	return ResponseCoder::process(http_response) + result;
}

