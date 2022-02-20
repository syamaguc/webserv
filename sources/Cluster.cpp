#include "Cluster.hpp"

Cluster::Cluster(Configuration config)
	: _max_fd(0)
{
	std::vector< Server >& servers = config._servers;
	for(size_t i = 0; i < servers.size(); i++)
	{
		socket_itr it = _check_virtual(servers[i]);
		if(it != _sockets.end())
		{
			it->add_server(servers[i]);
		}
		else
		{
			Socket sc(servers[i]);
			sc.open_listenfd();
			sc.add_server(servers[i]);
			_sockets.push_back(sc);
		}
	}
}

Cluster::~Cluster() { }

Cluster::socket_itr Cluster::_check_virtual(Server s)
{
	socket_itr s_it_end = _sockets.end();
	for(socket_itr s_it = _sockets.begin(); s_it != s_it_end; ++s_it)
	{
		if((s_it->get_port() == s.port) && (s_it->get_host() == s.host))
			return s_it;
	}
	return s_it_end;
}

/**
 * @brief _max_fdのセットを行う
 */
void Cluster::_set_max_fd()
{
	_max_fd = 2;
	int fd;
	socket_itr s_it_end = _sockets.end();
	for(socket_itr s_it = _sockets.begin(); s_it != s_it_end; ++s_it)
	{
		fd = s_it->get_lfd();
		_max_fd = _max_fd > fd ? _max_fd : fd;
		Socket::connection_citr c_it = s_it->get_connections().begin();
		Socket::connection_citr c_it_end = s_it->get_connections().end();
		for(; c_it != c_it_end; ++c_it)
		{
			enum phase phase = c_it->get_phase();
			const HttpResponse* response = c_it->get_response();
			if(response == NULL || phase == SEND)
			{
				fd = c_it->get_sfd();
			}
			else
			{
				fd = response->get_fd();
			}
			_max_fd = _max_fd > fd ? _max_fd : fd;
		}
	}
}

void Cluster::_set_select_fd()
{
	_max_fd = 2;
	FD_ZERO(&_write_set);
	FD_ZERO(&_read_set);
	socket_itr s_it_end = _sockets.end();
	for(socket_itr s_it = _sockets.begin(); s_it != s_it_end; ++s_it)
	{
		s_it->set_select_fd(_read_set, _write_set, _max_fd);
	}
}

void Cluster::loop()
{
	int ret = 0;
	while(42)
	{
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		_set_select_fd();
		Log(utility::to_string(_max_fd));
		ret = select(_max_fd + 1, &_read_set, &_write_set, NULL, &timeout);
		if(ret < 0)
		{
			Log("Select Error");
			break;
		}
		socket_itr s_it_end = _sockets.end();
		for(socket_itr s_it = _sockets.begin(); s_it != s_it_end; ++s_it)
		{
			s_it->check_and_handle(_read_set, _write_set);
			usleep(_usleep_time);
		}
		ret = 0;
	} //main loop end
}
