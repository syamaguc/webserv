#include "HttpMessage.hpp"
#include "Http.hpp"
#include "HttpInfo.hpp"

HttpMessage::HttpMessage()
	: _parse_phase(AWAIT)
{
	_raw_data.index = 0;
}

HttpMessage::HttpMessage(std::string delim)
	: _parse_phase(AWAIT)
	, _delim(delim)
{
	_raw_data.index = 0;
}

HttpMessage::~HttpMessage() { }

HttpMessage::HttpMessage(HttpMessage const& other)
{
	*this = other;
}

HttpMessage& HttpMessage::operator=(HttpMessage const& /*other*/)
{
	return *this;
}

HttpMessage::parse_phase HttpMessage::get_parse_phase() const
{
	return _parse_phase;
}

std::string HttpMessage::_get_next_line(size_t max, int status_code)
{
	return _get_next_word(true, max, status_code);
}

std::string HttpMessage::_get_next_word(bool eol, size_t max, int status_code)
{
	std::string word;
	size_t separator_size = eol ? _delim.size() : 1;
	size_t index;
	if(eol)
		index = _raw_data.buf.find(_delim, _raw_data.index);
	else
		index = _raw_data.buf.find_first_of(" \t", _raw_data.index);
	if(index == HttpString::npos)
		return ""; //for now

	if(index - _raw_data.index > max)
		throw http::StatusException(status_code);

	word = _raw_data.buf.substr(_raw_data.index, index - _raw_data.index).std_str();

	_raw_data.index = _raw_data.buf.find_first_not_of(" \t", index + separator_size);
	if(_raw_data.index == HttpString::npos)
		_raw_data.index = index + separator_size;

	return word;
}

void HttpMessage::_retrieve_startline()
{
	if(_parse_phase != START_LINE)
		return;

	_parse_startline();
	_validate_startline();

	_parse_phase = HEADER;
}

void HttpMessage::_retrieve_header()
{
	if(_parse_phase != HEADER)
		return;

	_parse_header();
	_validate_header();

	_parse_phase = BODY;
}

void HttpMessage::_retrieve_body()
{
	if(_parse_phase != BODY)
		return;

	_parse_body();

	_parse_phase = DONE;
}

void HttpMessage::_parse_startline() { }
void HttpMessage::_parse_header() { }
void HttpMessage::_parse_body() { }

std::pair< std::string, std::string > HttpMessage::_parse_headerfield(std::string line)
{
	size_t index, value_len;
	std::string key, value;
	index = line.find(":");
	value_len = (line.length() <= index + 1) ? 0 : line.length() - (index + 1);

	key = utility::to_titlecase(line.substr(0, index));
	value = utility::trim_ows(line.substr(index + 1, value_len));
	return std::make_pair(key, value);
}

void HttpMessage::_validate_startline() { }
void HttpMessage::_validate_header() { }