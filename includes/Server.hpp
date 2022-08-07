#ifndef __SERVER_HPP__
# define __SERVER_HPP__

# include "./Response.hpp"

class Server
{
	public:
		Server();
		Server(const Server& server);
		~Server();

		Server&						operator=(const Server& server);
		void						changeEvents(std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);

		void						disconnectRequest(int fd);
		void						checkConnection(int fd);
		int							initListen(const std::string& host_port);
		int							initServerSocket();
		int							eventError(int fd);
		void						requestAccept();

		std::string					getServerName() const;
		std::vector<std::string>	getServerAllowedMethods() const;
		std::string					getResponseRoot() const;
		std::map<int, std::string>	getServerErrorPages() const;
		t_listen					getListen() const;
		struct sockaddr_in			getServerAddr() const;
		int							getServerSocket() const;
		int							getKq();
		std::map<int, std::string>	getRequest();
		std::vector<struct kevent>	getChangeList();
		void						resetChangeList();
		struct kevent&				getEventList(int index);
		struct kevent*				getEventList();
		std::vector<LocationBlock>	getLocations();

		void						setServerName(const std::string& name);
		void						setServerAllowedMethods(const std::vector<std::string>& allwedMethods);
		void						setResponseRoot(const std::string& root);
		void						setServerErrorPages(const int& code, const std::string& html);
		void						setServerErrorPages(const std::map<int, std::string>& html);
		void						initServerErrorPages();
		void						setServerRoot(const std::string& root);
		void						setClientMaxBodySize(const size_t& size);
		void						setAutoindex(const int& autoindex);
		void						setServerAutoIndex(const int& serverautoindex);
		void						setIndex(const std::vector<std::string>& index);
		void						setServerIndex(const std::vector<std::string>& serverindex);
		void						addLocation(LocationBlock& lb);

		void						setCgiEnv(int fd);
		void						initCgiEnv(int fd);

		LocationBlock				selectLocationBlock(std::string requestURI);
		void						locationToServer(LocationBlock block, int fd);

		void						resetRequest(int fd);

		void						eventRead(int fd);
		void						eventWrite(int fd);

	private:
		struct sockaddr_in			_serverAddr;
		int							_serverSocket;
		t_listen					_listen;
		int							_kq;
		std::map<int, std::string>	_request;
		std::vector<struct kevent>	_changeList;
		struct kevent				_eventList[LISTEN_BUFFER_SIZE];

		std::map<int, Response>		_response;
		std::map<int, int>			_bodyCondition;
		std::map<int, int>			_requestEnd;
		std::map<int, bool>			_checkedRequestLine;
		std::map<int, size_t>		_bodyStartPos;
		std::map<int, int>			_bodyEnd;
		std::map<int, size_t>		_bodyVecSize;
		std::map<int, size_t>		_bodyVecTotalSize;
		std::map<int, size_t>		_bodyVecStartPos;
		std::map<int, size_t>		_rnPos;

		std::string					_serverRoot;
		std::string					_serverName;
		std::vector<std::string>	_serverAllowedMethods;
		std::string					_responseRoot;
		std::map<int, std::string>	_serverErrorPages;

		size_t						_clientMaxBodySize;
		int							_autoindex;
		int							_serverautoindex;
		std::vector<std::string>	_index;
		std::vector<std::string>	_serverindex;
		std::string					_configCgi;

		std::vector<LocationBlock>	_locations;
		std::map<int, Cgi>			_cgi;
};

#endif
