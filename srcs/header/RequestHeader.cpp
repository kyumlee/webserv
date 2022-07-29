# include "./../../includes/header/RequestHeader.hpp"

RequestHeader::RequestHeader () { }
RequestHeader::RequestHeader (const RequestHeader& rh)
{
	(void)rh;
}
RequestHeader::~RequestHeader () {}

RequestHeader&	RequestHeader::operator= (const RequestHeader& rh)
{ (void)rh; return (*this); }

int				RequestHeader::checkRequestLine (std::string requestLine)
{
	std::string							requestLineDeleteRN = strDeleteRN(requestLine);
	std::vector<std::string>			requestLineVec = split(requestLineDeleteRN, ' ');
	std::vector<std::string>::iterator	requestLineIt = requestLineVec.begin();
	
	if (requestLineVec.size() <= 1 || requestLineVec.size() > 3)
		return (printErr("request line's size is: " + intToStr(requestLineVec.size())));

	this->_method = *requestLineIt++;
	
	if (isStrUpper(this->_method) == 0)
		return (1);

	this->_path = *requestLineIt++;

	//path이 /이고 GET일 때만 작동
/*	if (this->_path == "/" && this->_method == "GET")
	{
		this->_path.clear();
		this->_path = this->_root;
	}
	else if (this->_path != "/")
	{
		if (this->_path.at(0) != '/')
			this->_path = "/" + this->_path;
		if (this->_path.find(this->_root) == std::string::npos)
			this->_path = this->_root + this->_path;
	}*/
	if (this->_path != "/" && this->_path.at(0) == '/')
		this->_path = this->_path.substr(1, this->_path.length() - 1);

	//GET method이고, request_line이 2개의 단어로 이루어져 있다면 HTTP/0.9버전이다.
	//일단 바로 종료하도록 9를 리턴하도록 하자
	if (this->_method == "GET" && requestLineVec.size() == 2)
	{
		this->_httpVersion = "HTTP/0.9";
		this->setConnection("close");
		return (9);
	}
	//GET method가 아닌데 2개의 단어로 이루어져 있다면 에러
	else if (requestLineVec.size() == 2)
		return (printErr("request line size is two but method is not get"));

	this->_httpVersion = *requestLineIt;
	std::cout << "http version : " << this->_httpVersion << std::endl;
	if (this->_httpVersion == "HTTP/1.0")
		this->setConnection("close");
	if (this->_httpVersion != "HTTP/1.0" && this->_httpVersion != "HTTP/1.1")
		return (printErr("request line http version is not 1.0 and not 1.1"));

	return (0);
}

int				RequestHeader::checkHeader (std::vector<std::string> header)
{
	for (std::vector<std::string>::iterator	it = header.begin() + 1; it != header.end(); it++)
	{
		//Host의 값을 찾아서 값을 넣어준다.
		if (strncmp((*it).c_str(), "Host:", 5) == 0)
		{
			this->_host = findHeaderValue(*it);
			// http://는 자른다.
			if (_host.find("http://", 0) == 0)
				_host = _host.substr(7, _host.length() - 7);
			if (this->setListen(this->_host) == 1)
				return (1);
		}
		else if (strncmp((*it).c_str(), "User-Agent:", 11) == 0)
			this->_userAgent = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Accept:", 7) == 0)
			this->_accept = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Accept-Charset:", 15) == 0)
			this->_acceptCharset = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Accept-Language:", 16) == 0)
			this->_acceptLanguage = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Accept-Encoding:", 16) == 0)
			this->_acceptEncoding = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Origin:", 7) == 0)
			this->_origin = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Authorization:", 14) == 0)
			this->_authorization = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Length:", 15) == 0)
			this->_contentLength = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Type:", 13) == 0)
			this->_contentType = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Transfer-Encoding:", 18) == 0)
			this->_transferEncoding = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Language:", 17) == 0)
			this->_contentLanguage = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Location:", 17) == 0)
			this->_contentLocation = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Encoding:", 17) == 0)
			this->_contentEncoding = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Connection:", 11) == 0)
			this->_connection = findHeaderValue(*it);
		//이상한 header값일 때 :(콜론)과 ' '(공백)을 확인하여 에러처리
		//:(콜론)이 없는 데 ' '(공백)이 있다면 에러, 콜론이 없더라도 공백이 없으면 에러가 아님
		//공백은 무조건 콜론 뒤에서만 허용된다.
		else
		{
			if (checkHeaderAvailable(*it) == 1)
			{
				std::cout << "request header has strange value\n";
				return (1);
			}
		}
	}
	//contentLength가 있고, transfer_encoding은 없을 때
	if (this->_contentLength != "" && this->_transferEncoding == "")
	{
		if (isNumber(this->_contentLength) == 0)
		{
			this->_contentLength = "";
			return (printErr("Content-Length is not number"));
		}
		else
		{
			this->_bodySize = std::atoi(this->_contentLength.c_str());
		}
	}
	if (this->checkEssentialHeader() == 1)
		return (1);

	return (0);
}

int				RequestHeader::checkHeaderAvailable (const std::string& header)
{
	size_t	colonPos, spacePos;

	//공백이 없으면 정상 작동
	if ((spacePos = header.find_first_of(' ')) == std::string::npos)
		return (0);
	//공백이 있고 콜론이 없으면 에러
	else if ((colonPos = header.find_first_of(':')) == std::string::npos)
		return (1);
	//공백이 있고 콜론이 있을 때 공백이 콜론보다 앞에 있으면 에러
	else if (spacePos < colonPos)
		return (1);
	//공백이 있고 콜론이 있을 때 공백이 콜론보다 뒤에 있으면 정상
	else
		return (0);
}

int				RequestHeader::checkEssentialHeader ()
{
	if (this->_httpVersion == "HTTP/0.9")
		return (0);
	std::cout << "host : " << this->_host << std::endl;
	if (this->_host == "")
		return (printErr("host header does not exist"));
	//Content-Type, Content-Length(or Transfer-Encoding)이 있어야 한다.
	if (this->_method == "PUT" || this->_method == "POST")
	{
		if (this->_contentLength != "" && this->_transferEncoding != "")
			return (printErr("both Content-Length and Transfer-Encoding exist"));
		else if (this->_contentLength != "" || this->_transferEncoding != "")
			return (0);
		else
			return (printErr("either Content-Length or Transfer-Encoding header does not exist"));
	}
	return (0);
}

int				RequestHeader::splitRequest (std::string request, int bodyCondition)
{
	size_t								rPos = 0, start = 0;
	std::string							str, body = "";
	std::vector<std::string>			strHeader;
	std::map<std::string, std::string>	ret;

	while ((rPos = request.find('\n', start)) != std::string::npos)
	{//\r을 계속 찾아서 그것을 기준으로 vector에 넣어주자.
		if (request.at(start) == '\r')
		{
			if (start + 1 == rPos)
			{
				rPos += 1;
				start = rPos;
				break ;
			}
		}
		str = request.substr(start, rPos - start - 1);
		strHeader.push_back(str);
		rPos += 1;
		start = rPos;
	}
	if (strHeader.size() == 1 && bodyCondition == No_Body)
	{//요청이 한 줄만 왔을 때
		if (this->_httpVersion != "HTTP/0.9")
			return (printErr("request is one line but has no essential header"));
		return (0);
	}
	if (this->_httpVersion == "HTTP/0.9")
		return (printErr("http version is HTTP/0.9 but it has header"));
	if (this->_httpVersion == "HTTP/1.1")
		this->setConnection();
	else
		this->setConnection("close");
	if (checkHeader(strHeader) == 1)
		return (1);
	return (0);
}

bool			RequestHeader::hostToInt(std::string host)
{//string형인 host를 사용할 수 있도록 unsigned int형으로 바꿔준다.
	size_t			sep = 0, start = 0;
	unsigned int	n, ret = 0;
	std::string		substr;

	if (host == "localhost")
		host = "127.0.0.1";
	//host가 그냥 숫자로 되어있을 떄
	if (isNumber(host) == 1)
	{
		ret = std::atoi(host.c_str());
		this->_listen.host = ret;
		return (0);
	}

	std::cout << 3 << std::endl;
	for (int i = 3; i > -1; i--)
	{
		sep = host.find_first_of('.', sep);

		if (i != 0 && sep == std::string::npos)
			return (printErr("invalid host address (missing .)"));

		if (i == 0)
			sep = host.length();

		substr = host.substr(start, sep - start);
		if (isNumber(substr) == 0)
			return (printErr("invalid host address (not number)"));

		n = std::atoi(substr.c_str());
		for (int j = 0; j < i; j++)
			n *= 256;
		ret += n;
		sep++; start = sep;
	}

	this->_listen.host = ret;
	return (0);
}

int				RequestHeader::setListen (const std::string& strHost)
{//에러가 발생하면 1을 리턴, 정상작동하면 0을 리턴
	if (strHost == "")
		return (printErr("host header does not exist"));

	std::vector<std::string>	hostPort;
	unsigned int				host;
	int							port;

	//포트는 생략할 수 있다. HTTP URL에서는 port default가 80이다.
	//일단 8000으로 default port를 하자
	hostPort = split(strHost, ':');
	if (*hostPort.begin() == strHost)
	{
		this->_listen.port = htons(DEFAULT_PORT);
		if ((host = hostToInt(strHost)) == 1)
		//strHost가 이상한 값을 가지고 있을 때
			return (printErr("host has strange value"));
		this->_listen.host = htonl(host);
		return (0);
	}

	if (isNumber(*(hostPort.begin() + 1)) == 0)
		return (printErr("port is not number"));

	port = std::atoi((*(hostPort.begin() + 1)).c_str());
	this->_listen.port = htons(port);

	if ((host = hostToInt(*hostPort.begin())) == 1)
		return (1);

	this->_listen.host = htonl(host);
	return (0);
}

void			RequestHeader::setHost (const std::string& host) { this->_host = host; }
void			RequestHeader::setUserAgent (const std::string& userAgent) { this->_userAgent = userAgent; }
void			RequestHeader::setAccept (const std::string& accept) { this->_accept = accept; }
void			RequestHeader::setAcceptCharset (const std::string& charset) { this->_acceptCharset = charset; }
void			RequestHeader::setAcceptLanguage (const std::string& lang) { this->_acceptLanguage = lang; }
void			RequestHeader::setOrigin (const std::string& origin) { this->_origin = origin; }
void			RequestHeader::setAuthorization (const std::string& authorization) { this->_authorization = authorization; }
void			RequestHeader::setMethod (const std::string& method) { this->_method = method; }
void			RequestHeader::setPath (const std::string& path) { this->_path = path; }
void			RequestHeader::setHttpVersion (const std::string& httpVersion) { this->_httpVersion = httpVersion; }
void			RequestHeader::setBody (const std::string& body) { this->_body = body; }
void			RequestHeader::setBodySize (const size_t& bodySize) { this->_bodySize = bodySize; }
void			RequestHeader::setRoot (const std::string& root) { this->_root = root; }

t_listen		RequestHeader::getListen () const { return (this->_listen); }
std::string		RequestHeader::getHost () const { return (this->_host); }
std::string		RequestHeader::getUserAgent () const { return (this->_userAgent); }
std::string		RequestHeader::getAccept () const { return (this->_accept); }
std::string		RequestHeader::getAcceptCharset () const { return (this->_acceptCharset); }
std::string		RequestHeader::getAcceptLanguage () const { return (this->_acceptLanguage); }
std::string		RequestHeader::getAcceptEncoding () const { return (this->_acceptEncoding); }
std::string		RequestHeader::getOrigin () const { return (this->_origin); }
std::string		RequestHeader::getAuthorization() const { return (this->_authorization); }
std::string		RequestHeader::getMethod () const { return (this->_method); }
std::string		RequestHeader::getPath () const { return (this->_path); }
std::string		RequestHeader::getHttpVersion () const { return (this->_httpVersion); }
std::string		RequestHeader::getBody () const { return (this->_body); }
size_t			RequestHeader::getBodySize () const { return (this->_bodySize); }
std::string		RequestHeader::getRoot () const { return (this->_root); }

void			RequestHeader::printRequestHeader() const
{
	std::cout << "listen host: " << this->getListen().host << ", port: " << this->getListen().port << std::endl;
	std::cout << "server host: " << this->_host << std::endl;
	std::cout << "user agent: " << this->_userAgent << std::endl;
	std::cout << "accept: " << this->_accept << std::endl;
	std::cout << "accept charset: " << this->_acceptCharset << std::endl;
	std::cout << "accept language: " << this->_acceptLanguage << std::endl;
	std::cout << "origin: " << this->_origin << std::endl;
	std::cout << "authorization: " << this->_authorization << std::endl;
	std::cout << "method: " << this->_method << std::endl;
	std::cout << "path: " << this->_path << std::endl;
	std::cout << "http version: " << this->_httpVersion << std::endl;
	std::cout << "body: " << this->_body << std::endl;
	std::cout << "body size: " << this->_bodySize << std::endl;
	std::cout << "root: " << this->_root << std::endl;
}
