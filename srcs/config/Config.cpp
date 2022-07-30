#include "./../../includes/Config.hpp"

Config::Config () {}

Config::Config (Config &conf) { _server_block = conf._server_block; }

Config::~Config () {}

Config						&Config::operator= (Config &conf) { _server_block = conf._server_block; return (*this); }

std::vector<ServerBlock>	Config::getServerBlocks () { return (_server_block); }

void						Config::addServerBlock (ServerBlock server_block) { _server_block.push_back(server_block); }

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
		_server_block[i].parse();
	}

	for (size_t i = 0; i < this->_server_block.size(); i++)
	{
		std::cout << "after parsing server block host port : " << this->_server_block[i].getAddresses()[0] << std::endl;
	}

	return (0);
}

int	Config::serverStart()
{
	for (std::vector<Server>::iterator it = this->_server_vec.begin();
		it < this->_server_vec.end(); it++)
	{
		if ((*it).init_server_socket() == 1)
		{
			std::cerr << "server init server socket error\n";
			this->_server_vec.erase(it);
			continue ;
		}
	}

	std::vector<int>	new_events_vec;
	std::vector<struct kevent*>	curr_event_vec;

	for (std::vector<Server>::iterator it = this->_server_vec.begin();
		it < this->_server_vec.end(); it++)
	{
		int	new_events = 0;
		struct kevent	curr_event;

		new_events_vec.push_back(new_events);
		curr_event_vec.push_back(&curr_event);
		(*it).init_server_member();
		std::cout << YELLOW << "host : " << (*it)._listen.host << ", port : " << (*it)._listen.port;
		std::cout << " server start!!!!!!\n" << RESET;
	}

	if (this->_server_vec.empty())
	{
		std::cerr << "there is no executable server\n";
		return (1);
	}

	while (1)
	{
		std::vector<Server>::iterator	it = this->_server_vec.begin();

		struct timespec time_val;
		time_val.tv_sec = 1;
		time_val.tv_nsec = 0;
		for (size_t i = 0; i < this->_server_vec.size(); i++, it++)
		{
			
			new_events_vec[i] = kevent(this->_server_vec[i]._kq, &this->_server_vec[i]._change_list[0],
				this->_server_vec[i]._change_list.size(), this->_server_vec[i]._event_list,
				LISTEN_BUFFER_SIZE, &time_val);
			
			if (new_events_vec[i] == -1)
			{
				std::cerr << "kevent error\n";
				this->_server_vec.erase(it);
				continue ;
			}
			this->_server_vec[i]._change_list.clear();

			for (int occur_event = 0; occur_event < new_events_vec[i];
				occur_event++)
			{
				curr_event_vec[i] = &this->_server_vec[i]._event_list[occur_event];
				if (curr_event_vec[i]->flags & EV_ERROR)
				{
					if (this->_server_vec[i].event_error(curr_event_vec[i]->ident) == 1)
					{
						std::cerr << "event error occured so server is stop\n";
						this->_server_vec.erase(it);
						break ;
					}
				}
				else if (curr_event_vec[i]->filter == EVFILT_READ)
					this->_server_vec[i].event_read(curr_event_vec[i]->ident);
				else if (curr_event_vec[i]->filter == EVFILT_WRITE)
					this->_server_vec[i].event_write(curr_event_vec[i]->ident);
			}
		}
	}
	return (0);
}

int							Config::initServer(const std::string& conf_file)
{
	//먼저 parsing부터
	if (parse(conf_file) == 1)
		return (1);

	for (size_t i = 0; i < _server_block.size(); i++)
	{
		for (size_t j = 0; j < _server_block[i].getAddresses().size(); j++)
			_server_vec.push_back(Server());
	}

	std::vector<Server>::iterator	it = _server_vec.begin();

	size_t	vec_i = 0;
	for (size_t i = 0; i < _server_block.size(); i++, it++)
	{
		for (size_t j = 0; j < _server_block[i].getAddresses().size(); j++, vec_i++) {
			//먼저 host와 port를 통해서 server의 listen을 초기화
			if (this->_server_vec[vec_i].init_listen(this->_server_block[i].getAddresses()[j]) == 1)
			{
//				std::cerr << "host port : " << this->_server_block[i].getAddresses()[j] << " listen error\n";
				this->_server_vec.erase(it);
				continue ;
			}

			//server name 초기화
			this->_server_vec[vec_i]._response.setServer(this->_server_block[i].getName());
//			std::cout << "server name : " << this->_server_vec[vec_i]._response._server << std::endl;

			//allow method를 초기화
			this->_server_vec[vec_i]._response.initAllowMethod(this->_server_block[i].getMethods());
//			std::cout << "allow method : ";
//			for (std::set<std::string>::iterator it = this->_server_vec[vec_i]._response._allow_method.begin();
//				it != this->_server_vec[vec_i]._response._allow_method.end(); it++)
//				std::cout << *it << ", ";
//			std::cout << std::endl;

			//root를 초기화, location을 고려해야 된다.
			this->_server_vec[vec_i]._response._root = this->_server_block[i].getRoot();
//			std::cout << "server root : " << this->_server_vec[vec_i]._response._root << std::endl;

			//error code에 따른 error page초기화
			if (this->_server_block[i].getErrPages().empty() == 1)
			{//config 파일에 정해진 error page가 없으면 임의로 초기화시켜준다.
				this->_server_vec[vec_i]._response.initErrorHtml();
			}
			else
			{
				this->_server_vec[vec_i]._response.setErrorHtml(this->_server_block[i].getErrPages());
			}
//			print_errmap(this->_server_vec[vec_i]._response._error_html);

			//clntsize 초기화
			this->_server_vec[vec_i]._client_max_body_size = this->_server_block[i].getClntSize();
//			std::cout << "client max body size : " << this->_server_vec[vec_i]._client_max_body_size << std::endl;

			//autoindex를 초기화
			this->_server_vec[vec_i]._auto_index = this->_server_block[i].getAutoindex();
//			std::cout << "auto index : " << (this->_server_vec[vec_i]._auto_index == true ? "true" : "false") << std::endl;

			//index를 초기화
			this->_server_vec[vec_i]._index = this->_server_block[i].getIndex();
//			std::cout << "index : ";
//			print_vec(this->_server_vec[vec_i]._index);

			//server block의 location을 server로 넘겨준다
			for (size_t location_num = 0; location_num < this->_server_block[i].getLocationBlocks().size();
				location_num++)
				this->_server_vec[vec_i]._locations.push_back(this->_server_block[i].getLocationBlocks()[location_num]);
		}
	}

	this->serverStart();

	return (0);
}
