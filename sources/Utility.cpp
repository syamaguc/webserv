#include "Utility.hpp"
#include <cctype>
#include <cstdlib>
#include <sys/stat.h>

namespace utility
{
bool is_upper(std::string str)
{
	size_t len = str.length();
	for(size_t i = 0; i < len; i++)
	{
		if(!std::isupper(str.at(i)))
			return false;
	}
	return true;
}

std::string to_titlecase(std::string const& str)
{
	std::string ret;
	size_t size = str.size();
	bool upper = true;

	for(size_t i = 0; i < size; ++i)
	{
		if(upper)
		{
			ret += std::toupper(str[i]);
			upper = false;
			continue;
		}
		if(str[i] == '-')
			upper = true;
		ret += std::tolower(str[i]);
	}
	return ret;
}

bool is_ows(char c)
{
	return (c == ' ' || c == '\t');
}

std::string trim_ows(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t");
	if(first == std::string::npos)
		return "";
	size_t last = str.find_last_not_of(" \t");

	return str.substr(first, last - first + 1);
}

/**
 * @brief ファイルのパスから拡張子を得る
 * @param file: ファイルのパス、またはファイル名
 * @return 拡張子（拡張子がない場合は""）
 */
std::string get_file_ext(const std::string& file)
{
	std::size_t dot_index = file.find_last_of('.');
	if(dot_index == std::string::npos)
	{
		std::string return_str = "";
		return return_str;
	}
	return file.substr(dot_index + 1);
}

/**
 * @brief fd_setにfdをセットしてmax_fdを更新する
 * @param fd: ファイルディスクリプタ
 * @param set: selectで使用するファイルディスクリプタのセット
 * @param max_fd: ファイルディスクリプタの最大値
 */
void set_fd(int fd, fd_set& set, int& max_fd)
{
	max_fd = std::max(max_fd, fd);
	FD_SET(fd, &set);
}

/**
 * @brief FD_ISSETでtrueだった場合FD_CLRを行う
 * @param fd: ファイルディスクリプタ
 * @param set: selectで使用するファイルディスクリプタのセット
 */
bool isset_clr_fd(int fd, fd_set& set)
{
	if(FD_ISSET(fd, &set))
	{
		FD_CLR(fd, &set);
		return true;
	}
	return false;
}

/**
 * @brief 符号なしstringをssize_tに変換する
 * @param str: 変換するstring
 * @return 数字以外の文字があった場合、SSIZE_MAXを超える場合-1
 */
ssize_t ustr_to_ssize_t(std::string str)
{
	if(str.compare("0") == 0)
		return 0;

	size_t ret = 0;
	int i = 0;
	for(; str[i] && std::isdigit(str[i]); ++i)
	{
		ret *= 10;
		ret += str[i] - '0';
		if(ret > SSIZE_MAX)
			return -1;
	}
	if(str[i] || ret == 0)
		return -1;

	return ret;
}

/**
 * @brief hex-stringをssize_tに変換する
 * @param str: 変換するstring
 * @return 数字以外の文字があった場合、SSIZE_MAXを超える場合-1
 */
ssize_t xstr_to_ssize_t(std::string str)
{
	if(str.compare("0") == 0)
		return 0;

	size_t ret = 0;
	int i = 0;
	for(; str[i] && std::isxdigit(str[i]); ++i)
	{
		ret *= 16;
		if(std::isdigit(str[i]))
			ret += str[i] - '0';
		else
			ret += std::tolower(str[i]) - 'a' + 10;
		if(ret > SSIZE_MAX)
			return -1;
	}
	if(str[i] || ret == 0)
		return -1;

	return ret;
}

bool end_with(const std::string& s, char c)
{
	if(s.size() == 0)
		return false;
	return (s[s.size() - 1] == c);
}

std::string clean_uri(std::string const& uri)
{
	std::string new_uri;
	size_t size = uri.size();
	if(size == 0)
		return std::string("");
	for(size_t i = 0; i + 1 < size; ++i)
	{
		if(uri[i] == '/' && uri[i + 1] == '/')
			continue;
		new_uri += uri[i];
	}
	new_uri += uri[size - 1];
	return new_uri;
}
} // namespace utility
