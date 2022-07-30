#ifndef RESPONSEHEADER_HPP
# define RESPONSEHEADER_HPP

# include "RequestHeader.hpp"

class ResponseHeader : public RequestHeader
{
	public:
		ResponseHeader() { }
		ResponseHeader(const ResponseHeader& rh) { (void)rh; }
		virtual ~ResponseHeader() {}

		ResponseHeader&	operator=(const ResponseHeader& rh)
		{
			(void)rh;
			return (*this);
		}

		void	initErrorHtml()
		{
			setErrorHtml(Bad_Request, "400.html");
			setErrorHtml(Forbidden, "403.html");
			setErrorHtml(Not_Found, "404.html");
			setErrorHtml(Method_Not_Allowed, "405.html");
			setErrorHtml(Payload_Too_Large, "413.html");
			setErrorHtml(Internal_Server_Error, "500.html");
			changeHtmlRelativePath();
		}

		void	setErrorHtml(int code, std::string html)
		{//code와 html의 이름을 받아서 error_html에 저장
			this->_error_html[code] = html;
		}

		void	setErrorHtml(std::map<int, std::string> html)
		{
			this->_error_html = html;
			changeHtmlRelativePath();
		}

		void	changeHtmlRelativePath()
		{//error_html에 저장되어있는 파일이름을 상대경로로 바꿔준다.
			for (std::map<int, std::string>::iterator it = this->_error_html.begin();
				it != this->_error_html.end(); it++)
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

	
		void	initErrorMap()
		{//errormap은 config파일에서 읽은 그대로 사용해야 한다.
		//일단 내 맘대로 초기화했다.
			this->_error_map[Continue] = "Continue";
			this->_error_map[OK] = "OK";
			this->_error_map[Created] = "Created";
			this->_error_map[No_Content] = "No Content";
			this->_error_map[Bad_Request] = "Bad Request";
			this->_error_map[Forbidden] = "Forbidden";
			this->_error_map[Not_Found] = "Not Found";
			this->_error_map[Method_Not_Allowed] = "Not Allowed";
			this->_error_map[Payload_Too_Large] = "Payload Too Large";
			this->_error_map[Internal_Server_Error] = "Internal Server Error";
		}

		std::string	getHeader()
		{
			std::string	header;
			setHeader();
			header = this->_http_version + " " + intToStr(this->_code) + " " + getStatusMessage(this->_code) + "\r\n";
			header += writeHeader(); 
			return (header);
		}

		std::string	writeHeader()
		{
			std::string	header = "";
			if (this->_allow != "")
				header += "Allow: " + this->_allow + "\r\n";
			
			//request header와 겹침
			if (this->_content_language != "")
				header += "Content-Language: " + this->_content_language + "\r\n";
			if (this->_content_length != "" && this->_transfer_encoding != "chunked")
				header += "Content-Length: " + this->_content_length + "\r\n";
			if (this->_transfer_encoding == "chunked")
				header += "Content-Length: 0\r\n";
			if (this->_content_location != "")
				header += "Content-Location: " + this->_content_location + "\r\n";
			if (this->_content_type != "")
				header += "Content-Type: " + this->_content_type + "\r\n";
			//
			
			if (this->_date != "")
				header += "Date: " + this->_date + "\r\n";
			if (this->_last_modified != "")
				header += "Last-Modified: " + this->_last_modified + "\r\n";
			if (this->_location != "")
				header += "Location: " + this->_location + "\r\n";
			if (this->_retry_after != "")
				header += "Retry-After: " + this->_retry_after + "\r\n";
			if (this->_server != "")
				header += "Server: " + this->_server + "\r\n";

			//request header와 겹치는 것
			if (this->_transfer_encoding != "" && this->_transfer_encoding != "chunked")
				header += "Transfer-Encoding: " + this->_transfer_encoding + "\r\n";
			if (this->_transfer_encoding == "chunked")
				header += "Transfer-Encoding: identity\r\n";
			//
			
			if (this->_www_authenticate != "")
				header += "WWW-Authenticate: " + this->_www_authenticate + "\r\n";
			header += "\r\n";
			return (header);
		}

		std::string	getStatusMessage(int code)
		{
			if (this->_error_map.find(code) != this->_error_map.end())
				return (this->_error_map[code]);
			return ("There is no error code");
		}

		void	initRequest()
		{
			//general header reset
			this->_date = "";
			this->_connection = "";
			this->_transfer_encoding = "";

			//Entity header reset
			this->_content_length = "";
			this->_content_type = "";
			this->_content_language = "";
			this->_content_location = "";
			this->_content_encoding = "";
			this->_allow = "";

			//request header reset
			this->_listen.host = 0;
			this->_listen.port = 0;
			this->_host = "";
			this->_user_agent = "";
			this->_accept = "";
			this->_accept_charset = "";
			this->_accept_language = "";
			this->_accept_encoding = "";
			this->_origin = "";
			this->_authorization = "";
			this->_method = "";
			this->_path = "";
			this->_http_version = "";
			this->_body = "";
			this->_code = 0;

			//response header reset
			this->_www_authenticate = "";
			this->_retry_after = "";
			this->_last_modified = "";
			this->_location = "";
			this->_body_size = 0;
		}

		void	resetRequest()
		{
			//response header reset
			this->initRequest();
			this->_server = "";
		}

		void	setHeader()
		{//response하는데 필요한 헤더들을 세팅한다.
			//general header value
			this->setDate();
			this->setConnection(this->_connection);
			this->setTransferEncoding(this->_transfer_encoding);

			//entity header
			this->setContentLength(this->_path, this->_content_length);
			this->setContentTypeLocation(this->_path, this->_content_type, this->_content_location);
			this->setContentLanguage(this->_content_language);
			this->setContentEncoding(this->_content_encoding);
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

		void	setServer(const std::string& server = "")
		{
			if (server == "")
				this->_server = "Webserv/1.0 (Unix)";
			else
				this->_server = server;
		}
		void	setWwwAuthenticate(int code)
		{
			if (code == Unauthorized)
				this->_www_authenticate = "Basic realm=\"Access requires authentification\", charset=\"UTF-8\"";
		}
		void	setRetryAfter(int code, int sec)
		{
			if (code == Service_Unavailable || code == Too_Many_Requests || code == Moved_Permanently)
				this->_retry_after = intToStr(sec);
		}

		void	setLastModified(const std::string& path)
		{
			char		buf[100];
			struct stat	file_stat;
			struct tm*	tm;
			if (stat(path.c_str(), &file_stat) == 0)
			{
				tm = gmtime(&file_stat.st_mtime);
				strftime(buf, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);
				std::string	last_modified(buf);
				this->_last_modified = last_modified;
			}
		}

		void	setLocation(int code, const std::string& path)
		{
			if (code == Created || code / 100 == 3)
				this->_location = path;
		}

		std::string					getServer() { return (this->_server); }
		std::string 				getRetryAfter() { return (this->_retry_after); }
		std::string					getLastModified() { return (this->_last_modified); }
		std::string					getLocation() { return (this->_location); }
		std::map<int, std::string>	getErrorMap() { return (this->_error_map); }
		std::map<int, std::string>	getErrorHtml() { return (this->_error_html); }

		void	printResponseHeader()
		{
			std::cout << "server name: " << this->getServer() << std::endl;
			std::cout << "retry after: " << this->getRetryAfter() << std::endl;
			std::cout << "last modified: " << this->getLastModified() << std::endl;
			std::cout << "location: " << this->getLocation() << std::endl;
			std::cout << "error map: ";
			print_errmap(this->getErrorMap());
			std::cout << "error html: ";
			print_errmap(this->getErrorHtml());
		}
	
	// protected:
		std::string					_server; //웹서버 정보
		std::string					_www_authenticate; //사용자 인증이 필요한 자원을 요구할 시, 서버가 제공하는 인증 방식
		std::string					_retry_after; //다시 접속하라고 알릴 때
		std::string					_last_modified; //리소스의 마지막 수정 날짜
		std::string					_location; //300번대 응답이나 201(created)응답일 때 어느 페이지로 이동할 지 알려주는 헤더
		
		std::map<int, std::string>	_error_map;
		std::map<int, std::string>	_error_html;

		//사용안할 것 같은 것
		// std::string	_access_control_allow_origin; //요청 Host와 응답 Host가 다를 때 CORS에러를 막기 위해 사용
		// std::string	_age; //시간이 얼마나 흘렀는지 초 단위로 알려줌
		// std::string	_referrer_policy; //서버 referrer정책을 알려줌
		// std::string	_proxy_authenticate; //요청한 서버가 프록시 서버인 경우 유저 인증을 위한 값
		// std::string	_set_cookie; //서버측에서 클라이언트에게 세션 쿠키 정보를 설정
};

#endif