#ifndef HTTPSTRING_HPP
#define HTTPSTRING_HPP

#include <limits.h>
#include <stdint.h>
#include <string>
#include <vector>

class HttpString
{
private:
	std::vector< unsigned char > _str;

public:
	typedef std::vector< unsigned char >::iterator iterator;
	typedef std::vector< unsigned char >::const_iterator const_iterator;

	HttpString();
	HttpString(HttpString const& other);
	~HttpString();
	HttpString& operator=(HttpString const& other);

	const_iterator begin() const;
	const_iterator end() const;

	size_t size() const;
	void clear();

	unsigned char operator[](size_t i) const;

	size_t append(unsigned char* str, size_t len);
	size_t append(std::string const& str);
	size_t append(HttpString const& str);
	void push_back(unsigned char c);
	void erase(size_t len, size_t pos = 0);

	const std::string std_str() const;
	unsigned char const* data() const;
	size_t find(const std::string& str, size_t pos = 0) const;
	size_t find_first_of(const std::string& str, size_t pos = 0) const;
	size_t find_first_not_of(const std::string& str, size_t pos = 0) const;
	HttpString substr(size_t pos = 0, size_t len = npos) const;
	int compare(const std::string& str) const;
	int compare(size_t pos, size_t len, const std::string& str) const;

	static const size_t npos = -1;
};

std::ostream& operator<<(std::ostream& os, const HttpString& http_string);

#endif /* HTTPSTRING_HPP */
