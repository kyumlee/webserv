#include "./../includes/Config.hpp"

Config::Config () {}
Config::Config (Config &conf) { _serverBlock = conf._serverBlock; }
Config::~Config () {}

Config						&Config::operator= (Config &conf) { _serverBlock = conf._serverBlock; return (*this); }

std::vector<ServerBlock>	Config::getServerBlocks () const { return (_serverBlock); }

void						Config::addServerBlock (ServerBlock serverBlock) { _serverBlock.push_back(serverBlock); }

int							Config::parse (std::string conf) {
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

	for (size_t i = 0; i < _serverBlock.size(); i++)
		std::cout << "after parsing server block host port : " << _serverBlock[i].getAddresses()[0] << std::endl;

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

		(*it).initServerMember();

		std::cout << YELLOW << "host : " << (*it)._listen.host << ", port : " << (*it)._listen.port;
		std::cout << " server start!!!!!!\n" << RESET;
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
				LISTEN_BUFFER_SIZE, &timeVal);
			
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
			//먼저 host와 port를 통해서 server의 listen을 초기화
			if (_serverVec[v].initListen(_serverBlock[i].getAddresses()[j]) == 1)
			{
//				printErr("failed to listen: " + _serverBlock[i].getAddresses()[j]);
				_serverVec.erase(it);
				continue ;
			}

			_serverVec[v]._response.setServer(_serverBlock[i].getName());
//			std::cout << "server name : " << _serverVec[v]._response._server << std::endl;

			_serverVec[v]._response.initAllowedMethods(_serverBlock[i].getMethods());
//			std::cout << "allow method : ";
//			for (std::set<std::string>::iterator it = _serverVec[v]._response._allowedMethods.begin(); it != _serverVec[v]._response._allowedMethods.end(); it++)
//				std::cout << *it << ", ";
//			std::cout << std::endl;

			_serverVec[v]._response._root = _serverBlock[i].getRoot();
//			std::cout << "server root : " << _serverVec[v]._response._root << std::endl;

			if (_serverBlock[i].getErrPages().empty() == 1)
				_serverVec[v]._response.initErrorHtml();
			else
				_serverVec[v]._response.setErrorHtml(_serverBlock[i].getErrPages());
//			printErrmap(_serverVec[v]._response._errorHtml);

			_serverVec[v]._clientMaxBodySize = _serverBlock[i].getClntSize();
//			std::cout << "client max body size : " << _serverVec[v]._clientMaxBodySize << std::endl;

			_serverVec[v]._autoindex = _serverBlock[i].getAutoindex();
//			std::cout << "auto index : " << (_serverVec[v]._autoindex == true ? "true" : "false") << std::endl;

			_serverVec[v]._index = _serverBlock[i].getIndex();
//			std::cout << "index : ";
//			printVec(_serverVec[v]._index);

			for (size_t l = 0; l < _serverBlock[i].getLocationBlocks().size(); l++)
				_serverVec[v]._locations.push_back(_serverBlock[i].getLocationBlocks()[l]);
		}
	}

	startServer();

	return (0);
}
