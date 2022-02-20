#ifndef CGIPYTHON_HPP
#define CGIPYTHON_HPP

#include "EnvVar.hpp"
#include "FileData.hpp"
#include "Http.hpp"
#include "HttpBody.hpp"
#include "HttpHeader.hpp"
#include "HttpInfo.hpp"
#include "HttpStatus.hpp"
#include "HttpString.hpp"
#include "TimeManage.hpp"
#include "Utility.hpp"
#include "WebServ.hpp"
#include <cstdlib>
#include <cstring>
#include <string>

class CGIPython
{
private:
	enum cgiphase
	{
		ENV,
		ARG,
		PIPE,
		PROCESS
	};

	static const int BUF_SIZE = 8192;

	int _write_fd;
	int _read_fd;
	pid_t _ch_pid;
	HttpString _data;
	HttpStatus& _httpstatus;
	HttpHeader& _httpheader;
	HttpBody& _body;
	HttpInfo& _info;
	std::string& _filepath;
	TimeManage _time;
	size_t _sent_size;

	void _check_python();
	void _check_execute_file();
	void _set_env(EnvVar& env);
	void _make_arg(char*** arg);
	void _set_pipe(int pipe_c2p[2], int pipe_p2c[2]);
	void _make_process();
	void _delete_resources(char** arg, int* pipe_c2p, int* pipe_p2c, int flag);
	void _child_process(int pipe_c2p[2], int pipe_p2c[2], char** arg, char** env);
	void _set_fd(int pipe_c2p[2], int pipe_p2c[2]);
	void _first_preparation_delete(char** env, char** arg);
	void _read();
	void _write();
	void _read_or_write_error(int status);
	void _kill_process();

public:
	CGIPython(HttpInfo& info,
			  HttpStatus& httpstatus,
			  HttpHeader& httpheader,
			  HttpBody& body,
			  std::string& filepath);
	~CGIPython();

	enum phase first_preparation();
	void set_select_fd(fd_set& read_set, fd_set& write_set, int& max_fd) const;
	enum phase check_and_handle(fd_set& read_set, fd_set& write_set);

	HttpString get_data() const;

private:
	CGIPython();
	CGIPython(CGIPython const& other);
	CGIPython& operator=(CGIPython const& other);
};

#endif
