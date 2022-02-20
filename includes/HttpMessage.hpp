#ifndef HTTPMESSAGE_HPP
#define HTTPMESSAGE_HPP

#include "HttpString.hpp"
#include <string>

class HttpMessage
{
public:
	enum parse_phase
	{
		AWAIT,
		START_LINE,
		HEADER,
		BODY,
		DONE
	};

	HttpMessage();
	HttpMessage(std::string delim);
	virtual ~HttpMessage();
	HttpMessage(HttpMessage const& other);
	HttpMessage& operator=(HttpMessage const& other);

	parse_phase get_parse_phase() const;

protected:
	struct MessageString
	{
		HttpString buf;
		size_t index;
	};
	MessageString _raw_data;
	parse_phase _parse_phase;
	std::string _delim;
	std::string _get_next_line(size_t max = SIZE_MAX, int status_code = 0);
	std::string _get_next_word(bool eol, size_t max = SIZE_MAX, int status_code = 0);

	std::pair< std::string, std::string > _parse_headerfield(std::string line);

	virtual void _retrieve_startline();
	virtual void _retrieve_header();
	virtual void _retrieve_body();

	virtual void _parse_startline();
	virtual void _parse_header();
	virtual void _parse_body();

	virtual void _validate_startline();
	virtual void _validate_header();
};

#endif /* HTTPMESSAGE_HPP */