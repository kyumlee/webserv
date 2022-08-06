#ifndef __CONFIG_HPP__
# define __CONFIG_HPP__

# include "./ServerBlock.hpp"
# include "./Server.hpp"

class	Config
{
	public:
		Config();
		Config(Config &conf);
		~Config();
		Config						&operator=(Config &conf);

		std::vector<ServerBlock>	getServerBlocks() const;

		void						addServerBlock(ServerBlock serverBlock);

		int							parse(std::string file);

		int							startServer();
		int							initServer(const std::string& conf);

	private:
		std::vector<ServerBlock>	_serverBlock;
		std::vector<Server>			_serverVec;
};

#endif
