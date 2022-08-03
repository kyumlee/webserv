#include "./../includes/ResponseHeader.hpp"

ResponseHeader::ResponseHeader () {}
ResponseHeader::ResponseHeader (const ResponseHeader& rh) { (void)rh; }
ResponseHeader::~ResponseHeader () {}

ResponseHeader&				ResponseHeader::operator= (const ResponseHeader& rh)
{
	(void)rh;
	return (*this);
}

void						ResponseHeader::initErrorHtml ()
{
	setErrorHtml(Bad_Request, "400.html");
	setErrorHtml(Forbidden, "403.html");
	setErrorHtml(Not_Found, "404.html");
	setErrorHtml(Method_Not_Allowed, "405.html");
	setErrorHtml(Payload_Too_Large, "413.html");
	setErrorHtml(Internal_Server_Error, "500.html");
	changeHtmlRelativePath();
}

void						ResponseHeader::setErrorHtml (int code, std::string html)
{ _errorHtml[code] = html; }

void						ResponseHeader::setErrorHtml (std::map<int, std::string> html)
{
	_errorHtml = html;
	changeHtmlRelativePath();
}

//errorHtml에 저장되어있는 파일이름을 상대경로로 바꿔준다.
void						ResponseHeader::changeHtmlRelativePath ()
{
	for (std::map<int, std::string>::iterator it = _errorHtml.begin(); it != _errorHtml.end(); it++)
	{
		if (it->second == "400.html")
			it->second = BAD_REQUEST_HTML;
		else if (it->second == "403.html")
			it->second = FORBIDDEN_HTML;
		else if (it->second == "404.html")
			it->second = NOT_FOUND_HTML;
		else if (it->second == "405.html")
			it->second = NOT_ALLOWED_HTML;
		else if (it->second == "413.html")
			it->second = PAYLOAD_TOO_LARGE_HTML;
		else if (it->second == "500.html")
			it->second = INTERNAL_SERVER_ERROR_HTML;
		else
			it->second = DEFAULT_HTML;
	}
}

//errormap은 config파일에서 읽은 그대로 사용해야 한다.
//일단 내 맘대로 초기화했다.
void						ResponseHeader::initErrorMap ()
{
	_errorMap[Continue] = "Continue";
	_errorMap[OK] = "OK";
	_errorMap[Created] = "Created";
	_errorMap[No_Content] = "No Content";
	_errorMap[Bad_Request] = "Bad Request";
	_errorMap[Forbidden] = "Forbidden";
	_errorMap[Not_Found] = "Not Found";
	_errorMap[Method_Not_Allowed] = "Not Allowed";
	_errorMap[Payload_Too_Large] = "Payload Too Large";
	_errorMap[Internal_Server_Error] = "Internal Server Error";
}

std::string					ResponseHeader::getHeader ()
{
	std::string	header;

	setHeader();

	header = _httpVersion + " " + intToStr(_code) + " " + getStatusMessage(_code) + "\r\n";
	header += writeHeader(); 

	return (header);
}

std::string					ResponseHeader::writeHeader ()
{
	std::string	header = "";
	if (_allow != "")
		header += "Allow: " + _allow + "\r\n";
	
	//request header와 겹침
	if (_contentLanguage != "")
		header += "Content-Language: " + _contentLanguage + "\r\n";
	if (_contentLength != "" && _transferEncoding != "chunked")
		header += "Content-Length: " + _contentLength + "\r\n";
	if (_transferEncoding == "chunked")
		header += "Content-Length: 0\r\n";
	if (_contentLocation != "")
		header += "Content-Location: " + _contentLocation + "\r\n";
	if (_contentType != "")
		header += "Content-Type: " + _contentType + "\r\n";
	
	if (_date != "")
		header += "Date: " + _date + "\r\n";
	if (_lastModified != "")
		header += "Last-Modified: " + _lastModified + "\r\n";
	if (_location != "")
		header += "Location: " + _location + "\r\n";
	if (_retryAfter != "")
		header += "Retry-After: " + _retryAfter + "\r\n";
	if (_server != "")
		header += "Server: " + _server + "\r\n";

	//request header와 겹치는 것
	if (_transferEncoding != "" && _transferEncoding != "chunked")
		header += "Transfer-Encoding: " + _transferEncoding + "\r\n";
	if (_transferEncoding == "chunked")
		header += "Transfer-Encoding: identity\r\n";
	//
	
	if (_wwwAuthenticate != "")
		header += "WWW-Authenticate: " + _wwwAuthenticate + "\r\n";
	header += "\r\n";

	return (header);
}

std::string					ResponseHeader::getStatusMessage (int code)
{
	if (_errorMap.find(code) != _errorMap.end())
		return (_errorMap[code]);

	return ("error code not found");
}

void						ResponseHeader::initRequest ()
{
	//general header reset
	_date = "";
	_connection = "";
	_transferEncoding = "";

	//Entity header reset
	_contentLength = "";
	_contentType = "";
	_contentLanguage = "";
	_contentLocation = "";
	_contentEncoding = "";
	_allow = "";

	//request header reset
	_listen.host = 0;
	_listen.port = 0;
	_host = "";
	_userAgent = "";
	_accept = "";
	_acceptCharset = "";
	_acceptLanguage = "";
	_acceptEncoding = "";
	_origin = "";
	_authorization = "";
	_method = "";
	_path = "";
	_httpVersion = "";
	_body = "";
	_code = 0;

	//response header reset
	_wwwAuthenticate = "";
	_retryAfter = "";
	_lastModified = "";
	_location = "";
	_bodySize = 0;
}

void						ResponseHeader::resetRequest ()
{
	//response header reset
	initRequest();
	_server = "";
}

void						ResponseHeader::setHeader ()
{//response하는데 필요한 헤더들을 세팅한다.
	//general header value
	setDate();
	setConnection(_connection);
	setTransferEncoding(_transferEncoding);

	//entity header
	setContentLength(_path, _contentLength);
	setContentTypeLocation(_path, _contentType, _contentLocation);
	setContentLanguage(_contentLanguage);
	setContentEncoding(_contentEncoding);
	// setAllow(config);
	//config파일을 파싱한 것을 인자로 받아서 보내주어 세팅하도록 한다.

	//request header는 request를 파싱할 때 모두 세팅되어있으므로 필요 없다.
	
	//response header
	setServer(_server);
	setWwwAuthenticate(_code);
	setRetryAfter(_code, DEFAULT_RETRY_AFTER);
	setLastModified(_path);
	setLocation(_code, _path);
}

void						ResponseHeader::setServer (const std::string& server)
{
	if (server == "")
		_server = "Webserv/1.0 (Unix)";
	else
		_server = server;
}
void						ResponseHeader::setWwwAuthenticate (int code)
{
	if (code == Unauthorized)
		_wwwAuthenticate = "Basic realm=\"Access requires authentification\", charset=\"UTF-8\"";
}
void						ResponseHeader::setRetryAfter (int code, int sec)
{
	if (code == Service_Unavailable || code == Too_Many_Requests || code == Moved_Permanently)
		_retryAfter = intToStr(sec);
}

void						ResponseHeader::setLastModified (const std::string& path)
{
	char		buf[100];
	struct stat	fileStat;
	struct tm*	tm;

	if (stat(path.c_str(), &fileStat) == 0)
	{
		tm = gmtime(&fileStat.st_mtime);
		strftime(buf, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
		std::string	lastModified(buf);
		_lastModified = lastModified;
	}
}

void						ResponseHeader::setLocation (int code, const std::string& path)
{
	if (code == Created || code / 100 == 3)
		_location = path;
}

std::string					ResponseHeader::getServer () const { return (_server); }
std::string 				ResponseHeader::getRetryAfter () const { return (_retryAfter); }
std::string					ResponseHeader::getLastModified () const { return (_lastModified); }
std::string					ResponseHeader::getLocation () const { return (_location); }
std::map<int, std::string>	ResponseHeader::getErrorMap () const { return (_errorMap); }
std::map<int, std::string>	ResponseHeader::getErrorHtml () const { return (_errorHtml); }

void						ResponseHeader::printResponseHeader ()
{
	std::cout << "server name: " << getServer() << std::endl;
	std::cout << "retry after: " << getRetryAfter() << std::endl;
	std::cout << "last modified: " << getLastModified() << std::endl;
	std::cout << "location: " << getLocation() << std::endl;
	std::cout << "error map: ";
	printErrmap(getErrorMap());
	std::cout << "error html: ";
	printErrmap(getErrorHtml());
}
