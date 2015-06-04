#include <errno.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "server.h"

/*
 *  初始化服务器, 带重试
 * */
int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen)
{
	int fd, err ;
	int reuse = 1;

	if ((fd = socket(addr->sa_family, type, 0)) < 0)
		return (-1) ;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse,
	  sizeof(int)) < 0)
		goto errout ;
	if (bind(fd, addr, alen) < 0)
		goto errout ;
	if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
		if (listen(fd, qlen) < 0)
			goto errout ;
	return(fd) ;

errout :
	err = errno ;
	close(fd) ;
	errno = err ;
	return (-1) ;
}


