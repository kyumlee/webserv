#ifndef __CGI_HPP__
# define __CGI_HPP__

# include "./ResponseHeader.hpp"

class Cgi
{
	public:
		Cgi();
		Cgi(Cgi const& cgi);
		virtual ~Cgi();

		Cgi&								operator=(Cgi const& cgi);

		std::map<std::string, std::string>	getEnv() const;
		int									getCgiExist() const;
		std::string							getName() const;
		std::string							getBody() const;

		void								setEnv(const std::string& key, const std::string& value);
		void								setCgiExist(const int& exist);
		void								setName(const std::string& name);
		void								setBody(const std::string& body);

		std::string							executeCgi(const std::string& scriptName);

		void								printEnv();
	
	private:
		std::map<std::string, std::string>	_env;
		int									_exists;
		std::string							_name;
		std::string							_body;
		char**								envToChar() const;
};

#endif
