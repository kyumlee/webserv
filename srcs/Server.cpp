#include "./../includes/Server.hpp"

Server::Server()
	: _serverAddr(),
	_serverSocket(),
	_listen(),
	_kq(),
	_request(),
	_changeList(),
	_eventList(),
	_response(),
	_bodyCondition(),
	_requestEnd(),
	_checkedRequestLine(),
	_bodyStartPos(),
	_bodyEnd(),
	_bodyVecSize(),
	_bodyVecTotalSize(),
	_bodyVecStartPos(),
	_rnPos(),
	_serverRoot(),
	_serverName(),
	_serverAllowedMethods(),
	_responseRoot(),
	_serverErrorPages(),
	_clientMaxBodySize(),
	_autoindex(),
	_serverautoindex(),
	_index(),
	_serverindex(),
	_configCgi(),
	_locations(),
	_cgi()
{}

Server::Server (const Server& server)
	: _serverAddr(server._serverAddr),
	_serverSocket(server._serverSocket),
	_listen(server._listen),
	_kq(server._kq),
	_request(server._request),
	_changeList(server._changeList),
	_response(server._response),
	_bodyCondition(server._bodyCondition),
	_requestEnd(server._requestEnd),
	_checkedRequestLine(server._checkedRequestLine),
	_bodyStartPos(server._bodyStartPos),
	_bodyEnd(server._bodyEnd),
	_bodyVecSize(server._bodyVecSize),
	_bodyVecTotalSize(server._bodyVecTotalSize),
	_bodyVecStartPos(server._bodyVecStartPos),
	_rnPos(server._rnPos),
	_serverRoot(server._serverRoot),
	_serverName(server._serverName),
	_serverAllowedMethods(server._serverAllowedMethods),
	_responseRoot(server._responseRoot),
	_serverErrorPages(server._serverErrorPages),
	_clientMaxBodySize(server._clientMaxBodySize),
	_autoindex(server._autoindex),
	_serverautoindex(server._serverautoindex),
	_index(server._index),
	_serverindex(server._serverindex),
	_configCgi(server._configCgi),
	_locations(server._locations),
	_cgi(server._cgi)
{
	for (size_t i = 0; i < LISTEN_BUFFER_SIZE; i++)
		_eventList[i] = server._eventList[i];
}

Server::~Server() {}

Server&						Server::operator=(const Server& server)
{
	_serverAddr = server._serverAddr;
	_serverSocket = server._serverSocket;
	_listen = server._listen;
	_kq = server._kq;
	_request = server._request;
	_changeList = server._changeList;
	for (size_t i = 0; i < LISTEN_BUFFER_SIZE; i++)
		_eventList[i] = server._eventList[i];

	_response = server._response;
	_bodyCondition = server._bodyCondition;
	_requestEnd = server._requestEnd;
	_checkedRequestLine = server._checkedRequestLine;
	_bodyStartPos = server._bodyStartPos;
	_bodyEnd = server._bodyEnd;
	_bodyVecSize = server._bodyVecSize;
	_bodyVecTotalSize = server._bodyVecTotalSize;
	_bodyVecStartPos = server._bodyVecStartPos;
	_rnPos = server._rnPos;

	_serverRoot = server._serverRoot;
	_serverName = server._serverName;
	_serverAllowedMethods = server._serverAllowedMethods;
	_responseRoot = server._responseRoot;
	_serverErrorPages = server._serverErrorPages;

	_clientMaxBodySize = server._clientMaxBodySize;
	_autoindex = server._autoindex;
	_serverautoindex = server._serverautoindex;
	_index = server._index;
	_serverindex = server._serverindex;
	_configCgi = server._configCgi;

	_locations = server._locations;
	_cgi = server._cgi;
	return *this;
}

void						Server::changeEvents(std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
	struct kevent	tempEvent;

	EV_SET(&tempEvent, ident, filter, flags, fflags, data, udata);
	changeList.push_back(tempEvent);
}

void						Server::disconnectRequest(int fd)
{
	std::cout << "request disconnected: " << fd << std::endl << std::endl << std::endl;
	// std::cout << "request disconnected: " << fd << std::endl;
	resetRequest(fd);
	close(fd);
	_request.erase(fd);
}

void						Server::checkConnection(int fd)
{
	if (_response[fd].getConnection() == "close")
		disconnectRequest(fd);
}

int							Server::initListen(const std::string& hostPort)
{
	if (setListen(hostPort, &_listen) == 1)
		return (printErr("failed to listen"));

	std::cout << "init listen host : " << _listen.host;
	std::cout << ", port : " << ntohs(_listen.port) << std::endl;

	return (0);
}

int							Server::initServerSocket()
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

int							Server::eventError(int fd)
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

void						Server::requestAccept()
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

	_response[requestSocket].resetRequest();
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

std::string					Server::getServerName() const { return (_serverName); }
std::vector<std::string>	Server::getServerAllowedMethods() const { return (_serverAllowedMethods); }
std::string					Server::getResponseRoot() const { return (_responseRoot); }
std::map<int, std::string>	Server::getServerErrorPages() const { return (_serverErrorPages); }
t_listen					Server::getListen() const { return (_listen); }
struct sockaddr_in			Server::getServerAddr() const { return (_serverAddr); }
int							Server::getServerSocket() const { return (_serverSocket); }
int							Server::getKq() { return (_kq); }
std::map<int, std::string>	Server::getRequest() { return (_request); }
std::vector<struct kevent>	Server::getChangeList() { return (_changeList); }
struct kevent&				Server::getEventList(int index) { return (_eventList[index]); }
struct kevent*				Server::getEventList() { return (_eventList); }
void						Server::resetChangeList() { _changeList.clear(); }
std::vector<LocationBlock>	Server::getLocations() { return (_locations); }

void						Server::setServerName(const std::string& name) { _serverName = name; }
void						Server::setServerAllowedMethods(const std::vector<std::string>& allowedMethods) { _serverAllowedMethods = allowedMethods; }
void						Server::setResponseRoot(const std::string& root) { _responseRoot = root; }
void						Server::setServerErrorPages(const int& code, const std::string& html) { _serverErrorPages[code] = html; }
void						Server::setServerErrorPages(const std::map<int, std::string>& html) { _serverErrorPages = html; }
void						Server::initServerErrorPages()
{
	setServerErrorPages(Bad_Request, "400.html");
	setServerErrorPages(Forbidden, "403.html");
	setServerErrorPages(Not_Found, "404.html");
	setServerErrorPages(Method_Not_Allowed, "405.html");
	setServerErrorPages(Payload_Too_Large, "413.html");
	setServerErrorPages(Internal_Server_Error, "500.html");
}
void						Server::setServerRoot(const std::string& root) { _serverRoot = root; }
void						Server::setClientMaxBodySize(const size_t& size) { _clientMaxBodySize = size; }
void						Server::setAutoindex(const int& autoindex) { _autoindex = autoindex; }
void						Server::setServerAutoIndex(const int& serverautoindex) { _serverautoindex = serverautoindex; }
void						Server::setIndex(const std::vector<std::string>& index) { _index = index; }
void						Server::setServerIndex(const std::vector<std::string>& serverindex) { _serverindex = serverindex; }

void						Server::addLocation(LocationBlock& lb) { _locations.push_back(lb); }

void						Server::setCgiEnv(int fd)
{
	_cgi[fd].setBody(_response[fd].getBody());
	_cgi[fd].setEnv("CONTENT_LENGTH", intToStr(_cgi[fd].getBody().length()));
	_cgi[fd].setEnv("PATH_INFO", _response[fd].getPath());
	_cgi[fd].setEnv("SERVER_PORT", "8000");
	_cgi[fd].setEnv("SERVER_PROTOCOL", "HTTP/1.1");
	_cgi[fd].setEnv("REDIRECT_STATUS", "200");
	if (_response[fd].getXHeader() != "")
		_cgi[fd].setEnv("HTTP_X_SECRET_HEADER_FOR_TEST", _response[fd].getXHeader());
}

void						Server::initCgiEnv(int fd)
{
	_cgi[fd].setName(_configCgi);
	_cgi[fd].setEnv("CONTENT_LENGTH", _response[fd].getContentLength());
	_cgi[fd].setEnv("CONTENT_TYPE", _response[fd].getContentType());
	_cgi[fd].setEnv("PATH_INFO", _response[fd].getPath());
	_cgi[fd].setEnv("REQUEST_METHOD", _response[fd].getMethod());
	_cgi[fd].setEnv("SERVER_PORT", intToStr(_response[fd].getListen().port));
	_cgi[fd].setEnv("SERVER_PROTOCOL", _response[fd].getHttpVersion());
}

LocationBlock				Server::selectLocationBlock(std::string requestURI)
{
	std::vector<LocationBlock>	locationBlocks;
	std::vector<std::string>	requestURIvec;
	size_t						max = 0, ret = 0;

	requestURIvec = split(requestURI, '/');

	for (size_t i = 1; i < requestURIvec.size(); i++)
		requestURIvec[i] = "/" + requestURIvec[i];

	for (size_t i = 0; i < _locations.size(); i++)
	{
		if (_locations[i].getMod() == NONE)
		{
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
				std::vector<LocationBlock>	nested = _locations[i].getLocationBlocks();
				for (size_t j = 0; j < nested.size(); j++)
				{
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

	if (locationBlocks.empty())
		return (LocationBlock());

	max = locationBlocks[ret].getURI().length();
	for (size_t i = 1; i < locationBlocks.size(); i++) {
		if (locationBlocks[i].getURI().size() > max) {
			max = locationBlocks[i].getURI().length();
			ret = i;
		}
	}
	return (locationBlocks[ret]);
}

void						Server::locationToServer(LocationBlock block, int fd)
{
	if (block.empty() == true)
		return ;

	if (block.getURI() != "" && (_response[fd].getMethod() == "GET" ||
		(_response[fd].getMethod() != "GET" && _response[fd].getPath() != "/")))
		_response[fd].getPath() = block.getPath();

	if (block.getClntSize() != 0)
		_clientMaxBodySize = block.getClntSize();

	if (block.getMethods().empty() == false)
		_response[fd].initAllowedMethods(block.getMethods());
	if (block.getRoot() != ".")
	{
		_response[fd].getRoot() = block.getRoot();
		size_t	uriPos = 0;
		if ((uriPos = _response[fd].getPath().find(block.getURI())) != std::string::npos)
			_response[fd].setPath(block.getPath());
	}

	_autoindex = block.getAutoindex();

	if (block.getIndex().empty() == false)
		_index = block.getIndex();

	if (block.getCGI() != "")
	{
		_configCgi = block.getCGI();
		_cgi[fd].setCgiExist(true);
		initCgiEnv(fd);
	}
}

void						Server::resetRequest(int fd)
{
	_request[fd].clear();
	_requestEnd[fd] = 0;
	_bodyCondition[fd] = No_Body;
	_response[fd].resetRequest();
	_checkedRequestLine[fd] = 0;
	_bodyStartPos[fd] = 0;
	_bodyEnd[fd] = 0;
	_bodyVecSize[fd] = 0;
	_bodyVecStartPos[fd] = 0;
	_rnPos[fd] = 0;
	_response[fd].resetBodyVec();
	_cgi[fd].setCgiExist(false);
	_response[fd].getTotalResponse().clear();
	_response[fd].setRemainSend(false);
	_clientMaxBodySize = 0;
	_response[fd].setBodySize(0);
	_bodyVecTotalSize[fd] = 0;
	_response[fd].setBody("");
	_response[fd].getRoot() = _serverRoot;
	_index.clear();
	_index = _serverindex;
	_autoindex = _serverautoindex;
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
		if (_response[fd].getContentLength() != "" && _bodyCondition[fd] == Body_Start)
		{
			if (_response[fd].getBodySize() >= _clientMaxBodySize && _clientMaxBodySize != 0)
			{
				printErr("content length is too big to receive");
				_response[fd].setCode(Payload_Too_Large);
				return ;
			}
		}
		n = ::recv(fd, buf, READ_BUFFER_SIZE - 1, 0);
		buf[n] = '\0';
		_request[fd] += buf;

		if (_response[fd].getBodySize() != 0 && _request[fd].length() - _bodyStartPos[fd] >= _response[fd].getBodySize())
		{
			_requestEnd[fd] = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos[fd] + _response[fd].getBodySize());
		}

		if (_request[fd].find("\r\n") != std::string::npos && _checkedRequestLine[fd] == 0)
		{
			std::string	temp_str = allDeleteRN(_request[fd]);
			if (temp_str == "")
			{
				_request[fd].clear();
				printErr("request line is empty");
				return ;
			}
			if (_request[fd].at(0) == '\r' && _request[fd].at(1) == '\n')
				_request[fd] = _request[fd].substr(2, _request[fd].length() - 2);
			if (_request[fd].at(0) == '\n')
				_request[fd] = _request[fd].substr(1, _request[fd].length() - 1);
			_checkedRequestLine[fd] = 1;

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
				_response[fd].setHttpVersion("HTTP/1.1");
				_response[fd].setContentLocation("/");
				_response[fd].setCode(Bad_Request);
				return ;
			}
			if (_response[fd].getPath() != _response[fd].getRoot() && _response[fd].getPath() != "/")
			{
				LocationBlock	test = selectLocationBlock(_response[fd].getPath());

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
			else if ((_response[fd].getPath() == "/" && _response[fd].getMethod() == "GET") || _response[fd].getPath() != "/")
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

		if ((_bodyCondition[fd] == Body_Start || _bodyCondition[fd] == Body_End) && _bodyEnd[fd] == 0)
		{
			if (checkRequestLine == 1 || checkRequestLine == 9)
				;
			else if (_response[fd].splitRequest(_request[fd], _bodyCondition[fd]) == 1)
			{
				printErr("failed to split request");
				_response[fd].setCode(Bad_Request);
				_requestEnd[fd] = 1;
			}
			_bodyEnd[fd] = 1;
		}

		if (_response[fd].getBodySize() != 0 && _response[fd].getContentLength() != "" && _request[fd].length() - _bodyStartPos[fd] >= _response[fd].getBodySize())
		{
			_requestEnd[fd] = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos[fd] + _response[fd].getBodySize());
		}
		if (_response[fd].getTransferEncoding() == "chunked")
		{
			while (_rnPos[fd] < _request[fd].length() && _response[fd].getPath() != "/")
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
					{
						std::cout << "body total size is big so set code payload\n";
						_response[fd].setCode(Payload_Too_Large);
					}
					_bodyVecStartPos[fd] = bodySize + 2;
					_rnPos[fd] = _bodyVecStartPos[fd] + _bodyVecSize[fd] + 2;
				}
				else if (bodySize == std::string::npos)
					break ;
				if (_request[fd].length() > _bodyVecStartPos[fd] + _bodyVecSize[fd] && _bodyVecStartPos[fd] != 0)
				{
					std::string	_bodyElement = _request[fd].substr(_bodyVecStartPos[fd], _bodyVecSize[fd]);
					_response[fd].addBodyVec(_bodyElement);
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
				_requestEnd[fd] = 1;
			}
		}
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
			std::cout << PINK << "request[" << fd << "] received file end" << std::endl << RESET;
			if (_request[fd].length() > 300)
				std::cout << BLUE << _request[fd].substr(0, 250) << " ... " << _request[fd].substr(_request[fd].length() - 20, 20) << RESET << std::endl;
			else
				std::cout << BLUE << _request[fd] << RESET << std::endl;
			if (_bodyStartPos[fd] != 0 && _bodyStartPos[fd] < _request[fd].length())
			{
				if (_response[fd].getTransferEncoding() == "chunked" && _response[fd].getBodyVec().empty() == false)
				{
					std::vector<std::string>	temp_body_vec = _response[fd].getBodyVec();
					for (std::vector<std::string>::iterator it = temp_body_vec.begin(); it != temp_body_vec.end(); it++)
						_response[fd].addBody(*it);
					_response[fd].setContentLength(_bodyVecTotalSize[fd]);
					setCgiEnv(fd);
				}
				else
				{
					_response[fd].setBody(_request[fd].substr(_bodyStartPos[fd], _request[fd].length() - _bodyStartPos[fd]));
					if (_clientMaxBodySize != 0 && _response[fd].getBody().length() > _clientMaxBodySize)
					{
						std::cout << "get body size is big so set code payload\n";
						_response[fd].setCode(Payload_Too_Large);
					}
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
			int	verifyMethodRet = _response[fd].verifyMethod(fd, _requestEnd[fd], _autoindex, _index[0], _cgi[fd]);
			if (verifyMethodRet == 1)
				disconnectRequest(fd);
			if (verifyMethodRet == 2)
				resetRequest(fd);
			if (_response[fd].getConnection() == "close")
				disconnectRequest(fd);
			else if (_response[fd].getRemainSend() == false)
				resetRequest(fd);
		}
	}
}
