#include "Logger.hpp"
#include "WebServer.hpp"
#include "Utility.hpp"
#include "RequestParser.hpp"
#include "ResponseCoder.hpp"

#include <sys/stat.h>
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <cstring>
#include <vector>
#include <stdexcept>

using std::vector;

void WebServer::worker_thread(const int& client_socket, const int& epoll_fd)
{
	RequestParser request_parser;
	HttpResponse http_response;
	string http_response_string;

	constexpr int buffer_size = 1024;
	char buffer[buffer_size] = { 0 };
	int file;
	ssize_t bRet;

	// Read data from client
	while(true)
	{
		bRet = recv(client_socket, buffer, buffer_size, MSG_DONTWAIT);
		if (bRet <= 0) break;
		request_parser.process(buffer, buffer_size);
	}

	// Process data
	file = open(request_parser.GetHttpRequest().path.c_str(), O_RDONLY);
	if(file < 0)
	{
		file = open("404.html", O_RDONLY);
		http_response.http_code = "404";
	}
	else
	{
		http_response.http_code = "200";
	}
	http_response.http_version = request_parser.GetHttpRequest().http_version;
	http_response_string = ResponseCoder::process(http_response);

	// Response
	send(client_socket, http_response_string.c_str(), http_response_string.size(), 0);
	if (file > 0)
	{
		struct stat file_stat;
		fstat(file, &file_stat);
		sendfile(client_socket, file, nullptr, file_stat.st_size);
		close(file);
	}

	Utility::epoll_reset_oneshot(epoll_fd, client_socket, EPOLLIN | EPOLLET);
	// There has some issues
	//if (request_parser.GetHttpRequest().connection.find("keep-alive") == string::npos)
	close(client_socket);
}


void WebServer::start(const int& server_port, const int& server_backlog, const string& server_path)
{
	int event_nums, client_socket;
	vector<epoll_event> server_events(server_backlog);
	sockaddr_in server_addr;
	sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	m_thread_pool = std::make_unique<ThreadPool>(server_backlog);

	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	m_server_socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	m_server_epoll = epoll_create(server_backlog);

	if (bind(m_server_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)))
		Logger::Error("WebServer.bind-> " + string(strerror(errno)));

	if(listen(m_server_socket, server_backlog))
		Logger::Error("WebServer.listen-> " + string(strerror(errno)));

	Utility::epoll_add(m_server_epoll, m_server_socket, EPOLLIN | EPOLLET, false);

	Logger::Log("WebServer started successfully");
	while(true)
	{
		event_nums = epoll_wait(m_server_epoll, server_events.data(),
			server_backlog, -1);

		if(event_nums < 0)
		{
			if(errno != EINTR)
				Logger::Error("WebServer.epoll_wait-> " + string(strerror(errno)));
		}
		if(event_nums == 0)
		{
			
		}
		else
		{
			// Events proccesor
			for(int i = 0;i < event_nums;++i)
			{
				if((server_events[i].data.fd == m_server_socket) 
					&& (server_events[i].events & EPOLLIN))
				{
					client_socket = accept(m_server_socket,
						reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
					if(client_socket < 0) Logger::Error("WebServer.accept-> " + string(strerror(errno)));
					Utility::epoll_add(m_server_epoll, client_socket, EPOLLIN | EPOLLET, true);
				}
				else if(server_events[i].events & ( EPOLLHUP | EPOLLERR | EPOLLRDHUP))
				{
					close(server_events[i].data.fd);
				}
				else if(server_events[i].events & EPOLLIN)
				{
					m_thread_pool->join(worker_thread, client_socket, m_server_epoll);
				}
				else
				{
					
				}
			}
		}
	}
}