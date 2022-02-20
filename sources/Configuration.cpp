#include "Configuration.hpp"
#include "Parser.hpp"

Configuration::Configuration(void) { }

Configuration::Configuration(std::string file)
{
	_parse_config(file);
	_complete_config();
}

Configuration& Configuration::operator=(const Configuration& other)
{
	this->_servers = other._servers;
	return (*this);
}

Configuration::~Configuration(void) { }

/**
 * @brief 設定ファイルをパースする
 * @param file: 設定ファイルの絶対パス
 */
void Configuration::_parse_config(std::string file)
{
	std::string file_content;
	size_t i;
	size_t size;
	std::vector< std::string > line;

	i = 0;
	file_content = read_file(file);
	size = count_lines(file_content);
	while(i < size)
	{
		if(!is_skippable(file_content, i))
		{
			line = split_white_space(get_line(file_content, i));
			if(line.size() > 0 && line[0] == "server")
			{
				_parse_server(file_content, i, get_closing_bracket(file_content, i));
				i = get_closing_bracket(file_content, i);
			}
			else
				throw ParsingException(i, "Unexpected token '" + line[0] + "'.");
		}
		++i;
	}
	_validate_config();
}

/**
 * @brief server構造体を配列に追加する
 * @param source: 設定ファイルの文字列
 * @param line_start: 設定ファイルの開始位置, aka "server {"
 * @param line_end: 設定ファイルの終了位置, aka "}"
 */
void Configuration::_parse_server(std::string source, size_t line_start, size_t line_end)
{
	Server s;
	std::vector< std::string > line;

	s = _default_server();
	for(size_t n = line_start + 1; n < line_end; ++n)
	{
		if(!is_skippable(source, n))
		{
			std::vector< std::string > words = split_white_space(get_line(source, n));
			if(words.size() > 0 && words[0] == "location")
			{
				s.locations.push_back(_parse_location(source, n, get_closing_bracket(source, n)));
				n = get_closing_bracket(source, n);
			}
			else
				_parse_server_property(source, n, s);
		}
	}
	_servers.push_back(s);
}

/**
 * @brief 設定ファイルからserverのパラメーターを取得し設定する
 * @param source: 設定ファイルの文字列
 * @param n: server contextの開始位置
 * @param s: server構造体
 */
void Configuration::_parse_server_property(std::string source, size_t n, Server& s)
{
	std::vector< std::string > line;

	line = parse_property(source, n, "server");
	if(line[0] == server_properties[0])
	{
		if(line.size() != 3)
			throw ParsingException(n, std::string(server_properties[0]) + " <port> <host>;");
		s.port = convert_to_size_t(line[1], n);
		s.host = line[2];
	}
	if(line[0] == server_properties[1])
	{
		for(size_t i = 1; i < line.size(); ++i)
			s.names.push_back(line[i]);
	}
	if(line[0] == server_properties[2])
	{
		if(line.size() != 3)
			throw ParsingException(n, std::string(server_properties[2]) + "<code> <file>;");
		s.error_pages[convert_to_size_t(line[1], n)] = line[2];
	}
	if(line[0] == server_properties[3])
	{
		s.root = line[1];
	}
}

/**
 * @brief Location構造体を返却する
 * @param source: 設定ファイルの文字列
 * @param location: contextの開始位置, aka "location / {"
 * @param line_end: location contextの終了位置, aka "}"
 * @return location構造体
 */
Location Configuration::_parse_location(std::string source, size_t line_start, size_t line_end)
{
	Location loc;
	std::vector< std::string > line;

	loc = _default_location();
	line = split_white_space(get_line(source, line_start));
	if(line.size() != 3)
		throw ParsingException(line_start, "Location should have a name.");
	loc.name = line[1];
	for(size_t n = line_start + 1; n < line_end; ++n)
	{
		if(!is_skippable(source, n))
			_parse_location_property(source, n, loc);
	}
	return (loc);
}

/**
 * @brief redirect directiveのstatusがただし以下チェック
 */

bool Configuration::check_redirect_status(std::string status)
{
	if(status == "301" || status == "302" || status == "303" || status == "307" || status == "308")
		return true;
	else
		return false;
}

/**
 * @brief 設定ファイルからLocationのパラメーターを取得し設定する
 * @param source: 設定ファイルの文字列
 * @param n: Location contextの開始位置
 * @param l: Location構造体
 */
void Configuration::_parse_location_property(std::string source, size_t n, Location& l)
{
	std::vector< std::string > line;
	char last;

	line = parse_property(source, n, "route");
	if(line[0] == route_properties[0])
	{
		for(size_t i = 1; i < line.size(); ++i)
		{
			if(!is_method(line[i]))
				throw ParsingException(n, "'" + line[i] + "' is not a valid method.");
			else
				l.methods.push_back(line[i]);
		}
	}
	if(line[0] == route_properties[1])
		l.root = line[1];
	if(line[0] == route_properties[2])
		l.autoindex = param_to_bool(line[1], n);
	if(line[0] == route_properties[3])
	{
		if(line.size() < 2)
			throw ParsingException(n, std::string(route_properties[3]) + "index is empty");
		l.index.erase(l.index.begin());
		for(size_t i = 1; i < line.size(); i++)
			l.index.push_back(line[i]);
	}
	if(line[0] == route_properties[4])
		l.cgi_path = line[1];
	if(line[0] == route_properties[5])
		l.upload_enable = param_to_bool(line[1], n);
	if(line[0] == route_properties[6])
		l.upload_path = line[1];
	if(line[0] == route_properties[7])
	{
		if(line.size() != 2)
			throw ParsingException(n, std::string(route_properties[7]) + " <size[K,M,G]>;");
		l.client_max_body_size = convert_to_size_t(line[1], n);
		last = line[1][line[1].size() - 1];
		if(last == 'K' || last == 'k')
			l.client_max_body_size *= 1024;
		else if(last == 'M' || last == 'm')
			l.client_max_body_size *= 1024 * 1024;
		else if(last == 'G' || last == 'G')
			l.client_max_body_size *= 1024 * 1024 * 1024;
		else if(!std::isdigit(last))
			throw ParsingException(n, std::string(route_properties[7]) + " <size[K,M,G]>;");
	}
	if(line[0] == route_properties[8])
	{
		if((line.size() != 3) || !check_redirect_status(line[1]))
			throw ParsingException(n, std::string(route_properties[8]) + " <status: 3xx> <URL>;");
		if(l.redirect.size() != 0)
			throw ParsingException(n,
								   std::string(route_properties[8]) + " multiple return directive");
		l.redirect[convert_to_size_t(line[1], n)] = line[2];
	}
}

/**
 * @brief server構造体の情報を出力する、デバッグ用
 */
void Configuration::show_server_info(void)
{
	std::map< int, std::string >::iterator it;
	std::vector< Location >::iterator it2;
	std::map< int, std::string >::iterator it3;

	for(size_t i = 0; i < _servers.size(); i++)
	{
		std::cout << "- Server" << std::endl;
		std::cout << "   * server_name: ";
		for(size_t j = 0; j < _servers[i].names.size(); ++j)
			std::cout << _servers[i].names[j] << " ";
		std::cout << std::endl;
		std::cout << "   * host: " + _servers[i].host << std::endl;
		std::cout << "   * port: " + utility::to_string(_servers[i].port) << std::endl;
		std::cout << "   * root: " + _servers[i].root << std::endl;
		it = _servers[i].error_pages.begin();
		while(it != _servers[i].error_pages.end())
		{
			std::cout << "   * error_page for " + utility::to_string(it->first) + ": " + it->second
					  << std::endl;
			++it;
		}
		it2 = _servers[i].locations.begin();
		while(it2 != _servers[i].locations.end())
		{
			std::cout << "   - Location " + it2->name << std::endl;
			std::cout << "     * methods: ";
			for(size_t j = 0; j < it2->methods.size(); ++j)
				std::cout << it2->methods[j] + " ";
			std::cout << std::endl;
			std::cout << "     * index: ";
			for(size_t j = 0; j < it2->index.size(); ++j)
				std::cout << it2->index[j] + " ";
			std::cout << std::endl;
			std::cout << "     * root: " << it2->root << std::endl;
			std::cout << "     * cgi_path: " << it2->cgi_path << std::endl;
			std::cout << "     * autoindex: " << it2->autoindex << std::endl;
			std::cout << "     * upload_enable: " << it2->upload_enable << std::endl;
			std::cout << "     * upload_path: " << it2->upload_path << std::endl;
			std::cout << "     * client_max_body_size: " +
							 utility::to_string(it2->client_max_body_size)
					  << std::endl;
			it3 = it2->redirect.begin();
			while(it3 != it2->redirect.end())
			{
				std::cout << "     * redirect for " + utility::to_string(it3->first) + " -> " +
								 it3->second
						  << std::endl;
				++it3;
			}
			++it2;
		}
	}
};

/**
 * @return server構造体の配列
 */
std::vector< Server > Configuration::get_servers(void)
{
	return (_servers);
}

/**
 * @brief 設定ファイルが正しいものかチェックする
 */
void Configuration::_validate_config(void)
{
	if(_servers.size() == 0)
		throw ParsingException(0, "Your configuration file must provide at least one server.");
	for(size_t i = 0; i < _servers.size(); ++i)
	{
		for(size_t j = 0; j < _servers[i].locations.size(); ++j)
		{
			if(_servers[i].locations[j].cgi_path.size() > 0 &&
			   !check_path(_servers[i].locations[j].cgi_path))
				throw ParsingException(0,
									   "The cgi path '" + _servers[i].locations[j].cgi_path +
										   "' is not a valid file.");
		}
	}
	for(size_t i = 0; i < _servers.size(); ++i)
	{
		for(size_t j = 0; j < _servers.size(); ++j)
		{
			if(i != j)
			{
				if(_servers[i].host == "127.0.0.1")
					_servers[i].host = "localhost";
				if(_servers[j].host == "127.0.0.1")
					_servers[j].host = "localhost";
				//if(_servers[i].host == _servers[j].host && _servers[i].port == _servers[j].port)
				//throw ParsingException(0, "Two servers have the same host and port.");
			}
		}
	}
}

/**
 * @brief デフォルトのsever構造体を返す
 */
Server Configuration::_default_server(void)
{
	Server s;

	s.names.push_back("");
	s.port = 8080;
	s.host = "127.0.0.1";
	s.root = webserv::DEFAULT_ROOT;
	return (s);
}

/**
 * @brief デフォルトのLocation構造体を返す
 */
Location Configuration::_default_location(void)
{
	Location l;

	l.name = "/";
	l.root = "";
	l.index.push_back("index.html");
	l.autoindex = false;
	l.cgi_path = "";
	l.upload_enable = false;
	l.upload_path = "./test/upload/";
	l.client_max_body_size = 1 * 1024 * 1024;
	return (l);
}

/**
 * @brief 設定ファイルに必要なparamが足りていない場合、補完する
 */
void Configuration::_complete_config(void)
{
	for(size_t i = 0; i < _servers.size(); ++i)
	{
		if(_servers[i].locations.size() == 0)
			_servers[i].locations.push_back(_default_location());
		for(size_t j = 0; j < _servers[i].locations.size(); ++j)
		{
			if(_servers[i].locations[j].methods.size() == 0)
				_servers[i].locations[j].methods.push_back("GET");
			if(_servers[i].locations[j].root.size() == 0)
				_servers[i].locations[j].root = _servers[i].root;
		}
	}
}
