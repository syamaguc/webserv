#include "Autoindex.hpp"
#include "Http.hpp"

Autoindex::Autoindex() { }

Autoindex::~Autoindex() { }

std::string Autoindex::generate_body(std::string script_name, std::string dirpath)
{
	DIR* d = _open_directory(dirpath);
	std::vector< std::string > entries = _get_file_entries(d);
	return _generate_html_body(script_name, entries);
}

std::string Autoindex::_generate_html_body(std::string script_name,
										   std::vector< std::string > const& entries)
{
	std::string content;
	content = "<h1>Index of " + script_name;
	content += "</h1><hr>" + utility::CRLF;
	content += "<pre><a href=\"../\">../</a>" + utility::CRLF;

	size_t size = entries.size();
	for(size_t i = 0; i < size; i++)
	{
		content += "<a href=\"";
		content += entries[i];
		content += "\">";
		content += entries[i];
		content += "</a>" + utility::CRLF;
	}
	content += "</pre><hr>";
	return content;
}

std::vector< std::string > Autoindex::_get_file_entries(DIR* d)
{
	std::vector< std::string > entries;
	struct dirent* entry;
	std::string name;
	while((entry = readdir(d)))
	{
		name = entry->d_name;
		if(name[0] == '.')
			continue;
		if(entry->d_type == DT_DIR)
			name += '/';
		entries.push_back(name);
	}
	return entries;
}

DIR* Autoindex::_open_directory(std::string dirpath)
{
	DIR* d = opendir(dirpath.c_str());
	if(!d)
	{
		throw http::StatusException(403);
	}
	return d;
}
