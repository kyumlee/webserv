# include "./../includes/Response.hpp"

Response::Response () {}
Response::Response (const Response& response) { (void)response; }
Response::~Response () {}

Response&		Response::operator= (const Response& response) { (void)response; return (*this); }

int				Response::checkAllowedMethods ()
{
	//method를 알고는 있지만, allow가 되지 않았을 때
	if (this->_possibleMethods.find(this->_method) != this->_possibleMethods.end()
		&& this->_allowedMethods.find(this->_method) == this->_allowedMethods.end())
	{
		this->_code = Method_Not_Allowed;
		return (1);
	}
	//method가 뭔지 모를 때
	if (this->_possibleMethods.find(this->_method) == this->_possibleMethods.end())
	{
		this->_code = Method_Not_Allowed;
		return (1);
	}

	return (0);
}

std::string		Response::responseErr (Response *response)
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

//요청마다 header를 만들어야 하고 에러가 발생했을 때에 errormap을 적절히 불러와야 한다.
//request의 method를 확인한다.
int				Response::verifyMethod (int fd, Response *response, int requestEnd)
{
	std::string	totalResponse;
	int			error = 0, code = response->_code;

	if (code == Bad_Request || code == Internal_Server_Error || code == Payload_Too_Large)
	{
		error = 1;
		totalResponse = responseErr(response);
	}

	if (requestEnd)
	{
		if (response->getPath() == "/")
		{
			error = 1;
			response->setCode(Method_Not_Allowed);
			totalResponse = responseErr(response);
		}

		else
		{
			if (checkAllowedMethods())
				totalResponse = this->getHeader();
			else if (this->_method == "GET")
				totalResponse = this->getMethod(this->_path);
			else if (this->_method == "HEAD")
				totalResponse = this->headMethod(this->_path);
			else if (this->_method == "POST")
				totalResponse = this->postMethod(this->_path, this->_body);
			else if (this->_method == "PUT")
				totalResponse = this->putMethod(this->_path, this->_body);
			else if (this->_method == "DELETE")
				totalResponse = this->deleteMethod(this->_path);

			std::map<int, std::string>::iterator	it = this->_errorHtml.find(this->_code);
			if (it != this->_errorHtml.end())
				totalResponse += this->readHtml(it->second);
			else if (this->_code != Created && this->_code != No_Content && this->_code != OK)
				totalResponse += ERROR_HTML;
		}
	}

	std::cout << YELLOW << "\n==========response=========\n" << totalResponse << RESET;

	int	writeSize = ::send(fd, totalResponse.c_str(), totalResponse.size(), 0);

	if (error || writeSize == -1)
	{
		if (!error)
			printErr("::send()");
		return (1);
	}
	
	return (0);
}

std::string		Response::getMethod (std::string& path)
{
	std::string			fileContent = "";
	std::ifstream		file;
	std::stringstream	buffer;
	std::cout << "GET METHOD PATH IS " << path << std::endl;

	if (compareEnd(path, "Yeah") == 0)
	{
		printErr("/Yeah not found");
		this->setCode(Not_Found);
	}
	else
	{
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			printErr("file open");
			this->setCode(Not_Found);
		}
		else
		{
			this->setCode(OK);
			buffer << file.rdbuf();
		}
	}
	this->setContentLength(buffer.str().length());
	fileContent = this->getHeader();
	fileContent += buffer.str();
	file.close();
	return (fileContent);
}

std::string		Response::headMethod (std::string& path)
{
/*			std::string			fileContent = "";
	std::ifstream		file;
	std::stringstream	buffer;
	if (compare_end(path, "Yeah") == 0)
	{
		std::cout << "path is Yeah so not found\n";
		this->setCode(Not_Found);
	}
	else
	{
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			std::cout << "FILE OPEN ERROR\n";
			this->setCode(Not_Found);
		}
		else
		{
			this->setCode(OK);
			buffer << file.rdbuf();
		}
	}
	this->setContentLength(buffer.str().length());
	fileContent = this->getHeader();
	file.close();*/
	std::string	fileContent = getMethod(path);
	fileContent = fileContent.substr(0, getHeader().length());
	return (fileContent);
}
std::string		Response::postMethod (const std::string& path, const std::string& body)
{
	std::ofstream	file;
	std::string		fileContent;
	if (pathIsFile(path))
	{
		std::cout << "post file already exists, so add the content\n";
		file.open(path.c_str(), std::ofstream::out | std::ofstream::app);
		if (file.is_open() == false)
		{
			printErr("file open");
			this->setCode(Forbidden);
		}
		file << body;
		file.close();
		this->setCode(No_Content);
	}
	else
	{
		file.open(path.c_str(), std::ofstream::out);
		if (file.is_open() == false)
		{
			printErr("file open");
			this->setCode(Not_Found);
		}
		file << body;
		file.close();
		this->setCode(Created);
	}
	fileContent = this->getHeader();
	return (fileContent);
}

std::string		Response::putMethod (const std::string& path, std::string& body)
{
	//ofstream형은 open mode를 따로 작성하지 않으면
	//기존에 파일이 존재했다면 삭제하는 trunc, 출력 전용을 뜻하는 out모드로 open된다.
	std::ofstream	file;
	std::string		fileContent;
	//file이 이미 존재하고 regular file일 때 파일을 갱신한다.
	if (pathIsFile(path))
	{
		std::cout << "put file is already exist\n";
		file.open(path.c_str());
		if (file.is_open() == false)
		{
			printErr("file open");
			this->setCode(Forbidden);
		}
		else
			this->setCode(No_Content);
		file << body;
		file.close();
	}
	//file이 존재하지 않거나, file이 regular file이 아닐 때 작동
	else
	{
		file.open(path.c_str());
		if (file.is_open() == false)
		{
			printErr("file open");
			this->setCode(Forbidden);
		}
		else
			this->setCode(Created);
		file << body;
		file.close();
	}
	fileContent = this->getHeader();
	return (fileContent);
}

//일단 delete가 제대로 되지 않을 때는 생각하지 않고 무조건 0을 리턴하는 것으로 했다.
std::string		Response::deleteMethod (std::string& path)
{
	std::string	fileContent;
	if (pathIsFile(path))
	{
		if (remove(path.c_str()) == 0)
			this->setCode(No_Content);
		else
		{
			printErr("file is regular file but failed to remove");
			this->setCode(Forbidden);
		}
	}
	else
	{
		printErr("file is regular file but failed to remove");
		this->setCode(Not_Found);
	}
	fileContent = this->getHeader();
	return (fileContent);
}

//path가 REGULAR file이면 open하여 파일 내용을 리턴한다.
//path가 REGULAR file이 아니면 errorHtml을 보낸다.
std::string		Response::readHtml (const std::string& path)
{
	std::ofstream		file;
	std::stringstream	buf;

	if (pathIsFile(path))
	{
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
			return (ERROR_HTML);
		buf << file.rdbuf();
		file.close();
		this->_contentType = "text/html";
		return (buf.str());
	}
	else
		return (ERROR_HTML);
}

void			Response::printResponseValue ()
{
	std::cout << WHITE;
	this->printGeneralHeader();
	this->printEntityHeader();
	this->printRequestHeader();
	this->printResponseHeader();
	std::cout << RESET;
}
