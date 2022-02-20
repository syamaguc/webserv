#include "HttpResponse.hpp"
#include <dirent.h>
#include <sstream>

/**
 * @brief cgiでレスポンスを作成するかどうかを判断する
 * @return cgiの場合はtrue
 * @note 仮の実装
 */
bool HttpResponse::_check_cgi()
{
	return !_info.location->cgi_path.empty() && utility::get_file_ext(_filepath) == "py";
}

/**
 * @brief redirectを行う場合trueを返す
 * @return redirectを行う場合true
 */
bool HttpResponse::_check_return_redirect()
{
	if(!_info.location)
		return false;
	return !_info.location->redirect.empty();
}

/**
 * @brief ステータスに従ってエラーページのパスを設定
 * @note リダイレクト時は_filepathはemptyになる
 */
void HttpResponse::_set_error_filepath()
{
	if(!_info.server)
	{
		_filepath = "";
		return;
	}
	std::map< int, std::string >& error_pages = _info.server->error_pages;
	int status_code = _httpstatus.get_status_code();
	if(error_pages.find(status_code) == error_pages.end())
	{
		_filepath = "";
	}
	else
	{
		_filepath = error_pages[status_code];
	}
}

/**
 * @brief ステータスコードを見てファイルパスをセットする
 */
void HttpResponse::_set_filepath()
{
	if(_httpstatus.is_success())
	{
		_filepath = _info.location->root + _info.script_name;
	}
	else
	{
		_set_error_filepath();
	}
	_filepath = utility::clean_uri(_filepath);
	_filedata.set_filepath(_filepath);
}

std::string HttpResponse::_set_upload_filename()
{
	struct timeval time_tv;
	gettimeofday(&time_tv, NULL);
	std::stringstream s;
	s << time_tv.tv_sec << time_tv.tv_usec;

	std::string timestamp(s.str());
	std::string extention(_info.headers.content_type_to_ext());
	extention = extention.empty() ? "" : '.' + extention;
	std::string filename = timestamp + extention;

	struct stat sb;
	for(size_t i = 1; stat(filename.c_str(), &sb) != -1; ++i)
	{
		filename = timestamp + '_' + utility::to_string(i) + extention;
	}
	return filename;
}

/**
 * @brief 失敗時にエラーページの設定をして、READ phaseからやりなおす
 * @param status_code: 失敗時のステータスコード
 */
void HttpResponse::_handle_error(int status_code)
{
	_info.method = "GET";
	_send_size = 0;
	int now_status = _httpstatus.get_status_code();

	if(200 <= now_status && now_status < 300)
	{
		_httpstatus.set_status_code(status_code);
		_set_error_filepath();
	}
	// エラー時にエラーが起きた場合と_filepathがない場合は、エラーページの読み込みを行わない
	if(now_status != 200 || _filepath.empty())
	{
		if(_body.set_error_default_body(_httpstatus.get_status_code()))
		{
			_phase = SEND;
		}
		else // errorページの設定に失敗した場合
		{
			_phase = CLOSE;
		}
	}
	else
	{
		if(_check_cgi()) // CGIの場合
		{
			_prepare_CGI();
		}
		else
		{
			_filedata.clear();
			_filedata.set_filepath(_filepath);
			_prepare_read();
		}
	}
}

/**
 * @brief ファイルを開いてfdをセットする
 * @param read: readかwriteか見分ける
 * @note エラー処理未完成
 */
void HttpResponse::_open_and_set_fd(bool read)
{
	if(read)
	{
		ssize_t body_size = _filedata.get_filesize();
		_body.set_body_size(body_size + utility::CRLF.size());
		_fd = open(_filepath.c_str(), (O_NONBLOCK | O_RDONLY));
	}
	else
	{
		_fd = open(_filepath.c_str(), (O_NONBLOCK | O_CREAT | O_WRONLY), 0600);
	}
	_body.set_fd(_fd);
	if(_fd < 0)
	{
		throw http::StatusException(500);
	}
}

/**
 * @brief indexを確認して、_filepathを更新する
 * @return 通常のfileであるindexが存在した場合true
 */
bool HttpResponse::_check_index_file()
{
	size_t size = _info.location->index.size();
	size_t i = 0;
	for(; i < size; ++i)
	{
		_filedata.clear();
		_filedata.set_filepath(_filepath + _info.location->index[i]);
		if(_filedata.is_file())
			break;
	}
	if(i == size)
	{
		return false;
	}
	_filepath += _info.location->index[i];
	return true;
}

/**
 * @brief 末尾にスラッシュがあった場合のgetの処理の準備
 */
void HttpResponse::_prepare_get_with_slash()
{
	if(!_filedata.is_dir())
	{
		throw http::StatusException(404);
	}
	if(!_check_index_file())
	{
		if(_info.location->autoindex)
		{
			_autoindex = true;
			_body._set_autoindex_body(_info.script_name, _filepath);
		}
		else
		{
			throw http::StatusException(403);
		}
	}
}

/**
 * @brief 末尾にスラッシュがなかった場合のgetの処理の準備
 */
void HttpResponse::_prepare_get_no_slash()
{
	if(!_filedata.exist())
	{
		throw http::StatusException(404);
	}
	if(_filedata.is_dir())
	{
		_filepath += "/";
		throw http::StatusException(301);
	}
}

/**
 * @brief get処理の準備
 */
void HttpResponse::_prepare_get()
{
	if(utility::end_with(_filepath, '/'))
		_prepare_get_with_slash();
	else
		_prepare_get_no_slash();

	Log(_filepath.c_str());
	if(_autoindex)
		_phase = SEND;
	else
	{
		if(_check_cgi())
			_prepare_CGI();
		else
			_prepare_read();
	}
}

/**
 * @brief read処理の準備
 */
void HttpResponse::_prepare_read()
{
	try
	{
		_open_and_set_fd(true);
		_phase = READ;
	}
	catch(const http::StatusException& e)
	{
		_handle_error(e.get_http_status());
	}
	catch(const std::exception& e)
	{
		_handle_error(500);
	}
}

/**
 * @brief write処理が可能かどうか確認
 * @note write処理不可時エラー送出
 */
void HttpResponse::_check_write()
{
	if(_info.location == NULL)
	{
		throw http::StatusException(500);
	}
	if(!_filedata.is_dir())
	{
		throw http::StatusException(403);
	}
	std::string upload_path = _info.location->upload_path;
	upload_path += "/";
	upload_path = utility::clean_uri(upload_path);
	if(upload_path != _filepath)
	{
		throw http::StatusException(403);
	}
}

/**
 * @brief write処理の準備
 */
void HttpResponse::_prepare_write()
{
	if(_check_cgi())
	{
		_prepare_CGI();
		return;
	}
	_check_write(); // post不可時エラー送出
	std::string new_file_name = _set_upload_filename();
	_filepath = _filepath + '/' + new_file_name;
	_filepath = utility::clean_uri(_filepath);
	Log(_filepath.c_str());
	_open_and_set_fd(false);
	std::string location_uri = http::generate_uri_head(*_info.server) + _info.uri + new_file_name;
	_httpheader.append("Location", location_uri);
	_phase = WRITE;
}

/**
 * @brief read失敗時のエラー処理
 * @param status_code: 失敗時のステータスコード
 * @note 送信途中でエラーになった場合の処理が未完成
 */
void HttpResponse::_handle_read_error(int status_code)
{
	if(_body.get_ready())
	{
		_body.close_fd();
	}
	if(_first_response)
	{
		_handle_error(status_code);
	}
	else
	{
		_phase = CLOSE;
	}
}

/**
 * @brief write失敗時のエラー処理
 * @param status_code: 失敗時のステータスコード
 * @note 送信途中でエラーになった場合の処理が未完成
 */
void HttpResponse::_handle_write_error(int status_code)
{
	if(_body.get_ready())
	{
		_body.close_fd();
	}
	_handle_error(status_code);
}

/**
 * @brief bodyの読み込みを行う
 */
void HttpResponse::_read()
{
	try
	{
		_body.read_body();
		_phase = SEND;
	}
	catch(const http::StatusException& e)
	{
		_handle_read_error(e.get_http_status());
	}
	catch(const std::exception& e)
	{
		_handle_read_error(500);
	}
}

/**
 * @brief POST時にアップロード先ファイルへの書き込みを行う
 */
void HttpResponse::_write()
{
	try
	{
		write(_fd, _info.body.data(), _info.body.size());
		_body.close_fd();
		_httpstatus.set_status_code(201);
		_phase = SEND;
	}
	catch(const http::StatusException& e)
	{
		_handle_write_error(e.get_http_status());
	}
	catch(const std::exception& e)
	{
		_handle_write_error(500);
	}
}

/**
 * @brief send失敗時のエラー処理
 * @param status_code: 失敗時のステータスコード
 * @note 送信途中でエラーになった場合の処理が未完成
 */
void HttpResponse::_handle_send_error(int status_code)
{
	if(_body.get_ready())
	{
		_body.close_fd();
	}
	if(!_first_response || status_code == http::StatusException::CLOSE)
	{
		_phase = CLOSE;
		return;
	}
	_handle_error(status_code);
}

/**
 * @brief headerの設定を行う
 */
void HttpResponse::_set_header()
{
	_httpheader.set_content_length(_body.get_all_body_size());
	_httpheader.set_content_type(_filepath);
	_httpheader.append("Connection", "close");
	_httpheader.append("Date", http::get_current_time());
}

/**
 * @brief 1回目のsend処理時の処理
 * @note エラー処理未完成
 */
void HttpResponse::_first_send()
{
	_set_header();
	HttpString str_message;
	str_message.append(_httpstatus.to_string());
	str_message.append(_httpheader.to_string());
	str_message.append(_body.get_body());
	size_t content_size = str_message.size();
	_send_content(str_message.data(), content_size);
}

/**
 * @brief sendを行う
 * @note エラー処理未完成（今はsendが失敗したときの処理と一部だけsendできたときの処理が一緒）
 */
void HttpResponse::_send_content(unsigned char const* content, size_t size)
{
	ssize_t ret = send(_connection_fd, content, size, 0);
	if(ret < 0 || static_cast< size_t >(ret) != size)
	{
		throw http::StatusException(http::StatusException::CLOSE);
	}
}

/**
 * @brief sendの処理を行う
 * @note エラー処理未完成
 */
void HttpResponse::_send()
{
	try
	{
		if(_first_response)
		{
			_first_send();
			_first_response = false;
		}
		else
		{
			_send_content(_body.get_body_data(), _body.get_body_size());
		}
		_send_size += _body.get_body_size();
		_phase = _body.get_ready() ? READ : CLOSE;
		if(_phase == CLOSE && _send_size != _body.get_all_body_size())
		{
			Log("send size error");
			_handle_send_error(http::StatusException::CLOSE);
		}
	}
	catch(const http::StatusException& e)
	{
		_handle_send_error(e.get_http_status());
	}
	catch(const std::exception& e)
	{
		_handle_send_error(500);
	}
}

/**
 * @brief cgiからの返答をparseをする
 * @note 現在は非常に簡易的な状態
 */
void HttpResponse::_parse_message()
{
	switch(_parse_phase)
	{
	case AWAIT:
		_parse_phase = HEADER;
		/* Falls through. */
	case HEADER:
		_retrieve_header();
		/* Falls through. */
	case BODY:
		_retrieve_body();
		/* Falls through. */
	default:
		break;
	}
}

void HttpResponse::_retrieve_header()
{
	HttpMessage::_retrieve_header();
}

void HttpResponse::_retrieve_body()
{
	HttpMessage::_retrieve_body();
}

/**
 * @brief ヘッダーフィールドのパース
 * @note 空行がない場合,または先頭にある場合は502エラーを送出
 */
void HttpResponse::_parse_header()
{
	std::string line;
	std::pair< std::string, std::string > headerfield;
	size_t size = _raw_data.buf.size();

	line = _get_next_line();
	while(!line.empty() && _raw_data.index < size)
	{
		if(line.rfind("HTTP/", 0) == 0)
			_status_line = line;
		else
		{
			headerfield = _parse_headerfield(line);
			_httpheader.append(headerfield);
		}
		line = _get_next_line();
		if(line.empty())
			return;
	}
	throw http::StatusException(502);
}

/**
 * @brief CGIレスポンスボディーのパース
 * @note Content-lengthがない場合に設定する
 */
void HttpResponse::_parse_body()
{
	if(_httpheader.contains("Content-Length"))
	{
		ssize_t len = _httpheader.get_content_length();
		if(len != -1)
		{
			_body.set_body(_raw_data.buf.substr(_raw_data.index, len));
			return;
		}
	}
	_body.set_body(_raw_data.buf.substr(_raw_data.index));
}

/**
 * @brief CGIの準備を行う
 */
void HttpResponse::_prepare_CGI()
{
	try
	{
		_phase = _cgi.first_preparation();
	}
	catch(const http::StatusException& e)
	{
		_handle_error(e.get_http_status());
	}
	catch(const std::exception& e)
	{
		_handle_error(500);
	}
}

/**
 * @brief CGIの処理を行う
 */
void HttpResponse::_handle_cgi(fd_set& read_set, fd_set& write_set)
{
	try
	{
		_phase = _cgi.check_and_handle(read_set, write_set);
		if(_phase == SEND)
		{
			_raw_data.buf = _cgi.get_data();
			_parse_message();
		}
	}
	catch(const http::StatusException& e)
	{
		_handle_error(e.get_http_status());
	}
	catch(const std::exception& e)
	{
		_handle_error(500);
	}
}

/**
 * @brief リダイレクトの準備を行う
 */
void HttpResponse::_prepare_redirect(bool to_directory)
{
	int status;
	std::string pass;

	if(to_directory)
	{
		status = 301;
		pass = http::generate_uri_head(*_info.server) + _info.script_name + '/';
	}
	else
	{
		status = _info.location->redirect.begin()->first;
		pass = _info.location->redirect.begin()->second;
	}
	_httpstatus.set_status_code(status);
	_httpheader["Location"] = pass;
	_phase = SEND;
}

/**
 * @brief DELETEの処理を行う
 */
void HttpResponse::_handle_delete()
{
	Log("DELETE");
	if(!_filedata.exist())
	{
		throw http::StatusException(404);
	}
	if(_filedata.is_file())
	{
		if(unlink(_filepath.c_str()) == -1)
			throw http::StatusException(500);
		_httpstatus.set_status_code(204);
	}
	else //not file
	{
		throw http::StatusException(403);
	}
	_phase = SEND;
}

/**
 * @brief 最初の処理を行う
 */
void HttpResponse::_first_preparation()
{
	Log(_filepath.c_str());
	if(_check_return_redirect())
	{
		_prepare_redirect();
	}
	else if(_filepath.empty())
	{
		if(_body.set_error_default_body(_httpstatus.get_status_code()))
		{
			_phase = SEND;
		}
		else // errorページの設定に失敗した場合
		{
			_phase = CLOSE;
		}
	}
	else
	{
		try
		{
			if(_info.method == "GET")
			{
				_prepare_get();
			}
			else if(_info.method == "POST")
			{
				_prepare_write();
			}
			else if(_info.method == "DELETE")
			{
				_handle_delete();
			}
		}
		catch(const http::StatusException& e)
		{
			if(e.get_http_status() == 301)
				_prepare_redirect(true);
			else
				_handle_error(e.get_http_status());
		}
		catch(const std::exception& e)
		{
			_handle_error(500);
		}
	}
}

HttpResponse::HttpResponse(HttpInfo& info, int connection_fd, int status_code)
	: HttpMessage("\n")
	, _fd(0)
	, _connection_fd(connection_fd)
	, _phase(RECV)
	, _httpstatus(info.protocol_version, status_code)
	, _info(info)
	, _cgi(info, _httpstatus, _httpheader, _body, _filepath)
	, _first_response(true)
	, _send_size(0)
	, _autoindex(false)
{
	if(!_check_return_redirect())
	{
		_set_filepath();
	}
}

HttpResponse::~HttpResponse() { }

/**
 * @brief phaseに応じてレスポンスの処理を行う
 * @return 次のphase
 */
enum phase HttpResponse::handle_response()
{
	switch(_phase)
	{
	case RECV:
		_first_preparation();
		break;
	case WRITE:
		_write();
		break;
	case READ:
		_read();
		break;
	case SEND:
		_send();
		break;
	default:
		break;
	}
	return _phase;
}

/**
 * @brief selectで必要なパラメータの設定をする
 * @param read_set: read用のset
 * @param write_set: write用のset
 * @param max_fd: fdのmax値
 * @note CGIの場合が未完成
 */
void HttpResponse::set_select_fd(fd_set& read_set, fd_set& write_set, int& max_fd) const
{
	switch(_phase)
	{
	case WRITE:
		utility::set_fd(_fd, write_set, max_fd);
		break;
	case READ:
		utility::set_fd(_fd, read_set, max_fd);
		break;
	case CGI:
		_cgi.set_select_fd(read_set, write_set, max_fd);
		break;
	default:
		break;
	}
}

/**
 * @brief fdが有効になっているかのチェックとそれに合わせた処理
 * @param read_set: read用のset
 * @param write_set: write用のset
 * @return phase
 */
enum phase HttpResponse::check_and_handle(fd_set& read_set, fd_set& write_set)
{
	switch(_phase)
	{
	case WRITE:
		if(utility::isset_clr_fd(_fd, write_set))
		{
			Log("write");
			return handle_response();
		}
		break;
	case READ:
		if(utility::isset_clr_fd(_fd, read_set))
		{
			Log("read");
			return handle_response();
		}
		break;
	case CGI:
		_handle_cgi(read_set, write_set);
		return _phase;
		break;
	default:
		break;
	}
	return CLOSE;
}

int HttpResponse::get_fd() const
{
	return _fd;
}
