#include "./../../includes/LocationBlock.hpp"

LocationBlock::LocationBlock ()
	: _block(),
	_mod(NONE),
	_uri(),
	_clntSize(READ_BUFFER_SIZE),
	_methods(),
	_redirect(),
	_root("."),
	_autoindex(DEFAULT_AUTOINDEX),
	_index(),
	_cgi(""),
	_locations(),
	_is_empty(true)
{}

LocationBlock::LocationBlock (std::string block)
	: _block(block),
	_mod(NONE),
	_uri(),
	_clntSize(READ_BUFFER_SIZE),
	_methods(),
	_redirect(),
	_root("."),
	_autoindex(DEFAULT_AUTOINDEX),
	_index(),
	_cgi(""),
	_locations(),
	_is_empty(true)
{}

LocationBlock::LocationBlock (const LocationBlock &lb)
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
	_is_empty(lb._is_empty),
	_path(lb._path)
{}

LocationBlock::~LocationBlock() {}

LocationBlock	&LocationBlock::operator= (const LocationBlock &lb) {
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
	
	this->_is_empty = lb._is_empty;
	_path = lb._path;

	return (*this);
}

std::string					LocationBlock::getBlock () const { return (_block); }
int							LocationBlock::getMod () const { return (_mod); }
std::string					LocationBlock::getURI () const { return (_uri); }
int							LocationBlock::getClntSize () const { return (_clntSize); }
std::vector<std::string>	LocationBlock::getMethods () const { return (_methods); }
int							LocationBlock::getRedirect () const { return (_redirect); }
std::string					LocationBlock::getRoot () const { return (_root); }
int							LocationBlock::getAutoindex () const { return (_autoindex); }
std::vector<std::string>	LocationBlock::getIndex () const { return (_index); }
std::string					LocationBlock::getCGI () const { return (_cgi); }
std::vector<LocationBlock>	LocationBlock::getLocationBlocks () const { return (_locations); }
bool						LocationBlock::getIsEmpty() const { return (this->_is_empty); }
std::string					LocationBlock::getPath() const { return (this->_path); }

void						LocationBlock::setMod (int mod) { _mod = mod; }
void						LocationBlock::setURI (std::string uri) { _uri = uri; }
void						LocationBlock::setClntSize (int clntSize) { _clntSize = clntSize; }
void						LocationBlock::setMethods (std::vector<std::string> methods) { _methods = methods; }
void						LocationBlock::setRedirect (int redirection) { _redirect = redirection; }
void						LocationBlock::setRoot (std::string root) { _root = root; }
void						LocationBlock::setAutoindex (int autoindex) { _autoindex = autoindex; }
void						LocationBlock::setIndex (std::vector<std::string> index) { _index = index; }
void						LocationBlock::setCGI (std::string cgi) { _cgi = cgi; }
void						LocationBlock::addLocationBlock (LocationBlock lc) { _locations.push_back(lc); }
void						LocationBlock::setPath(const std::string& path) { _path = path; }

int							LocationBlock::parseModMatch () {
	size_t	pos = 0, bracketPos = _block.find("{", 0);
	size_t	end = _block.find("\n", 0);

	while (std::isspace(_block[pos]))
		pos++;

	if (_block[pos] == '/')
		setMod(NONE);
	else if (_block[pos] == '=') {
		setMod(EXACT);
		pos++;
	}
	else if (_block[pos] == '^' && _block[pos] == '~') {
		setMod(PREFERENTIAL);
		pos += 2;
	}
	while (std::isspace(_block[pos]))
		pos++;

	if (pos > end)
		return (printErr("invalid location block"));

	setURI(_block.substr(pos, bracketPos - 1 - pos));
	if (_uri != "/" && _uri.at(0) == '/')
		_uri = _uri.substr(1, _uri.length() - 1);
	std::cout << GREEN << "location block uri: " << getURI() << RESET << std::endl;

	return (0);
}

int							LocationBlock::parseClntSize () {
	std::pair<bool, size_t>	res = skipKey(_block, "client_max_body_size", ";");
	int						clntSize;

	if (res.first == false)
		return (0);

	clntSize = MiBToBits(parseValue(_block, res.second, ";"));

	if (clntSize < 0)
		return (printErr("wrong client max body size (should be positive)"));

	setClntSize(clntSize);

	return (0);
}

int							LocationBlock::parseMethods () {
	std::string				methods;
	std::pair<bool, size_t>	res = skipKey(_block, "allow_methods", ";");

	if (res.first == false)
		return (0);

	methods = parseValue(_block, res.second, ";");
	setMethods(split(methods, ' '));

	if (_methods.empty())
		return (0);

	for (size_t i = 0; i < _methods.size(); i++) {
		if (_methods[i] != "GET" && _methods[i] != "POST" && _methods[i] != "DELETE"  && _methods[i] != "PUT" && _methods[i] != "HEAD")
			return (printErr("invalid method"));
	}

	return (0);
}

int							LocationBlock::parseRoot () {
	std::pair<bool, size_t>	res = skipKey(_block, "root", ";");

	if (res.first == false)
		return (0);

	setRoot(parseValue(_block, res.second, ";"));

	return (0);
}

int							LocationBlock::parseAutoindex () {
	std::string				is;
	std::pair<bool, size_t>	res = skipKey(_block, "autoindex", ";");

	if (res.first == false)
		return (0);

	is = parseValue(_block, res.second, ";");

	if (is == "on") setAutoindex(ON);
	else if (is == "off") setAutoindex(OFF);

	return (0);
}

int							LocationBlock::parseIndex () {
	std::string				index;
	std::pair<bool, size_t>	res = skipKey(_block, "index", ";");

	if (res.first == false)
		return (0);

	index = parseValue(_block, res.second, ";");
	setIndex(split(index, ' '));

	return (0);
}

int							LocationBlock::parseCGI () {
	std::pair<bool, size_t>	res = skipKey(_block, "cgi_pass", ";");

	if (res.first == false)
		return (0);

	setCGI(parseValue(_block, res.second, ";"));

	return (0);
}

int							LocationBlock::parse () {
	std::vector<std::string>	locBlocks = splitBlocks(_block, "location ");

	parseModMatch();
	for (size_t i = 0; i < locBlocks.size(); i++) {
		addLocationBlock(LocationBlock(locBlocks[i]));
		_locations[i].parse();
		
		std::string	nest_location_uri = this->_locations[i].getURI();
		// if (nest_location_uri.at(0) == '/')
		// 	this->_locations[i]._uri = this->_uri + nest_location_uri;
		// else
		// 	this->_locations[i]._uri = this->_uri + "/" + nest_location_uri;
	}
	
	parseClntSize();
	parseMethods();
	parseRoot();
	parseAutoindex();
	parseIndex();
	parseCGI();

	this->_is_empty = 0;

	return (0);
}

void	LocationBlock::print_location_block()
{
	std::cout << "location_block : " << this->_block << std::endl;
	std::cout << "location block mode : " << this->_mod << std::endl;
	std::cout << "location block uri : " << this->_uri << std::endl;
	std::cout << "location block clntsize : " << this->_clntSize << std::endl;
	if (this->_methods.size() != 0)
	{
		std::cout << "location block methods : ";
		print_vec(this->_methods);
	}
	else
		std::cout << "location block has no methods\n";
	std::cout << "location block redirect : " << this->_redirect << std::endl;
	std::cout << "location block root : " << this->_root << std::endl;
	std::cout << "location block autoindex : " << this->_autoindex << std::endl;
	if (this->_index.size() != 0)
	{
		std::cout << "location block index : ";
		print_vec(this->_index);
	}
	else
		std::cout << "location block has no index\n";
	std::cout << "location block cgi : " << this->_cgi << std::endl;
	std::cout << "location block is empty : " << this->_is_empty << std::endl;
	if (this->_locations.size() != 0)
	{
		std::cout << "location block's has location\n";
		std::cout << RED << "###########location block############\n";
		for (std::vector<LocationBlock>::iterator it = this->_locations.begin();
			it != this->_locations.end(); it++)
		{
			(*it).print_location_block();
		}
		std::cout << RESET;
	}
	else
		std::cout << "location block has no location\n";
}
