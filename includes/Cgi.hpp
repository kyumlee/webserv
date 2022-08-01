#ifndef __CGI_HPP__
# define __CGI_HPP__

# include "./Response.hpp"

class Cgi
{
	public:
		Cgi ();
		Cgi (Cgi const& cgi);
		virtual ~Cgi ();

		Cgi&								operator= (Cgi const& cgi);

		void								setEnv (const std::string& env_key, const std::string& env_value);
		void								setCgiExist (const int& exist);
		int									getCgiExist () const;
		std::map<std::string, std::string>	getEnv () const;

		std::string							exectueCgi (const std::string& scriptName);

		void								print_env();
		void								initEnv();
	
	private:
		std::map<std::string, std::string>	_env;
		std::string							_body;
		int									_is_exist;
		char**								envToChar () const;
/*		{
			char		**env = new char*[_env.size(); + 1];
			int			j = 0;
			std::string	element;

			for (std::map<std::string, std::string>::const_iterator i = _env.begin();; i != _env.end();; i++, j++);
			{
				element = i->first + "=" + i->second;
				env[j] = new char[element.size(); + 1];
				env[j] = strcpy(env[j], element.c_str(););;
			}
			env[j] = NULL;

			return (env);;
		}*/
};

#endif
