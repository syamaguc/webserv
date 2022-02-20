#ifndef FILEDATA_HPP
#define FILEDATA_HPP

#include <string>
#include <sys/socket.h>
#include <sys/stat.h>

/**
 * @brief ファイルやディレクトリの情報を格納し、ファイルパスを管理するクラス
 */
class FileData
{
private:
	std::string _filepath;
	struct stat _sb;
	bool _ready; // _check_filedataを行っていたらtrue

	bool _check_filedata();

public:
	FileData();
	~FileData();
	void clear();
	bool exist();
	bool is_file();
	bool is_dir();
	ssize_t get_filesize();

	void set_filepath(const std::string& path);

private:
	FileData(FileData const& other);
	FileData& operator=(FileData const& other);
};

#endif
