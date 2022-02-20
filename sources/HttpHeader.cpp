#include "HttpHeader.hpp"
#include <iostream>

HttpHeader::ContentType::ContentType()
{
	types["txt"] = "text/plain";
	types["csv"] = "text/csv";
	types["html"] = "text/html";
	types["css"] = "text/css";
	types["jpg"] = "image/jpeg";
	types["png"] = "image/png";
	types["gif"] = "image/gif";
	types["error"] = "";
}

struct HttpHeader::ContentType HttpHeader::_content_type;

/**
 * @brief 拡張子をヘッダーでのコンテントタイプに変換
 * @param ext: 拡張子
 * @return ヘッダーでのコンテントタイプ。リストにない場合は""を返す
 * @note 一覧にないtypeがきたときの処理は暫定的なもの
 */
const std::string& HttpHeader::_ext_to_content_type(const std::string& ext) const
{
	// 一覧にないtypeが来た場合は、""で返す(適切な返す方法があるかも)
	if(_content_type.types.find(ext) == _content_type.types.end())
	{
		return _content_type.types["error"];
	}
	return _content_type.types[ext];
}

/**
 * @brief コンテントタイプを拡張子に変換
 * @return 対応する拡張子
 */
std::string HttpHeader::content_type_to_ext() const
{
	if(!contains("Content-Type") || _headers.at("Content-Type").empty())
	{
		return "txt";
	}
	std::map< std::string, std::string >::iterator it = _content_type.types.begin();
	std::map< std::string, std::string >::iterator it_end = _content_type.types.end();
	for(; it != it_end; ++it)
	{
		if(_headers.at("Content-Type").compare(it->second) == 0)
			return it->first;
	}
	return "";
}

/**
 * @brief headerでデフォルトで設定されているものを格納する
 * @note デフォルトについてはまだ詳しく調べられていない
 */
void HttpHeader::_set_default_header()
{
	_headers["Server"] = "webserv";
}

HttpHeader::HttpHeader()
{
	_set_default_header();
}

HttpHeader::HttpHeader(const HttpHeader& other)
{
	*this = other;
}

HttpHeader& HttpHeader::operator=(const HttpHeader& other)
{
	if(this != &other)
	{
		_headers = other._headers;
	}
	return *this;
}

HttpHeader::~HttpHeader() { }

std::string& HttpHeader::operator[](std::string key)
{
	if(_headers.find(key) == _headers.end())
	{
		_headers[key] = "";
	}
	return _headers[key];
}

const std::string& HttpHeader::at(const std::string& key) const
{
	return _headers.at(key);
}

/**
 * @brief ファイルのパス、ファイル名から、Content-Typeを設定する
 * @param file: ファイルのパス、ファイル名
 */
void HttpHeader::set_content_type(const std::string& file)
{
	const std::string ext = utility::get_file_ext(file);
	_headers["Content-Type"] = _ext_to_content_type(ext);
	if(_headers["Content-Type"].empty())
		_headers["Content-Type"] = "text/html";
}

/**
 * @brief contentのサイズからContent-Lengthを設定する
 * @param length: contentのサイズ
 */
void HttpHeader::set_content_length(size_t length)
{
	_headers["Content-Length"] = utility::to_string(length);
}

/**
 * @brief contentのサイズからcontent-lengthを設定する
 * @param length: contentのサイズ
 */
ssize_t HttpHeader::get_content_length()
{
	std::string str_value = _headers["Content-Length"];
	return utility::ustr_to_ssize_t(str_value);
}

/**
 * @brief ヘッダーをstringとして返す
 * @return ヘッダーのstring。終端はCRLFCRLF
 */
const std::string HttpHeader::to_string() const
{
	std::map< std::string, std::string >::const_iterator it;
	std::map< std::string, std::string >::const_iterator it_begin = _headers.begin();
	std::map< std::string, std::string >::const_iterator it_end = _headers.end();
	std::string header_str;

	for(it = it_begin; it != it_end; ++it)
	{
		header_str += it->first + ": " + it->second;
		header_str += utility::CRLF;
	}
	header_str += utility::CRLF;
	return header_str;
}

void HttpHeader::show_headers() const
{
	std::map< std::string, std::string >::const_iterator it;
	std::map< std::string, std::string >::const_iterator it_end = _headers.end();
	std::string header_str;

	for(it = _headers.begin(); it != it_end; ++it)
	{
		std::cout << "  - " << it->first << ": " << it->second << std::endl;
	}
}

bool HttpHeader::contains(std::string key) const
{
	return (_headers.find(key) != _headers.end());
}

void HttpHeader::append(std::string key, std::string value)
{
	if(!contains(key))
		_headers[key] = value;
	else
	{
		std::string old_value = _headers[key];

		if(!old_value.empty() && old_value.at(old_value.size() - 1) != ',')
			value = ',' + value;
		_headers[key] += value;
	}
}

void HttpHeader::append(std::pair< std::string, std::string > key_value)
{
	append(key_value.first, key_value.second);
}
