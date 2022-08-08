#include "./../includes/Utils.hpp"

int							printErr(std::string errMsg)
{
	std::cerr << "ERROR: " << errMsg << std::endl;
	return (1);
}

std::vector<std::string>	split(std::string str, char delimiter)
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

std::string					trim(std::string str)
{
	size_t		startPos = 0, endPos = str.length() - 1;

	while (std::isspace(str[startPos]))
		startPos++;
	while (std::isspace(str[endPos]))
		endPos--;

	return (str.substr(startPos, endPos - startPos + 1));
}

size_t						skipBlock(std::string block)
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

std::vector<std::string>	splitBlocks(std::string block, std::string type)
{
	std::vector<std::string>	ret;
	size_t						startPos = 0, endPos = 0;

	while (block.find(type, endPos) != std::string::npos) {
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

std::pair<bool, size_t>		skipKey(std::string line, std::string key, std::string delimiter)
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

bool						isNumber(std::string str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			return (0);
	}
	return (1);
}

std::string					parseValue(std::string line, size_t pos, std::string delimiter)
{
	size_t	scPos = line.find(delimiter, pos);
	return (trim(line.substr(pos, scPos - pos)));
}

int							strToInt(std::string str)
{
	int					ret;
	std::stringstream	ssInt(str);

	ssInt >> ret;

	return (ret);
}

std::string					intToStr(int code)
{
	std::stringstream	ret;
	ret << code;

	return (ret.str());
}

std::string					sizetToStr(size_t code)
{
	std::stringstream	ret;
	ret << code;

	return (ret.str());
}

int							compareURIs(std::string URI, std::string request, int mod)
{
	size_t		pos = URI.find('/') + 1;
	char		start;
	std::string	temp;
	std::string	temp2;

	temp = &URI[pos];
	if (URI[pos] != '*')
	{
		if (temp.find(&request[1], 0) != std::string::npos)
			return (0);
		return (1);
	}

	start = URI[pos + 1];
	temp = &URI[pos + 1];
	if (request.find(start, 0) == std::string::npos)
		return (1);
	temp2 = request.substr(request.find(start, 0), request.length() - request.find(start, 0));

	if (mod == NONE)
	{
		if (temp.find(temp2, 0) != std::string::npos)
			return (0);
	}

	return (1);
}

int							pathIsFile(const std::string& path)
{
	struct stat	s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return (1);
		if (s.st_mode & S_IFDIR)
			return (2);
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

std::string					setHtml(const std::string& path, const std::string& lang, const std::string& charset, const std::string& h1, const std::string& host, const int port)

{
	DIR*	dir = opendir(path.c_str());
	if (dir == NULL)
		return "";

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
		html += setUri(std::string(dirList->d_name), dirName, host, port);
	html += "\t</body>\n</html>";

	closedir(dir);

	return (html);
}

int							setErrorPages(const std::string& errPages, std::map<int, std::string>* errMap)
{
	std::vector<std::string>	err = split(errPages, ' ');
	std::string	html = *(--err.end());
	err.erase(err.end() - 1);
	if (html.find(".html") == std::string::npos)
	{
		std::cout << "There is no error page html" << std::endl;
		return (1);
	}
	for (std::vector<std::string>::iterator it = err.begin(); it != err.end(); it++)
	{
		if (isNumber(*it) == 0)
		{
			std::cout << "error code has problem" << std::endl;
			errMap->clear();
			return (1);
		}
		(*errMap)[atoi((*it).c_str())] = html;
	}
	return (0);
}

void						printVec(std::vector<std::string> strVec)
{
	for (std::vector<std::string>::iterator it = strVec.begin();
		it != strVec.end(); it++)
		std::cout << *it << ", ";
	std::cout << std::endl;
}

void						printSet(std::set<std::string> strSet)
{
	for (std::set<std::string>::iterator it = strSet.begin();
		it != strSet.end(); it++)
		std::cout << *it << ", ";
	std::cout << std::endl;
}

void						printErrmap(std::map<int, std::string> errmap)
{
	std::cout << "==================err map==================" << std::endl;
	for (std::map<int, std::string>::iterator it = errmap.begin();
		it != errmap.end(); it++)
	{
		std::cout << (*it).first << ": " << (*it).second << ", ";
	}
	std::cout << std::endl;
}

int							compareEnd(const std::string& s1, const std::string& s2)
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

std::string					findExtension(std::string& file)
{
	size_t	extensionStart = file.find_last_of('.');
	if (extensionStart == std::string::npos)
		return ("");
	std::string	extension = file.substr(extensionStart + 1, file.length() - extensionStart - 1);
	return (extension);
}

std::string					findFileName(const std::string& path)
{
	size_t	fileNameStart = path.find_last_of('/');
	if (fileNameStart == std::string::npos)
		return (path);
	std::string	fileName = path.substr(fileNameStart + 1, path.length() - fileNameStart - 1);
	return (fileName);
}

std::string					findFileType(const std::string& file)
{
	if (file.length() == 1)
		return ("");
	size_t	fileTypeStart = file.find_first_of('.');
	if (fileTypeStart == std::string::npos)
		return ("");
	std::string	fileType = file.substr(fileTypeStart + 1, file.length() - fileTypeStart - 1);
	return (fileType);
}

std::string					eraseFileType(const std::string& file)
{
	if (file.length() == 1)
		return (file);
	size_t	fileTypeStart = file.find_first_of('.');
	if (fileTypeStart == std::string::npos)
		return (file);
	std::string	pureFileName = file.substr(0, fileTypeStart);
	return (pureFileName);
}

std::string					findHeaderValue(const std::string& header)
{
	size_t	colonPos = header.find(":");
	if (colonPos == std::string::npos)
		return ("");
	std::string	value = header.substr(colonPos + 1, header.length() - colonPos - 1);
	value = strTrimChar(value);
	return (value);
}

std::string					strTrimChar(const std::string& str, char deleteChar)
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

std::string					strDeleteRN(const std::string& str)
{
	std::string	retStr(str);
	size_t		rPos = 0;

	if ((rPos = retStr.find("\r\n")) != std::string::npos)
		retStr = retStr.substr(0, rPos);
	return (retStr);
}

std::string					allDeleteRN(const std::string& str)
{
	std::string	ret = str;
	while (ret.find("\r") != std::string::npos)
	{
		size_t	r_pos = ret.find("\r");
		ret.erase(ret.begin() + r_pos);
	}
	while (ret.find("\n") != std::string::npos)
	{
		size_t	n_pos = ret.find("\n");
		ret.erase(ret.begin() + n_pos);
	}
	return (ret);
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

int							makeHtml(const std::string& htmlName, int code, const std::string& codeStr, const std::string& serverName)
{
	std::ofstream	htmlFile;
	std::string		fileContent;

	htmlFile.open(htmlName);
	if (htmlFile.is_open() == false)
	{
		printErr("failed to make html");
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

bool					hostToInt(std::string host, t_listen* listen)
{
	size_t			sep = 0, start = 0;
	unsigned int	n, ret = 0;
	std::string		substr;

	if (host == "localhost")
		host = "127.0.0.1";

	if (isNumber(host) == 1)
	{
		ret = std::atoi(host.c_str());
		listen->host = ret;
		return (0);
	}
	for (int i = 3; i > -1; i--)
	{
		sep = host.find_first_of('.', sep);

		if (i != 0 && sep == std::string::npos)
			return (printErr("missing . in host address"));

		if (i == 0)
			sep = host.length();

		substr = host.substr(start, sep - start);
		if (isNumber(substr) == 0)
			return (printErr("host address is not a number"));

		n = std::atoi(substr.c_str());
		for (int j = 0; j < i; j++)
			n *= 256;
		ret += n;
		sep++; start = sep;
	}
	listen->host = ret;

	return (0);
}

int						setListen(std::string strHost, t_listen* listen)
{
	if (strHost == "")
		return (printErr("host header doesn't exist"));

	std::vector<std::string>	hostPort;
	unsigned int				host;
	int							port;

	hostPort = split(strHost, ':');

	if (*hostPort.begin() == strHost)
	{
		listen->port = htons(DEFAULT_PORT);
		if ((host = hostToInt(strHost, listen)) == 1)
			return (printErr("invalid host"));

		listen->host = htonl(host);

		return (0);
	}

	if (isNumber(*(hostPort.begin() + 1)) == 0)
		return (printErr("port is not a number"));

	port = std::atoi((*(hostPort.begin() + 1)).c_str());
	listen->port = htons(port);

	if ((host = hostToInt(*hostPort.begin(), listen)) == 1)
		return (1);

	listen->host = htonl(host);

	return (0);
}

size_t						calExponent(const std::string& str)
{
	size_t	ePos = str.find("e");
	std::string	numStr;
	std::string	exponentStr;
	size_t		num;
	size_t		exponent;
	if (ePos != std::string::npos)
	{
		numStr = str.substr(0, ePos);
		exponentStr = str.substr(ePos + 1, str.length() - ePos - 1);
		num = std::atoi(numStr.c_str());
		exponent = std::atoi(exponentStr.c_str());
		return (num * powl(10, exponent));
	}
	else
		num = std::atoi(str.c_str());
	return (num);
}

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

size_t						hexToDecimal(std::string& hex)
{
	size_t	ret = 0;
	std::string::iterator	hexIt = hex.begin();
	if (checkHex(hex) == 0)
		return (0);
	while (*hexIt == '0')
		hexIt++;
	for (; hexIt != hex.end(); hexIt++)
	{
		size_t	num = *hexIt;
		if (num >= 'a' && num <= 'f')
			num = num - 'a' + 10;
		else if (num >= '0' && num <= '9')
			num -= '0';
		ret = ret * 16 + num;
	}
	return (ret);
}

void						printStr(const std::string& str, const std::string& response)
{
	if (response == "response")
		std::cout << YELLOW << "============response============" << std::endl << RESET;
	else if (response == "request")
		std::cout << GREEN << "============request============" << std::endl << RESET;

	if (str.length() > 200)
		std::cout << str.substr(0, 150) << "..." << str.substr(str.length() - 20, 20) << std::endl;
	else
		std::cout << str << std::endl;

	std::cout << "total size: " << str.length() << std::endl;
}
