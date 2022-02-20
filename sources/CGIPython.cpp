#include "CGIPython.hpp"

CGIPython::CGIPython(HttpInfo& info,
					 HttpStatus& httpstatus,
					 HttpHeader& httpheader,
					 HttpBody& body,
					 std::string& filepath)
	: _write_fd(-1)
	, _read_fd(-1)
	, _httpstatus(httpstatus)
	, _httpheader(httpheader)
	, _body(body)
	, _info(info)
	, _filepath(filepath)
	, _sent_size(0)
{ }

CGIPython::~CGIPython() { }

/**
 * @brief pythonの確認を行う
 * @note エラー時500送出
 */
void CGIPython::_check_python()
{
	FileData filedata;
	filedata.set_filepath(_info.location->cgi_path.c_str());
	if(!filedata.is_file())
	{
		throw http::StatusException(500);
	}
}

/**
 * @brief 実行するファイルの確認を行う
 * @note エラー時404送出
 */
void CGIPython::_check_execute_file()
{
	FileData filedata;
	filedata.set_filepath(_filepath.c_str());
	if(!filedata.is_file())
	{
		throw http::StatusException(404);
	}
}

/**
 * @brief envの設定を行う
 */
void CGIPython::_set_env(EnvVar& env)
{
	try
	{
		env["CONTENT_LENGTH"] = _info.headers["Content-Length"];
		env["CONTENT_TYPE"] = _info.headers["Content-Type"];
		env["PATH_INFO"] = _info.script_name;
		env["PATH_TRANSLATED"] = _filepath;
		env["QUERY_STRING"] = _info.query_string;
		env["REQUEST_METHOD"] = _info.method;
		env["REQUEST_URI"] = _info.uri;
		env["SCRIPT_FILENAME"] = _filepath;
		env["SCRIPT_NAME"] = _info.script_name;
		env["SERVER_NAME"] = _info.server->name;
		env["SERVER_PORT"] = utility::to_string(_info.server->port);
		env["SERVER_PROTOCOL"] = _info.protocol_version;
		env["SERVER_SOFTWARE"] = "webserv/1.0";
		env.set_c_env();
	}
	catch(const std::exception& e)
	{
		throw http::StatusException(ENV);
	}
}

/**
 * @brief argの設定を行う
 */
void CGIPython::_make_arg(char*** arg)
{
	std::string python_path = _info.location->cgi_path;
	int delete_flag = 0;
	char** arg_list = NULL;
	try
	{
		arg_list = new char*[3];
		delete_flag = 1;
		arg_list[0] = new char[python_path.length() + 1];
		delete_flag = 2;
		arg_list[1] = new char[_filepath.length() + 1];
	}
	catch(const std::exception& e)
	{
		if(delete_flag == 2)
		{
			delete arg_list[0];
		}
		if(delete_flag >= 1)
		{
			delete[] arg_list;
		}
		throw http::StatusException(ARG);
	}
	std::strcpy(arg_list[0], python_path.c_str());
	std::strcpy(arg_list[1], _filepath.c_str());
	arg_list[2] = NULL;
	*arg = arg_list;
}

/**
 * @brief pipeを行う
 */
void CGIPython::_set_pipe(int pipe_c2p[2], int pipe_p2c[2])
{
	if(pipe(pipe_c2p) < 0)
	{
		throw http::StatusException(PIPE);
	}
	if(fcntl(pipe_c2p[0], F_SETFL, O_NONBLOCK) == -1 ||
	   fcntl(pipe_c2p[1], F_SETFL, O_NONBLOCK) == -1)
	{
		close(pipe_c2p[0]);
		close(pipe_c2p[1]);
		throw http::StatusException(PIPE);
	}
	if(_info.method == "POST")
	{
		if(pipe(pipe_p2c) < 0)
		{
			close(pipe_c2p[0]);
			close(pipe_c2p[1]);
			throw http::StatusException(PIPE);
		}
		if(fcntl(pipe_p2c[0], F_SETFL, O_NONBLOCK) == -1 ||
		   fcntl(pipe_p2c[1], F_SETFL, O_NONBLOCK) == -1)
		{
			close(pipe_c2p[0]);
			close(pipe_c2p[1]);
			close(pipe_p2c[0]);
			close(pipe_p2c[1]);
			throw http::StatusException(PIPE);
		}
	}
}

/**
 * @brief processの作成を行う
 */
void CGIPython::_make_process()
{
	_ch_pid = fork();
	if(_ch_pid == -1)
	{
		throw http::StatusException(PROCESS);
	}
}

/**
 * @brief flagによって、どのリソースを解放するか決める
 */
void CGIPython::_delete_resources(char** arg, int* pipe_c2p, int* pipe_p2c, int flag)
{
	if(flag > ARG)
	{
		for(int i = 0; arg[i]; ++i)
		{
			delete[] arg[i];
		}
		delete[] arg;
	}
	if(flag > PIPE)
	{
		close(pipe_c2p[0]);
		close(pipe_c2p[1]);
		if(_info.method == "POST")
		{
			close(pipe_p2c[0]);
			close(pipe_p2c[1]);
		}
	}
}

/**
 * @brief 子プロセスの処理を行う
 */
void CGIPython::_child_process(int pipe_c2p[2], int pipe_p2c[2], char** arg, char** env)
{
	close(pipe_c2p[0]);
	if(dup2(pipe_c2p[1], 1) == -1)
	{
		std::exit(1);
	}
	close(pipe_c2p[1]);

	if(_info.method == "POST")
	{
		close(pipe_p2c[1]);
		if(dup2(pipe_p2c[0], 0) == -1)
		{
			std::exit(1);
		}
		close(pipe_p2c[0]);
	}

	execve(arg[0], arg, env);

	exit(1);
}

/**
 * @brief read_fdとwrite_fdの設定と必要のないfdのクローズ
 */
void CGIPython::_set_fd(int pipe_c2p[2], int pipe_p2c[2])
{
	close(pipe_c2p[1]);
	_read_fd = pipe_c2p[0];

	if(_info.method == "POST")
	{
		close(pipe_p2c[0]);
		_write_fd = pipe_p2c[1];
	}
}

/**
 * @brief cgiからの返答をreadする
 */
void CGIPython::_read()
{
	unsigned char buf[BUF_SIZE] = {0};
	int n = read(_read_fd, buf, BUF_SIZE);
	if(n < 0)
	{
		throw http::StatusException(500);
	}
	_data.append(buf, n);

	if(n == 0)
	{
		close(_read_fd);
		_read_fd = -1;
	}
}

/**
 * @brief cgiにwriteする
 */
void CGIPython::_write()
{
	HttpString send_str = _info.body.substr(_sent_size, BUF_SIZE);
	size_t send_size = send_str.size();
	ssize_t n = write(_write_fd, send_str.data(), send_size);
	if(n < 0 || static_cast< size_t >(n) != send_size)
	{
		throw http::StatusException(500);
	}
	_sent_size += send_size;
	if(_sent_size == _info.body.size())
	{
		close(_write_fd);
		_write_fd = -1;
	}
}

/**
 * @brief readかwriteのエラー時の処理を行う
 * @param status: error status
 * @note この関数は必ずerror送出する
 */
void CGIPython::_read_or_write_error(int status)
{
	if(_read_fd != -1)
	{
		close(_read_fd);
		_read_fd = -1;
	}
	if(_write_fd != -1)
	{
		close(_write_fd);
		_write_fd = -1;
	}
	throw http::StatusException(status);
}

/**
 * @brief processをkillする
 * @note この関数は必ずerror送出する
 */
void CGIPython::_kill_process()
{
	kill(_ch_pid, SIGTERM); // SIGKILLもありだがどちらがいいか
	throw http::StatusException(504);
}

/**
 * @brief 最初の処理を行う
 * @note エラー時はStatusExceptionの送出
 */
enum phase CGIPython::first_preparation()
{
	EnvVar env;
	char** arg = NULL;
	int pipe_c2p[2];
	int pipe_p2c[2];

	_check_python();
	_check_execute_file();
	try
	{
		_set_env(env);
		_make_arg(&arg);
		_set_pipe(pipe_c2p, pipe_p2c);
		_make_process();
	}
	catch(const http::StatusException& e)
	{
		// エラー番号によってどのリソースを解放するか決める
		_delete_resources(arg, pipe_c2p, pipe_p2c, e.get_http_status());
		throw http::StatusException(500);
	}

	if(_ch_pid == 0)
	{
		_child_process(pipe_c2p, pipe_p2c, arg, env.get_c_env());
	}
	_time.start();
	_delete_resources(arg, NULL, NULL, PIPE);
	_set_fd(pipe_c2p, pipe_p2c);
	return CGI;
}

/**
 * @brief selectで必要なパラメータの設定をする
 * @param read_set: read用のset
 * @param write_set: write用のset
 * @param max_fd: fdのmax値
 */
void CGIPython::set_select_fd(fd_set& read_set, fd_set& write_set, int& max_fd) const
{
	if(_write_fd != -1)
	{
		utility::set_fd(_write_fd, write_set, max_fd);
	}
	if(_read_fd != -1)
	{
		utility::set_fd(_read_fd, read_set, max_fd);
	}
}

/**
 * @brief fdが有効になっているかのチェックとそれに合わせた処理
 * @param read_set: read用のset
 * @param write_set: write用のset
 * @return phase
 * @note エラー時はStatusExceptionの送出
 */
enum phase CGIPython::check_and_handle(fd_set& read_set, fd_set& write_set)
{
	// 時間制限チェック
	try
	{
		if(_write_fd != -1 && utility::isset_clr_fd(_write_fd, write_set))
		{
			Log("cgi write");
			_write();
		}
		if(_read_fd != -1 && utility::isset_clr_fd(_read_fd, read_set))
		{
			Log("cgi read");
			_read();
		}
		if(_write_fd == -1 && _read_fd == -1)
		{
			Log("finish cgi");
			return SEND;
		}
		else if(_time.check())
		{
			_kill_process();
		}
	}
	catch(const http::StatusException& e)
	{
		_read_or_write_error(e.get_http_status());
	}
	catch(const std::exception& e)
	{
		_read_or_write_error(500);
	}
	return CGI;
}

HttpString CGIPython::get_data() const
{
	return _data;
}