#include "Utility.hpp"

#include <fcntl.h>
#include <sys/epoll.h>

#include <stdexcept>
#include <unistd.h>
using std::runtime_error;

#define SHELL "/bin/sh"

static pid_t *childpid = nullptr; 
static int maxfd;

FILE* Utility::popen(char* cmdstring, const char* type, char **envp)
{  
    int     i, pfd[2];  
    pid_t   pid;  
    FILE    *fp;  

            /* only allow "r" or "w" */  
    if ((type[0] != 'r' && type[0] != 'w') || type[1] != 0) {  
        errno = EINVAL;     /* required by POSIX.2 */  
        return(NULL);  
    }  

    if (childpid == NULL) {     /* first time through */  
                /* allocate zeroed out array for child pids */  
        maxfd = 256;  
        if ( (childpid = (pid_t*)calloc(maxfd, sizeof(pid_t))) == NULL)  
            return(NULL);  
    }  

    if (pipe(pfd) < 0)  
        return(NULL);   /* errno set by pipe() */  

    if ( (pid = fork()) < 0)  
        return(NULL);   /* errno set by fork() */  
    else if (pid == 0) {                            /* child */  
        if (*type == 'r') {  
            close(pfd[0]);  
            if (pfd[1] != STDOUT_FILENO) {  
                dup2(pfd[1], STDOUT_FILENO);  
                close(pfd[1]);  
            }  
        } else {  
            close(pfd[1]);  
            if (pfd[0] != STDIN_FILENO) {  
                dup2(pfd[0], STDIN_FILENO);  
                close(pfd[0]);  
            }  
        }  
            /* close all descriptors in childpid[] */  
        for (i = 0; i < maxfd; i++)  
            if (childpid[ i ] > 0)  
                close(i);

    	char* args_str[] = { "sh", "-c", cmdstring,(char*)0 };
        execve(SHELL, args_str, envp);
        _exit(127);  
    }  
                                /* parent */  
    if (*type == 'r') {  
        close(pfd[1]);  
        if ( (fp = fdopen(pfd[0], type)) == NULL)  
            return(NULL);  
    } else {  
        close(pfd[0]);  
        if ( (fp = fdopen(pfd[1], type)) == NULL)  
            return(NULL);  
    }  
    childpid[fileno(fp)] = pid; /* remember child pid for this fd */  
    return(fp);  
}  

void Utility::fd_set_nonblock(const int& fd)
{
	int fl = fcntl(0,F_GETFL);
	if (fl < 0) throw runtime_error("fd_set_nonblock error");
    fcntl(0, F_SETFL, fl | O_NONBLOCK); 
}

void Utility::epoll_add(const int& epoll_fd, const int& fd, unsigned int events, const bool& oneshot)
{
	epoll_event event;
	event.events = !oneshot ? events : (oneshot | EPOLLONESHOT);
	event.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);

}

void Utility::epoll_reset_oneshot(const int& epoll_fd, const int& fd, unsigned int events)
{
	epoll_event event;
	event.events = events | EPOLLONESHOT;
	event.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}