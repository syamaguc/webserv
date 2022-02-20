#include "Socket.hpp"
#include "Utility.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <unistd.h> /* close */

Socket::Socket(Server s)
	: _host(s.host)
	, _port(s.port)
{ }

Socket::~Socket() { }

const std::string& Socket::get_host() const
{
	return _host;
}

size_t Socket::get_port() const
{
	return _port;
}

int Socket::get_lfd() const
{
	return _lfd;
}

const std::list< Connection >& Socket::get_connections() const
{
	return _connections;
}

void Socket::add_server(const Server& server)
{
	_servers.push_back(server);
}

/**
 * @brief selectを通して呼ばれる
 * - connectionsに新しいfdのエントリを追加する
 */
int Socket::accept_connection()
{
	int fd;
	try
	{
		if((fd = ::accept(_lfd, NULL, NULL)) == -1)
			throw SocketException("accept");
		if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
			throw SocketException("fcntl");
	}
	catch(const std::exception& e)
	{
		Log(e.what());
		return (-1);
	}
	Connection c(fd, _servers);
	_connections.push_back(c);
	return (fd);
}

/**
 * @brief listenfdをopenして準備し、listen開始する
 */
void Socket::open_listenfd()
{
	struct addrinfo hints, *results, *p;
	::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV | AI_PASSIVE;

	int rc = ::getaddrinfo(_host.c_str(), utility::to_string(_port).c_str(), &hints, &results);
	//int rc = ::getaddrinfo(NULL, utility::to_string(_port).c_str(), &hints, &results);
	if(rc != 0)
		throw SocketException("getaddrinfo");

	int optval = 1;
	for(p = results; p; p = p->ai_next)
	{
		_lfd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(_lfd < 0)
			continue;

		if(::setsockopt(_lfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(int)) < 0)
			throw SocketException("setsockopt");

		if(::bind(_lfd, p->ai_addr, p->ai_addrlen) == 0)
			break;
		_close();
	}
	freeaddrinfo(results);
	if(!p)
		throw SocketException("bind");
	if(fcntl(_lfd, F_SETFL, O_NONBLOCK) == -1)
		throw SocketException("fcntl");
	_listen();
}

/**
 * @brief open_listenfdから呼ばれる
 */
void Socket::_listen() const
{
	if(::listen(_lfd, SOMAXCONN) == -1)
	{
		_close();
		throw SocketException("listen");
	}
}

void Socket::_close() const
{
	if(::close(_lfd) == -1)
		throw SocketException("close");
}

/**
 * @brief selectで必要なパラメータの設定をする
 * @param read_set: read用のset
 * @param write_set: write用のset
 * @param max_fd: fdのmax値
 */
void Socket::set_select_fd(fd_set& read_set, fd_set& write_set, int& max_fd) const
{
	utility::set_fd(_lfd, read_set, max_fd);

	connection_citr it_end = _connections.end();
	for(connection_citr it = _connections.begin(); it != it_end; ++it)
	{
		it->set_select_fd(read_set, write_set, max_fd);
	}
}

/**
 * @brief fdが有効になっているかのチェックとそれに合わせた処理
 * @param read_set: read用のset
 * @param write_set: write用のset
 */
void Socket::check_and_handle(fd_set& read_set, fd_set& write_set)
{
	if(utility::isset_clr_fd(_lfd, read_set))
	{
		accept_connection();
	}
	connection_itr it_end = _connections.end();
	for(connection_itr it = _connections.begin(); it != it_end;)
	{
		if(it->check_and_handle(read_set, write_set))
		{
			it->close();
			it = _connections.erase(it);
		}
		else
			++it;
	}
}
