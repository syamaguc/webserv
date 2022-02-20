#include "Connection.hpp"
#include "Http.hpp"
#include "HttpRequest.hpp"
#include "WebServ.hpp" /* Log */
#include <sys/socket.h> /* recv */
#include <unistd.h> /* close */

size_t Connection::_total_connections = 0;

Connection::Connection(int fd, std::vector< Server >& servers)
	: _sfd(fd)
	, _phase(RECV)
	, _response(NULL)
	, _servers(servers)
	, _request(_servers)
{
#if DEBUG_ACTIVE == 1
	_total_connections += 1;
	_id = _total_connections;
#endif
	Log("Connection Established", _id);
}

Connection::~Connection() { }

Connection::Connection(const Connection& other)
	: _servers(other._servers)
	, _request(_servers)
{
	*this = other;
}

Connection& Connection::operator=(Connection const& other)
{
	if(this != &other)
	{
		_sfd = other._sfd;
		_phase = other._phase;
		_info = other._info;
		_response = other._response;
		_request = other._request;
		_servers = other._servers;
		_id = other._id;
	}
	return *this;
}

void Connection::recv_request()
{
	ssize_t buf_size = _recv();
	if(buf_size <= 0)
	{
		_phase = CLOSE;
		return;
	}

	bool request_complete = false;
	try
	{
		_request.append_raw_data(_buf, buf_size);
		request_complete = _request.handle_request();
	}
	catch(http::StatusException& e)
	{
		Log(std::string("Request ") + e.what());
		_request.setup_default_http_info(_info);
		_make_HttpResponse(e.get_http_status());
		return;
	}
	if(!request_complete)
	{
		Log("Request Suspeneded", _id);
		return;
	}
	_request.setup_http_info(_info);
	_make_HttpResponse(200);
	Log("Request Completed", _id);
}

/**
 * @brief HttpResponseクラスを作成する
 * @param status_code: ステータスコード
 * @note エラー処理は未完成
 */
void Connection::_make_HttpResponse(int status_code)
{
	if(_response != NULL)
	{
		delete _response;
		_response = NULL;
	}
	try
	{
		_response = new HttpResponse(_info, _sfd, status_code);
		_phase = _response->handle_response();
	}
	catch(const std::exception& e)
	{
		Log("_make_HttpResponse: New has failed.", _id);
		_phase = CLOSE;
	}
}

ssize_t Connection::_recv()
{
	ssize_t buf_size;
	buf_size = ::recv(_sfd, _buf, _max_buffer_size, 0);
	if(buf_size > 0)
		_buf[buf_size] = '\0';
	return buf_size;
}

void Connection::close() const
{
	if(::close(_sfd) == -1)
		throw std::exception();
}

/**
 * @brief selectで必要なパラメータの設定をする
 * @param read_set: read用のset
 * @param write_set: write用のset
 * @param max_fd: fdのmax値
 */
void Connection::set_select_fd(fd_set& read_set, fd_set& write_set, int& max_fd) const
{
	switch(_phase)
	{
	case RECV:
		utility::set_fd(_sfd, read_set, max_fd);
		break;
	case SEND:
		utility::set_fd(_sfd, write_set, max_fd);
		break;
	default:
		if(_response) // この地点で_responseがNULLのことはないが念のためチェック
		{
			_response->set_select_fd(read_set, write_set, max_fd);
		}
		break;
	}
}

/**
 * @brief fdが有効になっているかのチェックとそれに合わせた処理
 * @param read_set: read用のset
 * @param write_set: write用のset
 * @return このConnectionをcloseする場合はtrue
 */
bool Connection::check_and_handle(fd_set& read_set, fd_set& write_set)
{
	if(_phase == RECV)
	{
		if(utility::isset_clr_fd(_sfd, read_set))
		{
			Log("Requset Recieved", _id);
			recv_request();
		}
	}
	else if(_response == NULL) //ここに入る場合はないはず
	{
		Log("_response == NULL", _id);
		_phase = CLOSE;
	}
	else if(_phase == SEND)
	{
		if(utility::isset_clr_fd(_sfd, write_set))
		{
			Log("Response Send", _id);
			_phase = _response->handle_response();
		}
	}
	else
	{
		_phase = _response->check_and_handle(read_set, write_set);
	}

	if(_phase == CLOSE)
	{
		Log("Response Deleted", _id);
		delete _response;
		_response = NULL;
		// ここでcloseしないタイプの場合falseを返す
		return true;
	}
	return false;
}

int Connection::get_sfd() const
{
	return _sfd;
}

enum phase Connection::get_phase() const
{
	return _phase;
}

const HttpResponse* Connection::get_response() const
{
	return _response;
}
