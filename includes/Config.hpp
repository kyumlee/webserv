#ifndef __CONFIG_HPP__
# define __CONFIG_HPP__

# include "./ServerBlock.hpp"
# include "./Server.hpp"

class	Config {

	private:
		std::vector<ServerBlock>	_server_block;
		std::vector<Server>			_server_vec;

	public:
		Config ();
		Config (Config &conf);
		~Config ();
		Config						&operator= (Config &conf);

		std::vector<ServerBlock>	getServerBlocks ();

		void						addServerBlock (ServerBlock server_block);

		int							parse (std::string file);

		int							serverStart();

		int							initServer(const std::string& conf_file);

};

#endif
