# include "./../includes/Response.hpp"

Response::Response () {}
Response::Response (const Response& response) { (void)response; }
Response::~Response () {}

Response&	Response::operator= (const Response& response) { (void)response; return (*this); }

void		Response::setRemainSend (int value) { _remainSend = value; }
void		Response::setTotalSendSize (size_t size) { _totalSendSize = size; }
void		Response::setTotalResponse (const std::string& response) { _totalResponse = response; }
void		Response::setSendStartPos (const size_t& startPos) { _sendStartPos = startPos; }

int			Response::getRemainSend () const { return (_remainSend); }
size_t		Response::getTotalSendSize () const { return (_totalSendSize); }
std::string	Response::getTotalResponse () const { return (_totalResponse); }
size_t		Response::getSendStartPos () const { return (_sendStartPos); }

void		Response::initResponseValue () { _remainSend = false; }

int			Response::checkAllowedMethods ()
{
	//method를 알고는 있지만, allow가 되지 않았을 때
	if (_possibleMethods.find(_method) != _possibleMethods.end() && _allowedMethods.find(_method) == _allowedMethods.end())
	{
		_code = Method_Not_Allowed;
		return (1);
	}
	//method가 뭔지 모를 때
	if (_possibleMethods.find(_method) == _possibleMethods.end())
	{
		_code = Method_Not_Allowed;
		return (1);
	}

	return (0);
}

std::string	Response::responseErr ()
{
	if (_errorHtml.find(_code) != _errorHtml.end())
		_path = _errorHtml[_code];

	std::string	header = getHeader();
	std::map<int, std::string>::iterator	it = _errorHtml.find(_code);

	if (it != _errorHtml.end())
		header += readHtml(it->second);

	return (header);
}

//요청마다 header를 만들어야 하고 에러가 발생했을 때에 errormap을 적절히 불러와야 한다.
//request의 method를 확인한다.
int			Response::verifyMethod (int fd, int requestEnd, Cgi& cgi)
{
	int			error = 0, ret = 0;

	if (_code == Bad_Request || _code == Internal_Server_Error || _code == Payload_Too_Large)
	{
		error = 1;
		_totalResponse = responseErr();
		std::cout << PINK << "response path: " << _path << RESET << std::endl;
		std::cout << BLUE << "total response size: " << _totalResponse.length() << RESET << std::endl;
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
				if (getPath() == "/" && _method == "GET")
					setPath(getRoot() + "/index.html");
				if (checkAllowedMethods() == 1)
					_totalResponse = getHeader();
				else if (_method == "GET")
					_totalResponse = execGET(_path, fd);
				else if (_method == "HEAD")
					_totalResponse = execHEAD(_path, fd);
				else if (_method == "POST")
					_totalResponse = execPOST(_path, fd, _body, cgi);
				else if (_method == "PUT")
					_totalResponse = execPUT(_path, fd, _body);
				else if (_method == "DELETE")
					_totalResponse = execDELETE(_path, fd);

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

	if (_totalResponse.length() - _sendStartPos >= CGI_BUFFER_SIZE)
	{
		sendMsg = _totalResponse.substr(_sendStartPos, CGI_BUFFER_SIZE);
		writeSize = ::send(fd, sendMsg.c_str(), CGI_BUFFER_SIZE, 0);
		if (error || writeSize == -1)
		{
			if (!error)
				printErr("::send()");
			_totalResponse.clear();
			return (1);
		}
		_sendStartPos += writeSize;
		_totalSendSize += writeSize;
		_remainSend = true;
	}
	else if (_remainSend == true)
	{
		sendMsg = _totalResponse.substr(_sendStartPos, _totalResponse.length() - _sendStartPos);
		std::cout << YELLOW << "#######request[" << fd << "] !!!!!" << std::endl;
		std::cout << _totalResponse.substr(0, 100) << " ... " << _totalResponse.substr(_totalResponse.length() - 20, 20) << std::endl;
		std::cout << "total response size: " << _totalResponse.size() << ", sendMsg size: " << sendMsg.size() << RESET << std::endl;

		writeSize = ::send(fd, sendMsg.c_str(), sendMsg.length(), 0);
		if (writeSize == -1)
		{
			_totalResponse.clear();
			return (printErr("::send()"));
		}
		std::cout << RED << "#######response[" << fd << "] send!!!!!" << std::endl << RESET;
		_totalSendSize += writeSize;
		std::cout << PINK << "send start pos: " << _sendStartPos << ", total send size: " << _totalSendSize << RESET << std::endl;
		_sendStartPos = 0;
		_totalSendSize = 0;
		_remainSend = false;
		_totalResponse.clear();
	}
	else
	{
		writeSize = ::send(fd, _totalResponse.c_str(), _totalResponse.size(), 0);
		if (error || writeSize == -1)
		{
			if (!error)
				printErr("::send()");
			_totalResponse.clear();
			return (1);
		}
		_totalResponse.clear();
	}
	
	return (ret);
}

std::string	Response::execGET (std::string& path, int fd)
{
	std::string			fileContent = "";
	std::ifstream		file;
	std::stringstream	buffer;

	(void)fd;
	if (path == "YoupiBanane")
		path = "YoupiBanane/index.html";
	if (compareEnd(path, "nop") == 0)
		path = "YoupiBanane/nop/youpi.bad_extension";

	if (compareEnd(path, "Yeah") == 0)
	{
		printErr("Yeah not found");
		setCode(Not_Found);
	}
	else
	{
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			printErr("failed to open");
			setCode(Not_Found);
		}
		else
		{
			setCode(OK);
			buffer << file.rdbuf();
		}
	}
	setContentLength(buffer.str().length());
	fileContent = getHeader();
	fileContent += buffer.str();
	file.close();

	return (fileContent);
}

std::string	Response::execHEAD (std::string& path, int fd)
{
	std::string			fileContent = execGET(path, fd);
	fileContent = fileContent.substr(0, getHeader().length());
	return (fileContent);
}

std::string	Response::execPOST (const std::string& path, int fd, const std::string& body, Cgi& cgi)
{

	std::ofstream	file;
	std::string		fileContent;

	(void)fd;
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
			std::cout << "file already exists" << std::endl;
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

std::string	Response::execPUT (const std::string& path, int fd, std::string& body)
{
	//ofstream형은 open mode를 따로 작성하지 않으면
	//기존에 파일이 존재했다면 삭제하는 trunc, 출력 전용을 뜻하는 out모드로 open된다.
	(void)fd;
	std::ofstream	file;
	std::string		fileContent;

	//file이 이미 존재하고 regular file일 때 파일을 갱신한다.
	if (pathIsFile(path))
	{
		std::cout << "file already exists" << std::endl;
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
	//file이 존재하지 않거나, file이 regular file이 아닐 때 작동
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

std::string	Response::execDELETE (std::string& path, int fd)
{//일단 delete가 제대로 되지 않을 때는 생각하지 않고 무조건 0을 리턴하는 것으로 했다.
	(void)fd;
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

std::string	Response::readHtml (const std::string& path)
{//path가 REGULAR file이면 open하여 파일 내용을 리턴한다.
//path가 REGULAR file이 아니면 errorHtml을 보낸다.
	std::ofstream			file;
	std::stringstream		buf;

	if (pathIsFile(path))
	{
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
			return (ERROR_HTML);
		buf << file.rdbuf();
		file.close();
		_contentType = "text/html";
		return (buf.str());
	}
	else
		return (ERROR_HTML);
}

void		Response::printResponseValue ()
{
	std::cout << WHITE;
	printGeneralHeader();
	printEntityHeader();
	printRequestHeader();
	printResponseHeader();
	std::cout << RESET;
}
