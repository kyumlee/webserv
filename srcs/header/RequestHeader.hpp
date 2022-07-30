#ifndef REQUESTHEADER_HPP
# define REQUESTHEADER_HPP

# include "EntityHeader.hpp"



class RequestHeader : public EntityHeader
{//HTTP요청에서 사용되지만 메시지의 컨텐츠와 관련이 없는 패치될 리소스나 클라리언트 자체에 대한 자세한 정보를 포함하는 헤더
	public:
		RequestHeader() { }
		RequestHeader(const RequestHeader& rh)
		{
			(void)rh;
		}
		virtual ~RequestHeader() {}

		RequestHeader&	operator=(const RequestHeader& rh)
		{ (void)rh; return (*this); }

		int	check_request_line(std::string request_line)
		{
			std::string					request_line_delete_rn = str_delete_rn(request_line);
			std::vector<std::string>	request_line_vec = split(request_line_delete_rn, ' ');
			std::vector<std::string>::iterator	request_line_it = request_line_vec.begin();
			
			if (request_line_vec.size() <= 1 || request_line_vec.size() > 3)
			{
				std::cerr << "request line size is " << request_line_vec.size() << std::endl;
				return (1);
			}
			this->_method = *request_line_it;
			
			if (isStrUpper(this->_method) == 0)
			{//method가 대문자가 아니라면 바로 종료
				return (1);
			}

			request_line_it++;
			this->_path = *request_line_it;
			if (this->_path != "/" && this->_path.at(0) == '/')
				this->_path = this->_path.substr(1, this->_path.length() - 1);

			if (this->_method == "GET" && request_line_vec.size() == 2)
			{//GET method이고, request_line이 2개의 단어로 이루어져 있다면 HTTP/0.9버전이다.
			//일단 바로 종료하도록 9를 리턴하도록 하자
				this->_http_version = "HTTP/0.9";
				this->setConnection("close");
				return (9);
			}
			else if (request_line_vec.size() == 2)
			{//GET method가 아닌데 2개의 단어로 이루어져 있다면 에러
				std::cerr << "request line size is two but method is not get\n";
				return (1);
			}

			request_line_it++;
			this->_http_version = *request_line_it;
			std::cout << "http version : " << this->_http_version << std::endl;
			if (this->_http_version == "HTTP/1.0")
				this->setConnection("close");
			if (this->_http_version != "HTTP/1.0" && this->_http_version != "HTTP/1.1")
			{
				std::cerr << "request line http version is not 1.0 and not 1.1\n";
				return (1);
			}
			return (0);
		}

		int	check_header(std::vector<std::string> header)
		{
			for (std::vector<std::string>::iterator	it = header.begin() + 1;
				it != header.end(); it++)
			{
				if (strncmp((*it).c_str(), "Host:", 5) == 0)
				{//Host의 값을 찾아서 값을 넣어준다.
					this->_host = find_header_value(*it);
					// http://는 자른다.
					if (_host.find("http://", 0) != std::string::npos)
						_host = _host.substr(7, _host.length() - 7);
					if (this->setListen(this->_host) == 1)
						return (1);
				}
				else if (strncmp((*it).c_str(), "User-Agent:", 11) == 0)
					this->_user_agent = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Accept:", 7) == 0)
					this->_accept = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Accept-Charset:", 15) == 0)
					this->_accept_charset = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Accept-Language:", 16) == 0)
					this->_accept_language = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Accept-Encoding:", 16) == 0)
					this->_accept_encoding = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Origin:", 7) == 0)
					this->_origin = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Authorization:", 14) == 0)
					this->_authorization = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Content-Length:", 15) == 0)
					this->_content_length = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Content-Type:", 13) == 0)
					this->_content_type = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Transfer-Encoding:", 18) == 0)
					this->_transfer_encoding = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Content-Language:", 17) == 0)
					this->_content_language = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Content-Location:", 17) == 0)
					this->_content_location = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Content-Encoding:", 17) == 0)
					this->_content_encoding = find_header_value(*it);
				else if (strncmp((*it).c_str(), "Connection:", 11) == 0)
					this->_connection = find_header_value(*it);
				else
				{//이상한 header값일 때 :(콜론)과 ' '(공백)을 확인하여 에러처리
				//:(콜론)이 없는 데 ' '(공백)이 있다면 에러, 콜론이 없더라도 공백이 없으면 에러가 아님
				//공백은 무조건 콜론 뒤에서만 허용된다.
					if (check_header_available(*it) == 1)
					{
						std::cout << "request header has strange value\n";
						return (1);
					}
				}
			}
			if (this->_content_length != "" && this->_transfer_encoding == "")
			{//content_length가 있고, transfer_encoding은 없을 때
				if (isNumber(this->_content_length) == 0)
				{
					std::cerr << "content_length is not number\n";
					this->_content_length = "";
					return (1);
				}
				else
				{
					this->_body_size = std::atoi(this->_content_length.c_str());
				}
			}
			if (this->check_essential_header() == 1)
				return (1);
			return (0);
		}

		int	check_header_available(const std::string& header, char colon = ':', char space = ' ')
		{
			size_t	colon_pos;
			size_t	space_pos;
			if ((space_pos = header.find_first_of(space)) == std::string::npos)
			//공백이 없으면 정상 작동
				return (0);
			else if ((colon_pos = header.find_first_of(colon)) == std::string::npos)
			//공백이 있고 콜론이 없으면 에러
				return (1);
			else if (space_pos < colon_pos)
			//공백이 있고 콜론이 있을 때 공백이 콜론보다 앞에 있으면 에러
				return (1);
			else
			//공백이 있고 콜론이 있을 때 공백이 콜론보다 뒤에 있으면 정상
				return (0);
		}

		int	check_essential_header()
		{
			if (this->_http_version == "HTTP/0.9")
				return (0);
			std::cout << "host : " << this->_host << std::endl;
			if (this->_host == "")
			{
				std::cerr << "host header is not exist\n";
				return (1);
			}
			if (this->_method == "PUT" || this->_method == "POST")
			{//Content-Type, Content-Length(or Transfer-Encoding)이 있어야 한다.
				if (this->_content_length != "" && this->_transfer_encoding != "")
				{
					std::cerr << "content length, transfer encoding exist\n";
					return (1);
				}
				else if (this->_content_length != "" || this->_transfer_encoding != "")
					return (0);
				else
				{
					std::cerr << "content length or transfer encoding header is not exist\n";
					return (1);
				}
			}
			return (0);
		}

		int	request_split(std::string request, int body_condition)
		{
			size_t	r_pos = 0;
			std::vector<std::string>	str_header;
			size_t	start = 0;
			std::string		str;
			std::map<std::string, std::string>	ret;
			std::string		body = "";
			while ((r_pos = request.find('\n', start)) != std::string::npos)
			{//\r을 계속 찾아서 그것을 기준으로 vector에 넣어주자.
				if (request.at(start) == '\r')
				{
					if (start + 1 == r_pos)
					{
						r_pos += 1;
						start = r_pos;
						break ;
					}
				}
				str = request.substr(start, r_pos - start - 1);
				str_header.push_back(str);
				r_pos += 1;
				start = r_pos;
			}
			if (str_header.size() == 1 && body_condition == No_Body)
			{//요청이 한 줄만 왔을 때
				if (this->_http_version != "HTTP/0.9")
				{
					std::cerr << "request is one line but has no essential header\n";
					return (1);
				}
				return (0);
			}
			if (this->_http_version == "HTTP/0.9")
			{
				std::cerr << "http version is HTTP/0.9 but it has header\n";
				return (1);
			}
			if (this->_http_version == "HTTP/1.1")
				this->setConnection();
			else
				this->setConnection("close");
			if (check_header(str_header) == 1)
				return (1);
			return (0);
		}

		bool	host_to_int(std::string host)
		{//string형인 host를 사용할 수 있도록 unsigned int형으로 바꿔준다.
			size_t	sep = 0;
			unsigned int	n;
			size_t	start = 0;
			std::string	substr;
			unsigned int	ret = 0;
			if (host == "localhost")
				host = "127.0.0.1";
			if (isNumber(host) == 1)
			{//host가 그냥 숫자로 되어있을 떄
				ret = std::atoi(host.c_str());
				this->_listen.host = ret;
				return (0);
			}
			for (int i = 3; i > -1; i--)
			{
				sep = host.find_first_of('.', sep);
				if (i != 0 && sep == std::string::npos)
				{
					std::cerr << "host address has not .\n";
					return (1);
				}
				if (i == 0)
					sep = host.length();
				substr = host.substr(start, sep - start);
				if (isNumber(substr) == 0)
				{
					std::cerr << "host address is not number\n";
					return (1);
				}
				n = std::atoi(substr.c_str());
				for (int j = 0; j < i; j++)
					n *= 256;
				ret += n;
				sep++; start = sep;
			}
			this->_listen.host = ret;
			return (0);
		}
		
		int	setListen(const std::string& str_host = "")
		{//에러가 발생하면 1을 리턴, 정상작동하면 0을 리턴
			if (str_host == "")
			{
				std::cerr << "host header is not exist\n";
				return (1);
			}
			std::vector<std::string>	host_port;
			unsigned int				host;
			int							port;
			host_port = split(str_host, ':');
			if (*host_port.begin() == str_host)
			{//포트는 생략할 수 있다. HTTP URL에서는 port default가 80이다.
			//일단 8000으로 default port를 하자
				this->_listen.port = htons(DEFAULT_PORT);
				if ((host = host_to_int(str_host)) == 1)
				{//str_host가 이상한 값을 가지고 있을 때
					std::cerr << "host has strange value\n";
					return (1);
				}
				this->_listen.host = htonl(host);
				return (0);
			}

			if (isNumber(*(host_port.begin() + 1)) == 0)
			{
				std::cerr << "port is not number\n";
				return (1);
			}
			port = std::atoi((*(host_port.begin() + 1)).c_str());
			this->_listen.port = htons(port);

			if ((host = host_to_int(*host_port.begin())) == 1)
			{
				return (1);
			}
			this->_listen.host = htonl(host);
			return (0);
		}

		void	setHost(const std::string& host = "") { this->_host = host; }
		void	setUserAgent(const std::string& user_agent = "") { this->_user_agent = user_agent; }
		void	setAccept(const std::string& accept = "") { this->_accept = accept; }
		void	setAcceptCharset(const std::string& charset = "") { this->_accept_charset = charset; }
		void	setAcceptLanguage(const std::string& lang = "") { this->_accept_language = lang; }
		void	setOrigin(const std::string& origin = "") { this->_origin = origin; }
		void	setAuthorization(const std::string& authorization = "") { this->_authorization = authorization; }
		void	setMethod(const std::string& method = "") { this->_method = method; }
		void	setPath(const std::string& path = "") { this->_path = path; }
		void	setHttpVersion(const std::string& http_version = "") { this->_http_version = http_version; }
		void	setBody(const std::string& body = "") { this->_body = body; }
		void	setBodySize(const size_t& body_size = 0) { this->_body_size = body_size; }
		void	setRoot(const std::string& root = "") { this->_root = root; }

		t_listen	getListen() { return (this->_listen); }
		std::string	getHost() { return (this->_host); }
		std::string	getUserAgent() { return (this->_user_agent); }
		std::string	getAccept() { return (this->_accept); }
		std::string	getAcceptCharset() { return (this->_accept_charset); }
		std::string	getAcceptLanguage() { return (this->_accept_language); }
		std::string	getAcceptEncoding() { return (this->_accept_encoding); }
		std::string	getOrigin() { return (this->_origin); }
		std::string	getAuthorization() { return (this->_authorization); }
		std::string	getMethod() { return (this->_method); }
		std::string	getPath() { return (this->_path); }
		std::string	getHttpVersion() { return (this->_http_version); }
		std::string	getBody() { return (this->_body); }
		size_t		getBodySize() { return (this->_body_size); }
		std::string	getRoot() { return (this->_root); }

		void	printRequestHeader()
		{
			std::cout << "listen host: " << this->getListen().host << ", port: " << this->getListen().port << std::endl;
			std::cout << "server host: " << this->_host << std::endl;
			std::cout << "user agent: " << this->_user_agent << std::endl;
			std::cout << "accept: " << this->_accept << std::endl;
			std::cout << "accept charset: " << this->_accept_charset << std::endl;
			std::cout << "accept language: " << this->_accept_language << std::endl;
			std::cout << "origin: " << this->_origin << std::endl;
			std::cout << "authorization: " << this->_authorization << std::endl;
			std::cout << "method: " << this->_method << std::endl;
			std::cout << "path: " << this->_path << std::endl;
			std::cout << "http version: " << this->_http_version << std::endl;
			std::cout << "body: " << this->_body << std::endl;
			std::cout << "body size: " << this->_body_size << std::endl;
			std::cout << "root: " << this->_root << std::endl;
		}

	public:
	//일단 임시로 public으로 바꾼다.
		t_listen	_listen;
		std::string	_host; //요청하려는 서버 호스트 이름과 포트 번호
		std::string	_user_agent; //현재 사용자가 어떤 클라리언트(운영체제, 브라우저 등)을 통해 요청을 보냈는지 알 수 있다
		std::string	_accept; //클라이언트가 허용할 수 있는 파일 형식(MIME TYPE)
		std::string	_accept_charset; //클라이언트가 지원가능한 문자열 인코딩 방식
		std::string	_accept_language; //클라이언트가 지원가능한 언어 나열
		std::string	_accept_encoding; //클라이언트가 해석가능한 압축 방식 지정
		std::string	_origin; //POST같은 요청을 보낼 때, 요청이 어느 주소에서 시작되었는지를 나타냄, 경로 정보는 포함하지 않고 서버 이름만 포함
		std::string	_authorization; //인증 토큰을 서버로 보낼 때 사용하는 헤더
		//형식 -> Authorization: <auth-scheme> <authorization-parameters>
		//basic authentication -> Authorization: Basic <credentials>

		//내가 만든 것 request line을 파시할 때 사용
		std::string	_method; //request method를 저장
		std::string	_path; //request의 path를 저장
		std::string	_http_version; //HTTP버전을 확인
		std::string	_body;
		size_t		_body_size;
		std::string	_root;
		std::vector<std::string>	_body_vec;

		/*
		//사용안할 것 같은 것
		std::string	_referer; //현재 페이지로 연결되는 링크가 있던 이전 웹 페이지의 주소
		std::string	_cookie; //Set-Cookie헤더와 함께 서버로부터 이전에 전송됐던 저장된 HTTP 쿠키 포함
		std::string	_if_modified_since; //여기에 쓰여진 시간 이후로 변경된 리소스를 취득하고 캐시가 만료되었을 때에만 데이터를 전송하는데 사용
		*/
};

#endif
