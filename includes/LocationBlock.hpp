#ifndef __LOCATION_BLOCK_HPP__
# define __LOCATION_BLOCK_HPP__

# include "./Utils.hpp"

class	LocationBlock
{
	private:
		std::string					_block;
		int							_mod;
		std::string					_uri;
		int							_clntSize;
		std::vector<std::string>	_methods;
		int							_redirect;
		std::string					_root;
		int							_autoindex;
		std::vector<std::string>	_index;
		std::string					_cgi;
		std::vector<LocationBlock>	_locations;

		bool						_empty;
		std::string					_path;

	public:
		LocationBlock();
		LocationBlock(std::string block);
		LocationBlock(const LocationBlock &lb);
		~LocationBlock();
		LocationBlock	&operator=(const LocationBlock &lb);

		std::string					getBlock() const;
		int							getMod() const;
		std::string					getURI() const;
		int							getClntSize() const;
		std::vector<std::string>	getMethods() const;
		int							getRedirect() const;
		std::string					getRoot() const;
		int							getAutoindex() const;
		std::vector<std::string>	getIndex() const;
		std::string					getCGI() const;
		std::vector<LocationBlock>	getLocationBlocks() const;
		std::string					getPath() const;

		bool						empty() const;

		void						setMod(int mod);
		void						setURI(std::string uri);
		void						setClntSize(int clntSize);
		void						setMethods(std::vector<std::string> methods);
		void						setRedirect(int redirection);
		void						setRoot(std::string root);
		void						setAutoindex(int autoindex);
		void						setIndex(std::vector<std::string> index);
		void						setCGI(std::string cgi);
		void						addLocationBlock(LocationBlock lc);
		void						setEmpty(bool empty);
		void						setPath(const std::string& path);

		int							parseModMatch();
		int							parseClntSize();
		int							parseMethods();
		int							parseRoot();
		int							parseAutoindex();
		int							parseIndex();
		int							parseCGI();

		int							parse();

		void						printLocationBlock();
};

#endif
