# include "./../includes/Response.hpp"

Response::Response () {}
Response::Response (const Response& response) { (void)response; }
Response::~Response () {}

Response&	Response::operator= (const Response& response) { (void)response; return (*this); }

int			Response::checkAllowedMethods ()
{
	if (_possibleMethods.find(_method) != _possibleMethods.end()
		&& _allowedMethods.find(_method) == _allowedMethods.end())
	{//method를 알고는 있지만, allow가 되지 않았을 때
		_code = Method_Not_Allowed;
		return (1);
	}
	if (_possibleMethods.find(_method) == _possibleMethods.end())
	{//method가 뭔지 모를 때
		_code = Method_Not_Allowed;
		return (1);
	}
	return (0);
}

std::string	Response::responseErr (Response *response)
{
	int	code = response->_code;

	if (response->_errorHtml.find(code) != response->_errorHtml.end())
		response->_path = response->_errorHtml[code];

	std::string	header = response->getHeader();
	std::map<int, std::string>::iterator	it = response->_errorHtml.find(code);
	if (it != response->_errorHtml.end())
		header += response->readHtml(it->second);

	return (header);
}

int			Response::verifyMethod (int fd, Response *response, int requestEnd)
{//요청마다 header를 만들어야 하고 에러가 발생했을 때에 errormap을 적절히 불러와야 한다.
	//request의 method를 확인한다.
	std::string	totalResponse;
	int			error = 0;

	int			code = response->_code;

	if (code == Bad_Request || code == Internal_Server_Error || code == Payload_Too_Large)
	{
		error = 1;
		totalResponse = responseErr(response);
	}

	if (requestEnd)
	{
		if (response->getPath() == "/" && response->_method != "GET")
		{
			error = 1;
			response->setCode(Method_Not_Allowed);
			totalResponse = responseErr(response);
		}
		else
		{
			if (response->getPath() == "/" && response->_method == "GET")
				response->setPath(getRoot() + "/index.html");
			if (checkAllowedMethods() == 1)
				totalResponse = getHeader();
			else if (_method == "GET")
				totalResponse = execGET(_path, fd);
			else if (_method == "HEAD")
				totalResponse = execHEAD(_path, fd);
			else if (_method == "POST")
				totalResponse = execPOST(_path, fd, _body);
			else if (_method == "PUT")
				totalResponse = execPUT(_path, fd, _body);
			else if (_method == "DELETE")
				totalResponse = execDELETE(_path, fd);

			std::map<int, std::string>::iterator	it = _errorHtml.find(_code);
			if (it != _errorHtml.end())
				totalResponse += readHtml(it->second);
			else if (_code != Created && _code != No_Content && _code != OK)
				totalResponse += ERROR_HTML;
		}
	}
	if (totalResponse != "")
		std::cout << YELLOW << "#########response########\n" << totalResponse <<  RESET << std::endl;
	int	writeSize = ::send(fd, totalResponse.c_str(), totalResponse.size(), 0);
	if (error || writeSize == -1)
	{
		if (!error)
			return (printErr("::send()"));
		return (1);
	}
	
	return (0);
}

std::string	Response::execGET (std::string& path, int fd)
{
	std::string			fileContent = "";
	std::ifstream		file;
	std::stringstream	buffer;
	(void)fd;
	std::cout << "GET METHOD PATH IS " << path << std::endl;
	if (compareEnd(path, "Yeah") == 0)
	{
		std::cout << "path is Yeah so not found\n";
		setCode(Not_Found);
	}
	else
	{
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			std::cout << "FILE OPEN ERROR\n";
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

std::string	Response::execPOST (const std::string& path, int fd, const std::string& body)
{
	(void)fd;
	std::ofstream	file;
	std::string		fileContent;
	if (pathIsFile(path))
	{
		std::cout << "post file is already exist, so add the content\n";
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
	return (fileContent);
}

std::string	Response::execPUT (const std::string& path, int fd, std::string& body)
{
	//ofstream형은 open mode를 따로 작성하지 않으면
	//기존에 파일이 존재했다면 삭제하는 trunc, 출력 전용을 뜻하는 out모드로 open된다.
	(void)fd;
	std::ofstream	file;
	std::string		fileContent;
	if (pathIsFile(path))
	{//file이 이미 존재하고 regular file일 때 파일을 갱신한다.
		std::cout << "put file is already exist\n";
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
	{//file이 존재하지 않거나, file이 regular file이 아닐 때 작동
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
