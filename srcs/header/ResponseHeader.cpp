# include "./../../includes/header/ResponseHeader.hpp"

ResponseHeader::ResponseHeader () {}
ResponseHeader::ResponseHeader (const ResponseHeader& rh) { (void)rh; }
ResponseHeader::~ResponseHeader () {}

ResponseHeader&				ResponseHeader::operator= (const ResponseHeader& rh) { (void)rh; return (*this); }

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

//code와 html의 이름을 받아서 errorHtml에 저장
void						ResponseHeader::setErrorHtml (int code, std::string html)
{ this->_errorHtml[code] = html; }

void						ResponseHeader::setErrorHtml (std::map<int, std::string> html)
{
	this->_errorHtml = html;
	changeHtmlRelativePath();
}

//errorHtml에 저장되어있는 파일이름을 상대경로로 바꿔준다.
void						ResponseHeader::changeHtmlRelativePath ()
{
	for (std::map<int, std::string>::iterator it = this->_errorHtml.begin(); it != this->_errorHtml.end(); it++)
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
	this->_errorMap[Continue] = "Continue";
	this->_errorMap[OK] = "OK";
	this->_errorMap[Created] = "Created";
	this->_errorMap[No_Content] = "No Content";
	this->_errorMap[Bad_Request] = "Bad Request";
	this->_errorMap[Forbidden] = "Forbidden";
	this->_errorMap[Not_Found] = "Not Found";
	this->_errorMap[Method_Not_Allowed] = "Not Allowed";
	this->_errorMap[Payload_Too_Large] = "Payload Too Large";
	this->_errorMap[Internal_Server_Error] = "Internal Server Error";
}

std::string					ResponseHeader::getHeader ()
{
	std::string	header;

	setHeader();
	header = this->_httpVersion + " " + intToStr(this->_code) + " " + getStatusMessage(this->_code) + "\r\n";
	header += writeHeader(); 
	return (header);
}

std::string					ResponseHeader::writeHeader ()
{
	std::string	header = "";
	if (this->_allow != "")
		header += "Allow: " + this->_allow + "\r\n";
	
	//request header와 겹침
	if (this->_contentLanguage != "")
		header += "Content-Language: " + this->_contentLanguage + "\r\n";
	if (this->_contentLength != "" && this->_transferEncoding != "chunked")
		header += "Content-Length: " + this->_contentLength + "\r\n";
	if (this->_transferEncoding == "chunked")
		header += "Content-Length: 0\r\n";
	if (this->_contentLocation != "")
		header += "Content-Location: " + this->_contentLocation + "\r\n";
	if (this->_contentType != "")
		header += "Content-Type: " + this->_contentType + "\r\n";
	
	if (this->_date != "")
		header += "Date: " + this->_date + "\r\n";
	if (this->_lastModified != "")
		header += "Last-Modified: " + this->_lastModified + "\r\n";
	if (this->_location != "")
		header += "Location: " + this->_location + "\r\n";
	if (this->_retryAfter != "")
		header += "Retry-After: " + this->_retryAfter + "\r\n";
	if (this->_server != "")
		header += "Server: " + this->_server + "\r\n";

	//request header와 겹치는 것
	if (this->_transferEncoding != "" && this->_transferEncoding != "chunked")
		header += "Transfer-Encoding: " + this->_transferEncoding + "\r\n";
	if (this->_transferEncoding == "chunked")
		header += "Transfer-Encoding: identity\r\n";
	
	if (this->_wwwAuthenticate != "")
		header += "WWW-Authenticate: " + this->_wwwAuthenticate + "\r\n";
	header += "\r\n";
	return (header);
}

std::string					ResponseHeader::getStatusMessage (int code)
{
	if (this->_errorMap.find(code) != this->_errorMap.end())
		return (this->_errorMap[code]);
	return ("There is no error code");
}

void						ResponseHeader::initRequest ()
{
	//general header reset
	this->_date = "";
	this->_connection = "";
	this->_transferEncoding = "";

	//Entity header reset
	this->_contentLength = "";
	this->_contentType = "";
	this->_contentLanguage = "";
	this->_contentLocation = "";
	this->_contentEncoding = "";
	this->_allow = "";

	//request header reset
	this->_listen.host = 0;
	this->_listen.port = 0;
	this->_host = "";
	this->_userAgent = "";
	this->_accept = "";
	this->_acceptCharset = "";
	this->_acceptLanguage = "";
	this->_acceptEncoding = "";
	this->_origin = "";
	this->_authorization = "";
	this->_method = "";
	this->_path = "";
	this->_httpVersion = "";
	this->_body = "";
	this->_code = 0;

	//response header reset
	this->_wwwAuthenticate = "";
	this->_retryAfter = "";
	this->_lastModified = "";
	this->_location = "";
	this->_bodySize = 0;
}

void						ResponseHeader::resetRequest ()
{
	//response header reset
	this->initRequest();
	this->_server = "";
}

//response하는데 필요한 헤더들을 세팅한다.
//general header value
void						ResponseHeader::setHeader ()
{
	this->setDate();
	this->setConnection(this->_connection);
	this->setTransferEncoding(this->_transferEncoding);

	//entity header
	this->setContentLength(this->_path, this->_contentLength);
	this->setContentTypeLocation(this->_path, this->_contentType, this->_contentLocation);
	this->setContentLanguage(this->_contentLanguage);
	this->setContentEncoding(this->_contentEncoding);
	// this->setAllow(config);
	//config파일을 파싱한 것을 인자로 받아서 보내주어 세팅하도록 한다.

	//request header는 request를 파싱할 때 모두 세팅되어있으므로 필요 없다.
	
	//response header
	this->setServer(this->_server);
	this->setWwwAuthenticate(this->_code);
	this->setRetryAfter(this->_code, DEFAULT_RETRY_AFTER);
	this->setLastModified(this->_path);
	this->setLocation(this->_code, this->_path);
}

void						ResponseHeader::setServer (const std::string& server)
{
	if (server == "")
		this->_server = "Webserv/1.0 (Unix)";
	else
		this->_server = server;
}
void						ResponseHeader::setWwwAuthenticate (int code)
{
	if (code == Unauthorized)
		this->_wwwAuthenticate = "Basic realm=\"Access requires authentification\", charset=\"UTF-8\"";
}
void						ResponseHeader::setRetryAfter(int code, int sec)
{
	if (code == Service_Unavailable || code == Too_Many_Requests || code == Moved_Permanently)
		this->_retryAfter = intToStr(sec);
}

void						ResponseHeader::setLastModified (const std::string& path)
{
	char		buf[100];
	struct stat	file_stat;
	struct tm*	tm;

	if (stat(path.c_str(), &file_stat) == 0)
	{
		tm = gmtime(&file_stat.st_mtime);
		strftime(buf, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
		std::string	lastModified(buf);
		this->_lastModified = lastModified;
	}
}

void						ResponseHeader::setLocation (int code, const std::string& path)
{
	if (code == Created || code / 100 == 3)
		this->_location = path;
}

std::string					ResponseHeader::getServer () const { return (this->_server); }
std::string 				ResponseHeader::getRetryAfter () const { return (this->_retryAfter); }
std::string					ResponseHeader::getLastModified () const { return (this->_lastModified); }
std::string					ResponseHeader::getLocation () const { return (this->_location); }
std::map<int, std::string>	ResponseHeader::getErrorMap () const { return (this->_errorMap); }
std::map<int, std::string>	ResponseHeader::getErrorHtml () const { return (this->_errorHtml); }

void						ResponseHeader::printResponseHeader ()
{
	std::cout << "server name: " << this->getServer() << std::endl;
	std::cout << "retry after: " << this->getRetryAfter() << std::endl;
	std::cout << "last modified: " << this->getLastModified() << std::endl;
	std::cout << "location: " << this->getLocation() << std::endl;
	std::cout << "error map: ";
	printErrmap(this->getErrorMap());
	std::cout << "error html: ";
	printErrmap(this->getErrorHtml());
}
