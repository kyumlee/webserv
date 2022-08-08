#include "./../includes/Config.hpp"

Config::Config()
	: _serverBlock(),
	_serverVec()
{}

Config::Config(Config &conf)
	: _serverBlock(conf._serverBlock),
	_serverVec(conf._serverVec)
{}

Config::~Config () {}

Config						&Config::operator=(Config &conf)
{
	_serverBlock = conf._serverBlock;
	_serverVec = conf._serverVec;
	return (*this);
}

std::vector<ServerBlock>	Config::getServerBlocks() const { return (_serverBlock); }

void						Config::addServerBlock(ServerBlock serverBlock) { _serverBlock.push_back(serverBlock); }

int							Config::parse(std::string conf)
{
	std::string					buf;
	std::ifstream				f(conf);
	std::ostringstream			ss;
	std::vector<std::string>	blocks;

	if (!f)
		return (1);

	ss << f.rdbuf();
	buf = ss.str();

	blocks = splitBlocks(buf, "server ");

	for (size_t i = 0; i < blocks.size(); i++)
	{
		addServerBlock(ServerBlock(blocks[i]));
		_serverBlock[i].parse();
	}

	return (0);
}

int	Config::startServer ()
{
	for (size_t i = 0; i < _serverVec.size(); i++)
	{
		if (_serverVec[i].initServerSocket() == 1)
		{
			printErr("failed to init server");
			std::cerr << "erase server[" << i << "] port: " << ntohs((*(_serverVec.begin() + i)).getListen().port) << std::endl;
			_serverVec.erase(_serverVec.begin() + i);
			i--;
		}
	}

	std::vector<int>			newEventsVec;
	std::vector<struct kevent*>	currEventVec;

	for (std::vector<Server>::iterator it = _serverVec.begin(); it < _serverVec.end(); it++)
	{
		int				newEvents = 0;
		struct kevent	currEvent;

		newEventsVec.push_back(newEvents);
		currEventVec.push_back(&currEvent);
		std::cout << YELLOW << "host : " << (*it).getListen().host << ", port : " << ntohs((*it).getListen().port);
		std::cout << " server start!!!!!!" << std::endl << RESET;
	}

	if (_serverVec.empty())
		return (printErr("no executable server"));

	while (1)
	{
		if (_serverVec.empty() == true)
		{
			printErr("no executable server");
			break ;
		}

		struct timespec timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_nsec = 0;

		for (size_t i = 0; i < _serverVec.size(); i++)
		{
			newEventsVec[i] = kevent(_serverVec[i].getKq(),
				&_serverVec[i].getChangeList()[0],
				_serverVec[i].getChangeList().size(),
				_serverVec[i].getEventList(),
				LISTEN_BUFFER_SIZE,
				&timeVal);

			if (newEventsVec[i] == -1)
			{
				printErr(intToStr(ntohs(_serverVec[i].getListen().port)) + " port kevent error");
				std::cerr << "kq descriptor: " << _serverVec[i].getKq() << std::endl;
				std::cerr << errno << std::endl;
				_serverVec.erase(_serverVec.begin() + i);
				newEventsVec.erase(newEventsVec.begin() + i);
				currEventVec.erase(currEventVec.begin() + i);
				i--;
				continue ;
			}
			_serverVec[i].resetChangeList();

			for (int occurEvent = 0; occurEvent < newEventsVec[i]; occurEvent++)
			{
				currEventVec[i] = &_serverVec[i].getEventList(occurEvent);
				if (currEventVec[i]->flags & EV_ERROR)
				{
					if (_serverVec[i].eventError(currEventVec[i]->ident) == 1)
					{
						printErr(intToStr(_serverVec[i].getListen().port) + "stopping server due to event error");
						_serverVec.erase(_serverVec.begin() + i);
						newEventsVec.erase(newEventsVec.begin() + i);
						currEventVec.erase(currEventVec.begin() + i);
						i--;
						break ;
					}
				}
				else if (currEventVec[i]->filter == EVFILT_READ)
					_serverVec[i].eventRead(currEventVec[i]->ident);
				else if (currEventVec[i]->filter == EVFILT_WRITE)
					_serverVec[i].eventWrite(currEventVec[i]->ident);
			}
		}
	}
	return (0);
}

int							Config::initServer(const std::string& conf)
{
	if (parse(conf) == 1)
		return (1);

	for (size_t i = 0; i < _serverBlock.size(); i++)
	{
		for (size_t j = 0; j < _serverBlock[i].getAddresses().size(); j++)
			_serverVec.push_back(Server());
	}

	std::vector<Server>::iterator	it = _serverVec.begin();

	size_t	v = 0;
	for (size_t i = 0; i < _serverBlock.size(); i++, it++)
	{
		for (size_t j = 0; j < _serverBlock[i].getAddresses().size(); j++, v++) {
			if (_serverVec[v].initListen(_serverBlock[i].getAddresses()[j]) == 1)
			{
				_serverVec.erase(it);
				continue ;
			}

			_serverVec[v].setServerName(_serverBlock[i].getName());
			_serverVec[v].setServerAllowedMethods(_serverBlock[i].getMethods());
			_serverVec[v].setResponseRoot(_serverBlock[i].getRoot());
			_serverVec[v].initServerErrorPages();
			if (_serverBlock[i].getErrPages().empty() == false)
				_serverVec[v].setServerErrorPages(_serverBlock[i].getErrPages());

			_serverVec[v].setServerRoot(_serverBlock[i].getRoot());
			_serverVec[v].setClientMaxBodySize(_serverBlock[i].getClntSize());
			_serverVec[v].setAutoindex(_serverBlock[i].getAutoindex());
			_serverVec[v].setServerAutoIndex(_serverBlock[i].getAutoindex());
			_serverVec[v].setIndex(_serverBlock[i].getIndex());
			_serverVec[v].setServerIndex(_serverBlock[i].getIndex());

			for (size_t l = 0; l < _serverBlock[i].getLocationBlocks().size(); l++)
				_serverVec[v].addLocation(_serverBlock[i].getLocationBlocks()[l]);
		}
	}

	startServer();

	return (0);
}
