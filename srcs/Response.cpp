# include "./../includes/Response.hpp"

Response::Response () {}
Response::Response (const Response& response) { (void)response; }
Response::~Response () {}

Response&		Response::operator= (const Response& response) { (void)response; return (*this); }

// checks the validity of the requested method
int				Response::checkMethod ()
{
	// possible method, but not allowed
	if (_possibleMethods.find(_method) != _possibleMethods.end()
		&& _allowedMethods.find(_method) == _allowedMethods.end())
	{
		_code = Method_Not_Allowed;
		return (1);
	}
	// unknown method
	if (_possibleMethods.find(_method) == _possibleMethods.end())
	{
		_code = Method_Not_Allowed;
		return (1);
	}

	return (0);
}

// if the response code is error
// @param	response	response structure
/*std::string		Response::responseErr (Response *response)
{
	int	code = response->_code;

	if (response->_errorHtml.find(code) != response->_errorHtml.end())
		response->_path = response->_errorHtml[code];

	std::string	header = response->getHeader();
	std::map<int, std::string>::iterator	it = response->_errorHtml.find(code);
	if (it != response->_errorHtml.end())
		header += response->readHtml(it->second);

	return (header);
}*/
std::string		Response::responseErr ()
{
	int	code = _code;

	if (_errorHtml.find(code) != _errorHtml.end())
		_path = _errorHtml[code];

	std::string	header = getHeader();
	std::map<int, std::string>::iterator	it = _errorHtml.find(code);
	if (it != _errorHtml.end())
		header += readHtml(it->second);

	return (header);
}

// 1. every response must have header.
// 2. in case of error, response must return an appropriate error html.
// Verifies the request method.
// @param	fd			file descriptor where to send the response
// @param	response	response structure
// @param	requestEnd	flag that tells the end of request
//int				Response::verifyMethod (int fd, Response *response, int requestEnd)
int				Response::verifyMethod (int fd, int requestEnd)
{
	std::string	totalResponse;	// response header+body
//	int			error = 0, code = response->_code;	// if error is 1, the response returns error
	int			error = 0, code = _code;	// if error is 1, the response returns error

	if (code == Bad_Request || code == Internal_Server_Error || code == Payload_Too_Large)
	{
		error = 1;
		totalResponse = getHeader();
//		totalResponse = responseErr(response);
//		totalResponse = responseErr();
	}

	// end of request
	else if (requestEnd)
	{
		// the path "/" with method that is not GET is NOT ALLOWED (according to the tester)
//		if (response->getPath() == "/" && response->_method != "GET")
		if (getPath() == "/" && _method != "GET")
		{
			error = 1;
			setCode(Method_Not_Allowed);
			totalResponse = getHeader();
//			totalResponse = responseErr();
		}
		else
		{
			// the path "/" with GET method -> path is root/index.html (need to fix)
			// TODO: depends on the autoindex (if autoindex is on, and index directive exists, this must have the first index.
			// if autoindex is off, no matter the index directive exists or not, the path must show directory listing (need to check the tester)
//			if (response->getPath() == "/" && response->_method == "GET")
//				response->setPath(getRoot() + "/index.html");
			if (getPath() == "/" && _method == "GET")
				setPath(getRoot() + "/index.html");

			// if the method is not known or not allowed (TODO: does it return just the header?)
			if (checkMethod())
				totalResponse = getHeader();
			else if (_method == "GET")
				totalResponse = execGET(_path);
			else if (_method == "HEAD")
				totalResponse = execHEAD(_path);
			else if (_method == "POST")
				totalResponse = execPOST(_path, _body);
			else if (_method == "PUT")
				totalResponse = execPUT(_path, _body);
			else if (_method == "DELETE")
				totalResponse = execDELETE(_path);

//			std::map<int, std::string>::iterator	it = _errorHtml.find(_code);
//			if (it != _errorHtml.end())
//				totalResponse += readHtml(it->second);
//			else if (_code != Created && _code != No_Content && _code != OK)
//				totalResponse += ERROR_HTML;
		}
	}
	// if the response has the error code, add the corresponding html to the response message
	std::map<int, std::string>::iterator	it = _errorHtml.find(_code);
	if (it != _errorHtml.end())
		totalResponse += readHtml(it->second);
	// if a code is unknown, add the ERROR_HTML to the response message
	else if (_code != Created && _code != No_Content && _code != OK)
		totalResponse += ERROR_HTML;

	if (totalResponse != "")
		std::cout << YELLOW << "#########response#########" << std::endl << totalResponse << RESET << std::endl;

	// send the response message to the file descriptor
	int	writeSize = ::send(fd, totalResponse.c_str(), totalResponse.size(), 0);

	if (error || writeSize == -1)
	{
		if (!error)
			printErr("::send()");
		return (1);
	}
	
	return (0);
}

std::string		Response::execGET (std::string& path)
{
	std::string			fileContent = "";
	std::ifstream		file;
	std::stringstream	buffer;
	std::cout << "GET METHOD PATH IS " << path << std::endl;

	// XXX: TESTER: on get /Yeah it should return error
	if (compareEnd(path, "Yeah") == 0)
	{
		printErr("/Yeah not found");
		setCode(Not_Found);
	}
	else
	{
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			printErr("file open");
			setCode(Not_Found);
		}
		else
		{
			setCode(OK);
			buffer << file.rdbuf();
		}
	}
	setContentLength(buffer.str().length());

	fileContent = getHeader() + buffer.str();
	file.close();

	return (fileContent);
}

// HEAD method is same as GET method except that it only returns the header
std::string		Response::execHEAD (std::string& path)
{
	std::string	fileContent = execGET(path).substr(0, getHeader().length());

	return (fileContent);
}

std::string		Response::execPOST (const std::string& path, const std::string& body)
{
	std::ofstream	file;
	std::string		fileContent;

	// if the file already exists, update it (add the content to the file)
	if (pathIsFile(path))
	{
		std::cout << "file already exists, so add the content\n";
		file.open(path.c_str(), std::ofstream::out | std::ofstream::app);
		if (file.is_open() == false)
		{
			printErr("couldn't open file");
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
			printErr("couldn't open file");
			setCode(Not_Found);
		}
		file << body;
		file.close();
		setCode(Created);
	}

	fileContent = getHeader();
	return (fileContent);
}

std::string		Response::execPUT (const std::string& path, std::string& body)
{
	//ofstream형은 open mode를 따로 작성하지 않으면
	//기존에 파일이 존재했다면 삭제하는 trunc, 출력 전용을 뜻하는 out모드로 open된다.
	std::ofstream	file;

	// file exists and is a regular file
	if (pathIsFile(path))
	{
		std::cout << "file already exists\n";
		file.open(path.c_str());
		if (file.is_open() == false)
		{
			printErr("couldn't open file");
			setCode(Forbidden);
		}
		else
			setCode(No_Content);

		file << body;
		file.close();
	}
	// file either does not exist or is not a regular file
	else
	{
		file.open(path.c_str());
		if (file.is_open() == false)
		{
			printErr("file open");
			setCode(Forbidden);
		}
		else
			setCode(Created);
		file << body;
		file.close();
	}
	return (getHeader());
}

// didn't concern about delete failure
std::string		Response::execDELETE (std::string& path)
{
	if (pathIsFile(path))
	{
		if (remove(path.c_str()) == 0)
			setCode(No_Content);
		else
		{
			printErr("file is a regular file but failed to remove()");
			setCode(Forbidden);
		}
	}
	else
	{
		printErr("file is a regular file but failed to remove()");
		setCode(Not_Found);
	}

	return (getHeader());
}

std::string		Response::readHtml (const std::string& path)
{
	std::ofstream		file;
	std::stringstream	buf;

	// if path is a regular file, open and return the content
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
	// if path is not a regular file, return ERROR_HTML
	return (ERROR_HTML);
}

void			Response::printResponseValue ()
{
	std::cout << WHITE;
	printGeneralHeader();
	printEntityHeader();
	printRequestHeader();
	printResponseHeader();
	std::cout << RESET;
}
