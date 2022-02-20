#include "Parser.hpp"

const char* server_properties[5] = {"listen", "server_name", "error_page", "root", 0};

const char* route_properties[11] = {"method",
									"root",
									"autoindex",
									"index",
									"cgi_path",
									"upload_enable",
									"upload_path",
									"client_max_body_size",
									"return",
									0};

const char* methods[9] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", 0};

/**
 * @brief contextの終了位置を返す
 * @param source: 設定ファイルの文字列
 * @param line: contextの開始位置
 * @return contextの終了位置
 * @example "server {\na\nb\nc\n}" with line 0 will return line 4
 */
size_t get_closing_bracket(std::string source, size_t line)
{
	size_t n;
	size_t size;
	size_t open_brackets;

	open_brackets = 0;
	if(get_line(source, line)[get_line(source, line).size() - 1] != '{')
		throw ParsingException(line, "Expected '{'.");
	n = line + 1;
	size = count_lines(source);
	while(n < size)
	{
		if(!is_skippable(source, n) && end_with_open_bracket(source, n))
			++open_brackets;
		if(!is_skippable(source, n) && get_line(source, n) == "}")
		{
			if(open_brackets == 0)
				return (n);
			--open_brackets;
		}
		++n;
	}
	throw ParsingException(line, "Expected '}'.");
	return (-1);
}

/**
 * @brief 該当の行が"{" で終わっているかチェックする
 * @param source: 設定ファイルの文字列
 * @param line: チェックする位置
 * @return True/False
 */
bool end_with_open_bracket(std::string source, size_t line)
{
	std::vector< std::string > splits;

	splits = split_white_space(get_line(source, line));
	if(splits.size() > 0)
	{
		if(splits[splits.size() - 1] == "{")
			return (true);
	}
	return (false);
}

/**
 * @brief ファイルの中身をstd::stringにして返す
 * @param file: fileパス
 * @return std::string
 */
std::string read_file(std::string file)
{
	char buffer[BUFFER_SIZE + 1] = {0};
	int fd;
	int i;
	int res;
	std::string result;

	fd = open(file.c_str(), O_RDONLY);
	if(fd < 0)
	{
		throw ParsingException(0, "The file " + file + " does not exists.");
	}
	while((res = read(fd, buffer, BUFFER_SIZE)) > 0)
	{
		result += buffer;
		i = 0;
		while(i < BUFFER_SIZE)
			buffer[i++] = 0;
	}
	if(res < 0)
		throw ParsingException(0, "Error while reading " + file + ".");
	close(fd);
	return (result);
}

/**
 * @brief n行目の内容を前後の空白をトリムして返す
 * @param source: 設定ファイルの文字列
 * @param n: ほしい行
 * @return n行目の文字列（前後空白トリム済）
 */
std::string get_line(std::string source, size_t n)
{
	size_t i;
	size_t j;
	size_t line_count;

	if(n >= count_lines(source))
		return (std::string());
	line_count = 0;
	i = 0;
	while(line_count < n)
	{
		if(source[i++] == '\n')
			++line_count;
	}
	while(std::isspace(source[i]) && source[i] != '\n')
		++i;
	j = 0;
	while(source[i + j] && source[i + j] != '\n')
		++j;
	while(j > 0 && std::isspace(source[i + j - 1]))
		--j;
	return (std::string(source, i, j));
}

/**
 * @brief 文字列の行数を返す
 */
size_t count_lines(std::string source)
{
	size_t i;
	size_t lines;

	i = 0;
	lines = 1;
	while(source[i])
	{
		if(source[i++] == '\n')
			++lines;
	}
	return (lines);
}

/**
 * @brief parameterが適当かのチェックを行い、paramsを分割してvectorで返す
 * @example "listen 80 localhost;"  -> "listen", "80" "localhost" のvector
 */
std::vector< std::string > parse_property(std::string source, size_t line, std::string object)
{
	std::vector< std::string > result;
	std::string l;

	l = get_line(source, line);
	if(l[l.size() - 1] != ';')
		throw ParsingException(line, "Expected ';'.");
	l = std::string(l, 0, l.size() - 1);
	result = split_white_space(l);
	if(result.size() <= 1)
		throw ParsingException(line, "Properties should have at least one value.");
	if(!is_property_name(result[0], object == "server" ? server_properties : route_properties))
		throw ParsingException(line,
							   "'" + result[0] + "' is not a valid property for " + object + ".");
	return (result);
}

/**
 * @brief 文字列を空白で分割してvectorで返す
 */
std::vector< std::string > split_white_space(std::string str)
{
	std::vector< std::string > res;
	size_t i;
	size_t j;

	i = 0;
	j = 0;
	while(str[i])
	{
		if(std::isspace(str[i]))
		{
			if(i == j)
				++j;
			else
			{
				res.push_back(std::string(str, j, i - j));
				j = i + 1;
			}
		}
		++i;
	}
	if(i != j)
		res.push_back(std::string(str, j, i - j));
	return (res);
}

/**
 * @brief 読み込んだparameterが適当かチェックする
 */
bool is_property_name(std::string name, const char** valid_names)
{
	size_t i;

	i = 0;
	while(valid_names[i])
	{
		if(name == valid_names[i])
			return (true);
		++i;
	}
	return (false);
}

/**
 * @brief 該当行がスキップ可能かチェックする（空白行 or #コメント行）
 * @param source: 設定ファイル
 * @param line: 調べる行
 * @return スキップ可能か否か
 */
bool is_skippable(std::string source, size_t line)
{
	std::string l;

	l = get_line(source, line);
	return (split_white_space(l).size() == 0 || l.size() == 0 || l[0] == '#');
}

/**
 * @brief std::stringをsize_tに変換
 */
size_t convert_to_size_t(std::string param, size_t line)
{
	size_t value;
	std::istringstream convert(param);

	if(!(convert >> value))
		throw ParsingException(line, "'" + param + "' is not a positive integer.");
	return (value);
}

/**
 * @brief HTTPメソッドが適当かチェックする
 */
bool is_method(std::string method)
{
	size_t i;

	i = 0;
	while(methods[i])
	{
		if(methods[i] == method)
			return (true);
		++i;
	}
	return (false);
}

/**
 * @brief autoindexの on/offをboolに変換する
 */
bool param_to_bool(std::string param, size_t line)
{
	if(param == "on")
		return (true);
	else if(param == "off")
		return (false);
	else
		throw ParsingException(line, "autoindex parameter should be \"on\" or \"off\".");
}

/**
 * @brief sed -i -e 's/to_replace/new_value/g' source
 */
std::string replace(std::string source, std::string to_replace, std::string new_value)
{
	size_t start_pos = 0;
	while((start_pos = source.find(to_replace, start_pos)) != std::string::npos)
	{
		source.replace(start_pos, to_replace.length(), new_value);
		start_pos += new_value.length();
	}
	return (source);
}

/**
 * @brief ファイルの存在を確認する
 * @ref https://www.c-lang.net/stat/index.html
 */
bool check_path(std::string path)
{
	struct stat buffer;

	int exist = stat(path.c_str(), &buffer);
	if(exist == 0)
	{
		//通常のファイルの場合
		if(S_ISREG(buffer.st_mode))
			return (true);
		//ディレクトリの場合
		else
			return (false);
	}
	else
		return (false);
}
