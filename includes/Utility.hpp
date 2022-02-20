#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <climits>
#include <sstream>
#include <string>
#include <sys/socket.h>

namespace utility
{
static const std::string CRLF = "\r\n";

enum filetype
{
	DNE,
	FILE,
	DIRECTORY,
};
/**
 * @brief streamに流せるものをstringに変換する
 * @param n: streamに流せるもの(intなど)
 * @return 変換したstring
	 */
template < typename T >
std::string to_string(const T& n)
{
	std::ostringstream stm;
	stm << n;
	return stm.str();
}
bool is_upper(std::string str);
bool is_ows(char c);
std::string to_titlecase(std::string const& str);
std::string trim_ows(const std::string& str);
std::string get_file_ext(const std::string& filepath);
void set_fd(int fd, fd_set& set, int& max_fd);
bool isset_clr_fd(int fd, fd_set& set);
ssize_t ustr_to_ssize_t(std::string str);
ssize_t xstr_to_ssize_t(std::string str);

bool end_with(const std::string& s, char c);
std::string clean_uri(std::string const& uri);
} // namespace utility

#endif
