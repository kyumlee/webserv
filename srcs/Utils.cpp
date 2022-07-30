#include "./../includes/Utils.hpp"

int							printErr (std::string errMsg) {
	std::cerr << "ERROR: " << errMsg << std::endl;
	return (1);
}

std::vector<std::string>	split (std::string str, char delimiter) {
	size_t						startPos = 0, endPos;
	std::vector<std::string>	ret;
	std::string					token;

	if (str.find(delimiter, 0) == std::string::npos) {
		ret.push_back(str);
		return (ret);
	}

	while ((endPos = str.find(delimiter, startPos)) != std::string::npos) {
		token = str.substr(startPos, endPos - startPos);
		startPos = endPos + 1;
		ret.push_back(token);
	}

	ret.push_back(str.substr(startPos));

	return (ret);
}

std::string					trim (std::string str) {
	size_t		startPos = 0, endPos = str.length() - 1;

	while (std::isspace(str[startPos]))
		startPos++;
	while (std::isspace(str[endPos]))
		endPos--;

	return (str.substr(startPos, endPos - startPos + 1));
}

size_t						skipBlock (std::string block) {
	size_t	i = 1;

	while (block[i] != '}') {
		if (block[i] == '{')
			i += skipBlock(&block[i]);
		else
			i++;
	}
	i++;

	return (i);
}

std::vector<std::string>	splitBlocks (std::string block, std::string type) {
	std::vector<std::string>	ret;
	size_t						startPos = 0, endPos = 0;

	while (block.find(type, endPos) != std::string::npos) {
		startPos = block.find(type, endPos) + type.length() - 1;
		while (std::isspace(block[startPos]))
			startPos++;
		if (block[startPos] == '{')
			startPos++;
		endPos = startPos;

		if (type == "location ") {
			while (block[endPos] != '{')
				endPos++;
			endPos++;
		}

		while (block[endPos] != '}') {
			if (block[endPos] == '{')
				endPos += skipBlock(&block[endPos]);
			else
				endPos++;
		}
		if (type == "location ")
			endPos++;

		ret.push_back(block.substr(startPos, endPos - startPos));
	}

	return (ret);
}

std::pair<bool, size_t>		skipKey (std::string line, std::string key, std::string delimiter) {
	size_t	pos = line.find(key, 0);
	size_t	scPos = line.find(delimiter, pos);
	size_t	nlPos = line.find("\n", pos);
	size_t	locPos = line.find("location", 0);

	if (locPos < pos)
		return (std::make_pair(false, pos));

	if (pos == std::string::npos)
		return (std::make_pair(false, pos));

	if (scPos == std::string::npos || nlPos == std::string::npos || scPos > nlPos)
		return (std::make_pair(false, pos));

	pos += key.length();
	while (std::isspace(line[pos]))
		pos++;
	return (std::make_pair(true, pos));
}

bool						isNumber (std::string str) {
	for (size_t i = 0; i < str.size(); i++) {
		if (!std::isdigit(str[i]))
			return (0);
	}
	return (1);
}

std::string					parseValue (std::string line, size_t pos, std::string delimiter) {
	size_t	scPos = line.find(delimiter, pos);
	return (trim(line.substr(pos, scPos - pos)));
}

int							strToInt (std::string str) {
	int					ret;
	std::stringstream	ssInt(str);

	ssInt >> ret;

	return (ret);
}

std::string					intToStr (int code)
{
	std::stringstream	ret;
	ret << code;

	return (ret.str());
}

int							compareURIs (std::string URI, std::string request, int mod) {
	size_t		pos = URI.find('/', URI.find('/', 0) + 1) + 1;
	char		start;
	std::string	temp;
	std::string	temp2;

	temp = &URI[pos];
	if (URI[pos] != '*') {
		if (mod == EXACT) {
			if (temp == &request[1])
				return (0);
		}

		if (mod == NONE || mod == PREFERENTIAL) {
			if (temp.find(&request[1], 0) != std::string::npos)
				return (0);
		}

		return (1);
	}

	start = URI[pos + 1];
	temp = &URI[pos + 1];
	temp2 = request.substr(request.find(start, 0), request.length() - request.find(start, 0));

	if (mod == EXACT) {
		if (temp == temp2)
			return (0);
	}

	if (mod == NONE || mod == PREFERENTIAL) {
		if (temp.find(temp2, 0) != std::string::npos)
			return (0);
	}

	return (1);
}

/*unsigned int	host_to_int(std::string host)
{
	size_t	sep = 0;
	unsigned int	n;
	size_t	start = 0;
	std::string	substr;
	unsigned int	ret = 0;
	if (host == "localhost")
		host = "127.0.0.1";
	for (int i = 3; i > -1; i--)
	{
		sep = host.find_first_of('.', sep);
		if (i != 0 && sep == std::string::npos)
		{
			std::cerr << "host address has not .\n";
			return (0);
		}
		if (i == 0)
			sep = host.length();
		substr = host.substr(start, sep - start);
		// std::cout << "substr : " << substr << ", sep : " << sep << ", start : " << start << std::endl;
		if (isNumber(substr) == 0)
		{
			std::cerr << "host address is not number\n";
			return (0);
		}
		n = std::atoi(substr.c_str());
		for (int j = 0; j < i; j++)
			n *= 256;
		ret += n;
		sep++; start = sep;
	}
	return (ret);
}*/

//파일이 REG(regular file)이면 1을 리턴하고 다른 경우에는 0을 리턴한다.
int							pathIsFile(const std::string& path)
{
	struct stat	s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
		{
			std::cout << "file is regular file\n";
			return (1);
		}
		std::cout << "file type is " << (s.st_mode & S_IFMT) << std::endl;
		std::cout << "file is not regular file\n";
	}
	
	return (0);
}

std::string					setUri(const std::string& dirList, const std::string& dirName,
	const std::string& host, const int port)
{
	std::stringstream	ss;
	ss << "\t\t<p><a href=\"http://" + host + ":" <<\
		port << dirName + "/" + dirList + "\">" + dirList + "</a></p>\n";
	return (ss.str());
}

std::string					setHtml(const std::string& path, const std::string& lang,
	const std::string& charset, const std::string& h1, const std::string& host, const int port)
{
	DIR*	dir = opendir(path.c_str());
	if (dir == NULL)
	{
		std::cerr << "Error: could not open " << path << std::endl;
		//dir을 열지 못했을 때 어떤 값을 리턴할 지 생각하자
		return "";
	}
	std::string	dirName(path);
	if (dirName[0] != '/')
		dirName = "/" + dirName;
	std::string	html = "<!DOCTYPE html>\n";
	html += "<html lang=\"" + lang + "\">\n" + \
	"\t<head>\n" + \
	"\t\t<meta charset=\"" + charset + "\">\n" + \
	"\t\t<title>" + dirName + "</title>\n" + \
	"\t</head>\n" + \
	"\t<body>\n" + \
	"\t\t<h1>" + h1 + "</h1>\n";
	struct dirent*	dirList;
	while ((dirList = readdir(dir)) != NULL)
		html += set_uri(std::string(dirList->d_name), dirName, host, port);
	html += "\t</body>\n</html>";
	closedir(dir);
	return (html);
}

int							set_error_page(const std::string& errPages, std::map<int, std::string>* errMap)
{
	std::vector<std::string>	err = split(errPages, ' ');
	//errPages를 공백으로 나눈 것을 저장
	std::string	html = *(--err.end());
	err.erase(err.end() - 1);
	if (html.find(".html") == std::string::npos)
	{
		std::cout << "There is no error page html\n";
		return (1);
	}
	for (std::vector<std::string>::iterator it = err.begin(); it != err.end(); it++)
	{
		if (isNumber(*it) == 0)
		{
			std::cout << "error code has problem\n";
			errMap->clear();
			return (1);
		}
		(*errMap)[atoi((*it).c_str())] = html;
	}
	return (0);
}

void						print_vec(std::vector<std::string> str_vec)
{
	for (std::vector<std::string>::iterator it = str_vec.begin();
		it != str_vec.end(); it++)
		std::cout << *it << ", ";
	std::cout << std::endl;
}

void						print_set(std::set<std::string> str_set)
{
	for (std::set<std::string>::iterator it = str_set.begin();
		it != str_set.end(); it++)
		std::cout << *it << ", ";
	std::cout << std::endl;
}

void						print_errmap(std::map<int, std::string> errmap)
{
	std::cout << "==================err map==================\n";
	for (std::map<int, std::string>::iterator it = errmap.begin();
		it != errmap.end(); it++)
	{
		std::cout << (*it).first << ": " << (*it).second << ", ";
	}
	std::cout << std::endl;
}

//s1의 끝부분에 s2가 있다면 0을 리턴, s2가 없다면 1을 리턴
int							compare_end(const std::string& s1, const std::string& s2)
{
	size_t	s1_end = s1.size();
	size_t	s2_end = s2.size();
	while (s2_end > 0)
	{
		s1_end--; s2_end--;
		if (s1_end < 0 || s1[s1_end] != s2[s2_end])
			return (1);
	}
	return (0);
}

std::string					find_extension(std::string& file)
{//file의 확장자를 찾는다.
	size_t	extension_start = file.find_last_of('.');
	if (extension_start == std::string::npos)
		return ("");
	std::string	extension = file.substr(extension_start + 1, file.length() - extension_start - 1);
	return (extension);
}

//슬래쉬의 맨 뒤쪽으로 가서 파일 이름을 찾고, 슬래쉬가 없다면 그대로 path를 반환
std::string					find_file_name(const std::string& path)
{
	size_t	file_name_start = path.find_last_of('/');
	if (file_name_start == std::string::npos)
		return (path);
	std::string	file_name = path.substr(file_name_start + 1, path.length() - file_name_start - 1);
	return (file_name);
}

std::string					find_file_type(const std::string& file)
{
	if (file.length() == 1)
		return ("");
	size_t	file_type_start = file.find_first_of('.');
	if (file_type_start == std::string::npos)
		return ("");
	std::string	file_type = file.substr(file_type_start + 1, file.length() - file_type_start - 1);
	return (file_type);
}

std::string					erase_file_type(const std::string& file)
{
	if (file.length() == 1)
		return (file);
	size_t	file_type_start = file.find_first_of('.');
	if (file_type_start == std::string::npos)
		return (file);
	std::string	pure_file_name = file.substr(0, file_type_start);
	return (pure_file_name);
}

std::string					find_header_value(const std::string& header)
{
	size_t	colon_pos = header.find(":");
	if (colon_pos == std::string::npos)
		return ("");
	std::string	value = header.substr(colon_pos + 1, header.length() - colon_pos - 1);
	value = str_trim_char(value);
	return (value);
}

std::string					str_trim_char(const std::string& str, char delete_char)
{
	std::string	ret_str = str;
	size_t		ret_start = 0;
	size_t		ret_end = 0;

	if ((ret_start = ret_str.find_first_not_of(delete_char)) != std::string::npos)
		ret_str = ret_str.substr(ret_start, ret_str.length() - ret_start);
	else
		return ("");
	if ((ret_end = ret_str.find_last_not_of(delete_char)) != std::string::npos)
		ret_str = ret_str.substr(0, ret_end + 1);
	return (ret_str);
}

std::string					str_delete_rn(const std::string& str)
{
	std::string	ret_str(str);
	size_t		r_pos = 0;

	if ((r_pos = ret_str.find("\r\n")) != std::string::npos)
		ret_str = ret_str.substr(0, r_pos);
	return (ret_str);
}

int							isStrAlpha(const std::string& str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
	{
		if (std::isalpha(*it) == 0)
			return (0);
	}
	return (1);
}

int							isStrUpper(const std::string& str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
	{
		if (std::isupper(*it) == 0)
			return (0);
	}
	return (1);
}

int							make_html(const std::string& html_name, int code, const std::string& code_str, const std::string& server_name)
{
	std::ofstream	html_file;
	std::string		file_content;

	html_file.open(html_name);
	if (html_file.is_open() == false)
	{
		std::cerr << "HTML FILE MAKE ERROR\n";
		return (Internal_Server_Error);
	}
	file_content += "<html>\n"
	"<head><title>" + intToStr(code) + " " + code_str + "</title></head>\n"
	"<body>\n<center><h1>" + intToStr(code) + " " + code_str + "</h1></center>\n"
	"<hr><center>" + server_name + "</center>\n"
	"</body>\n"
	"</html>";
	html_file << file_content;
	html_file.close();
	return (0);
}

size_t						calExponent(const std::string& str)
{
	size_t	e_pos = str.find("e");
	std::string	num_str;
	std::string	exponent_str;
	size_t		num;
	size_t		exponent;
	if (e_pos != std::string::npos)
	{
		num_str = str.substr(0, e_pos);
		exponent_str = str.substr(e_pos + 1, str.length() - e_pos - 1);
		num = std::atoi(num_str.c_str());
		exponent = std::atoi(exponent_str.c_str());
		return (num * powl(10, exponent));
	}
	else
		num = std::atoi(str.c_str());
	return (num);
}

//string을 받아서 16진수로 이루어져 있으면 1을 리턴, 아니면 0을 리턴
int							checkHex(std::string& hex)
{
	for (std::string::iterator it = hex.begin(); it != hex.end(); it++)
	{
		if ((*it >= 'a' && *it <= 'f') || (*it >= '0' && *it <= '9'))
			continue ;
		else
			return (0);
	}
	return (1);
}

//string형으로 받은 hex가 16진수로 이루어져 있지 않으면 0을 리턴
size_t						hexToDecimal(std::string& hex)
{
	// std::string	hex_str = "0123456789abcdef";
	size_t	ret = 0;
	std::string::iterator	hex_it = hex.begin();
	if (checkHex(hex) == 0)
		return (0);
	while (*hex_it == '0')
		hex_it++;
	for (; hex_it != hex.end(); hex_it++)
	{
		size_t	num = *hex_it;
		if (num >= 'a' && num <= 'f')
			num = num - 'a' + 10;
		else if (num >= '0' && num <= '9')
			num -= '0';
		ret = ret * 16 + num;
	}
	return (ret);
}

// int	main()
// {
// 	std::string	str1 = "SHOW1234";
// 	std::string	str2 = "AHOQ";
// 	if (isStrUpper(str1) == 1)
// 		std::cout << "str1 is alpha\n";
// 	else
// 		std::cout << "str1 is not alpha\n";
// 	if (isStrUpper(str2) == 1)
// 		std::cout << "str2 is alpha\n";
// 	else
// 		std::cout << "str2 is not alpha\n";

// }
