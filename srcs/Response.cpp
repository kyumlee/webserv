# include "./../includes/Response.hpp"

Response::Response()
	: _remainSend(),
	_totalSendSize(),
	_sendStartPos(),
	_totalResponse()
{}

Response::Response (const Response& response)
	: _remainSend(response._remainSend),
	_totalSendSize(response._totalSendSize),
	_sendStartPos(response._sendStartPos),
	_totalResponse(response._totalResponse)
{}
Response::~Response() {}

Response&	Response::operator=(const Response& response)
{
	_remainSend = response._remainSend;
	_totalSendSize = response._totalSendSize;
	_sendStartPos = response._sendStartPos;
	_totalResponse = response._totalResponse;
	return (*this);
}

int			Response::getRemainSend() const { return (_remainSend); }
size_t		Response::getTotalSendSize() const { return (_totalSendSize); }
std::string	Response::getTotalResponse() const { return (_totalResponse); }
size_t		Response::getSendStartPos() const { return (_sendStartPos); }

void		Response::setRemainSend(int value) { _remainSend = value; }
void		Response::setTotalSendSize(size_t size) { _totalSendSize = size; }
void		Response::setTotalResponse(const std::string& response) { _totalResponse = response; }
void		Response::setSendStartPos(const size_t& startPos) { _sendStartPos = startPos; }

void		Response::initResponseValue() { _remainSend = false; }

int			Response::checkAllowedMethods()
{

	if (_possibleMethods.find(_method) != _possibleMethods.end() && _allowedMethods.find(_method) == _allowedMethods.end())
	{
		_code = Method_Not_Allowed;
		return (1);
	}

	if (_possibleMethods.find(_method) == _possibleMethods.end())
	{
		_code = Method_Not_Allowed;
		return (1);
	}

	return (0);
}

std::string	Response::responseErr()
{
	if (_errorHtml.find(_code) != _errorHtml.end())
		_path = _errorHtml[_code];

	std::string	header = getHeader();
	std::map<int, std::string>::iterator	it = _errorHtml.find(_code);

	if (it != _errorHtml.end())
		header += readHtml(it->second);

	return (header);
}

int			Response::verifyMethod(int fd, int requestEnd, int autoindex, std::string index, Cgi& cgi)
{
	int			error = 0, ret = 0;

	if (_code == Bad_Request || _code == Internal_Server_Error || _code == Payload_Too_Large)
	{
		error = 1;
		_totalResponse = responseErr();
		ret = 2;
	}
	else
	{
		if (requestEnd)
		{
			if (getPath() == "/" && _method != "GET")
			{
				error = 1;
				setCode(Method_Not_Allowed);
				_totalResponse = responseErr();
			}
			else if (getRemainSend() == false)
			{
				if (checkAllowedMethods() == 1)
					_totalResponse = getHeader();
				else if (_method == "GET")
					_totalResponse = execGET(_path, autoindex, index);
				else if (_method == "HEAD")
					_totalResponse = execHEAD(_path, autoindex, index);
				else if (_method == "POST")
					_totalResponse = execPOST(_path, _body, cgi);
				else if (_method == "PUT")
					_totalResponse = execPUT(_path, _body);
				else if (_method == "DELETE")
					_totalResponse = execDELETE(_path);

				std::map<int, std::string>::iterator	it = _errorHtml.find(_code);
				if (it != _errorHtml.end())
					_totalResponse += readHtml(it->second);
				else if (_code != Created && _code != No_Content && _code != OK)
					_totalResponse += ERROR_HTML;
			}
		}
	}

	std::string	sendMsg;
	int			writeSize;
	size_t		sendMsgSize;

	if (_totalResponse.length() - _sendStartPos >= CGI_BUFFER_SIZE)
	{
		sendMsg = _totalResponse.substr(_sendStartPos, CGI_BUFFER_SIZE);
		sendMsgSize = CGI_BUFFER_SIZE;
		_remainSend = true;
	}
	else if (_remainSend == true)
	{
		sendMsg = _totalResponse.substr(_sendStartPos, _totalResponse.length() - _sendStartPos);
		sendMsgSize = _totalResponse.length() - _sendStartPos;
		_remainSend = false;
	}
	else
	{
		sendMsg = _totalResponse;
		sendMsgSize = _totalResponse.length();
	}
	writeSize = ::send(fd, sendMsg.c_str(), sendMsgSize, 0);
	if (error || writeSize == -1)
	{
		if (!error)
			printErr("::send()");
		_totalResponse.clear();
		return (1);
	}
	_sendStartPos += writeSize;
	_totalSendSize += writeSize;
	if (writeSize != static_cast<int>(sendMsgSize))
	{
		_remainSend = true;
		return (0);
	}

	if (_remainSend == false)
	{

		_sendStartPos = 0;
		_totalSendSize = 0;
		if (getCode() == Internal_Server_Error)
			ret = 1;
		std::cout << RED << "#######response[" << fd << "] send!!!!!" << std::endl << RESET;
		if (_totalResponse.length() > 300)
			std::cout << YELLOW << _totalResponse.substr(0, 250) << " ... " << _totalResponse.substr(_totalResponse.length() - 20, 20) << RESET << std::endl;
		else
			std::cout << YELLOW << _totalResponse << RESET << std::endl;
		_totalResponse.clear();
	}
	
	return (ret);
}

std::string	Response::execGET(std::string& path, int autoindex, std::string index)
{
	std::string			fileContent = "";
	std::ifstream		file;
	std::stringstream	buffer;
	std::string			html = setHtml(path, "en", "utf-8", "directory listing", sizetToStr(_listen.host), _listen.port);

	if (autoindex == OFF && html != "")
	{
		setCode(OK);
		_contentType = "text/html";
		buffer << html;
	}
	else
	{
		if (autoindex == ON && pathIsFile(path) == 2)
		{
			if (path == "/")
				path = getRoot();
			if (path.back() != '/')
				path += "/";
			path += index;
		}
		if (pathIsFile(path) == 1)
		{
			file.open(path.c_str(), std::ifstream::in);
			if (file.is_open() == false)
			{
				std::cerr << GREEN << "@@@@@@@@@@@@@@@@@file open fail@@@@@@@@@@@@@@@@\n";
				std::cerr << path << " failed to open so code is Internal_Server_Error\n" << RESET;
				setCode(Internal_Server_Error);
			}
			else
			{
				// std::cout << YELLOW << "get response path: " << path << RESET << std::endl;
				setCode(OK);
				buffer << file.rdbuf();
			}
		}
		else
		{
			buffer << path << " is not file\n";
			setCode(Not_Found);
		}
	}
	setContentLength(buffer.str().length());
	fileContent = getHeader();
	fileContent += buffer.str();
	file.close();

	return (fileContent);
}

std::string	Response::execHEAD(std::string& path, int autoindex, std::string index)
{
	std::string			fileContent = execGET(path, autoindex, index);
	fileContent = fileContent.substr(0, getHeader().length());
	return (fileContent);
}

std::string	Response::execPOST(const std::string& path, const std::string& body, Cgi& cgi)
{

	std::ofstream	file;
	std::string		fileContent;

	if (cgi.getCgiExist() == true && getRemainSend() == false)
	{
		fileContent = cgi.executeCgi(cgi.getName());

		std::string	tmpHttp = "HTTP/1.1";
		fileContent.erase(0, 7);
		fileContent.insert(0, tmpHttp);

		if (fileContent.find("\r\n") != std::string::npos)
		{
			size_t	insertPos = fileContent.find("\r\n") + 2;
			fileContent.insert(insertPos, "Content-Length: " + _contentLength + "\r\n");
		}
		if (fileContent.find("Content-Type:") != std::string::npos)
		{
			setHeader();
			size_t	insertPos = fileContent.find("Content-Type:");
			insertPos = fileContent.find("\r\n", insertPos) + 2;
			fileContent.insert(insertPos, "Date: " + _date + "\r\nLast-Modified: " + _lastModified + "\r\nServer: " + _server + "\r\nTransfer-Encoding: identity\r\n");
		}
		setCode(OK);
		_contentType = "text/html";
	}

	else if (cgi.getCgiExist() == false)
	{
		if (pathIsFile(path))
		{
			file.open(path.c_str(), std::ofstream::out | std::ofstream::app);
			if (file.is_open() == false)
			{
				printErr("failed to open file");
				setCode(Forbidden);
			}
			file << body;
			file.close();
			setCode(No_Content);
		}
		else
		{
			file.open(path.c_str(), std::ofstream::out);
			if (file.is_open() == false)
			{
				printErr("failed to open file");
				setCode(Not_Found);
			}
			file << body;
			file.close();
			setCode(Created);
		}
		fileContent = getHeader();
	}

	return (fileContent);
}

std::string	Response::execPUT(const std::string& path, std::string& body)
{
	std::ofstream	file;
	std::string		fileContent;

	if (pathIsFile(path))
	{
		file.open(path.c_str());
		if (file.is_open() == false)
		{
			printErr("failed to open file");
			setCode(Forbidden);
		}
		else
			setCode(No_Content);
		file << body;
		file.close();
	}
	else
	{
		file.open(path.c_str());
		if (file.is_open() == false)
		{
			printErr("failed to open file");
			setCode(Forbidden);
		}
		else
			setCode(Created);
		file << body;
		file.close();
	}
	fileContent = getHeader();
	return (fileContent);
}

std::string	Response::execDELETE(std::string& path)
{
	std::string	fileContent;

	if (pathIsFile(path))
	{
		if (remove(path.c_str()) == 0)
			setCode(No_Content);
		else
		{
			printErr("failed to remove file");
			setCode(Forbidden);
		}
	}
	else
	{
		printErr("failed to remove file");
		setCode(Not_Found);
	}
	fileContent = getHeader();
	return (fileContent);
}

std::string	Response::readHtml(const std::string& path)
{
	std::ofstream			file;
	std::stringstream		buf;
	std::string				error_ret;

	error_ret = "error code: " + intToStr(getCode()) + " " + path;
	if (pathIsFile(path))
	{
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			// return (ERROR_HTML);
			error_ret += " open fail\n";
			return (error_ret);
		}
		buf << file.rdbuf();
		file.close();
		_contentType = "text/html";
		return (buf.str());
	}
	else
	{
		// return (ERROR_HTML);
		error_ret += "is not file\n";
		return (error_ret);
	}
}

void		Response::printResponseValue()
{
	std::cout << WHITE;
	printGeneralHeader();
	printEntityHeader();
	printRequestHeader();
	printResponseHeader();
	std::cout << RESET;
}
