#ifndef SOCKET_HPP
#define SOCKET_HPP

#include "Connection.hpp"
#include "Server.hpp"
#include "WebServ.hpp"
#include <list>
#include <string>
#include <vector>

class Socket
{
private:
	std::string _host;
	size_t _port;
	int _lfd;
	std::list< Connection > _connections;
	std::vector< Server > _servers;

	void _listen() const;
	void _close() const;

public:
	typedef std::list< Connection >::const_iterator connection_citr;
	typedef std::list< Connection >::iterator connection_itr;

	Socket(Server s);
	~Socket();

	const std::string& get_host() const;
	size_t get_port() const;
	int get_lfd() const;
	const std::list< Connection >& get_connections() const;

	void add_server(const Server& server);

	int accept_connection();
	void open_listenfd();
	void close_connection();

	void set_select_fd(fd_set& read_set, fd_set& write_set, int& max_fd) const;
	void check_and_handle(fd_set& read_set, fd_set& write_set);

	class SocketException : public std::exception
	{
	private:
		std::string _msg;

	public:
#if OS_LINUX
		SocketException(std::string funcname = "Socket Function")
			: _msg(funcname + ": " + strerror(errno)){};
#else
		SocketException(std::string funcname = "Socket Function")
			: _msg(funcname + ": " + std::strerror(errno)){};
#endif
		~SocketException() throw(){};
		const char* what() const throw()
		{
			return (_msg.c_str());
		};
	};
};
#endif
