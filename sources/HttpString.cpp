#include "HttpString.hpp"
#include "Http.hpp"
#include "Utility.hpp"
#include <iostream>
#include <stdexcept> /* out_of_range */
#include <string>

HttpString::HttpString() { }

HttpString::HttpString(HttpString const& other)
{
	*this = other;
}

HttpString::~HttpString() { }

HttpString& HttpString::operator=(HttpString const& other)
{
	if(this != &other)
	{
		_str = other._str;
	}
	return *this;
}

HttpString::const_iterator HttpString::begin() const
{
	return _str.begin();
}

HttpString::const_iterator HttpString::end() const
{
	return _str.end();
}

size_t HttpString::size() const
{
	return _str.size();
}

void HttpString::clear()
{
	_str.clear();
}

unsigned char HttpString::operator[](size_t i) const
{
	return _str[i];
}

/**
 * @brief lenの長さ分strを追加する
 * @return 追加後のサイズを返す
 * @note reserve(内部のvectorのサイズ確保)に失敗した場合エラー送出
 */
size_t HttpString::append(unsigned char* str, size_t len)
{
	size_t size = _str.size();

	_str.reserve(size + len + 1);
	for(size_t i = 0; i < len; ++i)
	{
		_str.push_back(str[i]);
	}
	size = _str.size();
	return size;
}

/**
 * @brief stringを追加する
 * @return 追加後のサイズを返す
 * @note reserve(内部のvectorのサイズ確保)に失敗した場合エラー送出
 */
size_t HttpString::append(std::string const& str)
{
	size_t size = _str.size();
	size_t len = str.size();

	_str.reserve(size + len + 1);
	for(size_t i = 0; i < len; ++i)
	{
		_str.push_back(str[i]);
	}
	size = _str.size();
	return size;
}

/**
 * @brief HttpStringを追加する
 * @return 追加後のサイズを返す
 * @note reserve(内部のvectorのサイズ確保)に失敗した場合エラー送出
 */
size_t HttpString::append(HttpString const& str)
{
	size_t size = _str.size();
	size_t len = str.size();

	_str.reserve(size + len + 1);
	for(size_t i = 0; i < len; ++i)
	{
		_str.push_back(str[i]);
	}
	size = _str.size();
	return size;
}

void HttpString::push_back(unsigned char c)
{
	_str.push_back(c);
}

void HttpString::erase(size_t len, size_t pos)
{
	if(len == 0)
		return;
	std::vector< unsigned char >::iterator head = _str.begin();
	_str.erase(head + pos, head + pos + len);
}

const std::string HttpString::std_str() const
{
	std::string ret;
	size_t size = _str.size();

	for(size_t i = 0; i < size; ++i)
	{
		ret += _str[i];
	}
	return ret;
}

unsigned char const* HttpString::data() const
{
	return &_str[0];
}

size_t HttpString::find(const std::string& str, size_t pos) const
{
	size_t len = str.length();
	size_t size = _str.size();

	if(size <= len)
		return npos;
	for(size_t i = pos; i <= size - len; ++i)
	{
		if(compare(i, len, str) == 0)
			return i;
	}
	return npos;
}

size_t HttpString::find_first_of(const std::string& str, size_t pos) const
{
	size_t size = _str.size();
	for(size_t i = pos; i < size; ++i)
	{
		if(str.find(_str[i]) != std::string::npos)
			return i;
	}
	return npos;
}

size_t HttpString::find_first_not_of(const std::string& str, size_t pos) const
{
	size_t size = _str.size();
	for(size_t i = pos; i < size; ++i)
	{
		if(str.find(_str[i]) == std::string::npos)
			return i;
	}
	return npos;
}

HttpString HttpString::substr(size_t pos, size_t len) const
{
	HttpString ret;
	size_t size = _str.size();
	for(size_t i = 0; i < len && pos + i < size; ++i)
	{
		ret.push_back(_str[pos + i]);
	}
	return ret;
}

int HttpString::compare(const std::string& str) const
{
	return compare(0, str.length(), str);
}

int HttpString::compare(size_t pos, size_t len, const std::string& str) const
{
	size_t size = _str.size();
	if(len == 0)
		return 0;
	if(pos >= size)
		throw std::out_of_range("HttpString: compare");

	size_t i = 0;
	while(i < len - 1 && i < (size - pos) && _str[i + pos] == str[i])
		++i;
	if(i >= size - pos)
		return ('\0' - str[size]);
	return (_str[i + pos] - str[i]);
}

std::ostream& operator<<(std::ostream& os, const HttpString& http_string)
{
	HttpString::const_iterator it_end = http_string.end();
	for(HttpString::const_iterator it = http_string.begin(); it != it_end; ++it)
		os << *it;
	return os;
}
