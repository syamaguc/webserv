#ifndef SERVER_HPP
#define SERVER_HPP

#include "WebServ.hpp"
#include <string>
#include <vector>

struct Location
{
	std::string name;
	std::string root;
	std::vector< std::string > methods;
	bool autoindex;
	std::vector< std::string > index;
	std::string cgi_path;
	bool upload_enable;
	std::string upload_path;
	ssize_t client_max_body_size;
	std::map< int, std::string > redirect;
};

struct Server
{
	std::vector< std::string > names;
	std::string name;
	std::string root;
	std::map< int, std::string > error_pages;
	std::vector< Location > locations;
	std::string host;
	size_t port;
};

#endif
