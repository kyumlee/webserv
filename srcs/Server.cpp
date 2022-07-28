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

void			Server::disconnectRequest(int request_fd)
{
	std::cout << "request disconnected: " << request_fd << std::endl;
	close(request_fd);
	this->_request.erase(request_fd);
	this->_requestEnd = 0;
	this->_bodyCondition = No_Body;
	this->_response.resetRequest();
	this->_checkedRequestLine = 0;
	this->_bodyStartPos = 0;
	this->_bodyEnd = 0;
	this->_bodyVecStartPos = 0;
	this->_bodyVecSize = 0;
	this->_rnPos = 0;
}

void			Server::checkConnection(int request_fd)
{
	if (this->_response._connection == "close")
		this->disconnectRequest(request_fd);
}

int				Server::initListen(const std::string& hostPort)
{
	if (this->_response.setListen(hostPort) == 1)
		return (printErr("init listen error"));

	this->_listen.host = this->_response._listen.host;
	this->_listen.port = this->_response._listen.port;
	std::cout << "init listen host : " << this->_listen.host;
	std::cout << ", port : " << this->_listen.port << std::endl;
	return (0);
}

int				Server::initServerSocket ()
{//에러가 발생했을 때 에러 메시지를 출력하고 1을 리턴, 정상 작동이면 0을 리턴
//server socket을 만들고, bind, listen, kqueue를 하고,
//바로 changeEvents를 통해 event를 등록한다.
	int	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
		return (printErr("init server socket error"));

	std::memset(&this->_serverAddr, 0, sizeof(this->_serverAddr));
	this->_serverAddr.sin_family = AF_INET;
	this->_serverAddr.sin_addr.s_addr = this->_listen.host;
	this->_serverAddr.sin_port = this->_listen.port;
	
	if (bind(serverSocket, (struct sockaddr*)&this->_serverAddr,
		sizeof(this->_serverAddr)) == -1)
		return (printErr("bind socket error"));

	if (listen(serverSocket, LISTEN_BUFFER_SIZE) == -1)
		return (printErr("listen socket error"));

	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	this->_serverSocket = serverSocket;

	int	kq = kqueue();
	if (kq == -1)
		return (printErr("init kqueue error"));

	this->_kq = kq;
	this->changeEvents(this->_changeList, this->_serverSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	return (0);
}

int				Server::eventError (int fd)
{
	//serverSocket이 에러라면 server를 종료하기 위해 -1을 리턴한다.
	if (fd == this->_serverSocket)
		return (printErr("server start but server socket error"));
	//request socket이 에러라면 request의 연결을 끊는다.
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

	//accept이 실패했을 때
	if ((requestSocket = accept(this->_serverSocket, NULL, NULL)) == -1)
	{
		printErr("server start but request socket accept error");
		return ;
	}
	std::cout << "accept new request: " << requestSocket << std::endl;
	fcntl(requestSocket, F_SETFL, O_NONBLOCK);

	changeEvents(this->_changeList, requestSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	changeEvents(this->_changeList, requestSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	std::cout << "request is readable and writable\n";
	this->_request[requestSocket] = "";
}

void			Server::initCgiEnv ()
{
	//header에 Auth-Scheme가 존재한다면 AUTH_TYPE환경변수에 header의 Authorization값을 집어넣는다.
	//AUTH_TYPE은 인증 타입.
	//만약 client가 액세스에 인증이 필요한 경우, Request Authorization header 필드의 auth-scheme 토큰을 바탕으로 값을 세팅해야 한다.
	// if (headers.find("Auth-Scheme") != headers.end() && headers["Auth-Scheme"] != "")
	// 	this->_env["AUTH_TYPE"] = headers["Authorization"];
	this->_cgi.setEnv("AUTH_TYPE", "");

	//원하는 status를 지정하여 PHP가 처리할 수 있는 status를 정한다.
	//php-cgi가 200을 처리할 수 있도록 정했다.
	this->_cgi.setEnv("REDIRECT_STATUS", "200");

	//request body의 길이, request body가 없으면 NULL값으로 세팅한다.
	//request body가 있을 때만 세팅되어야 하며, transfer-encoding이나 content-coding을 제거한 후의 값으로 세팅되어야 한다.
	this->_cgi.setEnv("CONTENT_LENGTH", this->_response.getContentLength());
	//request body의 mime.type을 세팅한다.
	//만약 request에 CONTENT_TYPE헤더가 존재한다면 무조건 세팅해야 한다.
	//CONTENT_TYPE헤더가 존재하지 않으면 올바른 CONTENT_TYPE을 추론하거나, CONTENT_TYPE을 생략해야 한다.
	// this->_cgi.setEnv("CONTENT_TYPE", headers["Content-Type"]);
	this->_cgi.setEnv("CONTENT_TYPE", this->_response.getContentType());

	//서버의 CGI타입과 개정레벨을 나타난다.
	//형식 CGI/revision
	// this->_cgi.setEnv("GATEWAY_INTERFACE", "CGI/1.1");

	//client에 의해 전달되는 추가 PATH정보
	//PATH_INFO를 이용하여 스크립트를 부를 수 있다.
	this->_cgi.setEnv("PATH_INFO", this->_response.getPath());
	//PATH_INFO에 나타난 가상 경로(path)를 실제의 물리적인 경로로 바꾼 값
	this->_cgi.setEnv("PATH_TRANSLATED", "");

	//GET방식에서 URL의 뒤에 나오는 정보를 저장하거나 폼입력 정보를 저장한다.
	//POST방식은 제외
	this->_cgi.setEnv("QUERY_STRING", "");

	//client의 IP주소
	this->_cgi.setEnv("REMOTEaddr", "");
	//client의 호스트 이름
	// this->_cgi.setEnv("REMOTED_IDENT", headers["Authorization"]);
	this->_cgi.setEnv("REMOTE_IDENT", "");
	//서버가 사용자 인증을 지원하고, 스크립트가 확인을 요청한다면, 이것이 확인된 사용자 이름이 된다.
	// this->_cgi.setEnv("REMOTE_USER", headers["Authorization"]);
	this->_cgi.setEnv("REMOTE_USER", "");

	//GET, POST, PUT과 같은 method
	this->_cgi.setEnv("REQUEST_METHOD", this->_response._method);
	//쿼리까지 포함한 모든 주소
	//ex) http://localhost:8000/cgi-bin/ping.sh?var1=value1&var2=with%20percent%20encodig
	this->_cgi.setEnv("REQUEST_URI", "");

	//실제 스크립트 위치
	//ex) http://localhost:8000/cgi-bin/ping.sh?var1=value1&var2=with%20percent%20encodig의
	// /cgi-bin/ping.sh를 뜻한다.
	//웹에서의 스크립트 경로를 뜻한다.
	this->_cgi.setEnv("SCRIPT_NAME", "");
	//서버(로컬)에서의 스크립트 경로를 뜻한다.
	//ex) "C:/Program Files (x86)/Apache Software Foundation/Apache2.2/cgi-bin/ping.sh"
	this->_cgi.setEnv("SCRIPT_FILENAME", "");

	//서버의 호스트 이름과 DNS alias 혹은 IP주소
	// if (headers.find("Hostname") != headers.end())
	// 	this->_cgi.setEnv("SERVER_NAME", headers["Hostname"]);
	// else
	// 	this->_cgi.setEnv("SERVER_NAME", this->_cgi.setEnv("REMOTEaddr",			this->_cgi.setEnv("SERVER_NAME", this->_response.getServer());
	//client request를 보내는 포트 번호
	this->_cgi.setEnv("SERVER_PORT", intToStr(this->_response.getListen().port));
	//client request가 사용하는 프로토콜
	this->_cgi.setEnv("SERVER_PROTOCOL", this->_response.getHttpVersion());
	// std::cout << GREEN << "cgi server version: " << this->_cgi.getEnv()["SERVER_PROTOCOL"] << RESET << std::endl;
	//웹서버의 이름과 버전을 나타낸다.
	// 형식: 이름/버전
	// this->_cgi.setEnv("SERVER_SOFTWARE", "Weebserv/1.0");
	
	// this->_cgi.setEnv(insert(config.,CgiParam().begin(), config.getCgiParam().end()));
}

LocationBlock	Server::selectLocationBlock (std::string requestURI)
{
	std::vector<LocationBlock>	locationBlocks;
	std::vector<std::string>	requestURIvec;
	size_t						max = 0,ret = 0;

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
			// if (requestURI.find(_locations[i].getURI()) != std::string::npos)
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
			// if (_locations[i].getURI().find(requestURIvec[0], 0) != std::string::npos) {
				// first, check if the two URIs match
			if (_locations[i].getURI() == requestURI)
			{
				std::string	changePath = "";
				if (requestURI.at(requestURI.length() - 1) == '/')
					requestURI.substr(0, requestURI.length() - 1);
				size_t	lastSlashPos = requestURI.find_last_of("/");
				if (lastSlashPos != std::string::npos)
					changePath = requestURI.substr(lastSlashPos, requestURI.length() - lastSlashPos);
				_locations[i].setPath(_response._root + changePath);
				locationBlocks.push_back(_locations[i]);
			}
			// then, check for the level of the request URI
			else if (requestURIvec.size() == 1)
			{
				std::string	changePath = "";
				if (requestURI.at(requestURI.length() - 1) == '/')
					requestURI.substr(0, requestURI.length() - 1);
				size_t	lastSlashPos = requestURI.find_last_of("/");
				if (lastSlashPos != std::string::npos)
					changePath = requestURI.substr(lastSlashPos, requestURI.length() - lastSlashPos);
				_locations[i].setPath(_response._root + changePath);
				locationBlocks.push_back(_locations[i]);
			}
			else
			{
				// if the request URI has more than one slashes (nested), get the nested locations and compare their URIs
				std::cout << CYAN << "request uri : " << requestURI << ", location uri: " << _locations[i].getURI() << ";;\n" << RESET;
				std::vector<LocationBlock>	nested = _locations[i].getLocationBlocks();
				for (size_t j = 0; j < nested.size(); j++)
				{
					std::cout << BLUE << "nested uri: " << nested[j].getURI() << RESET << std::endl;
					if (!compareURIsWithWildcard(nested[j].getURI(), requestURIvec[1], nested[j].getMod()))
					{
						std::string	changePath = "";
						size_t	lastSlashPos = requestURI.find_last_of("/");
						if (lastSlashPos != std::string::npos)
							changePath = requestURI.substr(lastSlashPos, requestURI.length() - lastSlashPos);
						nested[j].setPath(_response._root + changePath);
						locationBlocks.push_back(nested[j]);
					}
				}
				if (requestURI.find(_locations[i].getURI()) != std::string::npos)
				{
					std::string	changePath = "";
					size_t	firstSlashPos = requestURI.find_first_of("/");
					if (firstSlashPos != std::string::npos)
						changePath = requestURI.substr(firstSlashPos, requestURI.length() - firstSlashPos);
					_locations[i].setPath(_response._root + changePath);
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

void			Server::locationToServer (LocationBlock locationBlock)
{
	if (locationBlock.getIsEmpty() == true)
		return ;
	if (locationBlock.getURI() != "" && (_response._method == "GET" ||
		(_response._method != "GET" && _response._path != "/")))
		_response._path = locationBlock.getURI();
	if (locationBlock.getClntSize() != READ_BUFFER_SIZE)
		_clientMaxBodySize = locationBlock.getClntSize();
	if (locationBlock.getMethods().empty() == false)
		_response.initAllowedMethods(locationBlock.getMethods());

	std::cout << GREEN << "location block root: " << locationBlock.getRoot() << RESET << std::endl;
	if (locationBlock.getRoot() != ".")
	{
		this->_response._root = locationBlock.getRoot();
		size_t	uriPos = 0;
		if ((uriPos = this->_response.getPath().find(locationBlock.getURI())) != std::string::npos)
		{
			std::cout << CYAN << "location uri: " << locationBlock.getURI() << RESET << std::endl;
			this->_response.setPath(this->_response.getPath().substr(uriPos + locationBlock.getURI().length(),
				this->_response.getPath().length() - uriPos - locationBlock.getURI().length()));
			this->_response.setPath(this->_response.getRoot() + this->_response.getPath());
			std::cout << RED << "select location and path change : " << this->_response.getPath() << RESET << std::endl;
		}
	}
	if (locationBlock.getAutoindex() != DEFAULT_AUTOINDEX)
		_autoindex = locationBlock.getAutoindex();
	if (locationBlock.getIndex().empty() == false)
		_index = locationBlock.getIndex();
	if (locationBlock.getCGI() != "")
	{
		this->_configCgi = locationBlock.getCGI();
		std::cout << YELLOW << "cgi: " << this->_configCgi << RESET << std::endl;
		this->_cgi.setCgiExist(true);
		this->initCgiEnv();
	}
	
}

void			Server::eventRead (int fd)
{//fd에 맞게 실행된다.
//저장되어있는 request에 맞는 fd가 없다면 아무 행동도 하지 않는다.
	_requestEnd = 0;
	int	checkRequestLineRet = 0;
	if (fd == _serverSocket)
	// if (fd == static_cast<uintptr_t>(_serverSocket))
	{//read event인데 server socket일 때는 accept이라는 뜻이므로 requestAccept을 실행
		requestAccept();
	}
	else if (_request.find(fd) != _request.end())
	{//일단 1024만큼만 읽는다.
		// std::cout << "read start\n";
		char	buf[READ_BUFFER_SIZE] = {};
		int	n;
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
		std::cout << RED <<  "============request==========\n" << _request[fd] << RESET;
		if (_response._bodySize != 0
			&& _request[fd].length() - _bodyStartPos >= _response._bodySize)
		{//body size만큼 입력 받았을 때
			_requestEnd = 1;
			_request[fd] = _request[fd].substr(0, _bodyStartPos + _response._bodySize);
		}
		
		if (_request[fd] == "\r\n" && _checkedRequestLine == 0)
		{//아무 값도 없이 빈 칸만 왔다는 뜻
			_request[fd].clear();
			std::cout << "request is empty line\n";
			return ;
		}
		if (_request[fd].find("\r\n") != std::string::npos &&
			_checkedRequestLine == 0)
		{
			_checkedRequestLine = 1;
			std::cout << "check request line\n";
			checkRequestLineRet = _response.checkRequestLine(_request[fd]);
			if (checkRequestLineRet == 9)
			{
				std::cout << "request's http version is HTTP/0.9\n";
				_requestEnd = 1;
			}
			else if (checkRequestLineRet == 1)
			{
				printErr("check request line error");
				_response._httpVersion = "HTTP/1.1";
				_response._contentLocation = "/";
				_response.setCode(Bad_Request);
				return ;
			}
			//checkRequestLine으로 parsing한 path를 통해서 select location block으로 적절한 location block을 찾는다.
			//path이 root랑 다를 때 Location block의 변수들을 Server로 넘겨준다.
			//만약 location을 못 찾았다면 그냥 server block의 변수를 사용하면 된다.
			if (this->_response._path != this->_response._root && this->_response._path != "/")
			{
				std::cout << PINK << "@@@@@@@@@@@@@select location@@@@@@@@@@@@" << RESET << std::endl;
				std::cout << "response root: " << _response._root << ", response path: " << _response._path << std::endl;
				LocationBlock	test = this->selectLocationBlock(this->_response._path);
				if (test.getIsEmpty() == false)
				{//값을 가지고 있는 location block을 찾았을 경우에 이 location block의 변수들을 모두 넣어주면 된다.
					this->_response.setPath(test.getPath());
					this->locationToServer(test);
				}
				else
				{
					std::cout << CYAN << "path: " << this->_response.getPath() << std::endl;
					std::cout << "path last: " << this->_response.getPath().at(this->_response.getPath().length() - 1) << std::endl;
					if (this->_response.getPath().at(this->_response.getPath().length() - 1) == '/')
						this->_response.setPath(this->_response.getPath().substr(0, this->_response.getPath().length() - 1));
					if (this->_response.getPath().length() != 0 && this->_response.getPath().at(0) == '/')
						this->_response.setPath(this->_response.getRoot() + this->_response.getPath());
					else if (this->_response.getPath().length() != 0)
						this->_response.setPath(this->_response.getRoot() + "/" + this->_response.getPath());
					else
						this->_response.setPath(this->_response.getRoot() + this->_response.getPath());
					std::cout << "path: " << this->_response.getPath() << RESET << std::endl;
				}
			}
			else if ((this->_response.getPath() == "/" && this->_response._method == "GET") ||
				this->_response.getPath() != "/")
			{
				std::cout << RED << "path: " << this->_response.getPath() << std::endl;
				std::cout << "path last: " << this->_response.getPath().at(this->_response.getPath().length() - 1) << std::endl;
				if (this->_response.getPath().at(this->_response.getPath().length() - 1) == '/')
					this->_response.setPath(this->_response.getPath().substr(0, this->_response.getPath().length() - 1));
				if (this->_response.getPath().length() != 0 && this->_response.getPath().at(0) == '/')
					this->_response.setPath(this->_response.getRoot() + this->_response.getPath());
				else if (this->_response.getPath().length() != 0)
					this->_response.setPath(this->_response.getRoot() + "/" + this->_response.getPath());
				else
					this->_response.setPath(this->_response.getRoot() + this->_response.getPath());
				std::cout << "path: " << this->_response.getPath() << RESET << std::endl;
			}
		}

		if ((strncmp(this->_request[fd].c_str(), "POST", 4) == 0 ||
			strncmp(this->_request[fd].c_str(), "PUT", 3) == 0) && this->_bodyCondition == No_Body)
			this->_bodyCondition = Body_Exist;

		size_t	rnrnPos;
		if (this->_request[fd].empty() == false && (rnrnPos = this->_request[fd].find("\r\n\r\n")) != std::string::npos)
		{
			if (this->_bodyCondition == No_Body)
			{
				this->_requestEnd = 1;
				this->_bodyCondition = Body_End;
			}
			else if (this->_bodyCondition == Body_Exist)
			{
				this->_bodyCondition = Body_Start;
				this->_bodyStartPos = rnrnPos + 4;
			}
		}

		//check header
		if ((_bodyCondition == Body_Start || _bodyCondition == Body_End)
			&& _bodyEnd == 0)
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

		if (this->_response._bodySize != 0 && this->_response._contentLength != ""
			&& this->_request[fd].length() - this->_bodyStartPos >= this->_response._bodySize)
		{//body size만큼 입력 받았을 때
			this->_requestEnd = 1;
			this->_request[fd] = this->_request[fd].substr(0, this->_bodyStartPos + this->_response._bodySize);
		}
		if (this->_response._transferEncoding == "chunked")
		{//contentLength가 없고, transferEncoding이 chunked일 때 파일의 끝을 알리는 것이 들어올 때까지 계속 recv
			if (this->_response._path != "/")
			{
				if (_rnPos <= this->_bodyStartPos)
					_rnPos = this->_bodyStartPos;
				else
					_rnPos = this->_bodyVecStartPos;
				size_t	bodySize = this->_request[fd].find("\r\n", _rnPos + 3);
				std::cout << "rn pos: " << this->_rnPos << ", body start pos: " << this->_bodyStartPos << std::endl;
				// std::cout << "!!!!!!!!request!!!!!!! << this->_request[fd].substr(0, _rnPos) << std::endl;
				if (bodySize != std::string::npos && this->_bodyVecSize == 0)
				{
					std::string	bodySizeStr = this->_request[fd].substr(_rnPos, bodySize - _rnPos);
					if (bodySizeStr.find("e") != std::string::npos)
						this->_bodyVecSize = calExponent(bodySizeStr);
					else
						this->_bodyVecSize = std::atoi(bodySizeStr.c_str());
					this->_bodyVecStartPos = bodySize + 2;
					_rnPos = _bodyVecStartPos = this->_bodyVecSize;
					std::cout << GREEN << "body vec size: " << this->_bodyVecSize << std::endl;
					// std::cout << "bodySizeStr: " << bodySizeStr << RESET << std::endl;
					if (this->_bodyVecSize == 0)
						this->_response.setCode(Bad_Request);
				}
			}
			if (this->_request[fd].length() > this->_bodyVecStartPos + this->_bodyVecSize &&
				this->_bodyVecStartPos != 0)
			{
				std::string	_bodyElement = this->_request[fd].substr(this->_bodyVecStartPos,
					this->_bodyVecSize);
				this->_response._bodyVec.push_back(_bodyElement);
				this->_bodyVecStartPos += this->_bodyVecSize;
				this->_bodyVecSize = 0;
			}
			std::cout << RED << "body vec start pos: " << this->_bodyVecStartPos;
			std::cout << ", body vec size: " << this->_bodyVecSize;
			std::cout << "request length: " << this->_request[fd].length() << RESET << std::endl;
			usleep(100);

			if (this->_request[fd].find("0\r\n\r\n") != std::string::npos &&
				(this->_bodyVecStartPos == 0 ||
				(this->_bodyVecStartPos != 0 && this->_bodyVecSize > this->_request[fd].length() - this->_bodyVecStartPos)))
			{
				this->_request[fd] = this->_request[fd].substr(0, this->_request[fd].length() - 5);
				if (this->_bodyVecStartPos != 0)
				{
					std::string	_bodyElement = this->_request[fd].substr(this->_bodyVecStartPos, this->_request[fd].length() - this->_bodyVecStartPos);
					this->_response._bodyVec.push_back(_bodyElement);
				}
				std::cout << "receive file end\n";
				this->_requestEnd = 1;
			}
			
			else
				return ;
			
		}

		if (n <= 0)
		{//read가 에러가 났거나, request가 0을 보내면 request와 연결을 끊는다.
			if (n < 0)
			{
				printErr("request read error");
				_response.setCode(Internal_Server_Error);
			}
			else
				disconnectRequest(fd);
		}
		else if (_requestEnd == 1)
		{
			if (this->_bodyStartPos != 0 && this->_bodyStartPos < this->_request[fd].length())
			{
				this->_response._body = this->_request[fd].substr(this->_bodyStartPos,
					this->_request[fd].length() - this->_bodyStartPos);
				if (this->_response._transferEncoding == "chunked" && this->_response._bodyVec.empty() == false)
				{
					for (std::vector<std::string>::iterator it = _response._bodyVec.begin(); it != _response._bodyVec.end(); it++)
						this->_response._body += *it;
				}
				// std::cout << YELLOW << "=====BODY=====\n" << this->_response._body << RESET << std::endl;
			}
		}
	}
}

void			Server::eventWrite (int fd)
{
	if (_request.find(fd) != _request.end())
	{
		if (_response.verifyMethod(fd, &_response, _requestEnd) == 1)
			disconnectRequest(fd);
		if (this->_requestEnd == 1) 
			// this->_response.getErrorMap().find(this->_response.getCode()) != this->_response.getErrorMap().end())
		{
			
			std::cout << "verifyMethod, code :  " <<  _response._code << std::endl;
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
				this->_bodyVecSize = 0;
				this->_bodyVecStartPos = 0;
				this->_rnPos = 0;
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
