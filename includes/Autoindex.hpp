#ifndef AUTOINDEX_HPP
#define AUTOINDEX_HPP

#include <dirent.h>
#include <string>
#include <vector>

class Autoindex
{
public:
	Autoindex();
	~Autoindex();
	static std::string generate_body(std::string script_name, std::string dirpath);

private:
	static std::string _generate_html_body(std::string uri,
										   std::vector< std::string > const& entries);
	static std::vector< std::string > _get_file_entries(DIR* d);
	static DIR* _open_directory(std::string dirpath);

	Autoindex(Autoindex const& other);
	Autoindex& operator=(Autoindex const& other);
};

#endif /* AUTOINDEX_HPP */