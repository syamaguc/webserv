#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "Parser.hpp"
#include "Server.hpp"
#include "Utility.hpp"
#include "WebServ.hpp"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class Configuration
{
public:
private:
	void _parse_config(std::string file);
	void _parse_server(std::string source, size_t line_start, size_t line_end);
	void _parse_server_property(std::string source, size_t line, Server& s);
	void _parse_location_property(std::string source, size_t line, Location& l);
	void _validate_config(void);
	void _complete_config(void);
	Server _default_server(void);
	Location _default_location(void);
	Location _parse_location(std::string source, size_t line_start, size_t line_end);
	bool check_redirect_status(std::string);

public:
	std::vector< Server > _servers;
	Configuration(void);
	Configuration(std::string file);
	Configuration& operator=(const Configuration& other);
	~Configuration(void);
	std::vector< Server > get_servers(void);
	void show_server_info(void);
};

#endif
