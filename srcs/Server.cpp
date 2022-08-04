#include "./../includes/Server.hpp"

Server::Server () {}
Server::Server (const Server& server) { (void)server; }
Server::~Server () {}

Server&						Server::operator= (const Server& server) { (void)server; return *this; }

void						Server::changeEvents (std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
	struct kevent	tempEvent;

	EV_SET(&tempEvent, ident, filter, flags, fflags, data, udata);
	changeList.push_back(tempEvent);
}

void						Server::disconnectRequest (int fd)
{
	std::cout << "request disconnected: " << fd << std::endl;
	resetRequest(fd);
	close(fd);
	_request.erase(fd);
}

void						Server::checkConnection (int fd)
{
	if (_response[fd]._connection == "close")
		disconnectRequest(fd);
}

int							Server::initListen (const std::string& hostPort)
{
/*	if (_response.setListen(hostPort) == 1)
		return (printErr("failed to listen"));

	_listen.host = _response._listen.host;
	_listen.port = _response._listen.port;*/
	(void)hostPort;

	_listen.host = (unsigned int)0;
	_listen.port = htons(8000);

	std::cout << "init listen host : " << _listen.host;
	std::cout << ", port : " << _listen.port << std::endl;

	return (0);
}

//에러가 발생했을 때 에러 메시지를 출력하고 1을 리턴, 정상 작동이면 0을 리턴
//server socket을 만들고, bind, listen, kqueue를 하고,
//바로 changeEvents를 통해 event를 등록한다.
int							Server::initServerSocket ()
{	int	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
		return (printErr("failed to init server"));

	std::memset(&_serverAddr, 0, sizeof(_serverAddr));
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = _listen.host;
	_serverAddr.sin_port = _listen.port;
	
	if (bind(serverSocket, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) == -1)
		return (printErr("failed to bind socket"));

	if (listen(serverSocket, LISTEN_BUFFER_SIZE) == -1)
		return (printErr("failed to listen socket"));

	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	_serverSocket = serverSocket;

	int	kq = kqueue();
	if (kq == -1)
		return (printErr("failed to init kqueue"));

	_kq = kq;
	changeEvents(_changeList, _serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	return (0);
}

int							Server::eventError (int fd)
{
	if (fd == _serverSocket)
		return (printErr("socket error"));
	else
	{
		printErr("client socket error");
		disconnectRequest(fd);
	}

	return (0);
}

void						Server::requestAccept ()
{
	int	requestSocket;
	if ((requestSocket = accept(_serverSocket, NULL, NULL)) == -1)
	{
		printErr("failed to accept");
		return ;
	}
	std::cout << "accept new request: " << requestSocket << std::endl;
	fcntl(requestSocket, F_SETFL, O_NONBLOCK);

	changeEvents(_changeList, requestSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	changeEvents(_changeList, requestSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	_request[requestSocket] = "";

	_response[requestSocket].initRequest();
	_response[requestSocket].initPossibleMethods();
	_response[requestSocket].initErrorMap();
	_bodyCondition[requestSocket] = No_Body;
	_bodyEnd[requestSocket] = 0;
	_bodyStartPos[requestSocket] = 0;

	_response[requestSocket].setServer(getServerName());
	_response[requestSocket].initAllowedMethods(getServerAllowedMethods());
	_response[requestSocket].setRoot(getResponseRoot());
	_response[requestSocket].setErrorHtml(getServerErrorPages());

	_response[requestSocket].setRemainSend(false);
	_response[requestSocket].setTotalResponse("");
	_response[requestSocket].setTotalSendSize(0);
	_response[requestSocket].setSendStartPos(0);
}

void						Server::setServerName (const std::string& name) { _serverName = name; }
void						Server::setServerAllowedMethods (const std::vector<std::string>& allowedMethods) { _serverAllowedMethods = allowedMethods; }
void						Server::setResponseRoot (const std::string& root) { _responseRoot = root; }
void						Server::setServerErrorPages (const int& code, const std::string& html) { _serverErrorPages[code] = html; }
void						Server::setServerErrorPages (const std::map<int, std::string>& html) { _serverErrorPages = html; }
void						Server::initServerErrorPages ()
{
	setServerErrorPages(Bad_Request, BAD_REQUEST_HTML);
	setServerErrorPages(Forbidden, FORBIDDEN_HTML);
	setServerErrorPages(Not_Found, NOT_FOUND_HTML);
	setServerErrorPages(Method_Not_Allowed, NOT_ALLOWED_HTML);
	setServerErrorPages(Payload_Too_Large, PAYLOAD_TOO_LARGE_HTML);
	setServerErrorPages(Internal_Server_Error, INTERNAL_SERVER_ERROR_HTML);
}

std::string					Server::getServerName () const { return (_serverName); }
std::vector<std::string>	Server::getServerAllowedMethods () const { return (_serverAllowedMethods); }
std::string					Server::getResponseRoot () const { return (_responseRoot); }
std::map<int, std::string>	Server::getServerErrorPages () const { return (_serverErrorPages); }

void						Server::setCgiEnv (int fd)
{
	_cgi.setBody(_response[fd]._body);
	_cgi.setEnv("CONTENT_LENGTH", intToStr(_cgi.getBody().length()));
	_cgi.setEnv("PATH_INFO", _response[fd]._path);
	_cgi.setEnv("SERVER_PORT", "8000");
	_cgi.setEnv("SERVER_PROTOCOL", "HTTP/1.1");
	_cgi.setEnv("REDIRECT_STATUS", "200");
	if (_response[fd]._xHeader != "")
		_cgi.setEnv("HTTP_X_SECRET_HEADER_FOR_TEST", _response[fd]._xHeader);
}

void						Server::initCgiEnv (int fd)
{
	_cgi.setName(_configCgi);
	_cgi.setEnv("REDIRECT_STATUS", "200");
	_cgi.setEnv("CONTENT_LENGTH", _response[fd].getContentLength());
	_cgi.setEnv("CONTENT_TYPE", _response[fd].getContentType());
	_cgi.setEnv("PATH_INFO", _response[fd].getPath());
	_cgi.setEnv("REQUEST_METHOD", _response[fd]._method);
	_cgi.setEnv("SERVER_PORT", intToStr(_response[fd].getListen().port));
	_cgi.setEnv("SERVER_PROTOCOL", _response[fd].getHttpVersion());
}

LocationBlock				Server::selectLocationBlock (std::string requestURI, int fd)
{
	std::vector<LocationBlock>	locationBlocks;
	std::vector<std::string>	requestURIvec;
	size_t						max = 0, ret = 0;

//	std::cout << "requestURI : " << requestURI << std::endl;
	requestURIvec = split(requestURI, '/');

	for (size_t i = 1; i < requestURIvec.size(); i++)
		requestURIvec[i] = "/" + requestURIvec[i];

	for (size_t i = 0; i < _locations.size(); i++) {
		if (_locations[i].getMod() == EXACT) {
			if (_locations[i].getURI() == requestURI)
				return (_locations[i]);
			else
				break ;
		}
	}

	(void)fd;

	for (size_t i = 0; i < _locations.size(); i++)
	{
		if (_locations[i].getMod() == NONE || _locations[i].getMod() == PREFERENTIAL)
		{
//			std::cout << "locations uri : " << _locations[i].getURI() << ", requestURI first : " << requestURIvec[0] << std::endl;
			if (_locations[i].getURI() == requestURIvec[0])
			{
				std::string	changePath = "";
				if (requestURI.at(requestURI.length() - 1) == '/')
					requestURI.substr(0, requestURI.length() - 1);
				size_t	firstSlashPos = requestURI.find_first_of("/");
				if (firstSlashPos != std::string::npos)
					changePath = requestURI.substr(firstSlashPos, requestURI.length() - firstSlashPos);
				_locations[i].setPath(_locations[i].getRoot() + changePath);
				locationBlocks.push_back(_locations[i]);
			}
			if (requestURIvec.size() >= 2)
			{
//				std::cout << CYAN << "request uri : " << requestURI << ", location uri: " << _locations[i].getURI() << ";" << std::endl << RESET;
				std::vector<LocationBlock>	nested = _locations[i].getLocationBlocks();
				for (size_t j = 0; j < nested.size(); j++)
				{
//					std::cout << BLUE << "nested uri: " << nested[j].getURI() << RESET << std::endl;
					if (!compareURIs(nested[j].getURI(), requestURIvec[1], nested[j].getMod()))
					{
						std::string	changePath = "";
						size_t		lastSlashPos = requestURI.find_last_of("/");
						if (lastSlashPos != std::string::npos)
							changePath = requestURI.substr(lastSlashPos, requestURI.length() - lastSlashPos);
						nested[j].setPath(nested[j].getRoot() + changePath);
						locationBlocks.push_back(nested[j]);
					}
				}
				if (requestURI.find(_locations[i].getURI()) != std::string::npos)
				{
					std::string	changePath = "";
					size_t		firstSlashPos = requestURI.find_first_of("/");
					if (firstSlashPos != std::string::npos)
						changePath = requestURI.substr(firstSlashPos, requestURI.length() - firstSlashPos);
					_locations[i].setPath(_locations[i].getRoot() + changePath);
					locationBlocks.push_back(_locations[i]);
				}
			}
		}
	}

	// if no location was found, return empty location
	if (locationBlocks.empty())
	{
		std::cout << PINK << "no location was found so return empty location" << RESET << std::endl;
		return (LocationBlock());
	}

	// if there are more than one locations selected, return the location with longest URI
	max = locationBlocks[ret].getURI().length();
	for (size_t i = 1; i < locationBlocks.size(); i++) {
		if (locationBlocks[i].getURI().size() > max) {
			max = locationBlocks[i].getURI().length();
			ret = i;
		}
	}

//	std::cout << PINK << "ret: "<< ret << ", after select location path : " << locationBlocks[ret].getPath();
//	std::cout << ", select location uri: " << locationBlocks[ret].getURI() << RESET << std::endl;

	return (locationBlocks[ret]);
}

void						Server::locationToServer (LocationBlock block, int fd)
{
	if (block.empty() == true)
		return ;

	if (block.getURI() != "" && (_response[fd]._method == "GET" ||
		(_response[fd]._method != "GET" && _response[fd]._path != "/")))
		_response[fd]._path = block.getPath();

	if (block.getClntSize() != 0)
		_clientMaxBodySize = block.getClntSize();

	if (block.getMethods().empty() == false)
		_response[fd].initAllowedMethods(block.getMethods());

//	std::cout << GREEN << "location block root: " << block.getRoot() << RESET << std::endl;
	if (block.getRoot() != ".")
	{
		_response[fd]._root = block.getRoot();
		size_t	uriPos = 0;
		if ((uriPos = _response[fd].getPath().find(block.getURI())) != std::string::npos)
		{
			std::cout << CYAN << "location uri: " << block.getURI() << RESET << std::endl;
			_response[fd].setPath(block.getPath());
			std::cout << RED << "select location and path change : " << _response[fd].getPath() << RESET << std::endl;
		}
	}

	if (block.getAutoindex() != DEFAULT_AUTOINDEX)
		_autoindex = block.getAutoindex();

	if (block.getIndex().empty() == false)
		_index = block.getIndex();

	if (block.getCGI() != "")
	{
		_configCgi = block.getCGI();
		_cgi.setCgiExist(true);
		initCgiEnv(fd);
	}
	
}

void						Server::resetRequest (int fd)
{
	_request[fd].clear();
	_requestEnd[fd] = 0;
	_bodyCondition[fd] = No_Body;
	_response[fd].initRequest();
	_checkedRequestLine[fd] = 0;
	_bodyStartPos[fd] = 0;
	_bodyEnd[fd] = 0;
	_bodyVecSize[fd] = 0;
	_bodyVecStartPos[fd] = 0;
	_rnPos[fd] = 0;
	_response[fd]._bodyVec.clear();
	_cgi.setCgiExist(false);
	_response[fd].getTotalResponse().clear();
	_response[fd].setRemainSend(false);
	_clientMaxBodySize = 0;
	_response[fd]._bodySize = 0;
	_bodyVecTotalSize[fd] = 0;
	_response[fd]._body.clear();
	_response[fd]._root = _serverRoot;
}

void						Server::eventRead(int fd)
{
	_requestEnd[fd] = 0;
	int	checkRequestLine = 0;

	if (fd == _serverSocket)
		requestAccept();

	else if (_request.find(fd) != _request.end())
	{
		char	buf[READ_BUFFER_SIZE] = {};
		int		n;

		// read as much as content-length
		if (_response[fd]._contentLength != "" && _bodyCondition[fd] == Body_Start)
		{
			if (_response[fd]._bodySize >= _clientMaxBodySize && _clientMaxBodySize != 0)
			{
				printErr("content length is too big to receive");
				_response[fd].setCode(Payload_Too_Large);
				return ;
			}
		}
		n = ::recv(fd, buf, READ_BUFFER_SIZE - 1, 0);
		buf[n] = '\0';
		_request[fd] += buf;

		if (_response[fd]._bodySize != 0 && _request[fd].length() - _bodyStartPos[fd] >= _response[fd]._bodySize)
		{
			std::cout << PINK << "body size: " << _response[fd]._bodySize << ", body start pos: " << _bodyStartPos[fd] << RESET << std::endl;
			_requestEnd[fd] = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos[fd] + _response[fd]._bodySize);
		}
		
		if (_request[fd] == "\r\n" && _checkedRequestLine[fd] == 0)
		{
			_request[fd].clear();
			printErr("request line is empty");
			return ;
		}

		if (_request[fd].find("\r\n") != std::string::npos && _checkedRequestLine[fd] == 0)
		{
			// XXX
			if (_request[fd].at(0) == 'r' && _request[fd].at(1) == '\n')
				_request[fd] = _request[fd].substr(2, _request[fd].length() - 2);
			if (_request[fd].at(0) == '\n')
				_request[fd] = _request[fd].substr(1, _request[fd].length() - 1);
			_checkedRequestLine[fd] = 1;
//			std::cout << "check request line" << std::endl;

			checkRequestLine = _response[fd].checkRequestLine(_request[fd]);
			if (checkRequestLine == 9)
			{
				std::cout << "request's http version is HTTP/0.9" << std::endl;
				_requestEnd[fd] = 1;
			}
			else if (checkRequestLine == 1)
			{
				printErr("check request line");
				std::cout << RED << "@@@@@@@@request@@@@@@@@" << std::endl << _request[fd] << RESET << std::endl;
				_response[fd]._httpVersion = "HTTP/1.1";
				_response[fd]._contentLocation = "/";
				_response[fd].setCode(Bad_Request);
				return ;
			}
			//checkRequestLine으로 parsing한 path를 통해서 select location block으로 적절한 location block을 찾는다.
			//path이 root랑 다를 때 Location block의 변수들을 Server로 넘겨준다.
			//만약 location을 못 찾았다면 그냥 server block의 변수를 사용하면 된다.
			if (_response[fd]._path != _response[fd]._root && _response[fd]._path != "/")
			{
//				std::cout << PINK << "@@@@@@@@@@@@@select location@@@@@@@@@@@@" << RESET << std::endl;
//				std::cout << "response root: " << _response[fd]._root << ", response path: " << _response[fd]._path << std::endl;
				LocationBlock	test = selectLocationBlock(_response[fd]._path, fd);

				if (test.empty() == false)
				{
					_response[fd].setPath(test.getPath());
					locationToServer(test, fd);
				}
				else
				{
					if (_response[fd].getPath().at(0) == '/')
						_response[fd].setPath(_response[fd].getRoot() + _response[fd].getPath());
					else
						_response[fd].setPath(_response[fd].getRoot() + "/" + _response[fd].getPath());
				}
			}
			else if ((_response[fd].getPath() == "/" && _response[fd]._method == "GET") || _response[fd].getPath() != "/")
			{
				if (_response[fd].getPath() == "/")
					_response[fd].setPath(_response[fd].getRoot());
				else if (_response[fd].getPath().at(0) == '/')
					_response[fd].setPath(_response[fd].getRoot() + _response[fd].getPath());
				else
					_response[fd].setPath(_response[fd].getRoot() + "/" + _response[fd].getPath());
			}
		}

		if ((strncmp(_request[fd].c_str(), "POST", 4) == 0 ||
			strncmp(_request[fd].c_str(), "PUT", 3) == 0) && _bodyCondition[fd] == No_Body)
			_bodyCondition[fd] = Body_Exist;

		size_t	rnrnPos;

		if (_request[fd].empty() == false && (rnrnPos = _request[fd].find("\r\n\r\n")) != std::string::npos)
		{
			if (_bodyCondition[fd] == No_Body)
			{
				_requestEnd[fd] = 1;
				_bodyCondition[fd] = Body_End;
			}
			else if (_bodyCondition[fd] == Body_Exist)
			{
				_bodyCondition[fd] = Body_Start;
				_bodyStartPos[fd] = rnrnPos + 4;
			}
		}

		//check header
		if ((_bodyCondition[fd] == Body_Start || _bodyCondition[fd] == Body_End) && _bodyEnd[fd] == 0)
		{
			if (checkRequestLine == 1 || checkRequestLine == 9)
				;
			else if (_response[fd].splitRequest(_request[fd], _bodyCondition[fd]) == 1)
			{
				printErr("failed to split request");
				_response[fd].setCode(Bad_Request);
			}
			_bodyEnd[fd] = 1;
		}

		//body size만큼 입력 받았을 때
		if (_response[fd]._bodySize != 0 && _response[fd]._contentLength != "" && _request[fd].length() - _bodyStartPos[fd] >= _response[fd]._bodySize)
		{
			_requestEnd[fd] = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos[fd] + _response[fd]._bodySize);
		}
		//contentLength가 없고, transferEncoding이 chunked일 때 파일의 끝을 알리는 것이 들어올 때까지 계속 recv
		if (_response[fd]._transferEncoding == "chunked")
		{
			while (_rnPos[fd] < _request[fd].length() && _response[fd]._path != "/")
			{
				if (_rnPos[fd] <= _bodyStartPos[fd])
				{
					_rnPos[fd] = _bodyStartPos[fd];
					_bodyVecStartPos[fd] = _rnPos[fd];
				}

				size_t	bodySize = 0;
				if (_bodyVecSize[fd] == 0)
					bodySize = _request[fd].find("\r\n", _rnPos[fd]);

				if (bodySize != std::string::npos && _bodyVecSize[fd] == 0)
				{
					std::string	bodySizeStr = _request[fd].substr(_rnPos[fd], bodySize - _rnPos[fd]);
					_bodyVecSize[fd] = hexToDecimal(bodySizeStr);
					_bodyVecTotalSize[fd] += _bodyVecSize[fd];
					if (_bodyVecTotalSize[fd] > _clientMaxBodySize && _clientMaxBodySize != 0)
						_response[fd].setCode(Payload_Too_Large);
					_bodyVecStartPos[fd] = bodySize + 2;
					_rnPos[fd] = _bodyVecStartPos[fd] + _bodyVecSize[fd] + 2;
				}
				else if (bodySize == std::string::npos)
				{
					std::cout << CYAN << "there is no left body size" << std::endl << RESET;
					break ;
				}
				if (_request[fd].length() > _bodyVecStartPos[fd] + _bodyVecSize[fd] && _bodyVecStartPos[fd] != 0)
				{
					std::string	_bodyElement = _request[fd].substr(_bodyVecStartPos[fd], _bodyVecSize[fd]);
					_response[fd]._bodyVec.push_back(_bodyElement);
					_bodyVecStartPos[fd] += _bodyVecSize[fd];
					_bodyVecSize[fd] = 0;
				}
			}

			if (_request[fd].find("0\r\n\r\n") != std::string::npos &&
				(_bodyVecStartPos[fd] == 0 ||
				(_bodyVecStartPos[fd] != 0 &&
				 _response[fd].getRemainSend() == false)))
			{
				_request[fd] = _request[fd].substr(0, _request[fd].length() - 5);
				std::cout << PINK << "request[" << fd << "] received file end" << std::endl << RESET;
				_requestEnd[fd] = 1;
			}
		}
		//read가 에러가 났거나, request가 0을 보내면 request와 연결을 끊는다.
		if (n <= 0)
		{
			if (n < 0)
			{
				printErr("failed to read request");
				_response[fd].setCode(Internal_Server_Error);
			}
			else
				disconnectRequest(fd);
		}
		else if (_requestEnd[fd] == 1)
		{
			if (_bodyStartPos[fd] != 0 && _bodyStartPos[fd] < _request[fd].length())
			{
				if (_response[fd]._transferEncoding == "chunked" && _response[fd]._bodyVec.empty() == false)
				{
					for (std::vector<std::string>::iterator it = _response[fd]._bodyVec.begin(); it != _response[fd]._bodyVec.end(); it++)
						_response[fd]._body += *it;
					_response[fd]._contentLength = sizetToStr(_bodyVecTotalSize[fd]);
					setCgiEnv(fd);
				}
				else
				{
					_response[fd]._body = _request[fd].substr(_bodyStartPos[fd], _request[fd].length() - _bodyStartPos[fd]);
					if (_response[fd]._body.length() > _clientMaxBodySize)
						_response[fd].setCode(Payload_Too_Large);
				}
			}
		}
	}
}

void						Server::eventWrite(int fd)
{
	if (_request.find(fd) != _request.end())
	{
		if (_requestEnd[fd] == 1) 
		{
			int	verifyMethodRet = _response[fd].verifyMethod(fd, _requestEnd[fd], _cgi);
			if (verifyMethodRet == 1)
				disconnectRequest(fd);
			if (verifyMethodRet == 2)
				resetRequest(fd);
			if (_response[fd]._connection == "close")
				disconnectRequest(fd);
			else if (_response[fd].getRemainSend() == false)
				resetRequest(fd);
		}
	}
}

void			Server::initServerMember()
{
	for (std::map<int, Response>::iterator it = _response.begin(); it != _response.end(); it++)
	{
		int	fd = (*it).first;

		_response[fd].initRequest();
		_response[fd].initPossibleMethods();
		_response[fd].initErrorMap();
		_bodyCondition[fd] = No_Body;
		_bodyEnd[fd] = 0;
		_bodyStartPos[fd] = 0;
	}
}
