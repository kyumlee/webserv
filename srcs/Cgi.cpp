# include "./../includes/Cgi.hpp"

Cgi::Cgi()
	: _env(),
	_exists(false),
	_name(),
	_body("body")
{}

Cgi::Cgi(Cgi const& cgi)
	: _env(cgi._env),
	_exists(cgi._exists),
	_name(cgi._name),
	_body(cgi._body)
{}

Cgi::~Cgi () {}

Cgi&								Cgi::operator=(Cgi const& cgi)
{
	_env = cgi._env;
	_exists = cgi._exists;
	_name = cgi._name;
	_body = cgi._body;
	return (*this);
}

std::map<std::string, std::string>	Cgi::getEnv() const { return (_env); }
int									Cgi::getCgiExist() const { return (_exists); }
std::string							Cgi::getName() const { return (_name); }
std::string							Cgi::getBody() const { return (_body); }


void								Cgi::setEnv(const std::string& key, const std::string& value) { _env[key] = value; }
void								Cgi::setCgiExist(const int& exist) { _exists = exist; }
void								Cgi::setName(const std::string& name) { _name = name; }
void								Cgi::setBody(const std::string& body) { _body = body; }

std::string							Cgi::executeCgi(const std::string& scriptName)
{
	pid_t		pid;
	int			saveStdin, saveStdout;
	char**		env;
	std::string	newBody;

	try {
		env = envToChar();
	} catch(std::bad_alloc& e) {
		printErr(e.what());
	}

	saveStdin = dup(STDIN_FILENO);
	saveStdout = dup(STDOUT_FILENO);

	FILE*	fIn = tmpfile(); 
	FILE*	fOut = tmpfile(); 
	int		fdIn = fileno(fIn); 
	int		fdOut = fileno(fOut); 
	int		ret = 1;

	write(fdIn, _body.c_str(), _body.size());
	
	lseek(fdIn, 0, SEEK_SET);

	pid = fork();

	if (pid == -1)
	{
		printErr("failed to fork");
		return ("Status: 500\r\n\r\n");
	}
	else if (pid == 0)
	{
		char* const*	nll = NULL;

		std::cout << PINK << "cgi start" << std::endl << RESET;

		dup2(fdIn, STDIN_FILENO);
		
		dup2(fdOut, STDOUT_FILENO);
		execve(scriptName.c_str(), nll, env);

		printErr("failed to execve");
		write(STDOUT_FILENO, "Status: 500\r\n\r\n", 15);
	}
	else
	{
		char	buffer[CGI_BUFFER_SIZE] = {0};
		waitpid(-1, NULL, 0);
		lseek(fdOut, 0, SEEK_SET);
		ret = 1;
		while (ret > 0)
		{
			std::memset(buffer, 0, CGI_BUFFER_SIZE);
			ret = read(fdOut, buffer, CGI_BUFFER_SIZE - 1);
			newBody += buffer;
		}
	}

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
	{
		printErr("stop server due to cgi error");
		exit(0);
	}
	return (newBody);
}

void								Cgi::printEnv()
{
	char**	env = envToChar();

	for (size_t i = 0; i < _env.size(); i++)
		std::cout << env[i] << std::endl;
}

char**								Cgi::envToChar() const
{
	char		**env = new char*[_env.size() + 1];
	int			j = 0;
	std::string	element;

	for (std::map<std::string, std::string>::const_iterator i = _env.begin(); i != _env.end(); i++, j++)
	{
		element = i->first + "=" + i->second;
		env[j] = new char[element.size() + 1];
		env[j] = strcpy(env[j], element.c_str());
	}
	env[j] = NULL;

	return (env);
}
