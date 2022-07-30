#include "./../../includes/ServerBlock.hpp"

ServerBlock::ServerBlock ()
	: _block(""),
	_addresses(),
	_name(""),
	_errPages(),
	_clntSize(1024),
	_root(),
	_locations(),
	_methods(),
	_autoindex(DEFAULT_AUTOINDEX),
	_index()
{}

ServerBlock::ServerBlock (std::string block)
	: _block(block),
	_addresses(),
	_name(""),
	_errPages(),
	_clntSize(1024),
	_root(),
	_locations(),
	_methods(),
	_autoindex(DEFAULT_AUTOINDEX),
	_index()
{}

ServerBlock::ServerBlock (const ServerBlock &srv)
	: _block(srv._block),
	_addresses(srv._addresses),
	_name(srv._name),
	_errPages(srv._errPages),
	_clntSize(srv._clntSize),
	_root(srv._root),
	_locations(srv._locations),
	_methods(srv._methods),
	_autoindex(srv._autoindex),
	_index(srv._index),
	_host_port(srv._host_port)
{}

ServerBlock::~ServerBlock () {}

ServerBlock					&ServerBlock::operator= (const ServerBlock &srv) {
	_block = srv._block;
	_addresses = srv._addresses;
	_name = srv._name;
	_errPages = srv._errPages;
	_clntSize = srv._clntSize;
	_root = srv._root;
	_locations = srv._locations;
	_methods = srv._methods;
	_autoindex = srv._autoindex;
	_index = srv._index;

	return (*this);
}

std::string					ServerBlock::getBlock () const { return (_block); }
std::vector<std::string>	ServerBlock::getAddresses () const { return (_addresses); }
std::string					ServerBlock::getName () const { return (_name); }
std::map<int, std::string>	ServerBlock::getErrPages () const { return (_errPages); }
int							ServerBlock::getClntSize () const { return (_clntSize); }
std::string					ServerBlock::getRoot () const { return (_root); }
std::vector<LocationBlock>	ServerBlock::getLocationBlocks () const { return (_locations); }
std::vector<std::string>	ServerBlock::getMethods () const { return (_methods); }
int							ServerBlock::getAutoindex () const { return (_autoindex); }
std::vector<std::string>	ServerBlock::getIndex () const { return (_index); }

std::string					ServerBlock::getHostPort () const { return (this->_host_port); }

void						ServerBlock::setBlock (std::string block) { _block = block; }
void						ServerBlock::setAddresses (std::vector<std::string> addr) { _addresses = addr; }
void						ServerBlock::setName (std::string name) { _name = name; }
void						ServerBlock::setErrPages (std::map<int, std::string> pages) { _errPages = pages; }
void						ServerBlock::setClntSize (int size) { _clntSize = size; }
void						ServerBlock::setRoot (std::string root) { _root = root; }
void						ServerBlock::addLocationBlock (LocationBlock lc) { _locations.push_back(lc); }
void						ServerBlock::setMethods (std::vector<std::string> methods) { _methods = methods; }
void						ServerBlock::setAutoindex (int autoindex) { _autoindex = autoindex; }
void						ServerBlock::setIndex (std::vector<std::string> index) { _index = index; }

void						ServerBlock::setHostPort(std::string host_port) { this->_host_port = host_port; }

int							ServerBlock::parseAddresses () {
	std::vector<std::string>	addresses;
	std::pair<bool, size_t>		res = skipKey(_block, "listen", ";");
	size_t						lPos = 0, scPos = 0;

	if (res.first == false) {
		addresses.push_back("*:80");
		setAddresses(addresses);
		return (0);
	}

	while ((lPos = _block.find("listen", lPos)) != std::string::npos) {
		scPos = _block.find(";", lPos);
		addresses.push_back(_block.substr(lPos + 7, scPos - lPos - 7));
		lPos += 1;
		scPos += 1;
	}
	setAddresses(addresses);

	return (0);
}

int							ServerBlock::parseName () {
	std::string				names;
	std::pair<bool, size_t>	res = skipKey(_block, "server_name", ";");

	if (res.first == false)
		return (0);

	names = parseValue(_block, res.second, ";");

	setName(split(names, ' ')[0]);

	return (0);
}

int							ServerBlock::parseErrPages () {
	std::string				errPages;
	std::pair<bool, size_t>	res = skipKey(_block, "error_page", ";");
	std::vector<std::string>	err_vec;

	if (res.first == false)
		return (0);

	errPages = parseValue(_block, res.second, ";");

	err_vec = split(errPages, ' ');
	for (std::vector<std::string>::iterator it = err_vec.begin();
		it != err_vec.end(); it++)
	{

		if (isNumber(*it) == 0)
		{
			errPages = *it;
			break ;
		}
		else
		{
			this->_errPages[strToInt(*it)] = "";
		}
	}

	for (std::map<int, std::string>::iterator it = this->_errPages.begin();
		it != this->_errPages.end(); it++)
		it->second = errPages;

	return (0);
}

int							ServerBlock::parseClntSize () {
	std::pair<bool, size_t>	res = skipKey(_block, "client_max_body_size", ";");
	int						clntSize;

	if (res.first == false)
		return (0);

	clntSize = strToInt(parseValue(_block, res.second, ";"));

	if (clntSize < 0)
		return (printErr("wrong client max body size (should be positive)"));

	setClntSize(clntSize);

	return (0);
}

int							ServerBlock::parseRoot () {
	std::pair<bool, size_t>	res = skipKey(_block, "root", ";");

	if (res.first == false)
		return (0);

	setRoot(parseValue(_block, res.second, ";"));

	return (0);
}

int							ServerBlock::parseMethods () {
	std::string				methods;
	std::pair<bool, size_t>	res = skipKey(_block, "allow_methods", ";");

	if (res.first == false)
		return (0);

	methods = parseValue(_block, res.second, ";");
	setMethods(split(methods, ' '));

	if (_methods.empty())
		return (0);

	for (size_t i = 0; i < _methods.size(); i++) {
		if (_methods[i] != "GET" && _methods[i] != "POST" && _methods[i] != "DELETE" && _methods[i] != "PUT" && _methods[i] != "HEAD")
			return (printErr("wrong method (GET, POST, DELETE, PUT, HEAD)"));
	}

	return (0);
}

int							ServerBlock::parseAutoindex () {
	std::string				is;
	std::pair<bool, size_t>	res = skipKey(_block, "autoindex", ";");

	if (res.first == false)
		return (0);

	is = parseValue(_block, res.second, ";");

	if (is == "on")
		setAutoindex(ON);
	else if (is == "off")
		setAutoindex(OFF);

	return (0);
}

int							ServerBlock::parseIndex () {
	std::string				index;
	std::pair<bool, size_t>	res = skipKey(_block, "index", ";");

	if (res.first == false) {
		std::vector<std::string>	idx;
		idx.push_back("index.html");
		setIndex(idx);
		return (0);
	}

	index = parseValue(_block, res.second, ";");
	setIndex(split(index, ' '));

	return (0);
}

int							ServerBlock::parse () {
	std::vector<std::string>	locBlocks = splitBlocks(_block, "location ");

	parseAddresses();
	parseName();
	parseErrPages();
	parseClntSize();
	parseRoot();
	parseMethods();
	parseAutoindex();
	parseIndex();

	for (size_t i = 0; i < locBlocks.size(); i++) {
		addLocationBlock(LocationBlock(locBlocks[i]));
		_locations[i].parse();
		if (_locations[i].getRoot() == ".")
			_locations[i].setRoot(getRoot());
		// this->_locations[i].setURI(this->_root + this->_locations[i].getURI());
	}

	return (0);
}

void	ServerBlock::print_server_block()
{
	std::cout << "serveraddresses : ";
	for (size_t i = 0; i < _addresses.size(); i++)
		std::cout << _addresses[i] << ",";
	std::cout << std::endl;
	std::cout << "serverblock name : " << this->_name << std::endl;
	if (this->_errPages.size() != 0)
	{
		std::cout << "serverblock errorpages\n";
		print_errmap(this->_errPages);
	}
	else
		std::cout << "serverblock has no errorpages\n";
	std::cout << "serverblock clntsize : " << this->_clntSize << std::endl;
	std::cout << "root : " << this->_root << std::endl;
	if (this->_locations.size() != 0)
	{
		std::cout << GREEN << "serverblock has location block\n";
		for (std::vector<LocationBlock>::iterator it = this->_locations.begin();
			it != this->_locations.end(); it++)
		{
			std::cout << GREEN;
			(*it).print_location_block();
		}
		std::cout << RESET;
	}
	else
		std::cout << "serverblock has no location block\n";
	if (this->_methods.size() != 0)
	{
		std::cout << "serverblock has methods\n";
		print_vec(this->_methods);
	}
	else
		std::cout << "serverblock has no methods\n";
	std::cout << "serverblock autoindex : " << this->_autoindex << std::endl;
	if (this->_index.size() != 0)
	{
		std::cout << "serverblock has index\n";
		print_vec(this->_index);
	}
	else
		std::cout << "serverblock has no index\n";
}
