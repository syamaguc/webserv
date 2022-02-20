#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "HttpInfo.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Server.hpp"
#include "Utility.hpp"
#include <vector>

class Connection
{
private:
	//connection用のfd
	int _sfd;
	enum phase _phase;
	HttpInfo _info;
	HttpResponse* _response;
	std::vector< Server >& _servers;
	HttpRequest _request;

	void _make_HttpResponse(int status_code);
	ssize_t _recv();

	static const size_t _max_buffer_size = 1024;

	unsigned char _buf[_max_buffer_size + 1];

	size_t _id;
	static size_t _total_connections;

public:
	Connection(int fd, std::vector< Server >& servers);
	~Connection();
	Connection(Connection const& other);
	Connection& operator=(Connection const& other);

	void recv_request();
	void close() const;
	void set_select_fd(fd_set& read_set, fd_set& write_set, int& max_fd) const;
	bool check_and_handle(fd_set& read_set, fd_set& write_set);

	int get_sfd() const;
	enum phase get_phase() const;
	const HttpResponse* get_response() const;
};

#endif
