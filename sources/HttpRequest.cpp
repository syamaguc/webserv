#include "HttpRequest.hpp"
#include "Http.hpp"
#include "HttpInfo.hpp"
#include "Utility.hpp"
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

HttpRequest::HttpRequest(std::vector< Server >& servers)
	: HttpMessage(utility::CRLF)
	, _servers(servers)
	, _content_length(0)
	, _body_type(NONE)
{ }

HttpRequest::~HttpRequest() { }

HttpRequest::HttpRequest(HttpRequest const& other)
	: HttpMessage(other)
	, _servers(other._servers)
{
	*this = other;
}

HttpRequest& HttpRequest::operator=(HttpRequest const& other)
{
	if(this != &other)
	{
		_raw_data.buf = other._raw_data.buf;
		_raw_data.index = other._raw_data.index;
		_servers = other._servers;
		_server = other._server;
		_location = other._location;
		_parse_phase = other._parse_phase;
		_method = other._method;
		_uri = other._uri;
		_protocol_version = other._protocol_version;
		_headers = other._headers;
	}
	return *this;
}

/**
 * @brief Connection, recv_requestから呼ばれる
 */
bool HttpRequest::handle_request()
{
	try
	{
		switch(_parse_phase)
		{
		case AWAIT:
			_ignore_empty_lines();
			/* Falls through. */
		case START_LINE:
			_retrieve_startline();
			/* Falls through. */
		case HEADER:
			_retrieve_header();
			/* Falls through. */
		case BODY:
			_retrieve_body();
			_validate_request();
			/* Falls through. */
		default:
			break;
		}
	}
	catch(ReRecvException& e)
	{
		return false;
	}
	show_request();
	return true;
}

void HttpRequest::_ignore_empty_lines()
{
	size_t len = _delim.length();
	size_t size = _raw_data.buf.size();
	size_t index = 0;
	while(index <= size - len && _raw_data.buf.compare(index, len, _delim) == 0)
		index += len;

	if(index == size)
	{
		_raw_data.buf.clear();
		throw ReRecvException();
	}
	_raw_data.buf.erase(index);

	_parse_phase = START_LINE;
}

void HttpRequest::_parse_startline()
{
	size_t eol = _raw_data.buf.find(_delim);
	if(eol == HttpString::npos)
		throw ReRecvException();

	_method = _get_next_word(false, _max_method_len, 501);
	_uri = _get_next_word(false, _max_request_uri_len, 414);
	_protocol_version = _get_next_word(true);
}

void HttpRequest::_validate_startline()
{
	if(_method.empty() || _uri.empty() || _protocol_version.empty())
		throw http::StatusException(400);
	if(!utility::is_upper(_method) || _uri.at(0) != '/' || _protocol_version.compare(0, 5, "HTTP/"))
		throw http::StatusException(400);
	if(_method != "GET" && _method != "POST" && _method != "DELETE")
		throw http::StatusException(405);
	if(utility::trim_ows(_protocol_version).compare("HTTP/1.1"))
		throw http::StatusException(505);
}

/**
 * @brief HttpInfoに必要な値を設定する
 */
void HttpRequest::setup_default_http_info(HttpInfo& info) const
{
	info.method = "GET";
	info.uri = "";
	info.protocol_version = "HTTP/1.1";
	info.headers = HttpHeader();
	info.body = HttpString();
	info.server = NULL;
	info.location = NULL;
	info.script_name = "";
	info.query_string = "";
}

/**
 * @brief HttpInfoに必要な値を設定する
 */
void HttpRequest::setup_http_info(HttpInfo& info) const
{
	info.method = _method;
	info.uri = _uri;
	info.protocol_version = _protocol_version;
	info.headers = _headers;
	info.body = _body;
	info.server = _server;
	info.location = _location;

	size_t index = _uri.find('?');
	if(index != std::string::npos)
	{
		info.script_name = _uri.substr(0, index);
		info.query_string = _uri.substr(index + 1);
	}
	else
		info.script_name = _uri;

	_set_script_name(info);
}

/**
 * @brief uriのlocationのパスをrootに置き換えてscript_nameにセットする
 */
void HttpRequest::_set_script_name(HttpInfo& info)
{
	size_t size = info.location->name.size();
	if(utility::end_with(info.location->name, '/'))
		--size;
	info.script_name.replace(0, size, "");
	if(info.script_name.empty())
		info.script_name = "/";
}

/**
 * @brief ヘッダーフィールドのパース
 * @note 空行までがヘッダーフィールドのため空行がない場合はReRecvする
 */
void HttpRequest::_parse_header()
{
	std::string line;
	std::pair< std::string, std::string > headerfield;
	size_t size = _raw_data.buf.size();

	while(_raw_data.index < size)
	{
		line = _get_next_line(_max_header_fileds_size, 400);
		if(line.empty())
			return;
		if(line.find(":") == std::string::npos)
			continue;
		headerfield = _parse_headerfield(line);
		_validate_headerfield(headerfield);
		_headers.append(headerfield);
	}
	throw ReRecvException();
}

void HttpRequest::_validate_headerfield(std::pair< std::string, std::string >& headerfield)
{
	std::string key = headerfield.first;
	std::string value = headerfield.second;
	if(!http::is_token(key))
		throw http::StatusException(400);
	if(_headers.contains(key))
	{
		if(key == "Host" || key == "Content-Length")
			throw http::StatusException(400);
	}
	if(key == "Host")
	{
		if(value.empty() || http::is_comma_separated_list(value))
			throw http::StatusException(400);
	}
}

void HttpRequest::_validate_header()
{
	if(!_headers.contains("Host"))
		throw http::StatusException(400);
	if(_headers.contains("Content-Length"))
	{
		if(http::is_comma_separated_list(_headers["Content-Length"]))
			throw http::StatusException(400);
		_content_length = _headers.get_content_length();
		if(_content_length == -1)
			throw http::StatusException(400);
		if(_content_length > _location->client_max_body_size)
			throw http::StatusException(413);
		_body_type = CONTENT_LENGTH;
	}
	if(_headers.contains("Transfer-Encoding"))
	{
		if(_headers["Transfer-Encoding"] != "chunked")
			throw http::StatusException(501);
		if(_headers.contains("Content-Length"))
			throw http::StatusException(400);
		_body_type = CHUNKED;
	}
}

/**
 * @brief uriがlocationにマッチするかどうか判定する
 */
bool HttpRequest::_location_matches_uri(std::string uri, std::string location_name)
{
	size_t size = location_name.size();
	if(utility::end_with(location_name, '/'))
	{
		if(uri.compare(0, size, location_name) == 0)
			return true;
	}
	else
	{
		if(uri.compare(0, size + 1, location_name) == 0 ||
		   uri.compare(0, size + 1, location_name + '/') == 0)
			return true;
	}
	return false;
}

/**
 * @brief uriを使ってlocationを選択する
 */
void HttpRequest::_select_location()
{
	_server = &_servers[0];
	_server->name = _servers[0].names[0];

	size_t size = _servers.size();
	for(size_t i = 0; i < size; ++i)
	{
		for(size_t j = 0; j < _servers[i].names.size(); ++j)
		{
			if(_servers[i].names[j].compare(_headers["Host"]) == 0)
			{
				_server = &_servers[i];
				_server->name = _servers[i].names[j];
			}
		}
	}
	_location = &(_server->locations[0]);
	size = _server->locations.size();
	for(size_t i = 0; i < size; ++i)
	{
		if(_location_matches_uri(_uri, _server->locations[i].name))
			_location = &(_server->locations[i]);
	}
}

void HttpRequest::_retrieve_header()
{
	if(_parse_phase != HEADER)
		return;

	_parse_header();
	_select_location();
	_validate_header();

	if(_method == "POST")
		_parse_phase = BODY;
	else
		_parse_phase = DONE;
}

void HttpRequest::_parse_body()
{
	HttpString new_body_chunk;
	bool re_recv = false;

	switch(_body_type)
	{
	case CHUNKED:
		if(_content_length == 0)
		{
			if(_raw_data.buf.find(_delim, _raw_data.index) == HttpString::npos)
				throw ReRecvException();
			_content_length = utility::xstr_to_ssize_t(_get_next_line());
		}
		if(_content_length == -1)
			throw http::StatusException(400);
		if(_content_length > 0)
			re_recv = true;
		/* Falls through. */
	case CONTENT_LENGTH:
		new_body_chunk = _raw_data.buf.substr(_raw_data.index, _content_length);
		_content_length -= new_body_chunk.size();
		_raw_data.index += new_body_chunk.size();
		_body.append(new_body_chunk);
		if(_content_length > 0)
			re_recv = true;
		if(re_recv)
			throw ReRecvException();
	default:
		break;
	}
}

void HttpRequest::_validate_request()
{
	size_t size = _location->methods.size();
	size_t i = 0;
	for(; i < size; ++i)
	{
		if(_location->methods[i] == _method)
			break;
	}
	if(i >= size)
		throw http::StatusException(405);
}

void HttpRequest::append_raw_data(unsigned char* buf, ssize_t len)
{
	_raw_data.buf.append(buf, len);
}

/**
 * @brief HttpInfo構造体の情報を出力する、デバッグ用
 */
void HttpRequest::show_request(void) const
{
#if DEBUG_ACTIVE == 1
	std::cout << " HttpRequest Info" << std::endl;
	std::cout << "  - method: " << _method << std::endl;
	std::cout << "  - uri: '" << _uri << "'" << std::endl;
	_headers.show_headers();
	std::cout << "  - body: '" << _body << "'" << std::endl;
#else
#endif
}
