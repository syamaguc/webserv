#ifndef ENVVAR_HPP
#define ENVVAR_HPP

#include <cstring>
#include <map>
#include <string>

class EnvVar
{
private:
	std::map< std::string, std::string > _env;
	char** _c_env;

	void _delete_c_env();

public:
	EnvVar();
	~EnvVar();

	std::string& operator[](std::string key);
	void set_c_env();
	char** get_c_env();

private:
	EnvVar(EnvVar const& other);
	EnvVar& operator=(EnvVar const& other);
};

#endif
