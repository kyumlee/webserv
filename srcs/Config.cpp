#include "./../includes/Config.hpp"

Config::Config () {}
Config::Config (Config &conf) { _serverBlock = conf._serverBlock; }
Config::~Config () {}

Config						&Config::operator= (Config &conf) { _serverBlock = conf._serverBlock; return (*this); }

std::vector<ServerBlock>	Config::getServerBlocks () const { return (_serverBlock); }

void						Config::addServerBlock (ServerBlock serverBlock) { _serverBlock.push_back(serverBlock); }

int							Config::parse (std::string conf)
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

	for (size_t i = 0; i < blocks.size(); i++) {
		addServerBlock(ServerBlock(blocks[i]));
		_serverBlock[i].parse();
	}

	return (0);
}

int	Config::startServer ()
{
	for (std::vector<Server>::iterator it = _serverVec.begin(); it < _serverVec.end(); it++)
	{
		if ((*it).initServerSocket() == 1)
		{
			printErr("failed to init server");
			_serverVec.erase(it);
			continue ;
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

//		(*it).initServerMember();

		std::cout << YELLOW << "host : " << (*it)._listen.host << ", port : " << (*it)._listen.port;
		std::cout << " server start!!!!!!" << std::endl << RESET;
	}

	if (_serverVec.empty())
		return (printErr("no executable server"));

	while (1)
	{
		std::vector<Server>::iterator	it = _serverVec.begin();

		struct timespec timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_nsec = 0;

		for (size_t i = 0; i < _serverVec.size(); i++, it++)
		{
			newEventsVec[i] = kevent(_serverVec[i]._kq,
				&_serverVec[i]._changeList[0],
				_serverVec[i]._changeList.size(),
				_serverVec[i]._eventList,
				LISTEN_BUFFER_SIZE,
				&timeVal);

			if (newEventsVec[i] == -1)
			{
				printErr("kevent()");
				_serverVec.erase(it);
				continue ;
			}
			_serverVec[i]._changeList.clear();

			for (int occurEvent = 0; occurEvent < newEventsVec[i]; occurEvent++)
			{
				currEventVec[i] = &_serverVec[i]._eventList[occurEvent];
				if (currEventVec[i]->flags & EV_ERROR)
				{
					if (_serverVec[i].eventError(currEventVec[i]->ident) == 1)
					{
						printErr("stopping server due to event error");
						_serverVec.erase(it);
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

			if (_serverBlock[i].getErrPages().empty() == true)
				_serverVec[v].initServerErrorPages();
			else
				_serverVec[v].setServerErrorPages(_serverBlock[i].getErrPages());

			_serverVec[v]._serverRoot = _serverBlock[i].getRoot();
			_serverVec[v]._clientMaxBodySize = _serverBlock[i].getClntSize();
			_serverVec[v]._autoindex = _serverBlock[i].getAutoindex();
			_serverVec[v]._index = _serverBlock[i].getIndex();

			for (size_t l = 0; l < _serverBlock[i].getLocationBlocks().size(); l++)
				_serverVec[v]._locations.push_back(_serverBlock[i].getLocationBlocks()[l]);
		}
	}

	startServer();

	return (0);
}
