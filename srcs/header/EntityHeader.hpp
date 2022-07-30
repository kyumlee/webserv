#ifndef ENTITYHEADER_HPP
# define ENTITYHEADER_HPP

# include "GeneralHeader.hpp"

class EntityHeader : public GeneralHeader
{//컨텐츠 길이나 MIME 타입과 같이 entity 바디에 대한 자세한 정보를 포함하는 헤더
	public:
		EntityHeader() {}
		EntityHeader(const EntityHeader& eh) { (void)eh; }
		virtual ~EntityHeader() {}

		EntityHeader&	operator=(const EntityHeader& eh) { (void)eh; return (*this); }

		void	setContentLength(const size_t& size = 0)
		{ this->_content_length = intToStr(size); }
		void	setContentLength(const std::string& path = "" , const std::string& content_length = "")
		{
			if (content_length != "")
				this->_content_length = content_length;
			else if (path != "")
			{
				struct stat file_stat;
				if (!stat(path.c_str(), &file_stat))
				{
					this->_content_length = intToStr(file_stat.st_size);
				}
				else
				{
					std::cerr << "get file size has error\n";
					this->setCode(Internal_Server_Error);
				}
			}
		}

		void	setContentTypeLocation(const std::string& path, std::string type = "", std::string content_location = "")
		{//html, css, js, jpeg, png, bmp, plain만 정해놨는데 다른 것도 해야되는지 알아봐야 한다.
			if (type != "")
			{
				this->_content_type = type;
				this->_content_location = path;
				return ;
			}
			std::string	file_name = find_file_name(path);
			std::string	file_type = find_file_type(file_name);
			std::string	pure_file_name = erase_file_type(file_name);
			std::string	content_type;
			
			if (file_type == "css")
				content_type = "text/css";
			else if (file_type == "html")
				content_type = "text/html";
			else if (file_type == "js")
				content_type = "text/javascript";
			else if (file_type == "apng")
				content_type = "image/apng";
			else if (file_type == "avif")
				content_type = "image/avif";
			else if (file_type == "gif")
				content_type = "image/gif";
			else if (file_type == "jpeg" || file_type == "jpg")
				content_type = "image/jpeg";
			else if (file_type == "png")
				content_type = "image/png";
			else if (file_type == "svg")
				content_type = "image/svg+xml";
			else if (file_type == "webp")
				content_type = "image/webp";
			//bmp는 사용을 안 하는 것이 좋다고 한다.
			// else if (file_type == "bmp")
			// 	file_type = "image/bmp";
			//비디오, 음악 파일은 일단 제외했다.
			else
			{//file_type이 이상할 떄
				file_type = "";
				content_type = "text/plain";
			}
			this->_content_type = content_type;

			std::string	check_content_location = path.substr(0, path.length() - file_name.length());
			
			if (content_location != "")
				this->_content_location = content_location;
			else if (file_type == "")
				this->_content_location = check_content_location + pure_file_name;
			else
				this->_content_location = path;
			if (this->_content_location.at(0) != '/')
				this->_content_location.insert(0, "/");
		}
		void	setContentLanguage(const std::string& content_language = "en")
		{ this->_content_language = content_language;}
		
		void	setContentEncoding(const std::string& content_encoding = "")
		{ this->_content_encoding = content_encoding; }
		
		//둘 다 쓰는지 확인해보자
		void	setAllow(const std::string& allow)
		{ this->_allow = allow; }
		void	setAllow(const std::set<std::string>& methods)
		{
			std::set<std::string>::iterator	it = methods.begin();
			while (it != methods.end())
			{
				this->_allow = *it;
				this->_allow += ", ";
				it++;
			}
		}

		void	setCode(const int& code = 0)
		{
			if (code != 0)
				this->_code = code;
			else
				this->_code = 0;
		}

		void	initPossibleMethod()
		{
			this->_possible_method.insert("GET");
			this->_possible_method.insert("POST");
			this->_possible_method.insert("PUT");
			this->_possible_method.insert("DELETE");
			this->_possible_method.insert("HEAD");
		}

		void	setAllowMethod(const std::string& method)
		{ this->_allow_method.insert(method); }

		void	initAllowMethod(std::vector<std::string> allow_method)
		{
			for (std::vector<std::string>::iterator it = allow_method.begin();
				it != allow_method.end(); it++)
				this->setAllowMethod(*it);
		}

		std::string				getContentLength() { return (this->_content_length); }
		std::string				getContentType() { return (this->_content_type); }
		std::string				getContentLanguage() { return (this->_content_language); }
		std::string				getContentLocation() { return (this->_content_location); }
		std::string				getContentEncoding() { return (this->_content_encoding); }
		std::string				getAllow() { return (this->_allow); }
		int						getCode() { return (this->_code); }
		std::set<std::string>	getAllowMethod() { return (this->_allow_method); }
		std::set<std::string>	getPossibleMethod() { return (this->_possible_method); }

		void	printEntityHeader()
		{
			std::cout << "content length: " << this->getContentLength() << std::endl;
			std::cout << "content type: " << this->getContentType() << std::endl;
			std::cout << "content language: " << this->getContentLanguage() << std::endl;
			std::cout << "content location: " << this->getContentLocation() << std::endl;
			std::cout << "content encoding: " << this->getContentEncoding() << std::endl;
			std::cout << "allow: " << this->getAllow() << std::endl;
			std::cout << "code: " << this->getCode() << std::endl;
			std::cout << "allow method: ";
			print_set(this->getAllowMethod());
			std::cout << "possible method: ";
			print_set(this->getPossibleMethod());
		}
	
	// protected:
		std::string	_content_length; //메시지의 크기
		std::string	_content_type; //컨텐츠의 타입(MIME)과 문자열 인코딩(utf-8등)을 명시
		//ex) Content-Type: text/html; charset=utf-8
		std::string	_content_language; //사용자의 언어
		std::string	_content_location; //반환된 데이터 개체의 실제 위치
		std::string	_content_encoding; //컨텐츠가 압축된 방식
		std::string	_allow; //지원되는 HTTP 메소드, 만약 비어있다면 모든 메소드가 금지라는 뜻
		
		int			_code;
		std::set<std::string>	_allow_method; //configuration file에서 얻어온다.
		std::set<std::string>	_possible_method;
		/*
		//사용안할 것 같은 것
		std::string	_content_disposition; //응답 본문을 브라우저가 어떻게 표시해야할 지 알려주는 헤더
		std::string	_content_security_policy; //다른 외부 파일들을 불러오는 경우, 차단할 소스와 불러올 소스를 명시
		std::string	_expires; //자원의 만료 일자
		std::string	_etag; //리소스의 버전을 식별하는 고유한 문자열 검사기(주로 캐시 확인용으로 사용)
		*/
};

#endif
