#include "Http.hpp"
#include "Server.hpp"
#include <cctype> /* isalpha */
#include <ctime>
#include <string>

namespace http
{

int StatusException::get_http_status() const
{
	return _http_status;
}

bool is_token(std::string& str)
{
	for(size_t i = 0; str[i]; ++i)
	{
		if(std::isalpha(str[i]))
			continue;
		if(std::isdigit(str[i]))
			continue;
		if(str[i] == '!' || str[i] == '#' || str[i] == '$' || str[i] == '%' || str[i] == '&' ||
		   str[i] == '\'' || str[i] == '*' || str[i] == '+' || str[i] == '-' || str[i] == '.' ||
		   str[i] == '^' || str[i] == '_' || str[i] == '`' || str[i] == '|' || str[i] == '~')
			continue;
		return false;
	}
	return true;
}

bool is_comma_separated_list(std::string const& value)
{
	int in_quote = -1;
	int in_comment = -1;

	size_t len = value.length();
	for(size_t i = 0; i < len; ++i)
	{
		if(value[i] == '"')
			in_quote *= -1;
		else if(value[i] == '(')
			in_comment = 1;
		else if(value[i] == ')')
			in_comment = -1;
		else if(in_quote < 0 && in_comment < 0 && value[i] == ',')
		{
			return true;
		}
	}
	return false;
}

std::string get_current_time()
{
	time_t rawtime;
	char now[80];

	time(&rawtime);
	std::strftime(now, 80, "%a, %d %b %Y %T GMT", std::gmtime(&rawtime));
	return std::string(now);
}

std::string generate_uri_head(const Server& s)
{
	return "http://" + s.host + ":" + utility::to_string(s.port);
}
} // namespace http
