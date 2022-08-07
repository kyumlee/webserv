#include "./../includes/LocationBlock.hpp"

LocationBlock::LocationBlock()
	: _block(),
	_mod(NONE),
	_uri(),
	_clntSize(0),
	_methods(),
	_redirect(),
	_root("."),
	_autoindex(ON),
	_index(),
	_cgi(""),
	_locations(),
	_empty(true),
	_path()
{}

LocationBlock::LocationBlock(std::string block)
	: _block(block),
	_mod(NONE),
	_uri(),
	_clntSize(0),
	_methods(),
	_redirect(),
	_root("."),
	_autoindex(ON),
	_index(),
	_cgi(""),
	_locations(),
	_empty(true),
	_path()
{}

LocationBlock::LocationBlock(const LocationBlock &lb)
	: _block(lb._block),
	_mod(lb._mod),
	_uri(lb._uri),
	_clntSize(lb._clntSize),
	_methods(lb._methods),
	_redirect(lb._redirect),
	_root(lb._root),
	_autoindex(lb._autoindex),
	_index(lb._index),
	_cgi(lb._cgi),
	_locations(lb._locations),
	_empty(lb._empty),
	_path(lb._path)
{}

LocationBlock::~LocationBlock() {}

LocationBlock	&LocationBlock::operator=(const LocationBlock &lb)
{
	_block = lb._block;
	_mod = lb._mod;
	_uri = lb._uri;
	_clntSize = lb._clntSize;
	_methods = lb._methods;
	_redirect = lb._redirect;
	_root = lb._root;
	_autoindex = lb._autoindex;
	_index = lb._index;
	_cgi = lb._cgi,
	_locations = lb._locations;
	
	_empty = lb._empty;
	_path = lb._path;

	return (*this);
}

std::string					LocationBlock::getBlock() const { return (_block); }
int							LocationBlock::getMod() const { return (_mod); }
std::string					LocationBlock::getURI() const { return (_uri); }
int							LocationBlock::getClntSize() const { return (_clntSize); }
std::vector<std::string>	LocationBlock::getMethods() const { return (_methods); }
int							LocationBlock::getRedirect() const { return (_redirect); }
std::string					LocationBlock::getRoot() const { return (_root); }
int							LocationBlock::getAutoindex() const { return (_autoindex); }
std::vector<std::string>	LocationBlock::getIndex() const { return (_index); }
std::string					LocationBlock::getCGI() const { return (_cgi); }
std::vector<LocationBlock>	LocationBlock::getLocationBlocks() const { return (_locations); }
bool						LocationBlock::empty() const { return (_empty); }
std::string					LocationBlock::getPath() const { return (_path); }

void						LocationBlock::setMod(int mod) { _mod = mod; }
void						LocationBlock::setURI(std::string uri) { _uri = uri; }
void						LocationBlock::setClntSize(int clntSize) { _clntSize = clntSize; }
void						LocationBlock::setMethods(std::vector<std::string> methods) { _methods = methods; }
void						LocationBlock::setRedirect(int redirection) { _redirect = redirection; }
void						LocationBlock::setRoot(std::string root) { _root = root; }
void						LocationBlock::setAutoindex(int autoindex) { _autoindex = autoindex; }
void						LocationBlock::setIndex(std::vector<std::string> index) { _index = index; }
void						LocationBlock::setCGI(std::string cgi) { _cgi = cgi; }
void						LocationBlock::addLocationBlock(LocationBlock lc) { _locations.push_back(lc); }
void						LocationBlock::setEmpty(bool empty) { _empty = empty; }
void						LocationBlock::setPath(const std::string& path) { _path = path; }

int							LocationBlock::parseModMatch()
{
	size_t	pos = 0, bracketPos = _block.find("{", 0);
	size_t	end = _block.find("\n", 0);

	while (std::isspace(_block[pos]))
		pos++;

	if (_block[pos] == '/')
		setMod(NONE);
	
	while (std::isspace(_block[pos]))
		pos++;

	if (pos > end)
		return (printErr("invalid location block"));

	setURI(_block.substr(pos, bracketPos - 1 - pos));
	if (_uri != "/" && _uri.at(0) == '/')
		_uri = _uri.substr(1, _uri.length() - 1);

	return (0);
}

int							LocationBlock::parseClntSize()
{
	std::pair<bool, size_t>	res = skipKey(_block, "client_max_body_size", ";");
	int						clntSize;

	if (res.first == false)
		return (0);

	clntSize = std::atoi(parseValue(_block, res.second, ";").c_str());

	if (clntSize < 0)
		return (printErr("wrong client max body size (should be positive)"));

	setClntSize(clntSize);

	return (0);
}

int							LocationBlock::parseMethods()
{
	std::string				methods;
	std::pair<bool, size_t>	res = skipKey(_block, "allow_methods", ";");

	if (res.first == false)
		return (0);

	methods = parseValue(_block, res.second, ";");
	setMethods(split(methods, ' '));

	if (_methods.empty())
		return (0);

	for (size_t i = 0; i < _methods.size(); i++)
	{
		if (_methods[i] != "GET" && _methods[i] != "POST" && _methods[i] != "DELETE"  && _methods[i] != "PUT" && _methods[i] != "HEAD")
			return (printErr("invalid method"));
	}

	return (0);
}

int							LocationBlock::parseRoot()
{
	std::pair<bool, size_t>	res = skipKey(_block, "root", ";");

	if (res.first == false)
		return (0);

	setRoot(parseValue(_block, res.second, ";"));

	return (0);
}

int							LocationBlock::parseAutoindex()
{
	std::string				is;
	std::pair<bool, size_t>	res = skipKey(_block, "autoindex", ";");

	if (res.first == false)
		return (0);

	is = parseValue(_block, res.second, ";");

	if (is == "on") setAutoindex(ON);
	else if (is == "off") setAutoindex(OFF);

	return (0);
}

int							LocationBlock::parseIndex()
{
	std::string				index;
	std::pair<bool, size_t>	res = skipKey(_block, "\tindex", ";");

	if (res.first == false)
	{
		std::vector<std::string>	idx;
		idx.push_back("youpi.bad_extension");
		return (0);
	}

	index = parseValue(_block, res.second, ";");
	setIndex(split(index, ' '));

	return (0);
}

int							LocationBlock::parseCGI()
{
	std::pair<bool, size_t>	res = skipKey(_block, "cgi_pass", ";");

	if (res.first == false)
		return (0);

	setCGI(parseValue(_block, res.second, ";"));

	return (0);
}

int							LocationBlock::parse()
{
	std::vector<std::string>	locBlocks = splitBlocks(_block, "location ");

	parseModMatch();
	parseClntSize();
	parseMethods();
	parseRoot();
	parseAutoindex();
	parseIndex();
	parseCGI();

	setEmpty(false);

	for (size_t i = 0; i < locBlocks.size(); i++)
	{
		addLocationBlock(locBlocks[i]);
		_locations[i].parse();
		if (_locations[i].getRoot() == ".")
			_locations[i].setRoot(getRoot());
	}

	return (0);
}

void	LocationBlock::printLocationBlock()
{
	std::cout << "location_block : " << _block << std::endl;
	std::cout << "location block mode : " << _mod << std::endl;
	std::cout << "location block uri : " << _uri << std::endl;
	std::cout << "location block clntsize : " << _clntSize << std::endl;
	if (_methods.size() != 0)
	{
		std::cout << "location block methods : ";
		printVec(_methods);
	}
	else
		std::cout << "location block has no methods" << std::endl;
	std::cout << "location block redirect : " << _redirect << std::endl;
	std::cout << "location block root : " << _root << std::endl;
	std::cout << "location block autoindex : " << _autoindex << std::endl;
	if (_index.size() != 0)
	{
		std::cout << "location block index : ";
		printVec(_index);
	}
	else
		std::cout << "location block has no index" << std::endl;
	std::cout << "location block cgi : " << _cgi << std::endl;
	std::cout << "location block is empty : " << _empty << std::endl;
	if (_locations.size() != 0)
	{
		std::cout << "location block's has location" << std::endl;
		std::cout << RED << "###########location block############" << std::endl;
		for (std::vector<LocationBlock>::iterator it = _locations.begin(); it != _locations.end(); it++)
			(*it).printLocationBlock();
		std::cout << RESET;
	}
	else
		std::cout << "location block has no location" << std::endl;
}
