#ifndef HTTPHEADER_HPP
#define HTTPHEADER_HPP

#include "Utility.hpp"
#include <map>
#include <string>

/**
 * @brief Httpのレスポンスメッセージに使用するヘッダーのクラス
 * @note 追加すべきヘッダーの項目については調べられていない
 */
class HttpHeader
{
private:
	/**
	 * @brief コンテントのタイプとヘッダーでのコンテントタイプを対応付けるための構造体
	 */
	struct ContentType
	{
		std::map< std::string, std::string > types;
		ContentType();
	};

	static struct ContentType _content_type;

	std::map< std::string, std::string > _headers;

	const std::string& _ext_to_content_type(const std::string& ext) const;
	void _set_default_header();

public:
	HttpHeader();
	~HttpHeader();

	std::string& operator[](std::string key);
	const std::string& at(const std::string& key) const;

	void set_content_type(const std::string& file);
	void set_content_length(size_t length);
	ssize_t get_content_length();

	const std::string to_string() const;

	void show_headers() const;
	bool contains(std::string key) const;
	void append(std::string key, std::string value);
	void append(std::pair< std::string, std::string > key_value);

	std::string content_type_to_ext() const;

	HttpHeader& operator=(const HttpHeader& other);
	HttpHeader(const HttpHeader& other);

private: /* for now: preventing bugs */
};

#endif
