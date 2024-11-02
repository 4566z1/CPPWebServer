#pragma once
#include <cstdio>

namespace Utility{
	void epoll_add(const int& epoll_fd, const int& fd, unsigned int events, const bool& oneshot);
	void epoll_reset_oneshot(const int& epoll_fd, const int& fd, unsigned int events);
	void fd_set_nonblock(const int& fd);
	FILE* popen(char* cmdstring, const char* type, char **envp);
}
