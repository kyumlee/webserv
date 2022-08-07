#include "./../includes/RequestHeader.hpp"

RequestHeader::RequestHeader()
	: _listen(),
	_host(),
	_userAgent(),
	_accept(),
	_acceptCharset(),
	_acceptLanguage(),
	_acceptEncoding(),
	_origin(),
	_authorization(),
	_method(),
	_path(),
	_httpVersion(),
	_body(),
	_bodySize(),
	_root(),
	_bodyVec(),
	_xHeader()
{}
RequestHeader::RequestHeader(const RequestHeader& rh)
	: _listen(rh._listen),
	_host(rh._host),
	_userAgent(rh._userAgent),
	_accept(rh._accept),
	_acceptCharset(rh._acceptCharset),
	_acceptLanguage(rh._acceptLanguage),
	_acceptEncoding(rh._acceptEncoding),
	_origin(rh._origin),
	_authorization(rh._authorization),
	_method(rh._method),
	_path(rh._path),
	_httpVersion(rh._httpVersion),
	_body(rh._body),
	_bodySize(rh._bodySize),
	_root(rh._root),
	_bodyVec(rh._bodyVec),
	_xHeader(rh._xHeader)
{}

RequestHeader::~RequestHeader() {}

RequestHeader&				RequestHeader::operator=(const RequestHeader& rh)
{
	_listen = rh._listen;
	_host = rh._host;
	_userAgent = rh._userAgent;
	_accept = rh._accept;
	_acceptCharset = rh._acceptCharset;
	_acceptLanguage = rh._acceptLanguage;
	_acceptEncoding = rh._acceptEncoding;
	_origin = rh._origin;
	_authorization = rh._authorization;
	_method = rh._method;
	_path = rh._path;
	_httpVersion = rh._httpVersion;
	_body = rh._body;
	_bodySize = rh._bodySize;
	_root = rh._root;
	_bodyVec = rh._bodyVec;
	_xHeader = rh._xHeader;
	return (*this);
}

int							RequestHeader::checkRequestLine(std::string requestLine)
{
	std::string							requestLineDeleteRN = strDeleteRN(requestLine);
	std::vector<std::string>			requestLineVec = split(requestLineDeleteRN, ' ');
	std::vector<std::string>::iterator	requestLineIt = requestLineVec.begin();

	if (requestLineVec.size() <= 1 || requestLineVec.size() > 3)
		return (printErr("request line size is " + intToStr(requestLineVec.size())));

	_method = *requestLineIt++;
	if (isStrUpper(_method) == 0)
		return (1);

	_path = *requestLineIt++;

	if (_path != "/" && _path.at(0) == '/')
		_path = _path.substr(1, _path.length() - 1);
	if (_path != "/" && _path.at(_path.length() - 1) == '/')
		_path = _path.substr(0, _path.length() - 1);
	if (_method == "GET" && requestLineVec.size() == 2)
	{
		_httpVersion = "HTTP/0.9";
		setConnection("close");
		return (9);
	}
	else if (requestLineVec.size() == 2)
		return (printErr("invalid request line"));

	_httpVersion = *requestLineIt;

	if (_httpVersion == "HTTP/1.0")
		setConnection("close");

	if (_httpVersion != "HTTP/1.0" && _httpVersion != "HTTP/1.1")
		return (printErr("invalid request line"));

	// std::cout << RED << "request line: " << _method << " " << _path << " " <<  _httpVersion << RESET << std::endl;

	return (0);
}

int							RequestHeader::checkHeader(std::vector<std::string> header)
{
	for (std::vector<std::string>::iterator	it = header.begin() + 1; it != header.end(); it++)
	{
		if (strncmp((*it).c_str(), "Host:", 5) == 0)
		{
			_host = findHeaderValue(*it);
			if (_host.find("http://", 0) != std::string::npos)
				_host = _host.substr(7, _host.length() - 7);

			if (setListen(_host, &_listen) == 1)
				return (1);
		}
		else if (strncmp((*it).c_str(), "User-Agent:", 11) == 0)
			_userAgent = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Accept:", 7) == 0)
			_accept = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Accept-Charset:", 15) == 0)
			_acceptCharset = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Accept-Language:", 16) == 0)
			_acceptLanguage = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Accept-Encoding:", 16) == 0)
			_acceptEncoding = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Origin:", 7) == 0)
			_origin = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Authorization:", 14) == 0)
			_authorization = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Length:", 15) == 0)
			_contentLength = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Type:", 13) == 0)
			_contentType = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Transfer-Encoding:", 18) == 0)
			_transferEncoding = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Language:", 17) == 0)
			_contentLanguage = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Location:", 17) == 0)
			_contentLocation = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Content-Encoding:", 17) == 0)
			_contentEncoding = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "Connection:", 11) == 0)
			_connection = findHeaderValue(*it);
		else if (strncmp((*it).c_str(), "X-Secret-Header-For-Test:", 25) == 0)
			_xHeader = findHeaderValue(*it);
	
		else
		{
			if (checkAvailableHeader(*it) == 1)
				return (printErr("invalid request header"));
		}
	}

	if (_contentLength != "" && _transferEncoding == "")
	{
		if (isNumber(_contentLength) == 0)
		{
			_contentLength = "";
			return (printErr("content-length is not a number"));
		}
		else
			_bodySize = std::atoi(_contentLength.c_str());
	}
	if (checkEssentialHeader() == 1)
		return (1);

	return (0);
}

int							RequestHeader::checkAvailableHeader(const std::string& header, char colon, char space)
{
	size_t	colonPos, spacePos;


	if ((spacePos = header.find_first_of(space)) == std::string::npos)
		return (0);
	else if ((colonPos = header.find_first_of(colon)) == std::string::npos)
		return (1);
	else if (spacePos < colonPos)
		return (1);
	else
		return (0);
}

int							RequestHeader::checkEssentialHeader()
{
	if (_httpVersion == "HTTP/0.9")
		return (0);
	if (_host == "")
		return (printErr("host doesn't exist"));

	if (_method == "PUT" || _method == "POST")
	{
		if (_contentLength != "" && _transferEncoding != "")
			return (printErr("both content-length and transfer-encoding exist"));
		else if (_contentLength != "" || _transferEncoding != "")
			return (0);
		else
			return (printErr("either content-length or transfer-encoding doesn't exist"));
	}

	return (0);
}

int							RequestHeader::splitRequest(std::string request, int bodyCondition)
{
	size_t								rPos = 0, start = 0;
	std::vector<std::string>			strHeader;
	std::string							str, body = "";
	std::map<std::string, std::string>	ret;

	while ((rPos = request.find('\n', start)) != std::string::npos)
	{
	
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
	{
		if (_httpVersion != "HTTP/0.9")
			return (printErr("missing essential header"));
		return (0);
	}
	if (_httpVersion == "HTTP/0.9")
		return (printErr("header exists (HTTP/0.9)"));

	if (_httpVersion == "HTTP/1.1")
		setConnection();
	else
		setConnection("close");

	if (checkHeader(strHeader) == 1)
		return (1);

	return (0);
}

t_listen					RequestHeader::getListen() const { return (_listen); }
std::string					RequestHeader::getHost() const { return (_host); }
std::string					RequestHeader::getUserAgent() const { return (_userAgent); }
std::string					RequestHeader::getAccept() const { return (_accept); }
std::string					RequestHeader::getAcceptCharset() const { return (_acceptCharset); }
std::string					RequestHeader::getAcceptLanguage() const { return (_acceptLanguage); }
std::string					RequestHeader::getAcceptEncoding() const { return (_acceptEncoding); }
std::string					RequestHeader::getOrigin() const { return (_origin); }
std::string					RequestHeader::getAuthorization() const { return (_authorization); }
std::string					RequestHeader::getMethod() const { return (_method); }
std::string					RequestHeader::getPath() const { return (_path); }
std::string					RequestHeader::getHttpVersion() const { return (_httpVersion); }
std::string					RequestHeader::getBody() const { return (_body); }
size_t						RequestHeader::getBodySize() const { return (_bodySize); }
std::string					RequestHeader::getRoot() const { return (_root); }
std::string					RequestHeader::getXHeader() const { return (_xHeader); }
std::vector<std::string>	RequestHeader::getBodyVec() { return (_bodyVec); }

void						RequestHeader::setHost(const std::string& host) { _host = host; }
void						RequestHeader::setUserAgent(const std::string& userAgent) { _userAgent = userAgent; }
void						RequestHeader::setAccept(const std::string& accept) { _accept = accept; }
void						RequestHeader::setAcceptCharset(const std::string& charset) { _acceptCharset = charset; }
void						RequestHeader::setAcceptLanguage(const std::string& lang) { _acceptLanguage = lang; }
void						RequestHeader::setOrigin(const std::string& origin) { _origin = origin; }
void						RequestHeader::setAuthorization(const std::string& authorization) { _authorization = authorization; }
void						RequestHeader::setMethod(const std::string& method) { _method = method; }
void						RequestHeader::setPath(const std::string& path) { _path = path; }
void						RequestHeader::setHttpVersion(const std::string& httpVersion) { _httpVersion = httpVersion; }
void						RequestHeader::setBody(const std::string& body) { _body = body; }
void						RequestHeader::setBodySize(const size_t& bodySize) { _bodySize = bodySize; }
void						RequestHeader::setRoot(const std::string& root) { _root = root; }

void						RequestHeader::addBodyVec(const std::string& body)
{ _bodyVec.push_back(body); }
void						RequestHeader::addBody(const std::string& body)
{ _body += body; }
void						RequestHeader::resetBodyVec() { _bodyVec.clear(); }

void						RequestHeader::printRequestHeader()
{
	std::cout << "listen host: " << getListen().host << ", port: " << getListen().port << std::endl;
	std::cout << "server host: " << _host << std::endl;
	std::cout << "user agent: " << _userAgent << std::endl;
	std::cout << "accept: " << _accept << std::endl;
	std::cout << "accept charset: " << _acceptCharset << std::endl;
	std::cout << "accept language: " << _acceptLanguage << std::endl;
	std::cout << "origin: " << _origin << std::endl;
	std::cout << "authorization: " << _authorization << std::endl;
	std::cout << "method: " << _method << std::endl;
	std::cout << "path: " << _path << std::endl;
	std::cout << "http version: " << _httpVersion << std::endl;
	std::cout << "body: " << _body << std::endl;
	std::cout << "body size: " << _bodySize << std::endl;
	std::cout << "root: " << _root << std::endl;
}
