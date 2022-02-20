#include "EnvVar.hpp"

/**
 * @brief _c_envのリソースを解放する
 */
void EnvVar::_delete_c_env()
{
	if(_c_env)
	{
		for(int i = 0; _c_env[i]; ++i)
		{
			delete[] _c_env[i];
		}
		delete[] _c_env;
		_c_env = NULL;
	}
}

EnvVar::EnvVar()
	: _c_env(NULL)
{ }

EnvVar::~EnvVar()
{
	_delete_c_env();
}

std::string& EnvVar::operator[](std::string key)
{
	if(_env.find(key) == _env.end())
	{
		_env[key] = "";
	}
	return _env[key];
}

/**
 * @brief _envから_c_envを作成する
 * @note new失敗時にbad_alloc例外を送出
 */
void EnvVar::set_c_env()
{
	std::map< std::string, std::string >::const_iterator iter_begin = _env.begin();
	std::map< std::string, std::string >::const_iterator iter = iter_begin;
	std::map< std::string, std::string >::const_iterator iter_end = _env.end();
	size_t env_size = _env.size();
	if(_c_env)
	{
		_delete_c_env();
	}
	_c_env = new char*[env_size + 1];
	size_t i = 0;
	try
	{
		while(iter != iter_end)
		{
			_c_env[i] = new char[iter->first.size() + iter->second.size() + 2];
			std::strcpy(_c_env[i], iter->first.c_str());
			_c_env[i][iter->first.size()] = '=';
			std::strcpy(&_c_env[i][iter->first.size() + 1], iter->second.c_str());
			++i;
			++iter;
		}
	}
	catch(const std::exception& e)
	{
		while(i > 0)
		{
			i--;
			delete[] _c_env[i];
		}
		delete[] _c_env;
		_c_env = NULL;
		throw;
	}
	_c_env[i] = NULL;
}

/**
 * @brief NULL終端のenvを作成して返す
 * @return NULL終端のchar**のenv
 * @note new失敗時にbad_alloc例外を送出
 */
char** EnvVar::get_c_env()
{
	if(_c_env == NULL)
	{
		set_c_env();
	}
	return _c_env;
}
