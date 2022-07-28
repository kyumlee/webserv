#include "./../../includes/config/Config.hpp"

Config::Config () {}

Config::Config (Config &conf) { _serverBlock = conf._serverBlock; }

Config::~Config () {}

Config						&Config::operator= (Config &conf) { _serverBlock = conf._serverBlock; return (*this); }

std::vector<ServerBlock>	Config::getServerBlocks () { return (_serverBlock); }

void						Config::addServerBlock (ServerBlock serverBlock) { _serverBlock.push_back(serverBlock); }

int							Config::parse (std::string file) {
	std::string					buf;
	std::ifstream				f(file);
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

	for (size_t i = 0; i < this->_serverBlock.size(); i++)
	{
		std::cout << "after parsing server block host port : " << this->_serverBlock[i].getAddresses()[0] << std::endl;
	}

	return (0);
}

int							Config::serverStart ()
{
	for (std::vector<Server>::iterator it = this->_serverVec.begin();
		it < this->_serverVec.end(); it++)
	{
		if ((*it).initServerSocket() == 1)
		{
			std::cerr << "server init server socket error\n";
			this->_serverVec.erase(it);
			continue ;
		}
	}

	std::vector<int>	new_events_vec;
	std::vector<struct kevent*>	curr_event_vec;

	for (std::vector<Server>::iterator it = this->_serverVec.begin();
		it < this->_serverVec.end(); it++)
	{
		int	new_events = 0;
		struct kevent	curr_event;

		new_events_vec.push_back(new_events);
		curr_event_vec.push_back(&curr_event);
		(*it).initServerMember();
		std::cout << YELLOW << "host : " << (*it)._listen.host << ", port : " << (*it)._listen.port;
		std::cout << " server start!!!!!!\n" << RESET;
	}

	if (this->_serverVec.empty())
	{
		std::cerr << "there is no executable server\n";
		return (1);
	}

	while (1)
	{
		std::vector<Server>::iterator	it = this->_serverVec.begin();

		struct timespec time_val;
		time_val.tv_sec = 1;
		time_val.tv_nsec = 0;
		for (size_t i = 0; i < this->_serverVec.size(); i++, it++)
		{
			
			new_events_vec[i] = kevent(this->_serverVec[i]._kq, &this->_serverVec[i]._changeList[0],
				this->_serverVec[i]._changeList.size(), this->_serverVec[i]._eventList,
				LISTEN_BUFFER_SIZE, &time_val);
			
			if (new_events_vec[i] == -1)
			{
				printErr("kevent error");
				this->_serverVec.erase(it);
				continue ;
			}
			this->_serverVec[i]._changeList.clear();

			for (int occur_event = 0; occur_event < new_events_vec[i];
				occur_event++)
			{
				curr_event_vec[i] = &this->_serverVec[i]._eventList[occur_event];
				if (curr_event_vec[i]->flags & EV_ERROR)
				{
					if (this->_serverVec[i].eventError(curr_event_vec[i]->ident) == 1)
					{
						printErr("server stops due to event error");
						this->_serverVec.erase(it);
						break ;
					}
				}
				else if (curr_event_vec[i]->filter == EVFILT_READ)
					this->_serverVec[i].eventRead(curr_event_vec[i]->ident);
				else if (curr_event_vec[i]->filter == EVFILT_WRITE)
					this->_serverVec[i].eventWrite(curr_event_vec[i]->ident);
			}
		}
	}
	return (0);
}

int							Config::initServer (const std::string& confFile)
{
	std::cout << 2 << std::endl;
	//먼저 parsing부터
	if (parse(confFile) == 1)
		return (1);

	for (size_t i = 0; i < _serverBlock.size(); i++)
	{
		for (size_t j = 0; j < _serverBlock[i].getAddresses().size(); j++)
			_serverVec.push_back(Server());
	}

	std::vector<Server>::iterator	it = _serverVec.begin();

	size_t	vec_i = 0;
	for (size_t i = 0; i < _serverBlock.size(); i++, it++)
	{
		for (size_t j = 0; j < _serverBlock[i].getAddresses().size(); j++, vec_i++) {
			std::cout << "CHECK: " << std::endl;
			std::cout << this->_serverVec[vec_i].initListen(this->_serverBlock[i].getAddresses()[j]) << std::endl;
			// FIXME: std::out_of_range error!!!
			//먼저 host와 port를 통해서 server의 listen을 초기화
			if (this->_serverVec[vec_i].initListen(this->_serverBlock[i].getAddresses()[j]) == 1)
			{
				printErr("host port : " + this->_serverBlock[i].getAddresses()[j] + " listen error");
				this->_serverVec.erase(it);
				continue ;
			}

			//server name 초기화
			this->_serverVec[vec_i]._response.setServer(this->_serverBlock[i].getName());
			std::cout << "server name : " << this->_serverVec[vec_i]._response._server << std::endl;

			//allow method를 초기화
			this->_serverVec[vec_i]._response.initAllowedMethods(this->_serverBlock[i].getMethods());
			std::cout << "allow method : ";
			for (std::set<std::string>::iterator it = this->_serverVec[vec_i]._response._allowedMethods.begin();
				it != this->_serverVec[vec_i]._response._allowedMethods.end(); it++)
				std::cout << *it << ", ";
			std::cout << std::endl;

			//root를 초기화, location을 고려해야 된다.
			this->_serverVec[vec_i]._response._root = this->_serverBlock[i].getRoot();
			std::cout << "server root : " << this->_serverVec[vec_i]._response._root << std::endl;

			//error code에 따른 error page초기화
			if (this->_serverBlock[i].getErrPages().empty() == 1)
			{//config 파일에 정해진 error page가 없으면 임의로 초기화시켜준다.
				this->_serverVec[vec_i]._response.initErrorHtml();
			}
			else
			{
				this->_serverVec[vec_i]._response.setErrorHtml(this->_serverBlock[i].getErrPages());
			}
			printErrmap(this->_serverVec[vec_i]._response._errorHtml);

			//clntsize 초기화
			this->_serverVec[vec_i]._clientMaxBodySize = this->_serverBlock[i].getClntSize();
			std::cout << "client max body size : " << this->_serverVec[vec_i]._clientMaxBodySize << std::endl;

			//autoindex를 초기화
			this->_serverVec[vec_i]._autoindex = this->_serverBlock[i].getAutoindex();
			std::cout << "auto index : " << (this->_serverVec[vec_i]._autoindex == true ? "true" : "false") << std::endl;

			//index를 초기화
			this->_serverVec[vec_i]._index = this->_serverBlock[i].getIndex();
			std::cout << "index : ";
			printVec(this->_serverVec[vec_i]._index);

			//server block의 location을 server로 넘겨준다
			for (size_t location_num = 0; location_num < this->_serverBlock[i].getLocationBlocks().size();
				location_num++)
				this->_serverVec[vec_i]._locations.push_back(this->_serverBlock[i].getLocationBlocks()[location_num]);
		}
	}
	std::cout << 3 << std::endl;

	this->serverStart();

	return (0);
}
