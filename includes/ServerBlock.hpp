#ifndef __SERVER_BLOCK_HPP__
# define __SERVER_BLOCK_HPP__

# include "./LocationBlock.hpp"

class	ServerBlock
{
	public:
		ServerBlock();
		ServerBlock(std::string block);
		ServerBlock(const ServerBlock &srv);
		~ServerBlock();
		ServerBlock					&operator=(const ServerBlock &srv);

		std::string					getBlock() const;
		std::vector<std::string>	getAddresses() const;
		std::string					getName() const;
		std::map<int, std::string>	getErrPages() const;
		int							getClntSize() const;
		std::string					getRoot() const;
		std::vector<LocationBlock>	getLocationBlocks() const;
		std::vector<std::string>	getMethods() const;
		int							getAutoindex() const;
		std::vector<std::string>	getIndex() const;
		std::string					getHostPort() const;
	
		void						setBlock(std::string block);
		void						setAddresses(std::vector<std::string> addr);
		void						setName(std::string name);
		void						setErrPages(std::map<int, std::string> pages);
		void						setClntSize(int size);
		void						setRoot(std::string root);
		void						addLocationBlock(LocationBlock lc);
		void						setMethods(std::vector<std::string> methods);
		void						setAutoindex(int autoindex);
		void						setIndex(std::vector<std::string> index);

		void						setHostPort(std::string hostPort);

		int							parseAddresses();
		int							parseName();
		int							parseErrPages();
		int							parseClntSize();
		int							parseRoot();
		int							parseMethods();
		int							parseAutoindex();
		int							parseIndex();

		int							parse();

		LocationBlock				selectLocationBlock(std::string requestURI) const;

		void						printServerBlock();

	private:
		std::string					_block;
		std::vector<std::string>	_addresses;
		std::string					_name;
		std::map<int, std::string>	_errPages;
		int							_clntSize;
		std::string					_root;
		std::vector<LocationBlock>	_locations;
		std::vector<std::string>	_methods;
		int							_autoindex;
		std::vector<std::string>	_index;
		std::string					_hostPort;
};

# endif
