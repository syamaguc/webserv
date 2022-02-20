#ifndef PARSER_HPP
#define PARSER_HPP

#include "Utility.hpp"
#include "WebServ.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 256
extern const char* server_properties[];
extern const char* route_properties[];
extern const char* methods[];

size_t count_lines(std::string source);
size_t get_closing_bracket(std::string source, size_t line);
std::vector< std::string > parse_property(std::string source, size_t line, std::string object);
std::vector< std::string > split_white_space(std::string source);
bool is_property_name(std::string name, const char** valid_names);
std::string read_file(std::string file);
std::string get_line(std::string, size_t line);
bool is_skippable(std::string source, size_t line);
bool end_with_open_bracket(std::string source, size_t line);
size_t convert_to_size_t(std::string param, size_t line);
bool param_to_bool(std::string param, size_t line);
bool is_method(std::string method);
std::string replace(std::string source, std::string to_replace, std::string new_value);
bool check_path(std::string path);

class ParsingException : public std::exception
{
private:
	std::string _msg;

public:
	ParsingException(int line = 0, std::string msg = "Unable to parse the given config file.")
		: _msg("Line: " + utility::to_string(line + 1) + ": " + msg){};
	~ParsingException() throw(){};
	const char* what() const throw()
	{
		return (_msg.c_str());
	};
};

#endif
