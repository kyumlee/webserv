#ifndef CGI_HPP
# define CGI_HPP

# include "../response/Response.hpp"

class Cgi
{
	public:
		Cgi() { this->_body = "body"; }
		Cgi(Cgi const& cgi) { (void)cgi; }
		virtual ~Cgi() { }

		Cgi&	operator=(Cgi const& cgi) { (void)cgi; return (*this); }

		void	setEnv(const std::string& env_key, const std::string& env_value)
		{ _env[env_key] = env_value; }
		void	setCgiExist(const int& exist) { _is_exist = exist; }
		int		getCgiExist() { return (_is_exist); }
		std::map<std::string, std::string>	getEnv() { return (_env); }

		std::string	exectueCgi(const std::string& scriptName)
		{//인자로 받은 script를 실행하고 script결과를 리턴한다.
			pid_t	pid;
			int		saveStdin;
			int		saveStdout;
			char**	env;
			std::string	newBody;
			try
			{
				env = this->envToChar();
			}
			catch(std::bad_alloc& e)
			{//new가 실패하면 bad_alloc을 thorw한다.
				std::cerr << e.what() << '\n';
			}
			saveStdin = dup(STDIN_FILENO);
			saveStdout = dup(STDOUT_FILENO);

			FILE*	fIn = tmpfile(); //fIn이라는 임시파일을 만든다.
			FILE*	fOut = tmpfile(); //fOut이라는 임시파일을 만든다.
			int		fdIn = fileno(fIn); //fIn의 fd를 fdIn에 저장
			int		fdOut = fileno(fOut); //fOut의 fd를 fdOut에 저장
			int		ret = 1;

			//fIn에 body의 내용을 적어놓는다. 
			write(fdIn, this->_body.c_str(), this->_body.size());
			//fIn의 offset위치를 맨 앞으로 설정한다.
			lseek(fdIn, 0, SEEK_SET);

			pid = fork();

			if (pid == -1)
			{
				std::cerr << "Fork crashed.\n";
				return ("Status: 500\r\n\r\n");
			}
			else if (pid == 0)
			{//자식 프로세스일 때
				char* const*	nll = NULL;
				//입력받는 값이 fdIn으로 들어가도록 만든다..
				dup2(fdIn, STDIN_FILENO);
				//출력되는 값이 fdOut으로 들어가도록 만든다.
				dup2(fdOut, STDOUT_FILENO);
				execve(scriptName.c_str(), nll, env);
				std::cerr << "execve crashed.\n";
				write(STDOUT_FILENO, "Status: 500\r\n\r\n", 15);
			}
			else
			{//부모 프로세스일 때
				char	buffer[CGI_BUF_SIZE] = {0};
				//임의의 자식 프로세스를 기다린다.
				waitpid(-1, NULL, 0);
				//fdOut의 offset위치를 맨 앞으로 설정한다.
				lseek(fdOut, 0, SEEK_SET);
			
				ret = 1;
				while (ret > 0)
				{
					std::memset(buffer, 0, CGI_BUF_SIZE);
					ret = read(fdOut, buffer, CGI_BUF_SIZE - 1);
					newBody += buffer;
				}
			}

			//stdin과 stdout을 원래대로 복구한다.
			dup2(saveStdin, STDIN_FILENO);
			dup2(saveStdout, STDOUT_FILENO);
			fclose(fIn);
			fclose(fOut);
			close(fdIn);
			close(fdOut);
			close(saveStdin);
			close(saveStdout);

			for (size_t i = 0; env[i]; i++)
				delete[] env[i];
			delete[] env;

			if (pid == 0)
			//자식 프로세스가 제대로 execve를 실행하지 못했을 때 exit으로 프로그램 종료
				exit(0);
			return (newBody);
		}

		void	print_env()
		{//제대로 env가 들어갔는지 확인하려고 만든 함수 나중에 필요없을 거다.
			char**	env = this->envToChar();
			for (size_t i = 0; i < this->_env.size(); i++)
			{
				std::cout << env[i] << std::endl;
			}
		}
		void	initEnv()
		{
			//header에 Auth-Scheme가 존재한다면 AUTH_TYPE환경변수에 header의 Authorization값을 집어넣는다.
			//AUTH_TYPE은 인증 타입.
			//만약 client가 액세스에 인증이 필요한 경우, Request Authorization header 필드의 auth-scheme 토큰을 바탕으로 값을 세팅해야 한다.
			// if (headers.find("Auth-Scheme") != headers.end() && headers["Auth-Scheme"] != "")
			// 	this->_env["AUTH_TYPE"] = headers["Authorization"];
			this->_env["AUTH_TYPE"] = "";

			//원하는 status를 지정하여 PHP가 처리할 수 있는 status를 정한다.
			//php-cgi가 200을 처리할 수 있도록 정했다.
			this->_env["REDIRECT_STATUS"] = "200";

			//request body의 길이, request body가 없으면 NULL값으로 세팅한다.
			//request body가 있을 때만 세팅되어야 하며, transfer-encoding이나 content-coding을 제거한 후의 값으로 세팅되어야 한다.
			this->_env["CONTENT_LENGTH"] = "";
			//request body의 mime.type을 세팅한다.
			//만약 request에 CONTENT_TYPE헤더가 존재한다면 무조건 세팅해야 한다.
			//CONTENT_TYPE헤더가 존재하지 않으면 올바른 CONTENT_TYPE을 추론하거나, CONTENT_TYPE을 생략해야 한다.
			// this->_env["CONTENT_TYPE"] = headers["Content-Type"];
			this->_env["CONTENT_TYPE"] = "";

			//서버의 CGI타입과 개정레벨을 나타난다.
			//형식 CGI/revision
			this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";

			//client에 의해 전달되는 추가 PATH정보
			//PATH_INFO를 이용하여 스크립트를 부를 수 있다.
			this->_env["PAHT_INFO"] = "";
			//PATH_INFO에 나타난 가상 경로(path)를 실제의 물리적인 경로로 바꾼 값
			this->_env["PATH_TRANSLATED"] = "";

			//GET방식에서 URL의 뒤에 나오는 정보를 저장하거나 폼입력 정보를 저장한다.
			//POST방식은 제외
			this->_env["QUERY_STRING"] = "";

			//client의 IP주소
			this->_env["REMOTEaddr"] = "";
			//client의 호스트 이름
			// this->_env["REMOTED_IDENT"] = headers["Authorization"];
			this->_env["REMOTE_IDENT"] = "";
			//서버가 사용자 인증을 지원하고, 스크립트가 확인을 요청한다면, 이것이 확인된 사용자 이름이 된다.
			// this->_env["REMOTE_USER"] = headers["Authorization"];
			this->_env["REMOTE_USER"] = "";

			//GET, POST, PUT과 같은 method
			this->_env["REQUEST_METHOD"] = "";
			//쿼리까지 포함한 모든 주소
			//ex) http://localhost:8000/cgi-bin/ping.sh?var1=value1&var2=with%20percent%20encodig
			this->_env["REQUEST_URI"] = "";

			//실제 스크립트 위치
			//ex) http://localhost:8000/cgi-bin/ping.sh?var1=value1&var2=with%20percent%20encodig의
			// /cgi-bin/ping.sh를 뜻한다.
			//웹에서의 스크립트 경로를 뜻한다.
			this->_env["SCRIPT_NAME"] = "";
			//서버(로컬)에서의 스크립트 경로를 뜻한다.
			//ex) "C:/Program Files (x86)/Apache Software Foundation/Apache2.2/cgi-bin/ping.sh"
			this->_env["SCRIPT_FILENAME"] = "";

			//서버의 호스트 이름과 DNS alias 혹은 IP주소
			// if (headers.find("Hostname") != headers.end())
			// 	this->_env["SERVER_NAME"] = headers["Hostname"];
			// else
			// 	this->_env["SERVER_NAME"] = this->_env["REMOTEaddr"];
			this->_env["SERVER_NAME"] = "";
			//client request를 보내는 포트 번호
			this->_env["SERVER_PORT"] = "";
			//client request가 사용하는 프로토콜
			this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
			//웹서버의 이름과 버전을 나타낸다.
			// 형식: 이름/버전
			this->_env["SERVER_SOFTWARE"] = "Weebserv/1.0";
			
			// this->_env.insert(config.getCgiParam().begin(), config.getCgiParam().end());
		}
	
	private:
		std::map<std::string, std::string>	_env;
		std::string							_body;
		int									_is_exist;
		char**	envToChar() const
		{
			char	**env = new char*[this->_env.size() + 1];
			int		j = 0;
			std::string	element;
			for (std::map<std::string, std::string>::const_iterator i = this->_env.begin();
				i != this->_env.end(); i++, j++)
			{
				element = i->first + "=" + i->second;
				env[j] = new char[element.size() + 1];
				env[j] = strcpy(env[j], element.c_str());
			}
			env[j] = NULL;
			return (env);
		}
};

#endif