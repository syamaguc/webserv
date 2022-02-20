#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include "Utility.hpp"
#include <map>
#include <string>

/**
 * @brief Httpのレスポンスメッセージに使用するステータス行のクラス
 * @note 追加すべきステータスコードについては調べられていない
 */
class HttpStatus
{
private:
	/**
	 * @brief ステータスコードとそのテキストを対応付けるための構造体
	 */
	struct StatusDescription
	{
		std::map< int, std::string > description;
		StatusDescription();
	};

	static struct StatusDescription _status_description;

	const std::string _protocol_version;
	int _status_code;

public:
	HttpStatus();
	HttpStatus(const std::string& protocol_version, int status_code);
	~HttpStatus();

	static const std::string& status_code_to_string(int code);
	void set_status_code(int code);
	int get_status_code() const;

	bool is_success() const;
	bool is_redirect() const;

	const std::string to_string() const;

private: /* for now: preventing bugs */
	HttpStatus(const HttpStatus& other);
	HttpStatus& operator=(const HttpStatus& other);
};

#endif
