#include "./../includes/Utils.hpp"

int							printErr (std::string errMsg)
{
	std::cerr << "Error: " << errMsg << std::endl;
	return (1);
}

std::vector<std::string>	split (std::string str, char delimiter)
{
	size_t						startPos = 0, endPos;
	std::vector<std::string>	ret;
	std::string					token;

	if (str.find(delimiter, 0) == std::string::npos)
	{
		ret.push_back(str);
		return (ret);
	}

	while ((endPos = str.find(delimiter, startPos)) != std::string::npos)
	{
		token = str.substr(startPos, endPos - startPos);
		startPos = endPos + 1;
		ret.push_back(token);

	}
	ret.push_back(str.substr(startPos));

	return (ret);
}

std::string					trim (std::string str)
{
	size_t		startPos = 0, endPos = str.length() - 1;

	while (std::isspace(str[startPos]))
		startPos++;
	while (std::isspace(str[endPos]))
		endPos--;

	return (str.substr(startPos, endPos - startPos + 1));
}

size_t						skipBlock (std::string block)
{
	size_t	i = 1;

	while (block[i] != '}')
	{
		if (block[i] == '{')
			i += skipBlock(&block[i]);
		else
			i++;
	}
	i++;

	return (i);
}

std::vector<std::string>	splitBlocks (std::string block, std::string type)
{
	std::vector<std::string>	ret;
	size_t						startPos = 0, endPos = 0;

	while (block.find(type, endPos) != std::string::npos)
	{
		startPos = block.find(type, endPos) + type.length() - 1;

		while (std::isspace(block[startPos]))
			startPos++;
		if (block[startPos] == '{')
			startPos++;
		endPos = startPos;

		if (type == "location ")
		{
			while (block[endPos] != '{')
				endPos++;
			endPos++;
		}

		while (block[endPos] != '}')
		{
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

std::pair<bool, size_t>		skipKey (std::string line, std::string key, std::string delimiter)
{
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

bool						isNumber (std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			return (0);
	}
	return (1);
}

std::string					parseValue (std::string line, size_t pos, std::string delimiter)
{
	size_t	scPos = line.find(delimiter, pos);
	return (trim(line.substr(pos, scPos - pos)));
}

int							strToInt (std::string str)
{
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

int							compareURIs (std::string URI, std::string request, int mod)
{
	size_t		pos = URI.find('/', URI.find('/', 0) + 1) + 1;
	char		start;
	std::string	temp, temp2;

	temp = &URI[pos];
	if (URI[pos] != '*')
	{
		if (mod == EXACT)
		{
			if (temp == &request[1])
				return (0);
		}

		if (mod == NONE || mod == PREFERENTIAL)
		{
			if (temp.find(&request[1], 0) != std::string::npos)
				return (0);
		}

		return (1);
	}

	start = URI[pos + 1];
	temp = &URI[pos + 1];
	temp2 = request.substr(request.find(start, 0), request.length() - request.find(start, 0));

	if (mod == EXACT)
	{
		if (temp == temp2)
			return (0);
	}

	if (mod == NONE || mod == PREFERENTIAL)
	{
		if (temp.find(temp2, 0) != std::string::npos)
			return (0);
	}

	return (1);
}

unsigned int				hostToInt (std::string& host)
{
	std::cout << "HOST TO INT" << std::endl;
	size_t			sep = 0, start = 0;
	unsigned int	n, ret = 0;
	std::string		substr;

	if (host == "localhost")
		host = "127.0.0.1";

	for (int i = 3; i > -1; i--)
	{
		sep = host.find_first_of('.', sep);
		if (i != 0 && sep == std::string::npos)
		{
			printErr("invalid host address (missing .)");
			return (0);
		}
		if (i == 0)
			sep = host.length();

		substr = host.substr(start, sep - start);

		if (isNumber(substr) == 0)
		{
			printErr("invalid host address (not a number)");
			return (0);
		}

		n = std::atoi(substr.c_str());
		for (int j = 0; j < i; j++)
			n *= 256;

		ret += n;
		sep++; start = sep;
	}

	return (ret);
}

//파일이 REG(regular file)이면 1을 리턴하고 다른 경우에는 0을 리턴한다.
int							pathIsFile (const std::string& path)
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

std::string					setURI (const std::string& dirList, const std::string& dirName, const std::string& host, const int port)
{
	std::stringstream	ss;
	ss << "\t\t<p><a href=\"http://" + host + ":" <<\
		port << dirName + "/" + dirList + "\">" + dirList + "</a></p>\n";
	return (ss.str());
}

std::string					setHtml (const std::string& path, const std::string& lang, const std::string& charset, const std::string& h1, const std::string& host, const int port)
{
	DIR*	dir = opendir(path.c_str());

	//dir을 열지 못했을 때 어떤 값을 리턴할 지 생각하자
	if (dir == NULL)
	{
		printErr("could not open " + path);
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
		html += setURI(std::string(dirList->d_name), dirName, host, port);
	html += "\t</body>\n</html>";
	closedir(dir);
	return (html);
}

int							setErrorPage (const std::string& errPages, std::map<int, std::string>* errMap)
{
	//errPages를 공백으로 나눈 것을 저장
	std::vector<std::string>	err = split(errPages, ' ');
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

void						printVec (std::vector<std::string> strVec)
{
	for (std::vector<std::string>::iterator it = strVec.begin(); it != strVec.end(); it++)
		std::cout << *it << ", ";
	std::cout << std::endl;
}

void						printSet (std::set<std::string> strSet)
{
	for (std::set<std::string>::iterator it = strSet.begin(); it != strSet.end(); it++)
		std::cout << *it << ", ";
	std::cout << std::endl;
}

void						printErrmap (std::map<int, std::string> errmap)
{
	std::cout << "==================err map==================\n";
	for (std::map<int, std::string>::iterator it = errmap.begin(); it != errmap.end(); it++)
		std::cout << (*it).first << ": " << (*it).second << ", ";
	std::cout << std::endl;
}

//s1의 끝부분에 s2가 있다면 0을 리턴, s2가 없다면 1을 리턴
int							compareEnd (const std::string& s1, const std::string& s2)
{
	size_t	s1End = s1.size();
	size_t	s2End = s2.size();

	while (s2End > 0)
	{
		s1End--; s2End--;
		if (s1End < 0 || s1[s1End] != s2[s2End])
			return (1);
	}
	return (0);
}

std::string					getFileName (const std::string& path)
{
	size_t	fileNameStart = path.find_last_of('/');
	if (fileNameStart == std::string::npos)
		return (path);

	std::string	fileName = path.substr(fileNameStart + 1, path.length() - fileNameStart - 1);
	return (fileName);
}

std::string					getFileType (const std::string& file)
{
	if (file.length() == 1)
		return ("");

	size_t	fileTypeStart = file.find_first_of('.');
	if (fileTypeStart == std::string::npos)
		return ("");

	std::string	fileType = file.substr(fileTypeStart + 1, file.length() - fileTypeStart - 1);
	return (fileType);
}

std::string					eraseFileType (const std::string& file)
{
	if (file.length() == 1)
		return (file);

	size_t	fileTypeStart = file.find_first_of('.');
	if (fileTypeStart == std::string::npos)
		return (file);

	std::string	pureFileName = file.substr(0, fileTypeStart);
	return (pureFileName);
}

std::string					findHeaderValue (const std::string& header)
{
	size_t	colonPos = header.find(":");
	if (colonPos == std::string::npos)
		return ("");

	std::string	value = header.substr(colonPos + 1, header.length() - colonPos - 1);
	value = strTrimChar(value);
	return (value);
}

std::string					strTrimChar (const std::string& str, char deleteChar)
{
	std::string	retStr = str;
	size_t		retStart = 0;
	size_t		retEnd = 0;

	if ((retStart = retStr.find_first_not_of(deleteChar)) != std::string::npos)
		retStr = retStr.substr(retStart, retStr.length() - retStart);
	else
		return ("");
	if ((retEnd = retStr.find_last_not_of(deleteChar)) != std::string::npos)
		retStr = retStr.substr(0, retEnd + 1);
	return (retStr);
}

std::string					strDeleteRN (const std::string& str)
{
	std::string	retStr(str);
	size_t		rPos = 0;

	if ((rPos = retStr.find("\r\n")) != std::string::npos)
		retStr = retStr.substr(0, rPos);
	return (retStr);
}

int							isStrAlpha (const std::string& str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
	{
		if (std::isalpha(*it) == 0)
			return (0);
	}
	return (1);
}

int							isStrUpper (const std::string& str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
	{
		if (std::isupper(*it) == 0)
			return (0);
	}
	return (1);
}

int							makeHtml (const std::string& htmlName, int code,
	const std::string& codeStr, const std::string& serverName)
{
	std::ofstream	htmlFile;
	std::string		fileContent;

	htmlFile.open(htmlName);
	if (htmlFile.is_open() == false)
	{
		printErr("HTML file");
		return (Internal_Server_Error);
	}
	fileContent += "<html>\n"
	"<head><title>" + intToStr(code) + " " + codeStr + "</title></head>\n"
	"<body>\n<center><h1>" + intToStr(code) + " " + codeStr + "</h1></center>\n"
	"<hr><center>" + serverName + "</center>\n"
	"</body>\n"
	"</html>";
	htmlFile << fileContent;
	htmlFile.close();
	return (0);
}

// 슬래쉬의 맨 뒤쪽으로 가서 파일 이름을 찾고, 슬래쉬가 없다면 그대로 path를 반환
size_t						calExponent (const std::string& str)
{
	size_t	e_pos = str.find("e"), num, exponent;
	std::string	numStr, exponentStr;

	if (e_pos != std::string::npos)
	{
		numStr = str.substr(0, e_pos);
		exponentStr = str.substr(e_pos + 1, str.length() - e_pos - 1);
		num = std::atoi(numStr.c_str());
		exponent = std::atoi(exponentStr.c_str());
		return (num * powl(10, exponent));
	}
	else
		num = std::atoi(str.c_str());
	return (num);
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
