#include "./../includes/Server.hpp"

Server::Server() {}
Server::Server(const Server& server) { (void)server; }
Server::~Server() {}

Server&			Server::operator=(const Server& server) { (void)server; return *this; }

void			Server::changeEvents(std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
	struct kevent	temp_event;
	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	changeList.push_back(temp_event);
}

// disconnects the request file descriptor
// @param	fd	request file descriptor to disconnect
void			Server::disconnectRequest(int fd)
{
	std::cout << "request disconnected: [" << fd << "]" << std::endl;
	close(fd);
	_request.erase(fd);
	_requestEnd = 0;
	_bodyCondition = No_Body;
	_response.resetRequest();
	_checkedRequestLine = 0;
	_bodyStartPos = 0;
	_bodyEnd = 0;
	_bodyVecStartPos = 0;
	_bodyVecSize = 0;
	_rnPos = 0;
}

// initializes listen (address) of the server
int				Server::initListen(const std::string& hostPort)
{
	if (_response.setListen(hostPort) == 1)
		return (printErr("failed to listen"));

	_listen.host = _response._listen.host;
	_listen.port = _response._listen.port;

	std::cout << "listen: [" << _listen.host << ":" << _listen.port << "]" << std::endl;
	return (0);
}

// creates server socket
// binds the address with the socket with bind()
// listens to the address with listen()
// initializes the kq with kqueue()
// registers event with changeEvents()
int				Server::initServerSocket ()
{
	int	serverSocket = socket(PF_INET, SOCK_STREAM, 0);

	if (serverSocket == -1)
		return (printErr("failed to create server socket"));

	std::memset(&_serverAddr, 0, sizeof(_serverAddr));
	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_addr.s_addr = _listen.host;
	_serverAddr.sin_port = _listen.port;
	
	if (bind(serverSocket, (struct sockaddr*)&_serverAddr, sizeof(_serverAddr)) == -1)
		return (printErr("failed to bind the socket"));

	if (listen(serverSocket, LISTEN_BUFFER_SIZE) == -1)
		return (printErr("failed to listen"));

	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	_serverSocket = serverSocket;

	int	kq = kqueue();
	if (kq == -1)
		return (printErr("failed to initialize kqueue"));

	_kq = kq;
	changeEvents(_changeList, _serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	return (0);
}

int				Server::eventError (int fd)
{
	// if serverSocket is error, return 1 to quit server
	if (fd == _serverSocket)
		return (printErr("server start but server socket error"));
	// if the request socket is error, disconnect the server
	else
	{
		printErr("server start but client socket error");
		disconnectRequest(fd);
	}
	return (0);
}

void			Server::requestAccept ()
{
	int	requestSocket;

	if ((requestSocket = accept(_serverSocket, NULL, NULL)) == -1)
	{
		printErr("server start but request socket accept error");
		return ;
	}
	std::cout << "accept new request: [" << requestSocket << "]" << std::endl;

	fcntl(requestSocket, F_SETFL, O_NONBLOCK);

	changeEvents(_changeList, requestSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	changeEvents(_changeList, requestSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);

	std::cout << "request is readable and writable" << std::endl;

	_request[requestSocket] = "";
}

void			Server::initCgiEnv ()
{
	//header에 Auth-Scheme가 존재한다면 AUTH_TYPE환경변수에 header의 Authorization값을 집어넣는다.
	//AUTH_TYPE은 인증 타입.
	//만약 client가 액세스에 인증이 필요한 경우, Request Authorization header 필드의 auth-scheme 토큰을 바탕으로 값을 세팅해야 한다.
	// if (headers.find("Auth-Scheme") != headers.end() && headers["Auth-Scheme"] != "")
	// 	_env["AUTH_TYPE"] = headers["Authorization"];
	_cgi.setEnv("AUTH_TYPE", "");

	//원하는 status를 지정하여 PHP가 처리할 수 있는 status를 정한다.
	//php-cgi가 200을 처리할 수 있도록 정했다.
	_cgi.setEnv("REDIRECT_STATUS", "200");

	//request body의 길이, request body가 없으면 NULL값으로 세팅한다.
	//request body가 있을 때만 세팅되어야 하며, transfer-encoding이나 content-coding을 제거한 후의 값으로 세팅되어야 한다.
	_cgi.setEnv("CONTENT_LENGTH", _response.getContentLength());
	//request body의 mime.type을 세팅한다.
	//만약 request에 CONTENT_TYPE헤더가 존재한다면 무조건 세팅해야 한다.
	//CONTENT_TYPE헤더가 존재하지 않으면 올바른 CONTENT_TYPE을 추론하거나, CONTENT_TYPE을 생략해야 한다.
	// _cgi.setEnv("CONTENT_TYPE", headers["Content-Type"]);
	_cgi.setEnv("CONTENT_TYPE", _response.getContentType());

	//서버의 CGI타입과 개정레벨을 나타난다.
	//형식 CGI/revision
	// _cgi.setEnv("GATEWAY_INTERFACE", "CGI/1.1");

	//client에 의해 전달되는 추가 PATH정보
	//PATH_INFO를 이용하여 스크립트를 부를 수 있다.
	_cgi.setEnv("PATH_INFO", _response.getPath());
	//PATH_INFO에 나타난 가상 경로(path)를 실제의 물리적인 경로로 바꾼 값
	_cgi.setEnv("PATH_TRANSLATED", "");

	//GET방식에서 URL의 뒤에 나오는 정보를 저장하거나 폼입력 정보를 저장한다.
	//POST방식은 제외
	_cgi.setEnv("QUERY_STRING", "");

	//client의 IP주소
	_cgi.setEnv("REMOTEaddr", "");
	//client의 호스트 이름
	// _cgi.setEnv("REMOTED_IDENT", headers["Authorization"]);
	_cgi.setEnv("REMOTE_IDENT", "");
	//서버가 사용자 인증을 지원하고, 스크립트가 확인을 요청한다면, 이것이 확인된 사용자 이름이 된다.
	// _cgi.setEnv("REMOTE_USER", headers["Authorization"]);
	_cgi.setEnv("REMOTE_USER", "");

	//GET, POST, PUT과 같은 method
	_cgi.setEnv("REQUEST_METHOD", _response._method);
	//쿼리까지 포함한 모든 주소
	//ex) http://localhost:8000/cgi-bin/ping.sh?var1=value1&var2=with%20percent%20encodig
	_cgi.setEnv("REQUEST_URI", "");

	//실제 스크립트 위치
	//ex) http://localhost:8000/cgi-bin/ping.sh?var1=value1&var2=with%20percent%20encodig의
	// /cgi-bin/ping.sh를 뜻한다.
	//웹에서의 스크립트 경로를 뜻한다.
	_cgi.setEnv("SCRIPT_NAME", "");
	//서버(로컬)에서의 스크립트 경로를 뜻한다.
	//ex) "C:/Program Files (x86)/Apache Software Foundation/Apache2.2/cgi-bin/ping.sh"
	_cgi.setEnv("SCRIPT_FILENAME", "");

	//서버의 호스트 이름과 DNS alias 혹은 IP주소
	// if (headers.find("Hostname") != headers.end())
	// 	_cgi.setEnv("SERVER_NAME", headers["Hostname"]);
	// else
	// 	_cgi.setEnv("SERVER_NAME", _cgi.setEnv("REMOTEaddr",			_cgi.setEnv("SERVER_NAME", _response.getServer());
	//client request를 보내는 포트 번호
	_cgi.setEnv("SERVER_PORT", intToStr(_response.getListen().port));
	//client request가 사용하는 프로토콜
	_cgi.setEnv("SERVER_PROTOCOL", _response.getHttpVersion());
	// std::cout << GREEN << "cgi server version: " << _cgi.getEnv()["SERVER_PROTOCOL"] << RESET << std::endl;
	//웹서버의 이름과 버전을 나타낸다.
	// 형식: 이름/버전
	// _cgi.setEnv("SERVER_SOFTWARE", "Weebserv/1.0");
	
	// _cgi.setEnv(insert(config.,CgiParam().begin(), config.getCgiParam().end()));
}

std::string		trimTrailingSlash (std::string URI) {
	if (URI.at(URI.length() - 1) == '/')
		return (URI.substr(0, URI.length() - 1));
	return (URI);
}

std::string		changePath (std::string URI, int place) {
	size_t	pos = 0;

	if (place == 1)
		pos = URI.find_first_of("/");
	if (place == 2)
		pos = URI.find_last_of("/");

	if (pos != std::string::npos)
		return (URI.substr(pos, URI.length() - pos));
	return (URI);
}

// selects an appropriate location block that matches the request URI
LocationBlock	Server::selectLocationBlock (std::string requestURI)
{
	std::vector<LocationBlock>	locationBlocks;	// if there are multiple blocks that matches the request URI
	std::vector<std::string>	requestURIvec;
	size_t						max = 0, ret = 0;

	// split the request URI by '/'
	std::cout << "request URI: [" << requestURI << "]" << std::endl;
	requestURIvec = split(requestURI, '/');

	// add '/' to each of the string in the vector
	for (size_t i = 1; i < requestURIvec.size(); i++)
		requestURIvec[i] = "/" + requestURIvec[i];

	// look for locations whose modifier is '='; the request URI must match the location's URI
	for (size_t i = 0; i < _locations.size(); i++) {
		if (_locations[i].getMod() == EXACT) {
			if (_locations[i].getURI() == requestURI)
			// if (requestURI.find(_locations[i].getURI()) != std::string::npos)
				return (_locations[i]);
			else
				break ;
		}
	}
	std::cout << BLUE << "path (before selectLocationBlock(): [" << _response._path << "]" << RESET << std::endl;

	// look for locations who don't have modifiers or '~'; the location's URI must contain the request URI
	for (size_t i = 0; i < _locations.size(); i++)
	{
		if (_locations[i].getMod() == NONE || _locations[i].getMod() == PREFERENTIAL)
		{
			std::cout << "locations uri: [" << _locations[i].getURI() << "], requestURIvec[0]: [" << requestURIvec[0] << "]" << std::endl;

			// if the requestedURI perfectly matches the block's URI
			if (_locations[i].getURI() == requestURI)
			{
				// trim the trailing slash if there is one
				requestURI = trimTrailingSlash(requestURI);

				// find the last slash, change the path of the block to [root + path_until_last_slash]
				std::string	path = changePath(requestURI, 1);
				_locations[i].setPath(_response._root + path);

				locationBlocks.push_back(_locations[i]);
			}
			// if the request URI is single path (not nested)
			else if (requestURIvec.size() == 1)
			{
				requestURI = trimTrailingSlash(requestURI);

				std::string	path = changePath(requestURI, 2);
				_locations[i].setPath(_response._root + path);

				locationBlocks.push_back(_locations[i]);
			}
			if (requestURIvec.size() >= 2)
			{
				// if the request URI is not a single path (nested), get the nested locations and compare their URIs
				std::cout << CYAN << "request uri: [" << requestURI << "], location uri: [" << _locations[i].getURI() << "];" << std::endl << RESET;

				std::vector<LocationBlock>	nested = _locations[i].getLocationBlocks();
				for (size_t j = 0; j < nested.size(); j++)
				{
					std::cout << BLUE << "nested uri: [" << nested[j].getURI() << "]" << RESET << std::endl;
					if (!compareURIs(nested[j].getURI(), requestURIvec[1], nested[j].getMod()))
					{
						std::string	path = changePath(requestURI, 2);
						nested[j].setPath(_response._root + path);
						locationBlocks.push_back(nested[j]);
					}
				}
				if (requestURI.find(_locations[i].getURI()) != std::string::npos)
				{
					std::string	path = changePath(requestURI, 1);
					_locations[i].setPath(_response._root + path);
					locationBlocks.push_back(_locations[i]);
				}
			}
		}
	}

	// if no block was found, return empty location
	if (locationBlocks.empty())
	{
		std::cout << PINK << "no block was found. Returning empty location block . . ." << RESET << std::endl;
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
	std::cout << PINK << "ret: ["<< ret << "], path (after selectLocationBlock): [" << locationBlocks[ret].getPath() << "], uri: [" << locationBlocks[ret].getURI() << "]" << RESET << std::endl;

	return (locationBlocks[ret]);
}

void			Server::locationToServer (LocationBlock locationBlock)
{
	if (locationBlock.empty() == true)
		return ;

	if (locationBlock.getURI() != "" && (_response._method == "GET" || (_response._method != "GET" && _response._path != "/")))
		_response._path = locationBlock.getURI();

	if (locationBlock.getClntSize() != READ_BUFFER_SIZE)
		_clientMaxBodySize = locationBlock.getClntSize();

	if (locationBlock.getMethods().empty() == false)
		_response.initAllowedMethods(locationBlock.getMethods());

	std::cout << GREEN << "location block root: [" << locationBlock.getRoot() << "]" << RESET << std::endl;
	if (locationBlock.getRoot() != ".")
	{
		_response._root = locationBlock.getRoot();

		size_t	uriPos = 0;

		if ((uriPos = _response.getPath().find(locationBlock.getURI())) != std::string::npos)
		{
			std::cout << CYAN << "location uri: [" << locationBlock.getURI() << "]" << RESET << std::endl;

			_response.setPath(_response.getRoot() + _response.getPath().substr(
				uriPos + locationBlock.getURI().length(),
				_response.getPath().length() - uriPos - locationBlock.getURI().length()));

			std::cout << RED << "select location and path change: [" << _response.getPath() << "]" << RESET << std::endl;
		}
	}
	if (locationBlock.getAutoindex() != DEFAULT_AUTOINDEX)
		_autoindex = locationBlock.getAutoindex();

	if (locationBlock.getIndex().empty() == false)
		_index = locationBlock.getIndex();

	if (locationBlock.getCGI() != "")
	{
		_configCgi = locationBlock.getCGI();
		std::cout << YELLOW << "cgi: [" << _configCgi << "]" << RESET << std::endl;
		_cgi.setCgiExist(true);
		initCgiEnv();
	}
	
}

// TODO: READ AGAIN
void			Server::eventRead (int fd)
{
	_requestEnd = 0;
	int	checkRequestLineRet = 0;

	// if fd is serverSocket, accept that request
	if (fd == _serverSocket)
	// if (fd == static_cast<uintptr_t>(_serverSocket))
		requestAccept();

	else if (_request.find(fd) != _request.end())
	{
		// XXX: for now, read 1024 bytes
		char	buf[READ_BUFFER_SIZE] = {};
		int		n;

		if (_response._contentLength != "" && _bodyCondition == Body_Start)
		{
			if (_response._bodySize >= _clientMaxBodySize)
			{
				printErr("content length is too big to receive");
				_response.setCode(Payload_Too_Large);
				return ;
			}
		}

		// recv() from fd to buf (reads request message)
		n = ::recv(fd, buf, READ_BUFFER_SIZE - 1, 0);
		buf[n] = '\0';
		_request[fd] += buf;

		std::cout << RED <<  "============request==========" << std::endl << _request[fd] << RESET;

		// when read as much as the body size
		if (_response._bodySize != 0 && _request[fd].length() - _bodyStartPos >= _response._bodySize)
		{
			_requestEnd = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos + _response._bodySize);
		}
		
		// when request message is empty
		if (_request[fd] == "\r\n" && _checkedRequestLine == 0)
		{
			_request[fd].clear();
			std::cout << "empty request message" << std::endl;
			return ;
		}

		if (_request[fd].find("\r\n") != std::string::npos && _checkedRequestLine == 0)
		{
			_checkedRequestLine = 1;
			std::cout << "check request line" << std::endl;
			checkRequestLineRet = _response.checkRequestLine(_request[fd]);
			if (checkRequestLineRet == 9)
			{
				std::cout << "request's http version is HTTP/0.9" << std::endl;
				_requestEnd = 1;
			}
			else if (checkRequestLineRet == 1)
			{
				printErr("invalid request line");
				_response._httpVersion = "HTTP/1.1";
				_response._contentLocation = "/";
				_response.setCode(Bad_Request);
				return ;
			}
			// find appropriate location block (selectLocationBlock()) with the path parsed through checkRequestLine.
			// if path and root are not equal, the location block's variables are moved to server block
			// if location block is not found, just use the server block's values
			if (_response._path != _response._root && _response._path != "/")
			{
				std::cout << PINK << "@@@@@@@@@@@@@select location@@@@@@@@@@@@" << RESET << std::endl;
				std::cout << "response root: [" << _response._root << "], response path: [" << _response._path << "]" << std::endl;

				LocationBlock	loc = selectLocationBlock(_response._path);
				if (loc.empty() == false)
				{
					_response.setPath(loc.getPath());
					locationToServer(loc);
				}
				else
				{
					std::cout << CYAN << "path: [" << _response.getPath() << "]" << std::endl;
					std::cout << "last character of path: [" << _response.getPath().at(_response.getPath().length() - 1) << "]" << std::endl;

//					if (_response.getPath().at(_response.getPath().length() - 1) == '/')
//						_response.setPath(_response.getPath().substr(0, _response.getPath().length() - 1));
					_response.setPath(trimTrailingSlash(_response.getPath()));
//					if (_response.getPath().length() != 0 && _response.getPath().at(0) == '/')
//						_response.setPath(_response.getRoot() + _response.getPath());
//					else if (_response.getPath().length() != 0)
					if (_response.getPath().length() != 0 && _response.getPath().at(0) != '/')
						_response.setPath(_response.getRoot() + "/" + _response.getPath());
					else
						_response.setPath(_response.getRoot() + _response.getPath());

					std::cout << "path: [" << _response.getPath() << "]" << RESET << std::endl;
				}
			}
			else if ((_response.getPath() == "/" && _response._method == "GET") || _response.getPath() != "/")
			{
				std::cout << RED << "path: " << _response.getPath() << std::endl;
				std::cout << "last characther of path: [" << _response.getPath().at(_response.getPath().length() - 1) << "]" << std::endl;

//				if (_response.getPath().at(_response.getPath().length() - 1) == '/')
//					_response.setPath(_response.getPath().substr(0, _response.getPath().length() - 1));
				_response.setPath(trimTrailingSlash(_response.getPath()));
//				if (_response.getPath().length() != 0 && _response.getPath().at(0) == '/')
//					_response.setPath(_response.getRoot() + _response.getPath());
//				else if (_response.getPath().length() != 0)
				if (_response.getPath().length() != 0 && _response.getPath().at(0) != '/')
					_response.setPath(_response.getRoot() + "/" + _response.getPath());
				else
					_response.setPath(_response.getRoot() + _response.getPath());

				std::cout << "path: [" << _response.getPath() << "]" << RESET << std::endl;
			}
		}

		if ((strncmp(_request[fd].c_str(), "POST", 4) == 0 || strncmp(_request[fd].c_str(), "PUT", 3) == 0) && _bodyCondition == No_Body)
			_bodyCondition = Body_Exist;

		size_t	rnrnPos;

		if (_request[fd].empty() == false && (rnrnPos = _request[fd].find("\r\n\r\n")) != std::string::npos)
		{
			if (_bodyCondition == No_Body)
			{
				_requestEnd = 1;
				_bodyCondition = Body_End;
			}
			else if (_bodyCondition == Body_Exist)
			{
				_bodyCondition = Body_Start;
				_bodyStartPos = rnrnPos + 4;
			}
		}

		//check header
		if ((_bodyCondition == Body_Start || _bodyCondition == Body_End) && _bodyEnd == 0)
		{
			if (checkRequestLineRet == 1 || checkRequestLineRet == 9)
				;
			else if (_response.splitRequest(_request[fd], _bodyCondition) == 1)
			{
				printErr("request split error");
				_response.setCode(Bad_Request);
			}
			_bodyEnd = 1;
		}

		// if read as much as body size
		if (_response._bodySize != 0 && _response._contentLength != "" && _request[fd].length() - _bodyStartPos >= _response._bodySize)
		{
			_requestEnd = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos + _response._bodySize);
		}

		// if contentLength is empty and transferEncoding is chunked, recv until the end of file
		if (_response._transferEncoding == "chunked")
		{
			if (_response._path != "/")
			{
				if (_rnPos <= _bodyStartPos)
					_rnPos = _bodyStartPos;
				else
					_rnPos = _bodyVecStartPos;

				size_t	bodySize = _request[fd].find("\r\n", _rnPos + 3);
				std::cout << "rn pos: [" << _rnPos << "], body start pos: [" << _bodyStartPos << "]" << std::endl;
				// std::cout << "!!!!!!!!request!!!!!!! << _request[fd].substr(0, _rnPos) << std::endl;
				if (bodySize != std::string::npos && _bodyVecSize == 0)
				{
					std::string	bodySizeStr = _request[fd].substr(_rnPos, bodySize - _rnPos);

					if (bodySizeStr.find("e") != std::string::npos)
						_bodyVecSize = calExponent(bodySizeStr);
					else
						_bodyVecSize = std::atoi(bodySizeStr.c_str());

					_bodyVecStartPos = bodySize + 2;
					_rnPos = _bodyVecStartPos + _bodyVecSize;

					std::cout << GREEN << "body vec size: " << _bodyVecSize << std::endl;
					// std::cout << "bodySizeStr: " << bodySizeStr << RESET << std::endl;
					if (_bodyVecSize == 0)
						_response.setCode(Bad_Request);
				}
			}
			if (_request[fd].length() > _bodyVecStartPos + _bodyVecSize && _bodyVecStartPos != 0)
			{
				std::string	_bodyElement = _request[fd].substr(_bodyVecStartPos, _bodyVecSize);
				_response._bodyVec.push_back(_bodyElement);
				_bodyVecStartPos += _bodyVecSize;
				_bodyVecSize = 0;
			}

			std::cout << RED << "body vec start pos: [" << _bodyVecStartPos;
			std::cout << "], body vec size: [" << _bodyVecSize;
			std::cout << "], request length: [" << _request[fd].length() << "]" << RESET << std::endl;
			usleep(100);

			if (_request[fd].find("0\r\n\r\n") != std::string::npos &&
				(_bodyVecStartPos == 0 ||
				(_bodyVecStartPos != 0 && _bodyVecSize > _request[fd].length() - _bodyVecStartPos)))
			{
				_request[fd] = _request[fd].substr(0, _request[fd].length() - 5);
				if (_bodyVecStartPos != 0)
				{
					std::string	_bodyElement = _request[fd].substr(_bodyVecStartPos, _request[fd].length() - _bodyVecStartPos);
					_response._bodyVec.push_back(_bodyElement);
				}
				std::cout << "receive file end" << std::endl;
				_requestEnd = 1;
			}

			else
				return ;
		}

		if (n <= 0)
		{
			if (n < 0)
			{
				printErr("::recv()");
				_response.setCode(Internal_Server_Error);
			}
			else
				disconnectRequest(fd);
		}
		else if (_requestEnd == 1)
		{
			if (_bodyStartPos != 0 && _bodyStartPos < _request[fd].length())
			{
				_response._body = _request[fd].substr(_bodyStartPos, _request[fd].length() - _bodyStartPos);
				if (_response._transferEncoding == "chunked" && _response._bodyVec.empty() == false)
				{
					for (std::vector<std::string>::iterator it = _response._bodyVec.begin(); it != _response._bodyVec.end(); it++)
						_response._body += *it;
				}
				// std::cout << YELLOW << "=====BODY=====\n" << _response._body << RESET << std::endl;
			}
		}
	}
}

void			Server::eventWrite (int fd)
{
	if (_request.find(fd) != _request.end())
	{
		if (_response.verifyMethod(fd, _requestEnd) == 1)
			disconnectRequest(fd);

		if (_requestEnd == 1) 
			// _response.getErrorMap().find(_response.getCode()) != _response.getErrorMap().end())
		{
			
			std::cout << "verifyMethod, code: [" <<  _response._code << "]" << std::endl;
			std::cout << RED << "response path: [" << _response.getPath() << "]" << RESET << std::endl;

			if (_response._connection == "close")
				disconnectRequest(fd);
			else
			{
				_request[fd].clear();
				_requestEnd = 0;
				_bodyCondition = No_Body;
				_response.initRequest();
				_checkedRequestLine = 0;
				_bodyStartPos = 0;
				_bodyEnd = 0;
				_bodyVecSize = 0;
				_bodyVecStartPos = 0;
				_rnPos = 0;
			}
		}
	}
}

void			Server::initServerMember ()
{
	_response.initRequest();
	_response.initPossibleMethods();
	_response.initErrorMap();
	_bodyCondition = No_Body;
	_bodyEnd = 0;
	_bodyStartPos = 0;
}
