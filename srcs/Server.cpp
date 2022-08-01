#include "./../includes/Server.hpp"

Server::Server () {}
Server::Server (const Server& server) { (void)server; }
Server::~Server () {}

Server&			Server::operator= (const Server& server) { (void)server; return *this; }

void			Server::changeEvents (std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata)
{
	struct kevent	tempEvent;

	EV_SET(&tempEvent, ident, filter, flags, fflags, data, udata);
	changeList.push_back(tempEvent);
}

void			Server::disconnectRequest (int request_fd)
{
	std::cout << "request disconnected: " << request_fd << std::endl;
	close(request_fd);
	_request.erase(request_fd);
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

void			Server::checkConnection (int request_fd)
{
	if (_response._connection == "close")
		disconnectRequest(request_fd);
}

int				Server::initListen (const std::string& host_port)
{
	if (_response.setListen(host_port) == 1)
		return (printErr("failed to listen"));

	_listen.host = _response._listen.host;
	_listen.port = _response._listen.port;

	std::cout << "init listen host : " << _listen.host;
	std::cout << ", port : " << _listen.port << std::endl;

	return (0);
}

//에러가 발생했을 때 에러 메시지를 출력하고 1을 리턴, 정상 작동이면 0을 리턴
//server socket을 만들고, bind, listen, kqueue를 하고,
//바로 changeEvents를 통해 event를 등록한다.
int				Server::initServerSocket ()
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

int				Server::eventError (int fd)
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

void			Server::requestAccept ()
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

LocationBlock	Server::selectLocationBlock (std::string requestURI)
{
	std::vector<LocationBlock>	locationBlocks;
	std::vector<std::string>	requestURIvec;
	size_t						max = 0, ret = 0;

	// first, split the request URI by '/'
	std::cout << "requestURI : " << requestURI << std::endl;
	requestURIvec = split(requestURI, '/');

	// add '/' to each of the string in the vector
	for (size_t i = 1; i < requestURIvec.size(); i++)
		requestURIvec[i] = "/" + requestURIvec[i];

	// look for locations whose modifier is '='; the request URI must match the location's URI
	for (size_t i = 0; i < _locations.size(); i++) {
		if (_locations[i].getMod() == EXACT) {
			if (_locations[i].getURI() == requestURI)
				return (_locations[i]);
			else
				break ;
		}
	}
	std::cout << BLUE << "before select location path : " << _response._path << RESET << std::endl;
	// look for locations who don't have modifiers or '~'; the location's URI must contain the request URI
	for (size_t i = 0; i < _locations.size(); i++)
	{
		if (_locations[i].getMod() == NONE || _locations[i].getMod() == PREFERENTIAL)
		{
			std::cout << "locations uri : " << _locations[i].getURI() << ", requestURI first : " << requestURIvec[0] << std::endl;
				// first, check if the two URIs match
			if (_locations[i].getURI() == requestURIvec[0])
			{
				std::string	change_path = "";
				if (requestURI.at(requestURI.length() - 1) == '/')
					requestURI.substr(0, requestURI.length() - 1);
				size_t	first_slash_pos = requestURI.find_first_of("/");
				if (first_slash_pos != std::string::npos)
					change_path = requestURI.substr(first_slash_pos, requestURI.length() - first_slash_pos);
				// size_t	last_slash_pos = requestURI.find_last_of("/");
				// if (last_slash_pos != std::string::npos)
				// 	change_path = requestURI.substr(last_slash_pos, requestURI.length() - last_slash_pos);
				// 	_locations[i].getRoot();
				_locations[i].setPath(_locations[i].getRoot() + change_path);
				locationBlocks.push_back(_locations[i]);
			}
			// then, check for the level of the request URI
			else if (requestURIvec.size() == 1)
			{
				std::string	change_path = "";
				if (requestURI.at(requestURI.length() - 1) == '/')
					requestURI.substr(0, requestURI.length() - 1);
				size_t	last_slash_pos = requestURI.find_last_of("/");
				if (last_slash_pos != std::string::npos)
					change_path = requestURI.substr(last_slash_pos, requestURI.length() - last_slash_pos);
				_locations[i].setPath(_locations[i].getRoot() + change_path);
				locationBlocks.push_back(_locations[i]);
			}
			if (requestURIvec.size() >= 2)
			{
				// if the request URI has more than one slashes (nested), get the nested locations and compare their URIs
				std::cout << CYAN << "request uri : " << requestURI << ", location uri: " << _locations[i].getURI() << ";" << std::endl << RESET;
				std::vector<LocationBlock>	nested = _locations[i].getLocationBlocks();
				for (size_t j = 0; j < nested.size(); j++)
				{
					std::cout << BLUE << "nested uri: " << nested[j].getURI() << RESET << std::endl;
					if (!compareURIs(nested[j].getURI(), requestURIvec[1], nested[j].getMod()))
					{
						std::string	change_path = "";
						size_t	last_slash_pos = requestURI.find_last_of("/");
						if (last_slash_pos != std::string::npos)
							change_path = requestURI.substr(last_slash_pos, requestURI.length() - last_slash_pos);
						nested[j].setPath(nested[j].getRoot() + change_path);
						locationBlocks.push_back(nested[j]);
					}
				}
				if (requestURI.find(_locations[i].getURI()) != std::string::npos)
				{
					std::string	change_path = "";
					size_t	first_slash_pos = requestURI.find_first_of("/");
					if (first_slash_pos != std::string::npos)
						change_path = requestURI.substr(first_slash_pos, requestURI.length() - first_slash_pos);
					_locations[i].setPath(_locations[i].getRoot() + change_path);
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
	std::cout << PINK << "ret: "<< ret << ", after select location path : " << locationBlocks[ret].getPath();
	std::cout << ", select location uri: " << locationBlocks[ret].getURI() << RESET << std::endl;

	return (locationBlocks[ret]);
}

void			Server::locationToServer (LocationBlock block)
{
	if (block.getIsEmpty() == true)
		return ;

	if (block.getURI() != "" && (_response._method == "GET" ||
		(_response._method != "GET" && _response._path != "/")))
		_response._path = block.getPath();

	if (block.getClntSize() != READ_BUFFER_SIZE)
		_clientMaxBodySize = block.getClntSize();

	if (block.getMethods().empty() == false)
		_response.initAllowedMethods(block.getMethods());

	std::cout << GREEN << "location block root: " << block.getRoot() << RESET << std::endl;

	if (block.getRoot() != ".")
	{
		_response._root = block.getRoot();
		size_t	uriPos = 0;
		if ((uriPos = _response.getPath().find(block.getURI())) != std::string::npos)
		{
			std::cout << CYAN << "location uri: " << block.getURI() << RESET << std::endl;
			_response.setPath(block.getPath());
			std::cout << RED << "select location and path change : " << _response.getPath() << RESET << std::endl;
		}
	}

	if (block.getAutoindex() != DEFAULT_AUTOINDEX)
		_autoindex = block.getAutoindex();

	if (block.getIndex().empty() == false)
		_index = block.getIndex();

	if (block.getCGI() != "")
	{
		_configCgi = block.getCGI();
		std::cout << YELLOW << "cgi: " << _configCgi << RESET << std::endl;
		_cgi.setCgiExist(true);
		initCgiEnv();
	}
	
}

void			Server::eventRead(int fd)
{
	_requestEnd = 0;
	int	checkRequestLine = 0;

	if (fd == _serverSocket)
		requestAccept();
	else if (_request.find(fd) != _request.end())
	{
		char	buf[READ_BUFFER_SIZE] = {};
		int		n;

		if (_response._contentLength != "" && _bodyCondition == Body_Start)
		{//저장해놓은 contentLength만큼만 받도록 한다.
			if (_response._bodySize >= _clientMaxBodySize)
			{
				printErr("content length is too big to receive");
				_response.setCode(Payload_Too_Large);
				return ;
			}
		}
		n = ::recv(fd, buf, READ_BUFFER_SIZE - 1, 0);
		buf[n] = '\0';
		_request[fd] += buf;

		if (_response._bodySize != 0 && _request[fd].length() - _bodyStartPos >= _response._bodySize)
		{
			_requestEnd = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos + _response._bodySize);
		}
		
		if (_request[fd] == "\r\n" && _checkedRequestLine == 0)
		{
			_request[fd].clear();
			printErr("request line is empty");
			return ;
		}

		if (_request[fd].find("\r\n") != std::string::npos && _checkedRequestLine == 0)
		{
			_checkedRequestLine = 1;
			std::cout << "check request line" << std::endl;
			checkRequestLine = _response.checkRequestLine(_request[fd]);
			if (checkRequestLine == 9)
			{
				std::cout << "request's http version is HTTP/0.9" << std::endl;
				_requestEnd = 1;
			}
			else if (checkRequestLine == 1)
			{
				printErr("check request line");
				_response._httpVersion = "HTTP/1.1";
				_response._contentLocation = "/";
				_response.setCode(Bad_Request);
				return ;
			}
			//checkRequestLine으로 parsing한 path를 통해서 select location block으로 적절한 location block을 찾는다.
			//path이 root랑 다를 때 Location block의 변수들을 Server로 넘겨준다.
			//만약 location을 못 찾았다면 그냥 server block의 변수를 사용하면 된다.
			if (_response._path != _response._root && _response._path != "/")
			{
				std::cout << PINK << "@@@@@@@@@@@@@select location@@@@@@@@@@@@" << RESET << std::endl;
				std::cout << "response root: " << _response._root << ", response path: " << _response._path << std::endl;

				//값을 가지고 있는 location block을 찾았을 경우에 이 location block의 변수들을 모두 넣어주면 된다.
				LocationBlock	test = selectLocationBlock(_response._path);
				if (test.getIsEmpty() == false)
				{
					_response.setPath(test.getPath());
					locationToServer(test);
				}
				else
				{
					std::cout << CYAN << "path: " << _response.getPath() << std::endl;
					std::cout << "path last: " << _response.getPath().at(_response.getPath().length() - 1) << std::endl;

					if (_response.getPath().at(_response.getPath().length() - 1) == '/')
						_response.setPath(_response.getPath().substr(0, _response.getPath().length() - 1));

					if (_response.getPath().length() != 0 && _response.getPath().at(0) == '/')
						_response.setPath(_response.getRoot() + _response.getPath());

					else if (_response.getPath().length() != 0)
						_response.setPath(_response.getRoot() + "/" + _response.getPath());

					else
						_response.setPath(_response.getRoot() + _response.getPath());
					std::cout << "path: " << _response.getPath() << RESET << std::endl;
				}
			}
			else if ((_response.getPath() == "/" && _response._method == "GET") || _response.getPath() != "/")
			{
				std::cout << RED << "path: " << _response.getPath() << std::endl;
				std::cout << "path last: " << _response.getPath().at(_response.getPath().length() - 1) << std::endl;

				if (_response.getPath().at(_response.getPath().length() - 1) == '/')
					_response.setPath(_response.getPath().substr(0, _response.getPath().length() - 1));

				if (_response.getPath().length() != 0 && _response.getPath().at(0) == '/')
					_response.setPath(_response.getRoot() + _response.getPath());

				else if (_response.getPath().length() != 0)
					_response.setPath(_response.getRoot() + "/" + _response.getPath());

				else
					_response.setPath(_response.getRoot() + _response.getPath());

				std::cout << "path: " << _response.getPath() << RESET << std::endl;
			}
		}

		if ((strncmp(_request[fd].c_str(), "POST", 4) == 0 ||
			strncmp(_request[fd].c_str(), "PUT", 3) == 0) && _bodyCondition == No_Body)
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
		if ((_bodyCondition == Body_Start || _bodyCondition == Body_End)
			&& _bodyEnd == 0)
		{
			if (checkRequestLine == 1 || checkRequestLine == 9)
				;
			else if (_response.splitRequest(_request[fd], _bodyCondition) == 1)
			{
				printErr("failed to split request");
				_response.setCode(Bad_Request);
			}
			_bodyEnd = 1;
		}

		//body size만큼 입력 받았을 때
		if (_response._bodySize != 0 && _response._contentLength != ""
			&& _request[fd].length() - _bodyStartPos >= _response._bodySize)
		{
			_requestEnd = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos + _response._bodySize);
		}
		//contentLength가 없고, transferEncoding이 chunked일 때 파일의 끝을 알리는 것이 들어올 때까지 계속 recv
		if (_response._transferEncoding == "chunked")
		{
			if (_response._path != "/")
			{
				if (_rnPos <= _bodyStartPos)
					_rnPos = _bodyStartPos;

				size_t	bodySize = _request[fd].find("\r\n", _rnPos + 3);

				std::cout << "rn pos: " << _rnPos << ", body start pos: " << _bodyStartPos << std::endl;
				// std::cout << "!!!!!!!!!!request!!!!!!!" << _request[fd].substr(0, _rnPos) << std::endl;
				if (bodySize != std::string::npos && _bodyVecSize == 0)
				{
					std::string	bodySize_str = _request[fd].substr(_rnPos, bodySize - _rnPos);

					if (bodySize_str.find("e") != std::string::npos)
						_bodyVecSize = calExponent(bodySize_str);

					else
						_bodyVecSize = std::atoi(bodySize_str.c_str());

					_bodyVecStartPos = bodySize + 2;
					_rnPos = _bodyVecStartPos + _bodyVecSize;

					std::cout << GREEN << "body vec size: " << _bodyVecSize << std::endl;
					std::cout << "body size: " << bodySize_str.length() << RESET << std::endl;
					// std::cout << "bodySize_str: " << bodySize_str << RESET << std::endl;
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
			std::cout << RED << "body vec start pos: " << _bodyVecStartPos;
			std::cout << ", body vec size: " << _bodyVecSize;
			std::cout << "request length: " << _request[fd].length() << RESET << std::endl;
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
		std::cout << RED <<  "============request==========" << std::endl << _request[fd] << RESET;

		//read가 에러가 났거나, request가 0을 보내면 request와 연결을 끊는다.
		if (n <= 0)
		{
			if (n < 0)
			{
				printErr("failed to read request");
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

void			Server::eventWrite(int fd)
{
	if (_request.find(fd) != _request.end())
	{
		if (_response.verifyMethod(fd, &_response, _requestEnd) == 1)
			disconnectRequest(fd);

		if (_requestEnd == 1) 
			// _response.getErrorMap().find(_response.getCode()) != _response.getErrorMap().end())
		{
			std::cout << "verify_method, code :  " <<  _response._code << std::endl;
			std::cout << RED << "response path : " << _response.getPath() << RESET << std::endl;

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

void			Server::initServerMember()
{
	_response.initRequest();
	_response.initPossibleMethods();
	_response.initErrorMap();
	_bodyCondition = No_Body;
	_bodyEnd = 0;
	_bodyStartPos = 0;
}
