/**
 *    > Author:            UncP
 *    > Mail:         770778010@qq.com
 *    > Github:    https://www.github.com/UncP/Mushroom
 *    > Created Time:  2017-04-23 10:23:53
**/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cassert>
#include <cerrno>

#include "socket.hpp"

namespace Mushroom {

Socket::Socket():fd_(-1) { }

Socket::Socket(int fd):fd_(fd) { }

Socket::~Socket() { }

int Socket::fd() const
{
	return fd_;
}

bool Socket::Valid() const
{
	return fd_ != -1;
}

bool Socket::Create()
{
	fd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	return fd_ != -1;
}

bool Socket::Close()
{
	bool flag = true;
	if (fd_ != -1) {
		flag = !close(fd_);
		fd_ = -1;
	}
	return flag;
}

bool Socket::Connect(const EndPoint &end_point)
{
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family      = AF_INET;
	server.sin_port        = htons(8000);
	server.sin_addr.s_addr = end_point.Address();
	return !connect(fd_, (const struct sockaddr *)&server, sizeof(server));
}

bool Socket::Bind()
{
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	server.sin_family      = AF_INET;
	server.sin_port        = htons(8000);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	return !bind(fd_, (const struct sockaddr *)&server, sizeof(server));
}

bool Socket::Listen()
{
	return !listen(fd_, 32);
}

int Socket::Accept()
{
	struct sockaddr_in client;
	memset(&client, 0, sizeof(client));
	socklen_t len = sizeof(client);
	int fd = accept(fd_, (struct sockaddr *)&client, &len);
	return fd;
}

bool Socket::SetOption(int value, bool flag)
{
	return !setsockopt(fd_, SOL_SOCKET, value, &flag, sizeof(flag));
}

bool Socket::GetOption(int value, int *ret)
{
	socklen_t len = sizeof(*ret);
	return !getsockopt(fd_, SOL_SOCKET, value, ret, &len);
}

bool Socket::GetPeerName(EndPoint *endpoint)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t len = sizeof(addr);
	if (!getsockname(fd_, (struct sockaddr *)&addr, &len)) {
		*endpoint = EndPoint(addr.sin_addr.s_addr);
		return true;
	}
	return false;
}

bool Socket::GetSockName(EndPoint *endpoint)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t len = sizeof(addr);
	if (!getpeername(fd_, (struct sockaddr *)&addr, &len)) {
		*endpoint = EndPoint(addr.sin_addr.s_addr);
		return true;
	}
	return false;
}

bool Socket::AddFlag(int flag)
{
	int value = fcntl(fd_, F_GETFL, 0);
	assert(value != -1);
	return !fcntl(fd_, F_SETFL, value | flag);
}

bool Socket::SetNonBlock()
{
	int value = fcntl(fd_, F_GETFL, 0);
	assert(value != -1);
	return !fcntl(fd_, F_SETFL, value | O_NONBLOCK);
}

bool Socket::operator==(const Socket &that) const
{
	return fd_ == that.fd_;
}

uint32_t Socket::Write(const char *data, uint32_t len)
{
	uint32_t written = 0;
	for (; written < len;) {
		ssize_t r = write(fd_, data + written, len - written);
		if (r == -1 && errno == EINTR)
			continue;
		if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			break;
		if (r == 0)
			break;
		assert(r > 0);
		written += r;
	}
	return written;
}

uint32_t Socket::Read(char *data, uint32_t len)
{
	uint32_t has_read = 0;
	for (;;) {
		ssize_t r = read(fd_, data + has_read, len - has_read);
		if (r == -1 && errno == EINTR)
			continue;
		if (r == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			break;
		if (r == 0)
			break;
		assert(r > 0);
		has_read += r;
	}
	return has_read;
}

} // namespace Mushroom
