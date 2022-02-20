#include "FileData.hpp"

/**
 * @brief statを行ってfileのデータを確認する
 * @return statが成功した場合true
 */
bool FileData::_check_filedata()
{
	if(stat(_filepath.c_str(), &_sb) == -1)
	{
		return false;
	}
	_ready = true;
	return true;
}

FileData::FileData()
	: _ready(false)
{ }

FileData::~FileData() { }

/**
 * @brief file情報をクリアにする
 */
void FileData::clear()
{
	_filepath.clear();
	_ready = false;
}

/**
 * @brief fileが存在するかどうかを確認する
 * @return 存在した場合true
 * @note 存在の確認はstatが成功したかどうかで行っている
 */
bool FileData::exist()
{
	if(!_ready)
	{
		if(!_check_filedata())
		{
			return false;
		}
	}
	return true;
}

/**
 * @brief 通常のfileかどうかを確認する
 * @return 通常のfileだった場合true
 */
bool FileData::is_file()
{
	if(!_ready)
	{
		if(!_check_filedata())
		{
			return false;
		}
	}
	return ((_sb.st_mode & S_IFMT) == S_IFREG);
}

/**
 * @brief ディレクトリかどうかを確認する
 * @return ディレクトリだった場合true
 */
bool FileData::is_dir()
{
	if(!_ready)
	{
		if(!_check_filedata())
		{
			return false;
		}
	}
	return ((_sb.st_mode & S_IFMT) == S_IFDIR);
}

/**
 * @brief fileのサイズを取得する
 * @return fileのサイズ。取得できなかった場合-1
 */
ssize_t FileData::get_filesize()
{
	if(!_ready)
	{
		if(!_check_filedata())
		{
			return -1;
		}
	}
	return _sb.st_size;
}

/**
 * @brief filepathをセットする
 * @param path: filepath
 */
void FileData::set_filepath(const std::string& path)
{
	_filepath = path;
}
