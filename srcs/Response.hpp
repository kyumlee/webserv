#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "../header/ResponseHeader.hpp"

class Response : public ResponseHeader
{
	public:
		Response() {}
		Response(const Response& response) { (void)response; }
		virtual ~Response() {}

		Response&	operator=(const Response& response) { (void)response; return (*this); }

		int	check_allow_method()
		{
			if (this->_possible_method.find(this->_method) != this->_possible_method.end()
				&& this->_allow_method.find(this->_method) == this->_allow_method.end())
			{//method를 알고는 있지만, allow가 되지 않았을 때
				this->_code = Method_Not_Allowed;
				return (1);
			}
			if (this->_possible_method.find(this->_method) == this->_possible_method.end())
			{//method가 뭔지 모를 때
				this->_code = Method_Not_Allowed;
				return (1);
			}
			return (0);
		}

		std::string	responseErr(Response *response)
		{
			int	code = response->_code;

			if (response->_error_html.find(code) != response->_error_html.end())
				response->_path = response->_error_html[code];

			std::string	header = response->getHeader();
			std::map<int, std::string>::iterator	it = response->_error_html.find(code);
			if (it != response->_error_html.end())
				header += response->readHtml(it->second);

			return (header);
		}

		int	verify_method(int fd, Response *response, int request_end)
		{//요청마다 header를 만들어야 하고 에러가 발생했을 때에 errormap을 적절히 불러와야 한다.
			//request의 method를 확인한다.
			std::string	total_response;
			int			error = 0;

			int			code = response->_code;

			if (code == Bad_Request || code == Internal_Server_Error || code == Payload_Too_Large)
			{
				error = 1;
				total_response = responseErr(response);
			}

			if (request_end)
			{
				if (response->getPath() == "/" && response->_method != "GET")
				{
					error = 1;
					response->setCode(Method_Not_Allowed);
					total_response = responseErr(response);
				}
				else
				{
					if (response->getPath() == "/" && response->_method == "GET")
						response->setPath(getRoot() + "/index.html");
					if (check_allow_method() == 1)
						total_response = this->getHeader();
					else if (this->_method == "GET")
						total_response = this->getMethod(this->_path, fd);
					else if (this->_method == "HEAD")
						total_response = this->headMethod(this->_path, fd);
					else if (this->_method == "POST")
						total_response = this->postMethod(this->_path, fd, this->_body);
					else if (this->_method == "PUT")
						total_response = this->putMethod(this->_path, fd, this->_body);
					else if (this->_method == "DELETE")
						total_response = this->deleteMethod(this->_path, fd);

					std::map<int, std::string>::iterator	it = this->_error_html.find(this->_code);
					if (it != this->_error_html.end())
						total_response += this->readHtml(it->second);
					else if (this->_code != Created && this->_code != No_Content && this->_code != OK)
						total_response += ERROR_HTML;
				}
			}
			if (total_response != "")
				std::cout << YELLOW << "#########response########\n" << total_response <<  RESET << std::endl;
			int	write_size = ::send(fd, total_response.c_str(), total_response.size(), 0);
			if (error || write_size == -1)
			{
				if (!error)
					std::cerr << "RESPONSE SEND ERROR" << std::endl;
				return (1);
			}
			
			return (0);
		}

		std::string	getMethod(std::string& path, int fd)
		{
			std::string			file_content = "";
			std::ifstream		file;
			std::stringstream	buffer;
			(void)fd;
			std::cout << "GET METHOD PATH IS " << path << std::endl;
			if (compare_end(path, "Yeah") == 0)
			{
				std::cout << "path is Yeah so not found\n";
				this->setCode(Not_Found);
			}
			else
			{
				file.open(path.c_str(), std::ifstream::in);
				if (file.is_open() == false)
				{
					std::cout << "FILE OPEN ERROR\n";
					this->setCode(Not_Found);
				}
				else
				{
					this->setCode(OK);
					buffer << file.rdbuf();
				}
			}
			this->setContentLength(buffer.str().length());
			file_content = this->getHeader();
			file_content += buffer.str();
			file.close();
			return (file_content);
		}

		std::string	headMethod(std::string& path, int fd)
		{
			std::string			file_content = "";
			std::ifstream		file;
			std::stringstream	buffer;
			(void)fd;
			if (compare_end(path, "Yeah") == 0)
			{
				std::cout << "path is Yeah so not found\n";
				this->setCode(Not_Found);
			}
			else
			{
				file.open(path.c_str(), std::ifstream::in);
				if (file.is_open() == false)
				{
					std::cout << "FILE OPEN ERROR\n";
					this->setCode(Not_Found);
				}
				else
				{
					this->setCode(OK);
					buffer << file.rdbuf();
				}
			}
			this->setContentLength(buffer.str().length());
			file_content = this->getHeader();
			file.close();
			return (file_content);
		}
		std::string	postMethod(const std::string& path, int fd, const std::string& body)
		{
			(void)fd;
			std::ofstream	file;
			std::string		file_content;
			if (pathIsFile(path))
			{
				std::cout << "post file is already exist, so add the content\n";
				file.open(path.c_str(), std::ofstream::out | std::ofstream::app);
				if (file.is_open() == false)
				{
					std::cerr << "FILE OPEN ERROR\n";
					this->setCode(Forbidden);
				}
				file << body;
				file.close();
				this->setCode(No_Content);
			}
			else
			{
				file.open(path.c_str(), std::ofstream::out);
				if (file.is_open() == false)
				{
					std::cerr << "FILE OPEN ERROR\n";
					this->setCode(Not_Found);
				}
				file << body;
				file.close();
				this->setCode(Created);
			}
			file_content = this->getHeader();
			return (file_content);
		}
		std::string	putMethod(const std::string& path, int fd, std::string& body)
		{
			//ofstream형은 open mode를 따로 작성하지 않으면
			//기존에 파일이 존재했다면 삭제하는 trunc, 출력 전용을 뜻하는 out모드로 open된다.
			(void)fd;
			std::ofstream	file;
			std::string		file_content;
			if (pathIsFile(path))
			{//file이 이미 존재하고 regular file일 때 파일을 갱신한다.
				std::cout << "put file is already exist\n";
				file.open(path.c_str());
				if (file.is_open() == false)
				{
					std::cerr << "FILE OPEN ERROR\n";
					this->setCode(Forbidden);
				}
				else
					this->setCode(No_Content);
				file << body;
				file.close();
			}
			else
			{//file이 존재하지 않거나, file이 regular file이 아닐 때 작동
				file.open(path.c_str());
				if (file.is_open() == false)
				{
					std::cerr << "file open fail\n";
					this->setCode(Forbidden);
				}
				else
					this->setCode(Created);
				file << body;
				file.close();
			}
			file_content = this->getHeader();
			return (file_content);
		}
		std::string	deleteMethod(std::string& path, int fd)
		{//일단 delete가 제대로 되지 않을 때는 생각하지 않고 무조건 0을 리턴하는 것으로 했다.
			(void)fd;
			std::string	file_content;
			if (pathIsFile(path))
			{
				if (remove(path.c_str()) == 0)
					this->setCode(No_Content);
				else
				{
					std::cerr << "file is regular file but remove fail\n";
					this->setCode(Forbidden);
				}
			}
			else
			{
				std::cerr << "file is not regular file so remove fail\n";
				this->setCode(Not_Found);
			}
			file_content = this->getHeader();
			return (file_content);
		}

		std::string	readHtml(const std::string& path)
		{//path가 REGULAR file이면 open하여 파일 내용을 리턴한다.
		//path가 REGULAR file이 아니면 error_html을 보낸다.
			std::ofstream			file;
			std::stringstream		buf;

			if (pathIsFile(path))
			{
				file.open(path.c_str(), std::ifstream::in);
				if (file.is_open() == false)
					return (ERROR_HTML);
				buf << file.rdbuf();
				file.close();
				this->_content_type = "text/html";
				return (buf.str());
			}
			else
				return (ERROR_HTML);
		}

		void	printResponseValue()
		{
			std::cout << WHITE;
			this->printGeneralHeader();
			this->printEntityHeader();
			this->printRequestHeader();
			this->printResponseHeader();
			std::cout << RESET;
		}

	private:
		// int	_code;
};

#endif
