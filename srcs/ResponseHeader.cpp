#include "./../includes/ResponseHeader.hpp"

ResponseHeader::ResponseHeader()
	: _server(),
	_wwwAuthenticate(),
	_retryAfter(),
	_lastModified(),
	_location(),
	_errorMap(),
	_errorHtml()
{}

ResponseHeader::ResponseHeader(const ResponseHeader& rh)
	: _server(rh._server),
	_wwwAuthenticate(rh._wwwAuthenticate),
	_retryAfter(rh._retryAfter),
	_lastModified(rh._lastModified),
	_location(rh._location),
	_errorMap(rh._errorMap),
	_errorHtml(rh._errorHtml)
{}

ResponseHeader::~ResponseHeader() {}

ResponseHeader&				ResponseHeader::operator=(const ResponseHeader& rh)
{
	_server = rh._server;
	_wwwAuthenticate = rh._wwwAuthenticate;
	_retryAfter = rh._retryAfter;
	_lastModified = rh._lastModified;
	_location = rh._location;
	_errorMap = rh._errorMap;
	_errorHtml = rh._errorHtml;
	return (*this);
}

std::string					ResponseHeader::getServer() const { return (_server); }
std::string 				ResponseHeader::getRetryAfter() const { return (_retryAfter); }
std::string					ResponseHeader::getLastModified() const { return (_lastModified); }
std::string					ResponseHeader::getLocation() const { return (_location); }
std::map<int, std::string>	ResponseHeader::getErrorMap() const { return (_errorMap); }
std::map<int, std::string>	ResponseHeader::getErrorHtml() const { return (_errorHtml); }

std::string					ResponseHeader::getStatusMessage(int code)
{
	if (_errorMap.find(code) != _errorMap.end())
		return (_errorMap[code]);

	return ("error code not found");
}

std::string					ResponseHeader::getHeader()
{
	std::string	header;

	setHeader();
	header = _httpVersion + " " + intToStr(_code) + " " + getStatusMessage(_code) + "\r\n";
	header += writeHeader(); 

	return (header);
}

std::string					ResponseHeader::writeHeader()
{
	std::string	header = "";
	if (_allow != "")
		header += "Allow: " + _allow + "\r\n";
	
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

	
	if (_transferEncoding != "" && _transferEncoding != "chunked")
		header += "Transfer-Encoding: " + _transferEncoding + "\r\n";
	if (_transferEncoding == "chunked")
		header += "Transfer-Encoding: identity\r\n";
	
	if (_wwwAuthenticate != "")
		header += "WWW-Authenticate: " + _wwwAuthenticate + "\r\n";
	header += "\r\n";

	return (header);
}

void						ResponseHeader::setErrorHtml(std::map<int, std::string> html)
{
	_errorHtml = html;
	changeHtmlRelativePath();
}

void						ResponseHeader::changeHtmlRelativePath()
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

void						ResponseHeader::initErrorMap()
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

void						ResponseHeader::resetRequest()
{
	_date = "";
	_connection = "";
	_transferEncoding = "";

	_contentLength = "";
	_contentType = "";
	_contentLanguage = "";
	_contentLocation = "";
	_contentEncoding = "";
	_allow = "";

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

	_wwwAuthenticate = "";
	_retryAfter = "";
	_lastModified = "";
	_location = "";
	_bodySize = 0;

	_server = "";
}

void						ResponseHeader::setHeader()
{
	setDate();
	setConnection(_connection);
	setTransferEncoding(_transferEncoding);

	setContentLength(_path, _contentLength);
	setContentTypeLocation(_path, _contentType, _contentLocation);
	setContentLanguage(_contentLanguage);
	setContentEncoding(_contentEncoding);

	setServer(_server);
	setLastModified(_path);
	setLocation(_code, _path);
}

void						ResponseHeader::setServer(const std::string& server)
{
	if (server == "")
		_server = "Webserv/1.0 (Unix)";
	else
		_server = server;
}

void						ResponseHeader::setLastModified(const std::string& path)
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

void						ResponseHeader::setLocation(int code, const std::string& path)
{
	if (code == Created)
		_location = path;
}

void						ResponseHeader::printResponseHeader()
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
