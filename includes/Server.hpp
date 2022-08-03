#ifndef __SERVER_HPP__
# define __SERVER_HPP__

//# include "./Cgi.hpp"
# include "./Response.hpp"

class Server
{
	public:
	// private:
		struct sockaddr_in			_serverAddr; //server의 주소를 저장
		int							_serverSocket; //serverSocket의 fd를 저장
		t_listen					_listen; //listen할 host와 port를 저장
		int							_kq; //kqueue를 통해 받은 fd를 저장
		std::map<int, std::string>	_request; //request의 socket과 socket의 내용을 저장
		std::vector<struct kevent>	_changeList; //kevent를 모두 저장
		struct kevent				_eventList[LISTEN_BUFFER_SIZE]; //이벤트가 발생한 kevent를 저장

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

		//configuation file 관련
		size_t						_clientMaxBodySize;
		int							_autoindex;
		std::vector<std::string>	_index;
		std::string					_configCgi;

		std::vector<LocationBlock>	_locations;
		Cgi							_cgi;

	public:
		Server ();
		Server (const Server& server);
		~Server ();

		Server&						operator= (const Server& server);

		//인자로 받은 값들을 EV_SET을 이용해 kevent구조체 변수인 temp event를 초기화시키고,
		//changeList에 temp event를 추가한다.
		void						changeEvents (std::vector<struct kevent>& changeList, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata);

		void						disconnectRequest (int fd);

		void						checkConnection (int fd);

		int							initListen (const std::string& host_port);

		int							initServerSocket ();

		int							eventError (int fd);

		void						requestAccept ();

		void						setServerName (const std::string& name);
		void						setServerAllowedMethods (const std::vector<std::string>& allwedMethods);
		void						setResponseRoot (const std::string& root);
		void						setServerErrorPages (const int& code, const std::string& html);
		void						setServerErrorPages (const std::map<int, std::string>& html);

		void						initServerErrorPages ();

		std::string					getServerName () const;
		std::vector<std::string>	getServerAllowedMethods () const;
		std::string					getResponseRoot () const;
		std::map<int, std::string>	getServerErrorPages () const;

		void						setCgiEnv (int fd);

		void						initCgiEnv (int fd);

		LocationBlock				selectLocationBlock (std::string requestURI, int fd);
		void						locationToServer (LocationBlock block, int fd);

		void						resetRequest (int fd);

		void						eventRead (int fd);
		void						eventWrite (int fd);

		void						initServerMember ();
};

#endif
