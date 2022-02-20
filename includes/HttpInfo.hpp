#ifndef HTTPINFO_HPP
#define HTTPINFO_HPP

#include "HttpHeader.hpp"
#include "HttpRequest.hpp"
#include "HttpString.hpp"
#include <map>
#include <string>
#include <vector>

struct HttpInfo
{
	std::string method;
	std::string uri;
	std::string script_name;
	std::string query_string;
	std::string protocol_version;
	HttpHeader headers;
	HttpString body;
	Server* server;
	Location* location;
};

#endif /* HTTPINFO_CPP */