#pragma once

#include "ThreadPool.hpp"

#include <string>
#include <memory>

using std::unique_ptr;
using std::string;

class WebServer
{
public:
	virtual ~WebServer() = default;
	WebServer() = default;

	void start(const int& server_port, const int& server_backlog, const string& server_path);
private:
	unique_ptr<ThreadPool> m_thread_pool;

	int m_server_socket;
	int m_server_epoll;

	static void worker_thread(const int& client_socket, const int& epoll_fd);
};