#ifndef __CONFIG_HPP__
# define __CONFIG_HPP__

# include "./ServerBlock.hpp"
# include "./Server.hpp"

class	Config {

	private:
		std::vector<ServerBlock>	_serverBlock;
		std::vector<Server>			_serverVec;

	public:
		Config ();
		Config (Config &conf);
		~Config ();
		Config						&operator= (Config &conf);

		std::vector<ServerBlock>	getServerBlocks ();

		void						addServerBlock (ServerBlock serverBlock);

		int							parse (std::string file);

		int							serverStart();

		int							initServer(const std::string& confFile);

};

#endif
